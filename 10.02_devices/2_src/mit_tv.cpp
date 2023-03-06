/* mit_tv.hpp: MIT's custom TV-based raster display

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

 mit_tv:
 A device representing MIT's custom TV-based raster display used for LOGO and others
 Implements a
 Currently these are just there as stubs...
 No active register callbacks, just polling in worker()
 */

#include <string.h>

#include "logger.hpp"
#include "timeout.hpp"
#include "qunibusadapter.hpp"
#include "qunibusdevice.hpp"	// definition of class device_c
#include "mit_tv.hpp"

mit_tv_c::mit_tv_c() :
		qunibusdevice_c()  // super class constructor
{
	// static config
	name.value = "mit_tv";
	type_name.value = "mit_tv_c";
	log_label = "tv";

	set_default_bus_params(0764100, 31, 0, 4); // base addr, priority slot, intr-vector, intr level

	// controller has 26 registers
	register_count = 26;

	for( unsigned int i = 0; i < register_count; i ++ ) {
		csr_reg[i] = &(this->registers[i]); // @  base addr
		strcpy(csr_reg[i]->name, "CSR"); // "CSR"
		csr_reg[i]->active_on_dati = false; // no controller state change
		csr_reg[i]->active_on_dato = false;
		csr_reg[i]->reset_value = 0;
		csr_reg[i]->writable_bits = 0xffff; // not sure ... TBD
	}
}

mit_tv_c::~mit_tv_c() {
	// cleanup (nothing to do)
}

bool mit_tv_c::on_param_changed(parameter_c *param) {
	// no own parameter or "enable" logic
	return qunibusdevice_c::on_param_changed(param); // more actions (for enable)
}


// background worker.  Does nothing
void mit_tv_c::worker(unsigned instance) {
	UNUSED(instance); // only one
}

// process DATI/DATO access to one of my "active" registers
// !! called asynchronuously by PRU, with SSYN asserted and blocking QBUS/UNIBUS.
// The time between PRU event and program flow into this callback
// is determined by ARM Linux context switch
//
// QBUS/UNIBUS DATO cycles let dati_flipflops "flicker" outside of this proc:
//      do not read back dati_flipflops.
void mit_tv_c::on_after_register_access(qunibusdevice_register_t *device_reg,
		uint8_t unibus_control, DATO_ACCESS access) {
	// nothing todo
	UNUSED(device_reg);
	UNUSED(unibus_control);
	UNUSED(access);
}

// after QBUS/UNIBUS install, device is reset by DCLO cycle
 void mit_tv_c::on_power_changed(signal_edge_enum aclo_edge, signal_edge_enum dclo_edge) {
	UNUSED(aclo_edge) ;
	UNUSED(dclo_edge) ;
}

// QBUS/UNIBUS INIT: clear all registers
void mit_tv_c::on_init_changed(void) {
	// write all registers to "reset-values"
	if (init_asserted) {
		reset_unibus_registers();
		INFO("mit_tv_c::on_init()");
	}
}

