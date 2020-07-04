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
#include "BlockingQueue.h"
#include "QueueArray.h"
#include "OtpTask.h"

// Class for coordinating workers and cryptor
// N - number of tasks circulating in the system
template <size_t N>
class TaskManager
{
private:
    // Queue of scheduled tasks (requests for one-time pad blocks)
    BlockingQueue<OtpTask, N> scheduledTasks;
    
     // Queue of finished tasks (ready-to-use one-time pad blocks)
    QueueArray<OtpTask, N> finishedTasks;
    
    // Id of the next scheduleTask
    uint32_t nextTaskId = 0;

public:
    // base - buffer for tasks (it will be split in N chunks)
    // length - the length of the buffer
    TaskManager(uint32_t* base, size_t length)
    {
        // Schedule tasks for all available sub-buffers
        OtpTask task;
        const size_t bufferLength = length / N;
        for(int i = 0; i < N; i++)
        {
            task.buffer = base + i * bufferLength;
            task.length = bufferLength;
            scheduleTask(task);
        }
    }

    // Sends the shutdown signal to all underlying queues
    void shutdown()
    {
        // Shutdown queues to release waiting worker threads
        scheduledTasks.shutdown();
        finishedTasks.shutdown();
    }
    
    // Requests the next OTP block (called by cryptor)
    void scheduleTask(OtpTask& task)
    {
        task.id = nextTaskId++;
        scheduledTasks.push(task);
    }
    
    // Gets the next complete OTP block (called by cryptor)
    void processTask(OtpTask& task)
    {
        finishedTasks.next(task);
    }
 
    // Gets the next request for OTP block (called by workers)
    bool performTask(OtpTask& task)
    {
        return scheduledTasks.pop(task);
    }
    
    // Saves the next complete OTP block (called by workers)
    bool finishTask(OtpTask& task)
    {
        return finishedTasks.put(task, task.id);
    }
};