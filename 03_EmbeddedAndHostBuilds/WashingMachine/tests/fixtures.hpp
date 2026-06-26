/**
 * @file	fixtures.hpp
 * @brief	Fixture definitions for local tests.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef FIXTURES_HPP
#define FIXTURES_HPP

#include "mocks.hpp"

struct washer_components_fixture
{
    input_valve_mock input_valve;
    output_valve_mock output_valve;
    thermometer_mock thermometer;
    heater_mock heater;
    drum_rotator_mock motor;
    timer_mock timer;

    static washer_components_fixture create()
    {
        return washer_components_fixture{};
    }

    washer_components_state state() const
    {
        washer_components_state state;
        state.input_valve.is_open_flag = input_valve.is_open();
        state.output_valve.is_open_flag = output_valve.is_open();
        state.heater = heater.is_heating() ? heater_state::on : heater_state::off;
        state.motor = motor.is_slowly_rotating() ? motor_state::slow_spin : motor_state::off;
        state.motor = motor.is_fast_rotating() ? motor_state::fast_spin : state.motor;
        state.timer = timer.is_started() ? timer_state::started_with_timeout(timer.timeout) : timer_state::stopped();
        return state;
    }
    
};

#endif /* FIXTURES_HPP */
