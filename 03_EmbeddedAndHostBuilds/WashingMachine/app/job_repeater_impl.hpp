/**
 * @file	job_repeater_impl.hpp
 * @brief	Implements a job repeater for native build.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef JOB_REPEATER_IMPL_HPP
#define JOB_REPEATER_IMPL_HPP

#include <chrono>
#include <cpptime.h>

#include "job_repeater.hpp"

class job_repeater_impl : public job_repeater
{
  public:
    void repeat_job(unsigned milliseconds, std::function<void(void)> job_callback) override
    {
        auto repeat_period{std::chrono::milliseconds(milliseconds)};
        current_timer_id = timer.add(repeat_period, [=](auto) { job_callback(); }, repeat_period);
    }

    void cancel_all_jobs() override
    {
        timer.remove(current_timer_id);
    }

private:
    CppTime::Timer timer;
    CppTime::timer_id current_timer_id;
};

#endif /* JOB_REPEATER_IMPL_HPP */
