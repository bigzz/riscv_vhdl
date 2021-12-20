/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __AXI_MAPS_H__
#define __AXI_MAPS_H__

#include <inttypes.h>
#include "axi_const.h"
#include "maps/map_pnp.h"
#include "maps/map_gpio.h"
#include "maps/map_gptimers.h"
#include "maps/map_uart.h"
#include "maps/map_rfctrl.h"
#include "maps/map_gnssengine.h"
#include "maps/map_ethmac.h"
#include "maps/map_spiflash.h"
#include "maps/map_fsev2.h"
#include "maps/map_plic.h"

#define ADDR_BUS0_XSLV_FWIMAGE      0x00100000
#define ADDR_BUS0_XSLV_EXTFLASH     0x00200000
#define ADDR_BUS0_XSLV_CLINT        0x02000000 // Core-local interruptor
#define ADDR_BUS0_XSLV_SRAM         0x08000000 // 0x0800_0000..0x081F_FFFF = L2 Cache Controller
#define ADDR_BUS0_XSLV_PLIC         0x0C000000 // FU740 compatible
#define ADDR_BUS0_XSLV_UART0        0x10010000 // FU740 compatible
#define ADDR_BUS0_XSLV_UART1        0x10020000 // FU740 compatible
#define ADDR_BUS0_XSLV_GPIO         0x10060000 // FU740 compatible
#define ADDR_BUS0_XSLV_OTP          0x10070000
#define ADDR_BUS0_XSLV_ETHMAC       0x10090000 // 0x1009_0000 .. 0x1009_1FFF Ethernet on FU740
// GNSS Sub System
#define ADDR_BUS0_XSLV_RF_CTRL      0x100f0000 // Reserved region of FU740
#define ADDR_BUS0_XSLV_GNSS_SS      0x100f1000 // Reserved region of FU740
#define ADDR_BUS0_XSLV_FSE          0x100f2000 // Reserved region of FU740
#define ADDR_BUS0_XSLV_PNP          0x100ff000 // Reserved region of FU740

// Interrupt pins assignemts:
#define CFG_IRQ_UNUSED      0
#define CFG_IRQ_UART0       39   // The same in 740 (unmatched)
// Interrupt for the self-test that triggered on write access 
// into read-only register pnp->hwid
#define PLIC_IRQ_PNP        70
#define CFG_IRQ_GNSS_SS     71
#define PLIC_ISR_MAX        73 // Any number up to 1024


#endif  // __AXI_MAPS_H__
