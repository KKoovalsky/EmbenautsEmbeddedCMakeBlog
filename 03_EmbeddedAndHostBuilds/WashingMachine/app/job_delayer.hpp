/**
 * @file	job_delayer.hpp
 * @brief	Interface for the job delayer.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef JOB_DELAYER_HPP
#define JOB_DELAYER_HPP

#include <functional>

struct job_delayer
{
    virtual void delay_job(unsigned milliseconds, std::function<void(void)> job_callback) = 0;
    virtual ~job_delayer() = default;
};


#endif /* JOB_DELAYER_HPP */
