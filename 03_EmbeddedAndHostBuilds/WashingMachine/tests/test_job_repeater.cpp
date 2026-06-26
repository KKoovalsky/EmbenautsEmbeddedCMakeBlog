/**
 * @file	test_job_repeater.cpp
 * @brief	Tests the job repeater implementation.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include <catch2/catch.hpp>

#include "helpers.hpp"
#include "job_repeater_impl.hpp"

TEST_CASE("Jobs are repeated", "[job_repeater]")
{
    SECTION("Job is repeated multiple times")
    {
        test_helpers::flag job_called_few_times;

        job_repeater_impl j;
        j.repeat_job(40, [&]() {
            static unsigned called_times{0};
            ++called_times;
            if (called_times == 4)
                job_called_few_times.set();
        });

        job_called_few_times.wait();
        REQUIRE(job_called_few_times.is_set());
    }

    SECTION("New job is started after cancelling the previous one")
    {
        unsigned first_job_called_counter{0};
        test_helpers::flag first_job_called_few_times_flag;

        job_repeater_impl j;
        j.repeat_job(10, [&]() {
            ++first_job_called_counter;
            if (first_job_called_counter == 4)
                first_job_called_few_times_flag.set();
        });

        first_job_called_few_times_flag.wait();
        j.cancel_all_jobs();
        auto first_job_called_counter_after_cancelling_the_job{first_job_called_counter};

        test_helpers::flag second_job_called_few_times;
        j.repeat_job(40, [&]() {
            static unsigned called_times{0};
            ++called_times;
            if (called_times == 4)
                second_job_called_few_times.set();
        });

        second_job_called_few_times.wait();

        auto is_first_job_cancelled_for_sure{first_job_called_counter
                                             == first_job_called_counter_after_cancelling_the_job};
        REQUIRE(second_job_called_few_times.is_set());
        REQUIRE(is_first_job_cancelled_for_sure);
    }
}
