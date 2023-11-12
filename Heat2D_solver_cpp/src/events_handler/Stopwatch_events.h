#pragma once
#include <chrono>

class Stopwatch_events 
{
public:
    Stopwatch_events();
    ~Stopwatch_events();
    void start();
    void stop();
    double elapsed();


private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_EndTime;
    bool m_bRunning = false;
};