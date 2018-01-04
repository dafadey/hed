#pragma once
#include <chrono>
#include <iostream>

struct timer
{
  std::chrono::high_resolution_clock::time_point t1;
  std::ostream& s;
  std::string timer_msg;
  timer() : t1(std::chrono::high_resolution_clock::now()),
            s(std::cout),
            timer_msg("time is: ") {}
  timer(std::string _msg) : t1(std::chrono::high_resolution_clock::now()),
                            s(std::cout),
                            timer_msg(_msg) {}
  ~timer()
  {
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    s << timer_msg << time_span.count() << " sec." << std::endl;
  }
};