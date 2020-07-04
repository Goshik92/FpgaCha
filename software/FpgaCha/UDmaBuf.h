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
#include <string>
#include <fstream>
#include <stdexcept>
#include "LinuxDevice.h"

namespace FpgaCha
{
    class UDmaBuf
    {
    private:
        const LinuxDevice device;
        
        uint32_t getOffset(uint32_t* buffer) const
        {
            const auto b = reinterpret_cast<uint8_t*>(buffer);
            const auto c = reinterpret_cast<uint8_t*>(content);
            return (uint32_t)(b - c);
        }

    public:
        const uint32_t physical;
        const uint32_t size;
        uint32_t* const content;

        UDmaBuf(const std::string& name) : 
                device(LinuxDevice(name, "udmabuf", "size", false)),
                physical(device.readProperty("phys_addr")),
                size(device.size / sizeof(uint32_t)),
                content(reinterpret_cast<uint32_t*>(device.mapping)) { }
                
        void syncForCpu(uint32_t* buffer, size_t length) const
        {
            const int syncDirection = 1;
            const uint32_t syncOffset = getOffset(buffer);
            const uint32_t syncSize = length * sizeof(uint32_t);
           
            std::stringstream sstream;
            sstream << "0x" << std::hex;
            sstream << std::setw(8) << std::setfill('0') << syncOffset;
            sstream << std::setw(8) << std::setfill('0');
            sstream << ((syncSize & 0xFFFFFFF0) | (syncDirection << 2) | 1);
            
            device.writeProperty("sync_for_cpu", sstream.str());
        }
        
        void syncForDma(uint32_t* buffer, size_t length) const
        {
            const int syncDirection = 2;
            const uint32_t syncOffset = getOffset(buffer);
            const uint32_t syncSize = length * sizeof(uint32_t);
           
            std::stringstream sstream;
            sstream << "0x" << std::hex;
            sstream << std::setw(8) << std::setfill('0') << syncOffset;
            sstream << std::setw(8) << std::setfill('0');
            sstream << ((syncSize & 0xFFFFFFF0) | (syncDirection << 2) | 1);
            
            device.writeProperty("sync_for_device", sstream.str());
        }
        
        uint32_t toPhysical(uint32_t* virt) const
        {
            return getOffset(virt) + physical;
        }
    };
}