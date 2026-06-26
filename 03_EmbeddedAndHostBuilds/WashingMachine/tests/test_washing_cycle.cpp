/**
 * @file	test_washing_cycle.cpp
 * @brief	Groups tests of the washing stage
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include <catch2/catch.hpp>

#include "fixtures.hpp"
#include "mocks.hpp"
#include "washing_cycle.hpp"

TEST_CASE("Washing cycle is performed", "[washing_cycle]")
{
    auto components_fixture{washer_components_fixture::create()};
    auto& [input_valve, output_valve, thermometer, heater, motor, timer] = components_fixture;

    SECTION("Water is poored after start")
    {
        washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, 20};

        auto was_water_not_started_to_be_poured_before_washing_started{!input_valve.is_open()};
        w.start();

        REQUIRE(was_water_not_started_to_be_poured_before_washing_started);
        REQUIRE(input_valve.is_open());
    }

    SECTION("Water is stopped being poored when full level is achieved")
    {
        washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, 20};

        w.start();
        auto was_water_poured_before_water_full{input_valve.is_open()};
        w.handle(water_full{});

        REQUIRE(was_water_poured_before_water_full);
        REQUIRE(!input_valve.is_open());
    }

    SECTION("Water is heated, after being poored if it's too cold")
    {
        washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, 20};

        thermometer.current_water_temperature = 19;
        w.start();
        auto water_was_not_heated_before_not_poured_to_full_level{!heater.is_heating()};
        w.handle(water_full{});

        REQUIRE(water_was_not_heated_before_not_poured_to_full_level);
        REQUIRE(heater.is_heating());
    }

    SECTION("Water is stopped being heated and rotating phase starts")
    {
        SECTION("When set to be 20 deg. Celsius")
        {
            unsigned washing_temperature{20};
            washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, washing_temperature};

            thermometer.current_water_temperature = 19;
            w.start();
            w.handle(water_full{});

            REQUIRE_THAT(components_fixture.state(),
                         HaveState({heater_state::on, motor_state::off, timer_state::stopped()}));

            w.handle(reached_20_degrees{});

            REQUIRE_THAT(components_fixture.state(),
                         HaveState({heater_state::off, motor_state::slow_spin, timer_state::started_with_timeout(30)}));
        }

        SECTION("When set to be 40 deg. Celsius and poored water is below 20 degrees")
        {
            unsigned washing_temperature{40};
            washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, washing_temperature};

            thermometer.current_water_temperature = 19;
            w.start();
            w.handle(water_full{});
            w.handle(reached_20_degrees{});

            REQUIRE_THAT(components_fixture.state(),
                         HaveState({heater_state::on, motor_state::off, timer_state::stopped()}));

            w.handle(reached_40_degrees{});

            REQUIRE_THAT(components_fixture.state(),
                         HaveState({heater_state::off, motor_state::slow_spin, timer_state::started_with_timeout(30)}));
        }

        SECTION("When set to be 40 deg. Celsius and poored water is between 20 and 40 degrees")
        {
            unsigned washing_temperature{40};
            washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, washing_temperature};

            thermometer.current_water_temperature = 21;
            w.start();
            w.handle(water_full{});

            REQUIRE_THAT(components_fixture.state(),
                         HaveState({heater_state::on, motor_state::off, timer_state::stopped()}));

            w.handle(reached_40_degrees{});

            REQUIRE_THAT(components_fixture.state(),
                         HaveState({heater_state::off, motor_state::slow_spin, timer_state::started_with_timeout(30)}));
        }
    }

    SECTION("Machine waits for the water to get colder if it's too hot and then starts rotation")
    {

        SECTION("When set to be 20 deg. Celsius and the temperature is between 20 degress and 40 degrees")
        {
            unsigned washing_temperature{20};
            washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, washing_temperature};

            thermometer.current_water_temperature = 21;
            w.start();
            w.handle(water_full{});

            REQUIRE_THAT(components_fixture.state(),
                         HaveState({heater_state::off, motor_state::off, timer_state::stopped()}));

            w.handle(reached_20_degrees{});

            REQUIRE_THAT(components_fixture.state(),
                         HaveState({motor_state::slow_spin, timer_state::started_with_timeout(30)}));
        }

        SECTION("When set to be 20 deg. Celsius and the temperature is above 40 degrees")
        {
            unsigned washing_temperature{20};
            washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, washing_temperature};

            thermometer.current_water_temperature = 42;
            w.start();
            w.handle(water_full{});
            w.handle(reached_40_degrees{});

            REQUIRE_THAT(components_fixture.state(),
                         HaveState({heater_state::off, motor_state::off, timer_state::stopped()}));

            w.handle(reached_20_degrees{});

            REQUIRE_THAT(components_fixture.state(),
                         HaveState({motor_state::slow_spin, timer_state::started_with_timeout(30)}));
        }
    }

    SECTION("Machine starts slow rotation immediately after it is poored when the water temperature is good")
    {
        unsigned washing_temperature{40};
        washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, washing_temperature};

        thermometer.current_water_temperature = 40;
        w.start();
        w.handle(water_full{});

        REQUIRE_THAT(components_fixture.state(),
                     HaveState({motor_state::slow_spin, timer_state::started_with_timeout(30)}));
    }

    SECTION("Motor is stopped and drainage is started after 30 minutes from start of rotation")
    {
        unsigned washing_temperature{40};
        washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, washing_temperature};

        thermometer.current_water_temperature = 40;
        w.start();
        w.handle(water_full{});
        w.handle(timer_stopped{});

        REQUIRE_THAT(components_fixture.state(), HaveState({motor_state::off, output_valve_state::open()}));
    }

    SECTION("Water drainage is stopped after no water is in the drum")
    {
        unsigned washing_temperature{40};

        washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, washing_temperature};

        thermometer.current_water_temperature = 40;
        w.start();
        w.handle(water_full{});
        w.handle(timer_stopped{});
        w.handle(water_empty{});

        REQUIRE_THAT(components_fixture.state(), HaveState({output_valve_state::closed()}));
    }

    SECTION("Washing stage is finished after water drainage is finished")
    {
        unsigned washing_temperature{40};
        washing_cycle w{input_valve, output_valve, thermometer, heater, motor, timer, washing_temperature};

        thermometer.current_water_temperature = 40;
        w.start();
        w.handle(water_full{});
        w.handle(timer_stopped{});

        REQUIRE(!w.is_finished());

        w.handle(water_empty{});

        REQUIRE(w.is_finished());
    }
}

