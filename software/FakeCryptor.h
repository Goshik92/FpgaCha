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

#include <iostream>
#include <istream>
#include <ostream>
#include <array>
#include <thread>
#include "BlockingQueue.h"
#include "QueueArray.h"
#include "OtpTask.h"
#include "TaskManager.h"

// Consumes OPT blocks and discards them
// Can be used to test performance of workers
// N - number of tasks in the system (should match to that of TaskManager and Cryptor)
template <size_t N>
class FakeCryptor
{
private:
    // Thread for processing fake OTP blocks
    std::thread fakeThread;

public:
    // m - task manager
    // length - number of bytes to process
    FakeCryptor(TaskManager<N>& m, size_t length)
    {
        fakeThread = std::thread([&, length]()
        { 
            size_t processed = 0;
            OtpTask task;
            
            // Main loop
            while(true)
            {
                // Wait for the next otp task to be done
                m.processTask(task);
                
                // Count how many bytes has been processed
                processed += task.length * sizeof(uint32_t);
                
                // shutdown if enough was processed
                if (processed >= length) break;
                
                // Schedule a new task
                m.scheduleTask(task);
            }
            
            m.shutdown();
        });
    }
    
    ~FakeCryptor()
    {
        fakeThread.join();
    }
};