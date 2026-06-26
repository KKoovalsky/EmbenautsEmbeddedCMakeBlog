/**
 * @file	water_flow_simulation.hpp
 * @brief	Water flow simulation implementation
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef WATER_FLOW_SIMULATION_HPP
#define WATER_FLOW_SIMULATION_HPP

#include <functional>
#include <memory>
#include <stdexcept>

#include "common.hpp"
#include "job_delayer.hpp"

class water_flow_simulation
{
  public:
    water_flow_simulation(unsigned milliseconds_to_full_water,
                          unsigned milliseconds_to_empty_water,
                          std::shared_ptr<job_delayer> job_delayer_impl,
                          std::function<void(water_level)> callback_on_water_reached_full_or_empty) :
        milliseconds_to_full_water{milliseconds_to_full_water},
        milliseconds_to_empty_water{milliseconds_to_empty_water},
        job_delayer_impl{job_delayer_impl},
        callback_on_water_reached_full_or_empty{callback_on_water_reached_full_or_empty}
    {
    }

    void input_valve_opened()
    {
        if (current_water_flow_state == water_flow_state::emptied)
        {
            auto simulate_water_is_full_in_the_future{[&]() {
                job_delayer_impl->delay_job(milliseconds_to_full_water, [&]() {
                    current_water_flow_state = water_flow_state::full_still_filling;
                    callback_on_water_reached_full_or_empty(water_level::full);
                });
            }};

            current_water_flow_state = water_flow_state::filling;
            simulate_water_is_full_in_the_future();
        } else
        {
            throw_wrong_transition_in_water_flow();
        }
    }

    void input_valve_closed()
    {
        if (current_water_flow_state == water_flow_state::full_still_filling)
        {
            current_water_flow_state = water_flow_state::full;
        } else
        {
            throw_wrong_transition_in_water_flow();
        }
    }

    void output_valve_opened()
    {
        if (current_water_flow_state == water_flow_state::full)
        {
            auto simulate_water_is_emptied_in_the_future{[&]() {
                job_delayer_impl->delay_job(milliseconds_to_empty_water, [&]() {
                    current_water_flow_state = water_flow_state::emptied_still_emptying;
                    callback_on_water_reached_full_or_empty(water_level::empty);
                });
            }};

            current_water_flow_state = water_flow_state::emptying;
            simulate_water_is_emptied_in_the_future();
        } else
        {
            throw_wrong_transition_in_water_flow();
        }
    }

    void output_valve_closed()
    {
        if (current_water_flow_state == water_flow_state::emptied_still_emptying)
        {
            current_water_flow_state = water_flow_state::emptied;
        } else
        {
            throw_wrong_transition_in_water_flow();
        }
    }

  private:
    enum class water_flow_state
    {
        emptied,
        filling,
        full_still_filling,
        full,
        emptying,
        emptied_still_emptying
    };

    void throw_wrong_transition_in_water_flow() const
    {
        throw std::runtime_error("Wrong transition in water flow state machine");
    }

    water_flow_state current_water_flow_state{water_flow_state::emptied};

    unsigned milliseconds_to_full_water;
    unsigned milliseconds_to_empty_water;
    std::shared_ptr<job_delayer> job_delayer_impl;
    std::function<void(water_level)> callback_on_water_reached_full_or_empty;
};

#endif /* WATER_FLOW_SIMULATION_HPP */
