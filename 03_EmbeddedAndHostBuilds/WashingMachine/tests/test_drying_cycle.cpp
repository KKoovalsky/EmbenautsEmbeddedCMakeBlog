/**
 * @file	test_drying_cycle.cpp
 * @brief	Drying cycle is tested.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include <catch2/catch.hpp>

#include "drying_cycle.hpp"

#include "fixtures.hpp"
#include "mocks.hpp"

TEST_CASE("Drying cycle is performed", "[drying_cycle]")
{
    auto components_fixture{washer_components_fixture::create()};
    auto& [input_valve, output_valve, thermometer, heater, motor, timer] = components_fixture;

    SECTION("Drum is heated after start if poured water is below 40 degrees")
    {
        drying_cycle d{thermometer, heater, motor, timer};

        thermometer.current_water_temperature = 25;

        REQUIRE_THAT(components_fixture.state(),
                     HaveState({heater_state::off, motor_state::off, timer_state::stopped()}));

        d.start();

        REQUIRE_THAT(components_fixture.state(),
                     HaveState({heater_state::on, motor_state::off, timer_state::stopped()}));
    }

    SECTION("Heater is not turned on when poured water is too warm, after start")
    {
        drying_cycle d{thermometer, heater, motor, timer};

        thermometer.current_water_temperature = 42;

        d.start();

        REQUIRE_THAT(components_fixture.state(),
                     HaveState({heater_state::off, motor_state::off, timer_state::stopped()}));
    }

    SECTION("It is waited for the water to get colder if it's too warm after pouring, and then rotation starts")
    {
        drying_cycle d{thermometer, heater, motor, timer};

        thermometer.current_water_temperature = 42;

        d.start();
        d.handle(reached_40_degrees{});

        REQUIRE_THAT(components_fixture.state(),
                     HaveState({heater_state::on, motor_state::slow_spin, timer_state::started_with_timeout(30)}));
    }

    SECTION("Rotation starts after drum reaches target temperature when the water was too cold after pouring")
    {
        drying_cycle d{thermometer, heater, motor, timer};

        thermometer.current_water_temperature = 25;

        d.start();
        d.handle(reached_40_degrees{});

        REQUIRE_THAT(components_fixture.state(),
                     HaveState({heater_state::on, motor_state::slow_spin, timer_state::started_with_timeout(30)}));
    }

    SECTION("Rotation starts just after start when the poured water is equal to 40 degrees and heater is started")
    {
        drying_cycle d{thermometer, heater, motor, timer};

        thermometer.current_water_temperature = 40;

        d.start();

        REQUIRE_THAT(components_fixture.state(),
                     HaveState({heater_state::on, motor_state::slow_spin, timer_state::started_with_timeout(30)}));
    }

    SECTION("When temperature goes above 41 degrees while rotating then the heater is turned off")
    {
        drying_cycle d{thermometer, heater, motor, timer};

        thermometer.current_water_temperature = 25;

        d.start();
        d.handle(reached_40_degrees{});
        d.handle(reached_41_degrees{});

        REQUIRE_THAT(components_fixture.state(),
                     HaveState({heater_state::off, motor_state::slow_spin, timer_state::started_with_timeout(30)}));
    }

    SECTION("When temperature goes below 40 degrees while rotating then the heater is turned on")
    {
        drying_cycle d{thermometer, heater, motor, timer};

        thermometer.current_water_temperature = 25;

        d.start();
        d.handle(reached_40_degrees{});
        d.handle(reached_41_degrees{});
        d.handle(reached_40_degrees{});

        REQUIRE_THAT(components_fixture.state(),
                     HaveState({heater_state::on, motor_state::slow_spin, timer_state::started_with_timeout(30)}));
    }

    SECTION("Heater is turned off and rotation stops after timer has timeout, and heater was turned on")
    {
        drying_cycle d{thermometer, heater, motor, timer};

        thermometer.current_water_temperature = 25;

        d.start();
        d.handle(reached_40_degrees{});
        d.handle(reached_41_degrees{});
        d.handle(reached_40_degrees{});
        d.handle(timer_stopped{});

        REQUIRE_THAT(components_fixture.state(), HaveState({heater_state::off, motor_state::off}));
    }

    SECTION("Heater is turned off and rotation stops when timer timeouts and the heater was not heating")
    {
        drying_cycle d{thermometer, heater, motor, timer};

        thermometer.current_water_temperature = 25;

        d.start();
        d.handle(reached_40_degrees{});
        d.handle(reached_41_degrees{});
        d.handle(timer_stopped{});

        REQUIRE_THAT(components_fixture.state(), HaveState({heater_state::off, motor_state::off}));
    }

    SECTION("Drying is finished after timer timeouts")
    {
        drying_cycle d{thermometer, heater, motor, timer};

        thermometer.current_water_temperature = 25;

        d.start();
        d.handle(reached_40_degrees{});
        d.handle(reached_41_degrees{});
        d.handle(timer_stopped{});

        REQUIRE(d.is_finished());
    }
}

