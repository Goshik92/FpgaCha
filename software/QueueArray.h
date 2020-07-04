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

// Array of queues to access tasks in the order if increasing their IDs
// I - type of items stored
// N - number of items to store (also the number of queues)
template<typename I, size_t N>
class QueueArray
{
private:
    // Low level array of queues
    std::array<BlockingQueue<I, 1>, N> nodes;
    
    // Next queue to be read from
    size_t nextIndex = N - 1;

public:
    // Shutdowns all queues in the array
    void shutdown()
    {
        for(auto& node : nodes)
            node.shutdown();
    }

    // Puts <item> in the <index mod N> queue  
    bool put(I& item, size_t index)
    {
        return nodes[index % N].push(item);
    }
    
    // Get an element from the next queue
    bool next(I& item)
    {
        nextIndex = nextIndex == N - 1 ? 0 : nextIndex + 1;
        return nodes[nextIndex].pop(item);
    }
};