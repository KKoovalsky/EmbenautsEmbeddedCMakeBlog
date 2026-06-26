/**
 * @file	drivers.hpp
 * @brief	Contains all the hardware drivers
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef DRIVERS_HPP
#define DRIVERS_HPP

#include <functional>

#include "common.hpp"

struct input_valve_driver
{
    explicit input_valve_driver(std::function<void(void)> on_start_pour_water,
                                std::function<void(void)> on_stop_pour_water) :
        start_pour_water{on_start_pour_water}, stop_pour_water{on_stop_pour_water}
    {
    }

    std::function<void(void)> start_pour_water;
    std::function<void(void)> stop_pour_water;
};

struct output_valve_driver
{
    explicit output_valve_driver(std::function<void(void)> on_start_drain_water,
                                 std::function<void(void)> on_stop_drain_water) :
        start_drain_water{on_start_drain_water}, stop_drain_water{on_stop_drain_water}
    {
    }

    std::function<void(void)> start_drain_water;
    std::function<void(void)> stop_drain_water;
};

struct thermometer_driver
{
    explicit thermometer_driver(std::function<int(void)> on_get_current_drum_temperature) :
        get_current_drum_temperature{on_get_current_drum_temperature}
    {
    }

    std::function<int(void)> get_current_drum_temperature;
};

struct heater_driver
{
    explicit heater_driver(std::function<void(void)> on_start_heat_water,
                           std::function<void(void)> on_stop_heat_water) :
        start_heat_water{on_start_heat_water}, stop_heat_water{on_stop_heat_water}
    {
    }

    std::function<void(void)> start_heat_water;
    std::function<void(void)> stop_heat_water;
};

struct motor_driver
{
    explicit motor_driver(std::function<void(rotation_speed)> on_start_rotating,
                          std::function<void(void)> on_stop_rotating) :
        start_rotating{on_start_rotating}, stop_rotating{on_stop_rotating}
    {
    }

    std::function<void(rotation_speed)> start_rotating;
    std::function<void(void)> stop_rotating;
};

struct timer_driver
{
    explicit timer_driver(std::function<void(unsigned)> on_start) : start{on_start}
    {
    }

    std::function<void(unsigned)> start;
};

#endif /* DRIVERS_HPP */
