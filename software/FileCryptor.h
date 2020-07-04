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

#include <array>
#include <thread>
#include <algorithm>
#include "BlockingQueue.h"
#include "QueueArray.h"
#include "OtpTask.h"
#include "TaskManager.h"

// Crypor that utilizes OTP blocks for file encryption
// N - number of tasks in the system (should match to that of TaskManager and Workers)
template <size_t N>
class FileCryptor
{
private:
    // Thread used for encryption
    std::thread cryptorThread;
    
public:
    // m - task manager
    // in - input file
    // out - out file
    FileCryptor(TaskManager<N>& m, FileMapper& in, FileMapper& out)
    {
        cryptorThread = std::thread([&]()
        { 
            OtpTask task;
            size_t fileOffset = 0;
            uint32_t* inContent = in.getContent();
            uint32_t* outContent = out.getContent();
            size_t fileSize = out.getWordSize();
            
            while(1)
            {
                // Wait for the next otp task to be done
                m.processTask(task);
                
                const size_t jobSize = std::min(fileSize - fileOffset, task.length);
                
                // Do the cryption job
                for(size_t i = 0; i < jobSize; i++)
                {
                    outContent[fileOffset] = inContent[fileOffset] ^ task.buffer[i];
                    fileOffset++;
                }
                
                if (fileOffset >= fileSize - 1) break;
                
                // Schedule a new task
                m.scheduleTask(task);
            }
            
            // Send the shutdown signal when the file is encrypted
            m.shutdown();
        });
    }
    
    ~FileCryptor()
    {
        cryptorThread.join();
    }
};