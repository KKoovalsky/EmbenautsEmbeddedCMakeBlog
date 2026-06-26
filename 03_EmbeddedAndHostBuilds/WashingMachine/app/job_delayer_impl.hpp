/**
 * @file	job_delayer_impl.hpp
 * @brief	Implements a job delayer
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef JOB_DELAYER_IMPL_HPP
#define JOB_DELAYER_IMPL_HPP

#include <chrono>

#include "job_delayer.hpp"
#include <cpptime.h>

class job_delayer_impl : public job_delayer
{
  public:
    void delay_job(unsigned milliseconds, std::function<void(void)> job_callback) override
    {
        timer.add(std::chrono::milliseconds(milliseconds), [=](auto) { job_callback(); });
    }

  private:
    CppTime::Timer timer;
};

#endif /* JOB_DELAYER_IMPL_HPP */
