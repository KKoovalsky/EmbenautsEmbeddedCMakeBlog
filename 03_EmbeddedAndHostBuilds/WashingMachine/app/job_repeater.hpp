/**
 * @file	job_repeater.hpp
 * @brief	Defines interface for a job repeater
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef JOB_REPEATER_HPP
#define JOB_REPEATER_HPP

#include <functional>

struct job_repeater
{
    virtual void repeat_job(unsigned milliseconds, std::function<void(void)> job_callback) = 0;
    virtual void cancel_all_jobs() = 0;
    virtual ~job_repeater() = default;
};

#endif /* JOB_REPEATER_HPP */
