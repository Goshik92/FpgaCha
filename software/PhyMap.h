/***********************************************
* Author: Igor Semenov (is0031@uah.edu)
* Platform: Terasic DE2-115
* Date: Dec 2019
* Description: Maps physical address to data structure
***********************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdexcept>
#include "DevMem.h"

#pragma once

template<typename T>
class PhyMap
{
private:
    volatile T* pointer;
    void* rawPointer;
    
public:
    PhyMap(DevMem file, off_t baseAddress)
    {
        const size_t pageSize = sysconf(_SC_PAGESIZE);
        const off_t pageAddress = (baseAddress & (~(pageSize - 1)));
        const off_t pageOffset = baseAddress - pageAddress;
        const size_t alignedLength = sizeof(T) + pageOffset;

        void* rawPointer = mmap(NULL, alignedLength, PROT_READ | PROT_WRITE, 
                         MAP_SHARED, file(), pageAddress);
        
        if (rawPointer == MAP_FAILED)
        {
            std::string m = std::string("Error when calling mmap: ");
            throw std::runtime_error(m + strerror(errno));
        }
        
        pointer = (T*)((char*)rawPointer + pageOffset);
    }

    volatile T* operator()() const
    {
        return pointer;
    }
    
    ~PhyMap()
    {
        munmap(rawPointer, sizeof(T));
    }
};