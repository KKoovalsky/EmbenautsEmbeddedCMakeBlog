/**
 * @file	common.hpp
 * @brief	Common definitions.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef COMMON_HPP
#define COMMON_HPP

#include <variant>

enum class rotation_speed
{
    slow,
    fast
};

enum class heater_state
{
    on,
    off
};

enum class water_level
{
    empty,
    full
};

enum class valve_io_state
{
    open,
    closed
};

template<typename WantedType, typename Variant>
bool is(const Variant& the_variant)
{
    return std::holds_alternative<WantedType>(the_variant);
}

#endif /* COMMON_HPP */
