/* dh11.hpp: DH11 asynchronous serial line interface

 Copyright (c) 2018, Joerg Hoppe
 j_hoppe@t-online.de, www.retrocmp.com

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
 JOERG HOPPE BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 19-oct-2022  MR      Copied over from mit_ng device
 12-nov-2018  JH      entered beta phase

 DH11:
 The DH11 asynchronous serial line interface is a UNIBUS peripheral which provides up to 16 asynchronous serial lines. 
 This Unibone device does just one line
 Currently these are just there as stubs...
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

	set_default_bus_params(0760020, 31, 0340, 5); // base addr, priority slot, intr-vector, intr level

	// controller has 8*16=128 registers for 16 lines.  That might be too many for Unibone at the moment...
        // so this device just does one line
	register_count = 8;

	scr_reg = &(this->registers[0]); // @  base addr
	strcpy(scr_reg->name, "DHSCR"); // "CSR"
	scr_reg->active_on_dati = false; // no controller state change
	scr_reg->active_on_dato = false;
	scr_reg->reset_value = 0;
	scr_reg->writable_bits = 0xffff; // not sure ... TBD

	nrcr_reg = &(this->registers[1]); // @  base addr+2
	strcpy(nrcr_reg->name, "DHNRCR"); 
	nrcr_reg->active_on_dati = false; // no controller state change
	nrcr_reg->active_on_dato = false;
	nrcr_reg->reset_value = 0;
	nrcr_reg->writable_bits = 0xffff; // not sure ... TBD

	lpr_reg = &(this->registers[2]); // @  base addr+4
	strcpy(lpr_reg->name, "DHLPR"); 
	lpr_reg->active_on_dati = false; // no controller state change
	lpr_reg->active_on_dato = false;
	lpr_reg->reset_value = 0;
	lpr_reg->writable_bits = 0xffff; // not sure ... TBD

	car_reg = &(this->registers[3]); // @  base addr+6
	strcpy(car_reg->name, "DHCAR"); 
	car_reg->active_on_dati = false; // no controller state change
	car_reg->active_on_dato = false;
	car_reg->reset_value = 0;
	car_reg->writable_bits = 0xffff; // not sure ... TBD

	bcr_reg = &(this->registers[4]); // @  base addr+8
	strcpy(bcr_reg->name, "DHBCR"); 
	bcr_reg->active_on_dati = false; // no controller state change
	bcr_reg->active_on_dato = false;
	bcr_reg->reset_value = 0;
	bcr_reg->writable_bits = 0xffff; // not sure ... TBD

	bar_reg = &(this->registers[5]); // @  base addr+10
	strcpy(bar_reg->name, "DHBAR"); 
	bar_reg->active_on_dati = false; // no controller state change
	bar_reg->active_on_dato = false;
	bar_reg->reset_value = 0;
	bar_reg->writable_bits = 0xffff; // not sure ... TBD

	brcr_reg = &(this->registers[6]); // @  base addr+12
	strcpy(brcr_reg->name, "DHBRCR"); 
	brcr_reg->active_on_dati = false; // no controller state change
	brcr_reg->active_on_dato = false;
	brcr_reg->reset_value = 0;
	brcr_reg->writable_bits = 0xffff; // not sure ... TBD

	ssr_reg = &(this->registers[7]); // @  base addr+14
	strcpy(ssr_reg->name, "DHSSR"); 
	ssr_reg->active_on_dati = false; // no controller state change
	ssr_reg->active_on_dato = false;
	ssr_reg->reset_value = 0;
	ssr_reg->writable_bits = 0xffff; // not sure ... TBD
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
}

