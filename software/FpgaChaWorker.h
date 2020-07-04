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
#include <iostream>
#include "BlockingQueue.h"
#include "ChaCha20/BCount.h"
#include "ChaCha20/State.h"
#include "FpgaCha/FpgaCha.h"
#include "OtpTask.h"
#include "TaskManager.h"

// Worker that produces ChaCha20 OTP blocks using FpgaCha IP-core
// N - number of tasks in the system (should match to that of TaskManager and Cryptor)
// T - number of threads to do the summation stage in software (1 is enough)
template <size_t N, size_t T>
class FpgaChaWorker
{
private: 
    // Interface to FpgaCha core
    FpgaCha::FpgaCha fpgaCha;
    
    // Queue to connect the FpgaCha control thread to the summation thread
    BlockingQueue<OtpTask, 1> queue;
    
    // Encryption parameters
    ChaCha20::State state;
    
    // Thread
    std::thread roundsThread;
    std::thread summationThread[T];
    
public:
    // m - task manager
    // s - encryption parameters
    // devFile - FpgaCha UIO device file full name
    // uDmaBuff - uDmaBuff that is used as storage in tasks
    FpgaChaWorker(
        TaskManager<N>& m, 
        const ChaCha20::State& s, 
        const std::string& devFile,
        const FpgaCha::UDmaBuf& uDmaBuff) : 
        fpgaCha(FpgaCha::FpgaCha(devFile))
    { 
        // FpgaCha controlling thread
        roundsThread = std::thread([&]()
        {
            OtpTask task;
            ChaCha20::State state = s;
            
            // Main loop
            while(true)
            {
                // Wait for a task to arrive
                // Exit on shutdown condition
                if (!m.performTask(task))
                {
                    queue.shutdown();
                    break;
                }
                
                // Calculate block count
                task.getBCount(s.bCount, state.bCount);
                
                // Transfer ownership of the buffer to hardware
                uDmaBuff.syncForDma(task.buffer, task.length);
                
                // Start FpgaCha computation and sleep until it is finished
                const uint32_t physical = uDmaBuff.toPhysical(task.buffer);
                fpgaCha.setState(state);
                fpgaCha.start(physical, task.getOtpCount());
                fpgaCha.wait();
                
                // Transfer ownership of the buffer to CPU
                uDmaBuff.syncForCpu(task.buffer, task.length);
                
                // Send the result to the summation thread
                if (!queue.push(task)) break;
            }
        });
        
        // Summation thread
        auto summationRoutine = [&]()
        {
            OtpTask task;
            ChaCha20::State state = s;
            
            // Main loop
            while(true)
            {
                // Wait for a task to arrive
                // Exit on shutdown condition
                if (!queue.pop(task)) break;
                
                // Calculate block count
                task.getBCount(s.bCount, state.bCount);
                
                // Do the summation stage
                for(int i = 0; i < task.length; i += ChaCha20::State::WORD_SIZE)
                {
                    for(int j = 0; j < ChaCha20::State::WORD_SIZE; j++)
                    {
                        task.buffer[i + j] += state[j];
                    }
                    
                    state.bCount[0]++;
                }
                
                // Send the result to the queue of finished tasks
                // Exit on shutdown condition
                if (!m.finishTask(task))
                {
                    queue.shutdown();
                    break;
                }
            }
        };
        
        // Start T summation threads
        for(int i = 0; i < T; i++)
        {
            summationThread[i] = std::thread(summationRoutine);
        }
    }
    
    // Destroy
    ~FpgaChaWorker()
    {
        roundsThread.join();
        for(int i = 0; i < T; i++)
        {
            summationThread[i].join();
        }
    }
};