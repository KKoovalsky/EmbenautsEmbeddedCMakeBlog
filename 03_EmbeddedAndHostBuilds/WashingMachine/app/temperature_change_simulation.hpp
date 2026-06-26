/**
 * @file	temperature_change_simulation.hpp
 * @brief	Simulates change of the temperature basing on that whether heater is on or off.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef TEMPERATURE_CHANGE_PROCESS_HPP
#define TEMPERATURE_CHANGE_PROCESS_HPP

#include <functional>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <stdexcept>

#include "common.hpp"
#include "job_repeater.hpp"

class temperature_change_simulation
{
  public:
    explicit temperature_change_simulation(int current_temperature,
                                           int room_temperature,
                                           heater_state current_heater_state,
                                           unsigned heat_loss_degrees_per_second,
                                           unsigned heat_gain_degrees_per_second,
                                           std::shared_ptr<job_repeater> job_repeater_impl) :
        current_temperature{current_temperature},
        room_temperature{room_temperature},
        current_heater_state{current_heater_state},
        heat_loss_degrees_per_second{heat_loss_degrees_per_second},
        heat_gain_degrees_per_second{heat_gain_degrees_per_second},
        job_repeater_impl{job_repeater_impl}
    {
        if (current_temperature < room_temperature)
            throw std::runtime_error("Current temperature can't be lower than the room temperature");

        schedule_next_temperature_change_by_one_degree_basing_on_heater_state();
    }

    int get_current_temperature() const
    {
        std::lock_guard guard{mux};
        return current_temperature;
    }

    void heater_state_changed(heater_state state)
    {
        std::lock_guard guard{mux};
        if (state != current_heater_state)
        {
            current_heater_state = state;
            schedule_next_temperature_change_by_one_degree_basing_on_heater_state();
        }
    }

    void set_callback_on_temperature_values_reached(std::function<void(int)> callback,
                                                    std::initializer_list<int> temperature_values)
    {
        std::lock_guard guard{values_observed.mux};
        values_observed.callback = callback;
        values_observed.values = {temperature_values};
    }

  private:
    struct temperature_values_observed
    {
        std::function<void(int)> callback;
        std::vector<int> values;
        std::mutex mux;
    };

    void schedule_next_temperature_change_by_one_degree_basing_on_heater_state()
    {
        job_repeater_impl->cancel_all_jobs();

        auto next_change_time_milliseconds{get_next_change_of_temperature_in_milliseconds_basing_on_heater_state()};

        job_repeater_impl->repeat_job(next_change_time_milliseconds, [this]() {
            auto new_temperature{increase_or_decrease_current_temperature_basing_on_the_heater_state()};
            call_temperature_value_observer_if_is_observed_temperature_reached(new_temperature);
        });
    }

    unsigned get_next_change_of_temperature_in_milliseconds_basing_on_heater_state() const
    {
        unsigned heat_change_degrees_per_second{
            current_heater_state == heater_state::off ? heat_loss_degrees_per_second : heat_gain_degrees_per_second};

        auto time_after_which_temperature_changes_by_one_degree_in_milliseconds{1000 / heat_change_degrees_per_second};

        return time_after_which_temperature_changes_by_one_degree_in_milliseconds;
    }

    int increase_or_decrease_current_temperature_basing_on_the_heater_state()
    {
        std::lock_guard guard{mux};

        if (current_heater_state == heater_state::off)
        {
            --current_temperature;
            if (current_temperature < room_temperature)
                current_temperature = room_temperature;
        } else if (current_heater_state == heater_state::on)
            ++current_temperature;

        return current_temperature;
    }

    void call_temperature_value_observer_if_is_observed_temperature_reached(int new_temperature)
    {
        std::lock_guard guard{values_observed.mux};

        auto is_temperature_found_in_observed_values{[&](auto temperature) {
            auto beg = std::begin(values_observed.values);
            auto end = std::end(values_observed.values);
            return std::find(beg, end, temperature) != end;
        }};

        if (is_temperature_found_in_observed_values(new_temperature))
            values_observed.callback(new_temperature);
    }

    int current_temperature;
    int room_temperature;
    heater_state current_heater_state;
    unsigned heat_loss_degrees_per_second;
    unsigned heat_gain_degrees_per_second;
    mutable std::mutex mux;

    temperature_values_observed values_observed;

    std::shared_ptr<job_repeater> job_repeater_impl;
};

#endif /* TEMPERATURE_CHANGE_PROCESS_HPP */
