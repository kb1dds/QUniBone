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


 19-oct-2022  MR      Copied over from mit_tv device
 12-nov-2018  JH      entered beta phase
 */
#ifndef _MIT_TV_HPP_
#define _MIT_TV_HPP_

#include <fstream>

using namespace std;

#include "utils.hpp"
#include "qunibusdevice.hpp"

class mit_tv_c: public qunibusdevice_c {
private:

        qunibusdevice_register_t *csr_reg[32]; // Register file for the display

public:

	mit_tv_c();
	~mit_tv_c();

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
