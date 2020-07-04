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
#include "TaskManager.h"
#include "OtpTask.h"
#include "ChaCha20/ChaCha20.h"
#include "ChaCha20/State.h"

// Worker that produces ChaCha20 OTP blocks using software
// N - number of tasks in the system (should match to that of TaskManager and Cryptor)
template <size_t N>
class ChaCha20Worker
{
private:   
    // Thread to produce OTP blocks
    std::thread chacha20Thread;
    
public:
    // m - task manager
    // s - encryption parameters
    ChaCha20Worker(TaskManager<N>& m, const ChaCha20::State& s)
    {
        chacha20Thread = std::thread([&]()
        {
            ChaCha20::State state = s;
            ChaCha20::ChaCha20 chacha20;
            
            // Main loop
            while(true)
            {
                OtpTask task;
                
                // Get task from the queue
                if (!m.performTask(task)) break;
                
                // Calculate block count
                task.getBCount(s.bCount, state.bCount);
                
                // Compute ChaCha20 OTP blocks
                for(int i = 0; i < task.length; i += ChaCha20::State::WORD_SIZE)
                {
                    // 20 rounds
                    chacha20.compute(state);
                    
                    // Summation
                    for(int j = 0; j < ChaCha20::State::WORD_SIZE; j++)
                    {
                        task.buffer[i + j] = chacha20.state[j];
                    }
                    
                    state.bCount[0]++;
                }
                
                // Report task completion
                if (!m.finishTask(task)) break;
            }
        });
    }
    
    // Destroy
    ~ChaCha20Worker()
    {
        chacha20Thread.join();
    }
};