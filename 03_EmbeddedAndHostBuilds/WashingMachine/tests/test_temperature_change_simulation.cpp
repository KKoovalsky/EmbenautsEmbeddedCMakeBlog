/**
 * @file	test_temperature_change_simulation.cpp
 * @brief	Test the heat loss and increase process.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include <catch2/catch.hpp>
#include <memory>

#include "temperature_change_simulation.hpp"

struct job_repeater_mock : job_repeater
{
    void repeat_job(unsigned milliseconds, std::function<void(void)> job_callback) override
    {
        job_call_delay_milliseconds = milliseconds;
        delayed_job_callback = job_callback;
    }

    void cancel_all_jobs() override
    {
    }

    unsigned job_call_delay_milliseconds{0};
    std::function<void(void)> delayed_job_callback;
};

TEST_CASE("Temperature increases or decreases basing on that whether heater is on or off", "[temperature]")
{
    auto job_repeater_mock_impl{std::make_shared<job_repeater_mock>()};

    auto is_next_temperature_change_in_milliseconds{[&](unsigned milliseconds) {
        return job_repeater_mock_impl->job_call_delay_milliseconds == milliseconds;
    }};

    auto repeat_job_from_change_temperature_process{[&]() {
        job_repeater_mock_impl->delayed_job_callback();
    }};

    SECTION("Current temperature shall not be lower than the room temperature at start")
    {
        REQUIRE_THROWS_WITH(
            temperature_change_simulation(19, 20, heater_state::off, 1, 2, std::make_shared<job_repeater_mock>()),
            "Current temperature can't be lower than the room temperature");
    }

    GIVEN("Current temperature higher than room temperature")
    {
        int current_temperature{25};
        int room_temperature{20};

        WHEN("Heater is off")
        {
            auto heater_state{heater_state::off};

            THEN("Temperature decreases with pre-set pace")
            {
                unsigned heat_loss_degrees_per_second{2};

                temperature_change_simulation p{current_temperature,
                                                room_temperature,
                                                heater_state,
                                                heat_loss_degrees_per_second,
                                                10,
                                                job_repeater_mock_impl};

                REQUIRE(p.get_current_temperature() == current_temperature);
                REQUIRE(is_next_temperature_change_in_milliseconds(500));
                repeat_job_from_change_temperature_process();

                int temperature_after_500_milliseconds = current_temperature - 1;
                REQUIRE(temperature_after_500_milliseconds == p.get_current_temperature());
            }
        }
        WHEN("Heater is on")
        {
            auto heater_state{heater_state::on};

            THEN("Temperature increases with pre-set pace")
            {
                unsigned heat_gain_degrees_per_second{3};

                temperature_change_simulation p{current_temperature,
                                                room_temperature,
                                                heater_state,
                                                10,
                                                heat_gain_degrees_per_second,
                                                job_repeater_mock_impl};

                REQUIRE(p.get_current_temperature() == current_temperature);
                REQUIRE(is_next_temperature_change_in_milliseconds(333));
                repeat_job_from_change_temperature_process();

                int temperature_after_500_milliseconds = current_temperature + 1;
                REQUIRE(temperature_after_500_milliseconds == p.get_current_temperature());
            }
        }
    }

    SECTION("Temperature can't decrease to a lower value than the room temperature")
    {
        int current_temperature{21};
        int room_temperature{20};

        temperature_change_simulation p{
            current_temperature, room_temperature, heater_state::off, 1, 10, job_repeater_mock_impl};

        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();

        REQUIRE(room_temperature == p.get_current_temperature());
    }

    SECTION("Temperature increases after heater is turned on")
    {
        int current_temperature{26};
        int room_temperature{20};
        unsigned heat_loss_degrees_per_second{2};
        unsigned heat_gain_degrees_per_second{4};

        temperature_change_simulation p{current_temperature,
                                        room_temperature,
                                        heater_state::off,
                                        heat_loss_degrees_per_second,
                                        heat_gain_degrees_per_second,
                                        job_repeater_mock_impl};

        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();

        p.heater_state_changed(heater_state::on);

        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();

        REQUIRE(28 == p.get_current_temperature());
        REQUIRE(is_next_temperature_change_in_milliseconds(250));
    }

    SECTION("Temperature decreases after heater is turned off")
    {
        int current_temperature{26};
        int room_temperature{20};
        unsigned heat_loss_degrees_per_second{2};
        unsigned heat_gain_degrees_per_second{4};

        temperature_change_simulation p{current_temperature,
                                        room_temperature,
                                        heater_state::on,
                                        heat_loss_degrees_per_second,
                                        heat_gain_degrees_per_second,
                                        job_repeater_mock_impl};

        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();

        p.heater_state_changed(heater_state::off);

        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();

        REQUIRE(24 == p.get_current_temperature());
        REQUIRE(is_next_temperature_change_in_milliseconds(500));
    }

    SECTION("It is informed when specific temperature threshold is reached")
    {
        int current_temperature{22};
        int room_temperature{18};
        unsigned heat_loss_degrees_per_second{2};
        unsigned heat_gain_degrees_per_second{4};

        temperature_change_simulation p{current_temperature,
                                        room_temperature,
                                        heater_state::off,
                                        heat_loss_degrees_per_second,
                                        heat_gain_degrees_per_second,
                                        job_repeater_mock_impl};

        bool temperature_reached_callback_called{false};
        p.set_callback_on_temperature_values_reached([&](int) { temperature_reached_callback_called = true; }, {20});

        repeat_job_from_change_temperature_process();

        auto callback_not_called_before_target_temperature_reached{!temperature_reached_callback_called};

        repeat_job_from_change_temperature_process();

        REQUIRE(callback_not_called_before_target_temperature_reached);
        REQUIRE(temperature_reached_callback_called);
    }

    SECTION("Valid temperature value is propagated on observed temperature value reached")
    {
        int current_temperature{23};
        int room_temperature{18};
        unsigned heat_loss_degrees_per_second{2};
        unsigned heat_gain_degrees_per_second{4};

        temperature_change_simulation p{current_temperature,
                                        room_temperature,
                                        heater_state::on,
                                        heat_loss_degrees_per_second,
                                        heat_gain_degrees_per_second,
                                        job_repeater_mock_impl};

        int temperature_propagated{0};
        p.set_callback_on_temperature_values_reached([&](int temperature) { temperature_propagated = temperature; },
                                                     {24});
        repeat_job_from_change_temperature_process();

        REQUIRE(temperature_propagated == 24);
    }

    SECTION("It is informed when specific temperature threshold is reached second time after heater state changed")
    {
        int current_temperature{23};
        int room_temperature{18};
        unsigned heat_loss_degrees_per_second{2};
        unsigned heat_gain_degrees_per_second{4};

        temperature_change_simulation p{current_temperature,
                                        room_temperature,
                                        heater_state::off,
                                        heat_loss_degrees_per_second,
                                        heat_gain_degrees_per_second,
                                        job_repeater_mock_impl};

        unsigned temperature_reached_callback_called_times{0};
        p.set_callback_on_temperature_values_reached([&](int) { ++temperature_reached_callback_called_times; }, {22});

        repeat_job_from_change_temperature_process();

        auto callback_called_first_time_on_reaching_target_temperature{temperature_reached_callback_called_times == 1};

        repeat_job_from_change_temperature_process();
        p.heater_state_changed(heater_state::on);
        repeat_job_from_change_temperature_process();

        REQUIRE(callback_called_first_time_on_reaching_target_temperature);
        REQUIRE(temperature_reached_callback_called_times == 2);
    }

    SECTION("It is informed that multiple temperature thresholds were reached")
    {
        int current_temperature{23};
        int room_temperature{18};
        unsigned heat_loss_degrees_per_second{2};
        unsigned heat_gain_degrees_per_second{4};

        temperature_change_simulation p{current_temperature,
                                        room_temperature,
                                        heater_state::on,
                                        heat_loss_degrees_per_second,
                                        heat_gain_degrees_per_second,
                                        job_repeater_mock_impl};

        unsigned temperature_reached_callback_called_times{0};
        p.set_callback_on_temperature_values_reached([&](int) { ++temperature_reached_callback_called_times; },
                                                     {24, 26});

        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();
        repeat_job_from_change_temperature_process();

        REQUIRE(temperature_reached_callback_called_times == 2);
    }
}
