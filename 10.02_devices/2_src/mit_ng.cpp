/* mit_ng.hpp: MIT's custom Knight vector display

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

 18-oct-2022  MR      Copied over from JH's demo_io device
 12-nov-2018  JH      entered beta phase

 mit_ng:
 A device representing MIT's custom Knight vector display used for LOGO and others
 Implements a CSR at 0764040 and a relocation address at 0764042
 Currently these are just there as stubs...
 No active register callbacks, just polling in worker()
 */

#include <string.h>

#include "logger.hpp"
#include "timeout.hpp"
#include "qunibusadapter.hpp"
#include "qunibusdevice.hpp"	// definition of class device_c
#include "mit_nh.hpp"

mit_ng_c::mit_ng_c() :
		qunibusdevice_c()  // super class constructor
{
	// static config
	name.value = "MIT_NG";
	type_name.value = "mit_ng_c";
	log_label = "ng";

	set_default_bus_params(0764040, 0270, 5, 0); // base addr, intr-vector, intr level

	// controller has 2 registers
	register_count = 2;

	csr_reg = &(this->registers[0]); // @  base addr
	strcpy(csr_reg->name, "CSR"); // "CSR"
	csr_reg->active_on_dati = false; // no controller state change
	csr_reg->active_on_dato = false;
	csr_reg->reset_value = 0;
	csr_reg->writable_bits = 0xfffff; // not sure ... TBD

	rel_reg = &(this->registers[1]); // @  base addr + 2
	strcpy(rel_reg->name, "REL"); // relocation
	rel_reg->active_on_dati = false; // no controller state change
	rel_reg->active_on_dato = false;
	rel_reg->reset_value = 0;
	rel_reg->writable_bits = 0xffff; // not sure ... TBD
}

mit_ng_c::~mit_ng_c() {
	// close all gpio value files
}

bool mit_ng_c::on_param_changed(parameter_c *param) {
	// no own parameter or "enable" logic
	return qunibusdevice_c::on_param_changed(param); // more actions (for enable)
}


// background worker.  Does nothing
void mit_ng_c::worker(unsigned instance) {
	UNUSED(instance); // only one
}

// process DATI/DATO access to one of my "active" registers
// !! called asynchronuously by PRU, with SSYN asserted and blocking QBUS/UNIBUS.
// The time between PRU event and program flow into this callback
// is determined by ARM Linux context switch
//
// QBUS/UNIBUS DATO cycles let dati_flipflops "flicker" outside of this proc:
//      do not read back dati_flipflops.
void mit_ng_c::on_after_register_access(qunibusdevice_register_t *device_reg,
		uint8_t unibus_control) {
	// nothing todo
	UNUSED(device_reg);
	UNUSED(unibus_control);
}

// after QBUS/UNIBUS install, device is reset by DCLO cycle
 void mit_ng_c::on_power_changed(signal_edge_enum aclo_edge, signal_edge_enum dclo_edge) {
	UNUSED(aclo_edge) ;
	UNUSED(dclo_edge) ;
}

// QBUS/UNIBUS INIT: clear all registers
void mit_ng_c::on_init_changed(void) {
	// write all registers to "reset-values"
	if (init_asserted) {
		reset_unibus_registers();
		INFO("mit_ng_c::on_init()");
	}
}

