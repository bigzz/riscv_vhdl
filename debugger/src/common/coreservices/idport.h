/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#pragma once

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *const IFACE_DPORT = "IDPort";

class IDPort : public IFace {
 public:
    IDPort() : IFace(IFACE_DPORT) {}

    virtual void resumereq() = 0;
    virtual void haltreq() = 0;
    virtual bool isHalted() = 0;
#if 1
    virtual uint64_t readRegDbg(uint32_t regno) = 0;
    virtual void writeRegDbg(uint32_t regno, uint64_t val) = 0;
#else
    // Read/Write Control Status Registers
    virtual uint64_t readCSR(uint32_t regno) = 0;
    virtual void writeCSR(uint32_t regno, uint64_t val) = 0;
    // Read/Write General Purpose Registers + FPU registers
    virtual uint64_t readGPR(uint32_t regno) = 0;
    virtual void writeGPR(uint32_t regno, uint64_t val) = 0;
    // Read/Write Non-standard extension registers
    virtual uint64_t readNonStandardReg(uint32_t regno) = 0;
    virtual void writeNonStandardReg(uint32_t regno, uint64_t val) = 0;
#endif
    virtual bool executeProgbuf(uint32_t *progbuf) = 0;
    virtual bool isExecutingProgbuf() = 0;
    virtual void setResetPin(bool val) = 0;
};

}  // namespace debugger

