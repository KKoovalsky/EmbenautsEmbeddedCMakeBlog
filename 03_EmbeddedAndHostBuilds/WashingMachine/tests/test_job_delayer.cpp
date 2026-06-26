/**
 * @file	test_job_delayer.cpp
 * @brief	Tests the job delayer for the native build.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include <catch2/catch.hpp>

#include "helpers.hpp"
#include "job_delayer_impl.hpp"

TEST_CASE("Jobs are delayed", "[job_delayer]")
{
    SECTION("Job is executed after specified time")
    {
        test_helpers::flag job_executed_flag;
        job_delayer_impl j;
        j.delay_job(100, [&]() { job_executed_flag.set(); });

        job_executed_flag.wait();
        REQUIRE(job_executed_flag.is_set());
    }
}
