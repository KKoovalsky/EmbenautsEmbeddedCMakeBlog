/**
 * @file	spinning_cycle.hpp
 * @brief	Defines the spinning cycle
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef SPINNING_CYCLE_HPP
#define SPINNING_CYCLE_HPP

#include <variant>

#include "common.hpp"
#include "cycle.hpp"
#include "states.hpp"

template<typename DrumRotator, typename Timer>
class spinning_cycle : public cycle_template_method<rotating>
{
  public:
    spinning_cycle(DrumRotator& drum_rotator, Timer& timer) : drum_rotator{drum_rotator}, timer{timer}
    {
    }

    void handle(Event event) override
    {
        if (is<rotating>(current_state))
        {
            if (is<timer_stopped>(event))
            {
                drum_rotator.stop_rotating();
                current_state = finished{};
            }
        }
    }

  protected:
    State on_start() override
    {
        drum_rotator.start_rotating(rotation_speed::fast);
        timer.start(10);
        return rotating{};
    }

  private:
    DrumRotator& drum_rotator;
    Timer& timer;
};

#endif /* SPINNING_CYCLE_HPP */
