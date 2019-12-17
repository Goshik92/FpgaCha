#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <stdexcept>
#include "DevMem.h"
#include "PhyMap.h"
#include "ClockCounter.h"

#define CLOCK_COUNTER_FREQ 50000000UL

int main()
{
    try
    {
        DevMem devMem = DevMem();
        
        // Get the mapping to hardware registers of the Clock Counter module
        auto clockCounterMap = PhyMap<ClockCounter::Dev>(devMem, 0xFF200180);
        
        // Get pointer the struct with Clock Counter control registers
        auto clockCounterDev = clockCounterMap();

        std::cout << "usleep delay measurement using Clock Counter hw module:" << std::endl;

        clockCounterDev->Reset(); // Reset the counter
        usleep(1); // Sleep for 1 us
        double oneUSec = 1000000.0 * clockCounterDev->GetTime() / CLOCK_COUNTER_FREQ;
        std::cout << "usleep(1): " << oneUSec << " us" << std::endl;

        clockCounterDev->Reset(); // Reset the counter
        usleep(1000); // Sleep for 1 ms
        double oneMSec = 1000.0 * clockCounterDev->GetTime() / CLOCK_COUNTER_FREQ;
        std::cout << "usleep(1000): " << oneMSec << " ms" << std::endl;

        clockCounterDev->Reset(); // Reset the counter
        usleep(1000000); // Sleep for 1 sec
        double oneSec = 1.0 * clockCounterDev->GetTime() / CLOCK_COUNTER_FREQ;
        std::cout << "usleep(1000000): " << oneSec << " sec" << std::endl;
    }
    
    catch (std::runtime_error e)
    {
        std::cout << e.what() << std::endl;
    }
}