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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fstream>
#include <ios>

// Helper to mmap files
class FileMapper
{
private:
    int descriptor;
    size_t size;
    void* content;
    
public:
    // fileName - file to map
    // size - if 0 an existing file is used, if non-zero a new file of that size is created
    FileMapper(const std::string& fileName, size_t size = 0)
    {
        // Open file
        descriptor = open(fileName.c_str(), O_RDWR | O_CREAT);
        
        // Throw exception if file is not opened properly
        if (descriptor < 0)
        {
            std::string m = std::string("Error when opening '");
            throw std::runtime_error(m + fileName + "': " + strerror(errno));
        }
        
        // Use real file size
        if (size == 0)
        {
            struct stat fileStat;
            fstat(descriptor, &fileStat);
            this->size = fileStat.st_size;
        }
        
        // Truncate file to size
        else
        {
            truncate(fileName.c_str(), size);
            this->size = size;
        }
        
        // Do the mapping
        content = mmap(NULL, this->size, PROT_READ | PROT_WRITE, MAP_SHARED, descriptor, 0);
        
        // Process error condition
        if (content == MAP_FAILED)
        {
            std::string m = std::string("Error when calling mmap for '");
            throw std::runtime_error(m + fileName + "': " + strerror(errno));
        }
    }
    
    // Gets file size bytes
    size_t getSize()
    {
        return size;
    }
    
    // Gets file size in 
    size_t getWordSize()
    {
        return size / sizeof(uint32_t);
    }
    
    // Gets pointer to file's content
    uint32_t* getContent()
    {
        return (uint32_t*)content;
    }
    
    ~FileMapper()
    {
        munmap(content, size);
        close(descriptor);
    }
};