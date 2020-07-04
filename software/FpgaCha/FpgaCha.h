/*******************************************************************************
 * Copyright 2020 Igor Semenov (goshik92@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *******************************************************************************/

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdexcept>
#include "LinuxDevice.h"
#include "ChaCha20Map.h"
#include "StToMemMap.h"

namespace FpgaCha
{
    // Controls FpgaCha IP-core using UIO driver
    class FpgaCha
    {
    private:
        // Address offset for ChaCha20 accelerator
        const uint32_t CHACHA20_OFF = 0x0000;
        
        // Address offset for S2M adapter
        const uint32_t ST2MEM_OFF = 0x0100;
        
        // UIO device
        const LinuxDevice device;
        
        // Register maps of hw devices
        ChaCha20Map* const chacha20;
        StToMemMap* const stToMem;
        
        // Enables interrupts for S2M adapter
        void enableIrq() const
        {
            // We need to write 1 to UIO file to enable interrupts
            uint32_t one = 1;
            
            // Try to write 1
            const ssize_t length = write(device.descriptor, &one, sizeof(one));
            
            // If write did not work
            if (length != (ssize_t)sizeof(one))
            {
                std::string m = "Error when enabling interrupts for '";
                throw std::runtime_error(m + device.name + "': " + strerror(errno));
            }
        }
        
        // Blocks current thread until ISR from S2M adapter happens
        void waitForIrq() const
        {
            uint32_t result;
            
            // Wait for interrupt by reading from UIO device file
            const ssize_t length = read(device.descriptor, &result, sizeof(result));
            
            // If an error happened
            if (length != (ssize_t)sizeof(result))
            {
                std::string m = "Error when waiting for interrupt from '";                
                throw std::runtime_error(m + device.name + "': " + strerror(errno));
            } 
        }

    public:
        // name - UIO file name of FpgaCha device
        FpgaCha(const std::string& name) : 
                device(LinuxDevice(name, "uio", "maps/map0/size")),
                chacha20((ChaCha20Map*)((uint8_t*)device.mapping + CHACHA20_OFF)),
                stToMem((StToMemMap*)((uint8_t*)device.mapping + ST2MEM_OFF)) { }
        
        // Sets encryption parameters
        void setState(ChaCha20::State& s) const
        {
            chacha20->setState(s);
        }
        
        // Asks FpgaCha to generate <otpCount> OTP blocks and place
        // them starting from physical address <physical>
        void start(uint32_t physical, uint32_t otpCount) const
        {
            stToMem->clearIrq();
            chacha20->start(otpCount);
            enableIrq();
            stToMem->start(physical, otpCount * 2);
        }
        
        // Blocks current thread until the computation is finished
        void wait() const
        {
            waitForIrq();
        }
    };
}