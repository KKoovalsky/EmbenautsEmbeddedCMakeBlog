/**
 * @file	cycle.hpp
 * @brief       High level cycle definition. Cycle can be washing, rinsing, drying, etc.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef CYCLE_HPP
#define CYCLE_HPP

#include <variant>

#include "events.hpp"
#include "states.hpp"

struct cycle
{
    virtual void handle(Event) = 0;
    virtual bool is_finished() const = 0;
    virtual void start() = 0;
    virtual ~cycle() = default;
};

template<typename... States>
class cycle_template_method : public cycle
{
  public:
    bool is_finished() const override
    {
        auto is_current_state_equal_to_finish_state{[&]() {
            return std::holds_alternative<finished>(current_state);
        }};

        return is_current_state_equal_to_finish_state();
    }

    void start() override
    {
        auto is_current_state_equal_to_idle_state{[&]() {
            return std::holds_alternative<idle>(current_state);
        }};

        if (is_current_state_equal_to_idle_state())
            current_state = on_start();
    }

    virtual ~cycle_template_method() = default;

  protected:
    using State = std::variant<idle, error, finished, States...>;

    virtual State on_start() = 0;

    State current_state{idle{}};
};

#endif /* CYCLE_HPP */
