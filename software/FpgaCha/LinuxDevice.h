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

#include <errno.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace FpgaCha
{
    class LinuxDevice
    {
    private:
        int openFile(const std::string& fileName, bool syncEnable)
        {
            // Open device file
            int fd = open(fileName.c_str(), O_RDWR | (syncEnable ? O_SYNC : 0));
            
            // Throw exception if file is not opened properly
            if (fd < 0)
            {
                std::string m = std::string("Error when opening '");
                throw std::runtime_error(m + fileName + "': " + strerror(errno));
            }
            
            return fd;
        }
        
        void* mapFile()
        {
            // Do the mapping
            void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, descriptor, 0);
            
            // Process error condition
            if (ptr == MAP_FAILED)
            {
                std::string m = std::string("Error when calling mmap for '");
                throw std::runtime_error(m + name + "': " + strerror(errno));
            }
            
            return ptr;
        }
        
    public:
        const std::string name;
        const std::string clazz;
        const size_t size;
        const int descriptor;
        void* const mapping;

        LinuxDevice
        (
            const std::string& name, 
            const std::string& clazz,
            const std::string& sizePath,
            bool syncEnable = true
        ) : 
            name(name),
            clazz(clazz),
            size(readProperty(sizePath)),
            descriptor(openFile("/dev/" + name, syncEnable)),
            mapping(mapFile()) { }

        uint32_t readProperty(const std::string& property) const
        {
            const std::string fileName = "/sys/class/" + clazz + "/" + name + "/" + property;
            std::ifstream file(fileName);
            
            std::string result;
            if (!getline(file, result))
                throw std::runtime_error("Error when reading from '" + fileName + "'");
            
            const int radix = result.substr(0, 2) == "0x" ? 16 : 10;
            try { return std::stoi(result, nullptr, radix); }
            catch(std::runtime_error e)
            {
                std::string m = std::string("Error when converting content of '");
                throw std::runtime_error(m + name + "'; content: " + result);
            }
        }
        
        void writeProperty(const std::string& property, const std::string& content) const
        {
            const std::string fileName = "/sys/class/" + clazz + "/" + name + "/" + property;
            std::ofstream file(fileName);
            file << content;
            
            if (!file)
            {
                std::string m = std::string("Error when writing to '");
                throw std::runtime_error(m + fileName + "'");
            }
        }

        ~LinuxDevice()
        {
            // Close device file
            close(descriptor);
            
            munmap(mapping, size);
        }
    };
}