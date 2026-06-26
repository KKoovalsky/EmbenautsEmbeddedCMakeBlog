/**
 * @file	test_spinning_cycle.cpp
 * @brief	Spinning cycle tests.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include <catch2/catch.hpp>

#include "spinning_cycle.hpp"

#include "mocks.hpp"
#include "fixtures.hpp"

TEST_CASE("Spinning cycle is performed")
{
    auto components_fixture{washer_components_fixture::create()};
    auto &[input_valve, output_valve, thermometer, heater, motor, timer] = components_fixture;

    SECTION("Motor is running fast after start and timer is started to timeout in 10 minutes")
    {
        spinning_cycle s{motor, timer};

        REQUIRE_THAT(components_fixture.state(), HaveState({motor_state::off, timer_state::stopped()}));

        s.start();

        REQUIRE_THAT(components_fixture.state(),
                     HaveState({motor_state::fast_spin, timer_state::started_with_timeout(10)}));
    }

    SECTION("Motor stops after timer has timeout")
    {
        spinning_cycle s{motor, timer};
        s.start();
        s.handle(timer_stopped{});

        REQUIRE_THAT(components_fixture.state(), HaveState({motor_state::off}));
    }

    SECTION("Spinning is finished after timer has timeout after rotating")
    {
        spinning_cycle s{motor, timer};
        s.start();
        s.handle(timer_stopped{});

        REQUIRE(s.is_finished());
    }
}
