/* dh11.hpp: DH11 asynchronous serial line interface

 Copyright (c) 2023, Michael Robinson
 Copyright (c) 2018, Joerg Hoppe

 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 JOERG HOPPE NOR MICHAEL ROBINSON BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 IN THE SOFTWARE.

 20-aug-2023  MR      Starting to emulate correct device behavior
 19-oct-2022  MR      Copied over from mit_ng device
 12-nov-2018  JH      entered beta phase

 DH11:
 The DH11 asynchronous serial line interface is a UNIBUS peripheral which provides up to 16 asynchronous serial lines.
 No active register callbacks, just polling in worker()
 */

#include <string.h>

#include "logger.hpp"
#include "timeout.hpp"
#include "qunibusadapter.hpp"
#include "qunibusdevice.hpp"	// definition of class device_c
#include "dh11.hpp"

dh11_c::dh11_c() :
		qunibusdevice_c()  // super class constructor
{
	// static config
	name.value = "dh11";
	type_name.value = "dh11_c";
	log_label = "dh11";

	set_default_bus_params(DH11_ADDR, DH11_SLOT, DH11_VECTOR, DH11_LEVEL);

	// controller has 8 programmer-visible registers, though maintains other per-line registers
	register_count = 8;

	scr_reg = &(this->registers[0]); // @  base addr
	strcpy(scr_reg->name, "DHSCR"); // System control register (selects line and other parameters that apply to all lines)
	scr_reg->active_on_dati = false; // no controller state change
	scr_reg->active_on_dato = false;
	scr_reg->reset_value = 0;
	scr_reg->writable_bits = 0xffff; // all bits writable

	nrcr_reg = &(this->registers[1]); // @  base addr+2
	strcpy(nrcr_reg->name, "DHNRCR"); // Next character received (bottom of silo)
	nrcr_reg->active_on_dati = false; // no controller state change
	nrcr_reg->active_on_dato = false;
	nrcr_reg->reset_value = 0;
	nrcr_reg->writable_bits = 0x0000; // read only

	lpr_reg = &(this->registers[2]); // @  base addr+4
	strcpy(lpr_reg->name, "DHLPR");  // Line parameter register (mirrors a per-line register)
	lpr_reg->active_on_dati = false; // no controller state change
	lpr_reg->active_on_dato = false;
	lpr_reg->reset_value = 0;
	lpr_reg->writable_bits = 0xffff; // Write-only, in fact

	car_reg = &(this->registers[3]); // @  base addr+6
	strcpy(car_reg->name, "DHCAR");  // Current address register (mirrors a per-line register)
	car_reg->active_on_dati = false; // no controller state change
	car_reg->active_on_dato = false;
	car_reg->reset_value = 0;
	car_reg->writable_bits = 0xffff; // Write-only, in fact

	bcr_reg = &(this->registers[4]); // @  base addr+8
	strcpy(bcr_reg->name, "DHBCR");  // Byte count register (mirrors a per-line register)
	bcr_reg->active_on_dati = false; // no controller state change
	bcr_reg->active_on_dato = false;
	bcr_reg->reset_value = 0;
	bcr_reg->writable_bits = 0xffff; // read/write

	bar_reg = &(this->registers[5]); // @  base addr+10
	strcpy(bar_reg->name, "DHBAR");  // Buffer active register (applies to all lines, one bit per line)
	bar_reg->active_on_dati = false; // no controller state change
	bar_reg->active_on_dato = false;
	bar_reg->reset_value = 0;
	bar_reg->writable_bits = 0xffff; // Read/write

	brcr_reg = &(this->registers[6]); // @  base addr+12
	strcpy(brcr_reg->name, "DHBRCR"); // Break control register (applies to all lines, one bit per line)
	brcr_reg->active_on_dati = false; // no controller state change
	brcr_reg->active_on_dato = false;
	brcr_reg->reset_value = 0;
	brcr_reg->writable_bits = 0xffff; // Write-only, in fact

	ssr_reg = &(this->registers[7]); // @  base addr+14
	strcpy(ssr_reg->name, "DHSSR");  // Silo status register (applies to all lines)
	ssr_reg->active_on_dati = false; // no controller state change
	ssr_reg->active_on_dato = false;
	ssr_reg->reset_value = 0;
	ssr_reg->writable_bits = 0xffff; // read/write

	// Initialize the silo
	this.silo_count = 0;
	this.silo_alarm = 0;

	// Clear per-line registers
	for (int i = 0; i < 16; i ++ ) {
	  this.lpr_line_reg[i] = 0;
	  this.car_line_reg[i] = 0;
	  this.bcr_line_reg[i] = 0;
	}     

}

dh11_c::~dh11_c() {
	// cleanup (nothing to do)
}

bool dh11_c::on_param_changed(parameter_c *param) {
	// no own parameter or "enable" logic
	return qunibusdevice_c::on_param_changed(param); // more actions (for enable)
}


// background worker.  Does nothing
void dh11_c::worker(unsigned instance) {
	UNUSED(instance); // only one
}

// process DATI/DATO access to one of my "active" registers
// !! called asynchronuously by PRU, with SSYN asserted and blocking QBUS/UNIBUS.
// The time between PRU event and program flow into this callback
// is determined by ARM Linux context switch
//
// QBUS/UNIBUS DATO cycles let dati_flipflops "flicker" outside of this proc:
//      do not read back dati_flipflops.
void dh11_c::on_after_register_access(qunibusdevice_register_t *device_reg,
		uint8_t unibus_control, DATO_ACCESS access) {
	// nothing todo
	UNUSED(device_reg);
	UNUSED(unibus_control);
	UNUSED(access);
}

// after QBUS/UNIBUS install, device is reset by DCLO cycle
 void dh11_c::on_power_changed(signal_edge_enum aclo_edge, signal_edge_enum dclo_edge) {
	UNUSED(aclo_edge) ;
	UNUSED(dclo_edge) ;
}

// QBUS/UNIBUS INIT: clear all registers
void dh11_c::on_init_changed(void) {
	// write all registers to "reset-values"
	if (init_asserted) {
		reset_unibus_registers();
		INFO("dh11_c::on_init()");
	}

	// Initialize the silo
	this.silo_count = 0;
	this.silo_alarm = 0;

	// Clear per-line lpr registers.  (Per EK-0DH11-MM-003_Apr75, paragraph 3.3.7 per-line CAR and BCR are not touched during initialize)
	for (int i = 0; i < 16; i ++ ) {
	  this.lpr_line_reg[i] = 0;
	}     
}


// Add a character to the top of the silo
// Inputs: incoming : incoming character (lowest 8 bits only)
//         line     : line that received the character (lowest 4 bits only)
//         parity_error, framing_error, data_overrun : various kinds of error conditions from UART
// Outputs: character stored on silo
// Returns: true if success, false if silo is full
// Fires interrupts according to
//  * If silo alarm level is tripped
//  * If storage interrupt is tripped (silo is already full)
// Note: in case of silo full, character is *not* placed into the silo; it's left in limbo
bool dh11_c::silo_enqueue( uint8_t incoming, uint8_t line, bool parity_error, bool framing_error, bool data_overrun ) {

  if( this.silo_count > 63 ){
    uint16_t val = get_register_dato_value(scr_reg);
    set_register_dati_value(ssr_reg, val | SCR_STORAGE_INT ,  __func__);

    // Don't do anything to the silo, 'cause it's full!
    
    if (get_register_dato_value(scr) & SCR_S_INT_ENABLE) {
      
      // Trigger storage interrupt
      
    }
    return false;
  }

  // Add the data to the silo, including the data valid flag
  this.silo[this.silo_count] = (incoming & 0x00ff) | ((line & 0x000f) << 8) |
    (parity_error & NCR_PARITY_ERR) | (framing_error & NCR_FRAMING_ERR) | (data_overrun & NCR_DATA_OVERRUN) | NCR_VALID_DATA;
  this.silo_count ++;

  uint16_t val = get_register_dato_value(ssr_reg);

  // Update SSR with new silo fill level
  set_register_dati_value(ssr_reg, (val & (~SSR_SILO_FILL)) | ((this.silo_count & 0x003f) << 8) , __func__);

  int silo_alarm_level = val & SSR_SILO_ALARM;

  if( (this.silo_count > silo_alarm_level) & (get_register_dato_value(scr) & SCR_RX_INT_ENABLE) ){
    // Trigger silo alarm interrupt
    return true;
  }
  return true
}

// Remove a character (it's actually a word) from the bottom of the silo.  NB: If valid data present, bit 15 is set.  If the silo is empty, bit 15 is cleared
void dh11_c::silo_dequeue( uint16_t *data ) {
  if( this.silo_count == 0 ){
    *data = this.silo[0] & (~NCR_VALID_DATA);
  }
  else {
    *data = this.silo[0] | NCR_VALID_DATA;

    for( int i = 1; i < this.silo_count; i ++ ) {
      this.silo[i-1] = this.silo[i];
    }
    this.silo_count --;
  }

  uint16_t val = get_register_dato_value(ssr_reg);

  // Update SSR with new silo fill level
  set_register_dati_value(ssr_reg, (val & (~SSR_SILO_FILL)) | ((this.silo_count & 0x003f) << 8) , __func__);
}
