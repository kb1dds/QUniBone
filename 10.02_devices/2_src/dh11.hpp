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


 19-oct-2022  MR      Copied over from dh11 device
 12-nov-2018  JH      entered beta phase
 */
#ifndef _DH11_HPP_
#define _DH11_HPP_

#include <fstream>

using namespace std;

#include "utils.hpp"
#include "qunibusdevice.hpp"

class dh11_c: public qunibusdevice_c {
private:

	// One of each of the following for each line
        qunibusdevice_register_t *scr_reg; // System control register
        qunibusdevice_register_t *nrcr_reg; // Next recieved character register
        qunibusdevice_register_t *lpr_reg; // Line parameter register
        qunibusdevice_register_t *car_reg; // Current address register
        qunibusdevice_register_t *bcr_reg; // Byte count register
        qunibusdevice_register_t *bar_reg; // Buffer active register
        qunibusdevice_register_t *brcr_reg; // Break control register
        qunibusdevice_register_t *ssr_reg; // Silo status register

public:

	dh11_c();
	~dh11_c();

	bool on_param_changed(parameter_c *param) override;  // must implement

	// background worker function
	void worker(unsigned instance) override;

	// called by qunibusadapter on emulated register access
	void on_after_register_access(qunibusdevice_register_t *device_reg, uint8_t unibus_control)
			override;

	void on_power_changed(signal_edge_enum aclo_edge, signal_edge_enum dclo_edge) override;
	void on_init_changed(void) override;
};

#endif
