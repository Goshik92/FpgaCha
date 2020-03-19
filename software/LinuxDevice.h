#pragma once

#include <errno.h>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace FpgaCha
{
    class LinuxDevice
    {
    private:
        int openFile(const std::string& fileName)
        {
            // Open device file
            int fd = open(fileName.c_str(), O_RDWR | O_SYNC);
            
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

        LinuxDevice(const std::string& name, const std::string& clazz, const std::string& sizePath) : 
                name(name),
                clazz(clazz),
                size(readProperty(sizePath)),
                descriptor(openFile("/dev/" + name)),
                mapping(mapFile()) {}

        uint32_t readProperty(const std::string& property) const
        {
            const std::string fileName = "/sys/class/" + clazz + "/" + name + "/" + property;
            auto file = std::ifstream(fileName);

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

        ~LinuxDevice()
        {
            // Close device file
            close(descriptor);
            
            munmap(mapping, size);
        }
    };
}