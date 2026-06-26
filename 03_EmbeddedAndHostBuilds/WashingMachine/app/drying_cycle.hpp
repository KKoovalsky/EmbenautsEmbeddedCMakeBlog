/**
 * @file	drying_cycle.hpp
 * @brief	Defines the drying cycle
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef DRYING_CYCLE_HPP
#define DRYING_CYCLE_HPP

#include <variant>

#include "common.hpp"
#include "cycle.hpp"
#include "overload.hpp"
#include "states.hpp"

template<typename TemperatureSensor, typename Heater, typename DrumRotator, typename Timer>
class drying_cycle : public cycle_template_method<heating, waiting_drum_gets_cool, rotating>
{
  public:
    drying_cycle(TemperatureSensor& temperature_sensor, Heater& heater, DrumRotator& drum_rotator, Timer& timer) :
        temperature_sensor{temperature_sensor}, heater{heater}, drum_rotator{drum_rotator}, timer{timer}
    {
    }

    void handle(Event event) override
    {
        auto new_state{std::visit(overload{
                                      [&](error) -> State { return error{}; },
                                      [&](idle) { return current_state; },
                                      [&](waiting_drum_gets_cool) {
                                          if (is<reached_40_degrees>(event))
                                              return start_slow_rotation_for_30_minutes_and_heat_drum();
                                          else
                                              return current_state;
                                      },
                                      [&](heating) {
                                          if (is<reached_40_degrees>(event))
                                              return start_slow_rotation_for_30_minutes();
                                          else
                                              return current_state;
                                      },
                                      [&](rotating) -> State {
                                          if (is<reached_41_degrees>(event))
                                              heater.stop_heat_water();
                                          else if (is<reached_40_degrees>(event))
                                              heater.start_heat_water();
                                          else if (is<timer_stopped>(event))
                                          {
                                              heater.stop_heat_water();
                                              drum_rotator.stop_rotating();
                                              return finished{};
                                          }
                                          return current_state;
                                      },
                                      [&](finished) { return current_state; },
                                  },
                                  current_state)};
        current_state = new_state;
    }

  protected:
    State on_start() override
    {
        auto current_drum_temperature{temperature_sensor.get_current_drum_temperature()};
        if (current_drum_temperature < 40)
        {
            heater.start_heat_water();
            return heating{};
        } else if (current_drum_temperature > 40)
        {
            return waiting_drum_gets_cool{};
        } else
        {
            return start_slow_rotation_for_30_minutes_and_heat_drum();
        }
    }

  private:
    State start_slow_rotation_for_30_minutes()
    {
        drum_rotator.start_rotating(rotation_speed::slow);
        timer.start(30);
        return rotating{};
    }

    State start_slow_rotation_for_30_minutes_and_heat_drum()
    {
        heater.start_heat_water();
        return start_slow_rotation_for_30_minutes();
    }

    TemperatureSensor& temperature_sensor;
    Heater& heater;
    DrumRotator& drum_rotator;
    Timer& timer;
};

#endif /* DRYING_CYCLE_HPP */
