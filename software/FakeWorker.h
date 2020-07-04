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

#include <thread>
#include "OtpTask.h"
#include "TaskManager.h"

// Generates fake OPT blocks
// Can be used to test performance of cryptors
// N - number of tasks in the system (should match to that of TaskManager and Cryptor)
template <size_t N>
class FakeWorker
{
private:
    // Thread for generating fake OTP blocks
    std::thread fakeThread;
    
public:
    // m - task manager
    FakeWorker(TaskManager<N>& m)
    {
        fakeThread = std::thread([&]()
        { 
            OtpTask task;
            
            // Main loop
            while(true)
            {
                // Read task
                if (!m.performTask(task)) break;
                
                // Immediately report that it is finisheds
                if (!m.finishTask(task)) break;
            }
        });
    }
    
    // Destroy
    ~FakeWorker()
    {
        fakeThread.join();
    }
};