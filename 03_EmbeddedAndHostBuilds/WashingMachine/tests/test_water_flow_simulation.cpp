/**
 * @file	test_water_flow_simulation.cpp
 * @brief	Tests the water flow simulation
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include <catch2/catch.hpp>
#include <memory>

#include "common.hpp"
#include "water_flow_simulation.hpp"

struct job_delayer_mock : public job_delayer
{
    void delay_job(unsigned milliseconds, std::function<void(void)> job_callback) override
    {
        milliseconds_to_delayed_job = milliseconds;
        delayed_job = job_callback;
    }

    void call_delayed_job()
    {
        delayed_job();
    }

    unsigned milliseconds_to_delayed_job;
    std::function<void(void)> delayed_job;
};

TEST_CASE("Water flow is simulated", "[water_flow]")
{
    auto job_delayer_mock_impl{std::make_shared<job_delayer_mock>()};

    SECTION("Water gets full after specific time from opening input valve")
    {
        unsigned time_to_water_full_in_milliseconds{200};
        bool is_water_full{false};

        water_flow_simulation w{
            time_to_water_full_in_milliseconds, 2, job_delayer_mock_impl, [&is_water_full](water_level level) {
                if (level == water_level::full)
                    is_water_full = true;
            }};

        w.input_valve_opened();

        auto was_not_water_full_before_filling{!is_water_full};

        job_delayer_mock_impl->call_delayed_job();

        REQUIRE(was_not_water_full_before_filling);
        REQUIRE(is_water_full);
        REQUIRE(job_delayer_mock_impl->milliseconds_to_delayed_job == time_to_water_full_in_milliseconds);
    }

    SECTION("Water gets empty after specific time from opening output valve when water is full")
    {
        unsigned time_to_water_empty_in_milliseconds{511};
        bool is_water_empty{false};

        water_flow_simulation w{
            2, time_to_water_empty_in_milliseconds, job_delayer_mock_impl, [&is_water_empty](water_level level) {
                if (level == water_level::empty)
                    is_water_empty = true;
            }};

        w.input_valve_opened();
        job_delayer_mock_impl->call_delayed_job();
        w.input_valve_closed();

        auto was_water_not_empty_before_emptying{!is_water_empty};

        w.output_valve_opened();
        job_delayer_mock_impl->call_delayed_job();

        REQUIRE(was_water_not_empty_before_emptying);
        REQUIRE(is_water_empty);
        REQUIRE(job_delayer_mock_impl->milliseconds_to_delayed_job == time_to_water_empty_in_milliseconds);
    }
}
