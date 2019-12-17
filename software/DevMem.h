/***********************************************
* Author: Igor Semenov (is0031@uah.edu)
* Platform: Terasic DE2-115
* Date: Dec 2019
* Description: Hides opening of dev mem 
***********************************************/

#include <exception>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdexcept>

#pragma once

class DevMem
{
private:
    int descriptor;

public:
    DevMem()
    {
        descriptor = open("/dev/mem", O_RDWR | O_SYNC);
        
        if (descriptor < 0)
        {
            std::string m = std::string("Error when opening /dev/mem: ");
            throw std::runtime_error(m + strerror(errno));
        }
    }
    
    int operator()() const
    {
        return descriptor;
    }
    
    ~DevMem()
    {
        close(descriptor);
    }
};