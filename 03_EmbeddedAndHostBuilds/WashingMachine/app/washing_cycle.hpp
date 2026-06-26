/**
 * @file	washing_cycle.cpp
 * @brief	Implements washing cycle
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef WASHING_STAGE_CPP
#define WASHING_STAGE_CPP

#include <variant>

#include "common.hpp"
#include "cycle.hpp"
#include "events.hpp"
#include "overload.hpp"
#include "states.hpp"

template<typename InputWaterController,
         typename OutputWaterController,
         typename TemperatureSensor,
         typename Heater,
         typename DrumRotator,
         typename Timer>
class washing_cycle : public cycle_template_method<pouring_water, waiting_drum_gets_cool, heating, rotating, drainage>
{
  public:
    explicit washing_cycle(InputWaterController& input_water_controller,
                           OutputWaterController& output_water_controller,
                           TemperatureSensor& temperature_sensor,
                           Heater& heater,
                           DrumRotator& drum_rotator,
                           Timer& timer,
                           unsigned washing_temperature) :
        input_water_controller{input_water_controller},
        output_water_controller{output_water_controller},
        temperature_sensor{temperature_sensor},
        heater{heater},
        drum_rotator{drum_rotator},
        timer{timer},
        washing_temperature{washing_temperature}
    {
    }

    void handle(Event event) override
    {
        auto new_state{std::visit(overload{
                                      [&](error) -> State { return error{}; },
                                      [&](idle) { return current_state; },
                                      [&](pouring_water) { return handle_pouring_water(event); },
                                      [&](waiting_drum_gets_cool) { return handle_waiting_drum_gets_cool(event); },
                                      [&](heating) { return handle_heating(event); },
                                      [&](rotating) { return handle_rotating(event); },
                                      [&](drainage) { return handle_drainage(event); },
                                      [&](finished) { return current_state; },
                                  },
                                  current_state)};
        current_state = new_state;
    }

  protected:
    State on_start() override
    {
        input_water_controller.start_pour_water();
        return pouring_water{};
    }

  private:
    State handle_pouring_water(Event event)
    {
        if (is<water_full>(event))
        {
            input_water_controller.stop_pour_water();
            if (is_current_water_temperature_too_cold_for_washing())
            {
                heater.start_heat_water();
                return heating{};
            } else if (is_current_water_temperature_too_hot_for_washing())
            {
                return waiting_drum_gets_cool{};
            } else
            {
                return start_slow_rotation_for_30_minutes();
            }
        } else
        {
            return current_state;
        }
    }

    State handle_waiting_drum_gets_cool(Event event)
    {
        if (is_target_temperature_reached(event))
            return start_slow_rotation_for_30_minutes();
        else
            return current_state;
    }

    State handle_heating(Event event)
    {
        if (is_target_temperature_reached(event))
        {
            heater.stop_heat_water();
            return start_slow_rotation_for_30_minutes();
        } else
            return current_state;
    }

    State handle_rotating(Event event)
    {
        if (is<timer_stopped>(event))
        {
            drum_rotator.stop_rotating();
            output_water_controller.start_drain_water();
            return drainage{};
        } else
        {
            return current_state;
        }
    }

    State handle_drainage(Event event)
    {
        if (is<water_empty>(event))
        {
            output_water_controller.stop_drain_water();
            return finished{};
        } else
        {
            return current_state;
        }
    }

    bool is_current_water_temperature_too_cold_for_washing() const
    {
        auto current_water_temperature{temperature_sensor.get_current_drum_temperature()};
        return current_water_temperature < washing_temperature;
    }

    bool is_current_water_temperature_too_hot_for_washing() const
    {
        auto current_water_temperature{temperature_sensor.get_current_drum_temperature()};
        return current_water_temperature > washing_temperature;
    }

    bool is_target_temperature_reached(const Event& event) const
    {
        if (is<reached_20_degrees>(event) && washing_temperature == 20)
            return true;
        else if (is<reached_40_degrees>(event) && washing_temperature == 40)
            return true;
        else
            return false;
    }

    State start_slow_rotation_for_30_minutes()
    {
        drum_rotator.start_rotating(rotation_speed::slow);
        timer.start(30);
        return rotating{};
    }

    InputWaterController& input_water_controller;
    OutputWaterController& output_water_controller;
    TemperatureSensor& temperature_sensor;
    Heater& heater;
    DrumRotator& drum_rotator;
    Timer& timer;
    unsigned washing_temperature;
};

#endif /* WASHING_STAGE_CPP */
