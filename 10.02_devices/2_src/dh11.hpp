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
 19-oct-2022  MR      Copied over from dh11 device
 12-nov-2018  JH      entered beta phase
 */
#ifndef _DH11_HPP_
#define _DH11_HPP_

#include <fstream>

using namespace std;

#include "utils.hpp"
#include "qunibusdevice.hpp"

// Overall device definitions
#define DH11_ADDR       0760020
#define DH11_SLOT       31
#define DH11_LEVEL	05
#define DH11_VECTOR	0340	

// register bit definitions
// SCR
#define SCR_RX_INT_ENABLE  0x0040
#define SCR_RX_INT         0x0080
#define SCR_CLR_NXM        0x0100
#define SCR_MAINT          0x0200
#define SCR_NXM            0x0400
#define SCR_MASTER_CLR     0x0800
#define SCR_S_INT_ENABLE   0x1000
#define SCR_NXM_INT_ENABLE 0x2000
#define SCR_STORAGE_INT    0x4000
#define SCR_TX_INT         0x8000

// NCRCR
#define NCR_PARITY_ERR   0x1000
#define NCR_FRAMING_ERR  0x2000
#define NCR_DATA_OVERRUN 0x4000
#define NCR_VALID_DATA   0x8000

// LPR
#define LPR_CHARLEN       0x0003
#define LPR_STOP_BITS     0x0004
#define LPR_PARITY_ENABLE 0x0010
#define LPR_ODD_PARITY    0x0020
#define LPR_RX_SPEED      0x03c0
#define LPR_TX_SPEED      0x3c00
#define LPR_DUPLEX        0x4000
#define LPR_AUTO_ECHO     0x8000

// SSR
#define SSR_SILO_ALARM 0x003f
#define SSR_READ_XM    0x00c0
#define SSR_SILO_FILL  0x3f00
#define SSR_MAINT      0x8000

class dh11_c: public qunibusdevice_c {
private:

	// Programmer-visible registers 
        qunibusdevice_register_t *scr_reg;  // System control register
        qunibusdevice_register_t *nrcr_reg; // Next recieved character register
        qunibusdevice_register_t *lpr_reg;  // Line parameter register
        qunibusdevice_register_t *car_reg;  // Current address register
        qunibusdevice_register_t *bcr_reg;  // Byte count register
        qunibusdevice_register_t *bar_reg;  // Buffer active register
        qunibusdevice_register_t *brcr_reg; // Break control register
        qunibusdevice_register_t *ssr_reg;  // Silo status register

        // Silo contents and top (shared across all lines for incoming data)
        uint16_t silo_count;  // Zero means that the silo is empty, so this is effectively the *next* silo location.
                                  // Also, the seventh bit gets placed into bit 14 of SCR
        uint16_t silo[64];

        // Per-line registers (not programmer-visible)
        uint16_t lpr_line_reg[16];
        uint16_t car_line_reg[16];
        uint16_t bcr_line_reg[16];

        // Add a character to the top of the silo
        bool silo_enqueue( uint8_t incoming, uint8_t line, bool parity_error, bool framing_error, bool data_overrun ); 

        // Remove a character (it's actually a word) from the bottom of the silo.  NB: If valid data present, bit 15 is set.  If the silo is empty, bit 15 is cleared
        void silo_dequeue( uint16_t *data );
  
public:

	dh11_c();
	~dh11_c();

	bool on_param_changed(parameter_c *param) override;  // must implement

	// background worker function
	void worker(unsigned instance) override;

	// called by qunibusadapter on emulated register access
	void on_after_register_access(qunibusdevice_register_t *device_reg, uint8_t unibus_control, DATO_ACCESS access)
			override;

	void on_power_changed(signal_edge_enum aclo_edge, signal_edge_enum dclo_edge) override;
	void on_init_changed(void) override;
};

#endif
