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

    public:
        const uint32_t physical;
        const uint32_t size;
        uint32_t* const content;

        UDmaBuf(const std::string& name) : 
                device(LinuxDevice(name, "udmabuf", "size")),
                physical(device.readProperty("phys_addr")),
                size(device.size / sizeof(uint32_t)),
                content(reinterpret_cast<uint32_t*>(device.mapping)) { }
    };
}