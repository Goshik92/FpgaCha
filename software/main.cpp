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

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <fstream>
#include "ChaCha20/Key.h"
#include "ChaCha20/Nonce.h"
#include "ChaCha20/BCount.h"
#include "FpgaCha/UDmaBuf.h"
#include "TaskManager.h"
#include "FakeCryptor.h"
#include "FakeWorker.h"
#include "ChaCha20Worker.h"
#include "FpgaChaWorker.h"
#include "FileMapper.h"
#include "FileCryptor.h"

// Set encryption parameters
ChaCha20::State state
{
    ChaCha20::DEFAULT_CONSTS,
    
    // Key from an example in RFC8439 (see page 10)
    ChaCha20::Key 
    {
        0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
        0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c
    },

    // Block count from an example in RFC8439 (see page 10)
    ChaCha20::BCount{ 0x00000001 },

    // Nonce from an example in RFC8439 (see page 10)
    ChaCha20::Nonce{ 0x09000000, 0x4a000000, 0x00000000 }
};

// Number of buffers (and tasks)
const uint32_t N = 8;

// Total buffer size in words
// Must be less or equal to uDmaBuf.size
const size_t buffSize = N*1024*256;

int main(int argc, char* argv[])
{
    try
    {   
        // Validate input arguments
        if (argc < 3) throw std::runtime_error("too few arguments passed");
        
        // Open input and output files
        FileMapper inFile(argv[1]);
        FileMapper outFile(argv[2], inFile.getSize());
        
        // Initialize buffer for DMA
        FpgaCha::UDmaBuf uDmaBuf("udmabuf0");
        
        // Task manager to coordinate cryptor and workers
        TaskManager<N> m(uDmaBuf.content, buffSize);

        // Choose one of the cryptors
        FileCryptor<N> fc(m, inFile, outFile);
        //FakeCryptor<N> fc(m, 1024*1024*256);
        
        // Choose workers
        //FakeWorker<N> fw(m);
        //ChaCha20Worker<N> ccw0(m, state);
        //ChaCha20Worker<N> ccw1(m, state);
        FpgaChaWorker<N, 1> fcw0(m, state, "uio0", uDmaBuf);
        //FpgaChaWorker<N, 1> fcw1(m, state, "uio1", uDmaBuf);
        //FpgaChaWorker<N, 1> fcw2(m, state, "uio2", uDmaBuf);
        //FpgaChaWorker<N, 1> fcw3(m, state, "uio3", uDmaBuf);
    }
    
    catch (std::runtime_error e)
    {
        std::cout << e.what() << std::endl;
    }
}