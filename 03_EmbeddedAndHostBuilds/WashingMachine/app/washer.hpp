/**
 * @file	washer.hpp
 * @brief	Washer machine main class definition.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef WASHER_HPP
#define WASHER_HPP

#include <memory>

#include "drivers.hpp"
#include "drying_cycle.hpp"
#include "process.hpp"
#include "spinning_cycle.hpp"
#include "washing_cycle.hpp"

#include "temperature_change_simulation.hpp"
#include "water_flow_simulation.hpp"

template<template<typename> typename Active,
         typename JobRepeater,
         typename JobDelayer,
         typename BlockingFlag,
         typename Logger>
class washer
{
  public:
    washer(int washing_temperature, bool drying_enabled, Logger& logger) :
        input_valve{[&]() { input_valve_opener(); },
                    [&]() {
                        input_valve_closer();
                    }},
        output_valve{[&]() { output_valve_opener(); },
                     [&]() {
                         output_valve_closer();
                     }},
        thermometer{[&]() {
            return temperature_getter();
        }},
        heater{[&]() { water_heating_starter(); },
               [&]() {
                   water_heating_stopper();
               }},
        motor{[&](auto speed) { rotation_starter(speed); },
              [&]() {
                  rotation_stopper();
              }},
        timer{[&](auto timeout) {
            timer_starter(timeout);
        }},
        worker{[&](auto&& message) {
            process_executor(std::move(message));
        }},
        logger{logger},
        temperature_change{25, 18, heater_state::off, 1, 2, std::make_shared<JobRepeater>()},
        water_flow{2000,
                   1000,
                   std::make_shared<JobDelayer>(),
                   [&](auto level) {
                       on_water_level_reached(level);
                   }},
        washing_process{make_cycles(washing_temperature, drying_enabled)}
    {
        temperature_change.set_callback_on_temperature_values_reached([&](int t) { on_temperature_value_reached(t); },
                                                                      {20, 40, 41});
    }

    ~washer()
    {
        process_finished_flag.wait();
    }

  private:
    void input_valve_opener()
    {
        logger << "Input valve: opened\n";
        water_flow.input_valve_opened();
    }

    void input_valve_closer()
    {
        logger << "Input valve: closed\n";
        water_flow.input_valve_closed();
    }

    void output_valve_opener()
    {
        logger << "Output valve: opened\n";
        water_flow.output_valve_opened();
    }

    void output_valve_closer()
    {
        logger << "Output valve: closed\n";
        water_flow.output_valve_closed();
    }

    int temperature_getter()
    {
        return temperature_change.get_current_temperature();
    }

    void water_heating_starter()
    {
        logger << "Heater: enabled\n";
        temperature_change.heater_state_changed(heater_state::on);
    }

    void water_heating_stopper()
    {
        logger << "Heater: disabled\n";
        temperature_change.heater_state_changed(heater_state::off);
    }

    void rotation_starter(rotation_speed speed)
    {
        if (speed == rotation_speed::slow)
            logger << "Motor: Starting slow rotation\n";
        else if (speed == rotation_speed::fast)
            logger << "Motor: Starting fast rotation\n";
    }

    void rotation_stopper()
    {
        logger << "Motor: Stopped rotating\n";
    }

    void timer_starter(unsigned timeout_seconds)
    {
        static JobDelayer job_delayer;
        unsigned timeout_milliseconds{timeout_seconds * 1000};
        job_delayer.delay_job(timeout_milliseconds, [&]() { worker.send(timer_stopped{}); });
    }

    void on_temperature_value_reached(int temperature)
    {
        logger << "Reached temperature: " << temperature << " degrees\n";
        if (temperature == 20)
            worker.send(reached_20_degrees{});
        else if (temperature == 40)
            worker.send(reached_40_degrees{});
        else if (temperature == 41)
            worker.send(reached_41_degrees{});
    }

    void on_water_level_reached(water_level level)
    {
        logger << "Water reached level: " << (level == water_level::full ? "full" : "empty") << '\n';
        if (level == water_level::full)
            worker.send(water_full{});
        else if (level == water_level::empty)
            worker.send(water_empty{});
    }

    std::vector<std::shared_ptr<cycle>> make_cycles(int washing_temperature, bool drying_enabled)
    {
        using washing = washing_cycle<input_valve_driver,
                                      output_valve_driver,
                                      thermometer_driver,
                                      heater_driver,
                                      motor_driver,
                                      timer_driver>;
        using rinsing = washing;
        using spinning = spinning_cycle<motor_driver, timer_driver>;
        using drying = drying_cycle<thermometer_driver, heater_driver, motor_driver, timer_driver>;

        std::vector<std::shared_ptr<cycle>> cycles;

        auto wash{std::make_shared<washing>(
            input_valve, output_valve, thermometer, heater, motor, timer, washing_temperature)};
        auto rinse{std::make_shared<rinsing>(input_valve, output_valve, thermometer, heater, motor, timer, 20)};
        auto spin{std::make_shared<spinning>(motor, timer)};

        cycles.emplace_back(std::move(wash));
        cycles.emplace_back(std::move(rinse));
        cycles.emplace_back(std::move(spin));

        if (drying_enabled)
        {
            auto dry{std::make_shared<drying>(thermometer, heater, motor, timer)};
            cycles.emplace_back(std::move(dry));
        }

        return cycles;
    }

    void process_executor(Event event)
    {
        if (!washing_process.is_finished())
        {
            washing_process.handle(event);
            if (washing_process.is_finished())
                process_finished_flag.set();
        }
    }

    input_valve_driver input_valve;
    output_valve_driver output_valve;
    thermometer_driver thermometer;
    heater_driver heater;
    motor_driver motor;
    timer_driver timer;

    BlockingFlag process_finished_flag;
    Active<Event> worker;
    Logger& logger;

    temperature_change_simulation temperature_change;
    water_flow_simulation water_flow;

    process washing_process;
};

#endif /* WASHER_HPP */
