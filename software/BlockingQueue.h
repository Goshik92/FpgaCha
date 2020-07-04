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
#include <mutex>
#include <iostream>
#include <condition_variable>

// Blocking fixed size queue with shutdown support
// I - type of items in the queue
// N - queue size
template<typename I, size_t N>
class BlockingQueue
{
private:
    std::mutex mutex;
    std::condition_variable cvNotEmpty;
    std::condition_variable cvNotFull;
    std::array<I, N> items;
    size_t head = 0;
    size_t tail = 0;
    bool full = false;
    bool stopped = false;
 
public:
    // Returns the number of available slots in the queue
    size_t count() const
    {
        const size_t c = tail - head;
        return full ? N : (c < 0 ? c + N : c);
    }
    
    // Enables the shutdown mode
    void shutdown()
    {
        stopped = true;
        cvNotEmpty.notify_all();
        cvNotFull.notify_all();
    }

    // Push an element to queue
    // Returns false if the shutdown mode was enabled
    // In this case <item> is not pushed
    bool push(I& item)
    {
        if (stopped) return false;
        
        {
            auto lock = std::unique_lock<std::mutex>(mutex);
            cvNotFull.wait(lock, [&]{ return count() < N || stopped; });
            if (stopped) return false;
            items[tail] = item;
            tail = tail == N - 1 ? 0 : tail + 1;
            if (head == tail) full = true;
        }
        
        cvNotEmpty.notify_one();
        
        return true;
    }
    
    // Get an element from queue
    // Returns false if the shutdown mode was enabled
    // In this case <item> is invalid
    bool pop(I& item)
    {
        if (stopped) return false;
        
        {
            auto lock = std::unique_lock<std::mutex>(mutex);
            cvNotEmpty.wait(lock, [&]{ return count() > 0 || stopped; });
            if (stopped) return false;
            item = items[head];
            head = head == N - 1 ? 0 : head + 1;
            if (head == tail) full = false;
        }
        
        cvNotFull.notify_one();
        
        return true;
    }
};