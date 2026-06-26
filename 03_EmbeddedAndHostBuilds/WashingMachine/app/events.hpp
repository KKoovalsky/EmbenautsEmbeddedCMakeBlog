/**
 * @file	events.hpp
 * @brief	Defines events which happen to the washer state machine.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef WASHER_EVENTS_HPP
#define WASHER_EVENTS_HPP

#include <variant>

struct water_full
{
};

struct reached_20_degrees
{
};

struct reached_40_degrees
{
};

struct reached_41_degrees
{
};

struct timer_stopped
{
};

struct water_empty
{
};

using Event =
    std::variant<water_full, reached_20_degrees, reached_40_degrees, reached_41_degrees, timer_stopped, water_empty>;

#endif /* WASHER_EVENTS_HPP */
