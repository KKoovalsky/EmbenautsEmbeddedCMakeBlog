/**
 * @file	mocks.hpp
 * @brief	Defines test mocks.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef MOCKS_HPP
#define MOCKS_HPP

#include <catch2/catch.hpp>
#include <cctype>
#include <initializer_list>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <variant>

#include "common.hpp"
#include "overload.hpp"

template<typename ValveType>
struct valve_state
{
    valve_state(bool is_open) : is_open_flag{is_open}
    {
    }

    static valve_state<ValveType> open()
    {
        return valve_state{true};
    }

    static valve_state<ValveType> closed()
    {
        return valve_state{false};
    }

    std::string to_string() const
    {
        return is_open() ? "_valve_state::open" : "_valve_state::closed";
    }

    bool is_open() const
    {
        return is_open_flag;
    }

    bool is_closed() const
    {
        return !is_open();
    }

    bool is_open_flag{false};
};

struct generated_input_valve_state : public valve_state<generated_input_valve_state>
{
};

struct generated_output_valve_state : public valve_state<generated_output_valve_state>
{
};

using input_valve_state = valve_state<generated_input_valve_state>;
using output_valve_state = valve_state<generated_output_valve_state>;

enum class motor_state
{
    slow_spin,
    fast_spin,
    off
};

class timer_state
{
    explicit constexpr timer_state(unsigned expected_timeout_in_minutes, bool is_started_flag) :
        expected_timeout_in_minutes{expected_timeout_in_minutes}, is_started_flag{is_started_flag}
    {
    }

  public:
    static constexpr timer_state started_with_timeout(unsigned timeout_in_minutes)
    {
        return timer_state{timeout_in_minutes, true};
    }

    static constexpr timer_state stopped()
    {
        return timer_state{0, false};
    }

    constexpr bool is_started() const
    {
        return is_started_flag;
    }

    unsigned expected_timeout_in_minutes{0};
    bool is_started_flag{false};
};

CATCH_REGISTER_ENUM(heater_state, heater_state::on, heater_state::off);
CATCH_REGISTER_ENUM(motor_state, motor_state::slow_spin, motor_state::fast_spin, motor_state::off);

struct washer_components_state
{
    input_valve_state input_valve{input_valve_state::closed()};
    output_valve_state output_valve{output_valve_state::closed()};
    heater_state heater{heater_state::off};
    motor_state motor{motor_state::off};
    timer_state timer{timer_state::stopped()};
};

template<typename Enum>
std::string enum_to_string(Enum e)
{
    return Catch::StringMaker<Enum>::convert(e);
}

template<typename T>
bool operator==(const valve_state<T>& l, const valve_state<T>& r)
{
    return l.is_open() == r.is_open();
}

static inline std::ostream& operator<<(std::ostream& os, input_valve_state state)
{
    return os << "input" << state.to_string();
}

static inline std::ostream& operator<<(std::ostream& os, output_valve_state state)
{
    return os << "output" << state.to_string();
}

static inline std::ostream& operator<<(std::ostream& os, heater_state state)
{
    return os << "heater_state::" << enum_to_string(state);
}

static inline std::ostream& operator<<(std::ostream& os, motor_state state)
{
    return os << "motor_state::" << enum_to_string(state);
}

static inline std::ostream& operator<<(std::ostream& os, timer_state state)
{
    os << "timer_state::";
    if (state.is_started())
        os << "started_with_timeout: " << state.expected_timeout_in_minutes;
    else
        os << "not_running";
    return os;
}

static inline std::ostream& operator<<(std::ostream& os, washer_components_state state)
{
    return os << state.input_valve << ' ' << state.output_valve << ' ' << state.heater << ' ' << state.motor << ' '
              << state.timer;
}

static inline bool operator==(const timer_state& l, const timer_state& r)
{
    return l.is_started_flag == r.is_started_flag && l.expected_timeout_in_minutes == r.expected_timeout_in_minutes;
}

class HaveState : public Catch::MatcherBase<washer_components_state>
{
  public:
    using ComponentStates = std::variant<input_valve_state, output_valve_state, heater_state, motor_state, timer_state>;

    HaveState(std::initializer_list<ComponentStates> component_states) : component_states{component_states}
    {
    }

    bool match(const washer_components_state& washer_states_snapshot) const override
    {
        for (auto component_state : component_states)
        {
            auto result{
                std::visit(overload{
                               [&](input_valve_state state) { return state == washer_states_snapshot.input_valve; },
                               [&](output_valve_state state) { return state == washer_states_snapshot.output_valve; },
                               [&](heater_state state) { return state == washer_states_snapshot.heater; },
                               [&](motor_state state) { return state == washer_states_snapshot.motor; },
                               [&](timer_state state) { return state == washer_states_snapshot.timer; },
                           },
                           component_state)};
            if (!result)
                return false;
        }
        return true;
    }

    virtual std::string describe() const override
    {
        auto component_state_to_string{[](auto component_state, auto& stream) {
            return std::visit([&stream](auto state) { stream << state; }, component_state);
        }};

        std::ostringstream ss;
        ss << "have state: ";
        for (auto c : component_states)
        {
            std::visit([&ss](auto state) { ss << state << ' '; }, c);
        }
        return ss.str();
    }

  private:
    std::vector<ComponentStates> component_states;
};

struct valve_mock
{
    void open()
    {
        ++opened_times;
    }

    void close()
    {
        ++closed_times;
    }

    bool is_open() const
    {
        return opened_times > closed_times;
    }

    unsigned opened_times{0};
    unsigned closed_times{0};
};

struct thermometer_mock
{
    int get_current_drum_temperature()
    {
        return current_water_temperature;
    }

    int current_water_temperature{0};
};

struct heater_mock
{
    void start_heat_water()
    {
        is_heating_flag = true;
    }

    void stop_heat_water()
    {
        is_heating_flag = false;
    }

    bool is_heating() const
    {
        return is_heating_flag;
    }

    bool is_stopped() const
    {
        return !is_heating();
    }

    bool is_heating_flag{false};
};

struct drum_rotator_mock
{
    void start_rotating(rotation_speed speed)
    {
        if (speed == rotation_speed::slow)
            ++start_slow_rotation_called_count;
        else if (speed == rotation_speed::fast)
            ++start_fast_rotation_called_count;
    }

    void stop_rotating()
    {
        ++stop_rotating_called_count;
    }

    bool is_slowly_rotating() const
    {
        return start_slow_rotation_called_count == 1 && stop_rotating_called_count == 0;
    }

    bool is_fast_rotating() const
    {
        return start_fast_rotation_called_count == 1 && stop_rotating_called_count == 0;
    }

    bool is_stopped() const
    {
        return start_slow_rotation_called_count == 0 && stop_rotating_called_count == 0;
    }

    unsigned start_slow_rotation_called_count{0};
    unsigned start_fast_rotation_called_count{0};
    unsigned stop_rotating_called_count{0};
};

struct timer_mock
{
    void start(unsigned minutes_to_timeout)
    {
        ++start_timer_called_count;
        timeout = minutes_to_timeout;
    }

    bool is_started() const
    {
        return start_timer_called_count == 1;
    }

    unsigned start_timer_called_count{0};
    unsigned timeout{0};
};

struct input_valve_mock : public valve_mock
{
    void start_pour_water()
    {
        valve_mock::open();
    }

    void stop_pour_water()
    {
        valve_mock::close();
    }
};

struct output_valve_mock : public valve_mock
{
    void start_drain_water()
    {
        valve_mock::open();
    }

    void stop_drain_water()
    {
        valve_mock::close();
    }
};

#endif /* MOCKS_HPP */
