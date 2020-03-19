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
    class FpgaCha
    {
    private:
        const uint32_t CHACHA20_OFF = 0x0000;
        const uint32_t ST2MEM_OFF = 0x0100;
        const LinuxDevice device;

    public:
        ChaCha20Map* const chacha20;
        StToMemMap* const stToMem;
        
        FpgaCha(const std::string& name) : 
                device(LinuxDevice(name, "uio", "maps/map0/size")),
                chacha20((ChaCha20Map*)((uint8_t*)device.mapping + CHACHA20_OFF)),
                stToMem((StToMemMap*)((uint8_t*)device.mapping + ST2MEM_OFF)) { }
        
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
    };
}