/**
 * @file	test_cycles.cpp
 * @brief	Test whether cycles are handled properly.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */

#include <catch2/catch.hpp>
#include <memory>
#include <variant>

#include "cycle.hpp"
#include "process.hpp"

template<typename Finisher>
struct mock_cycle : public cycle_template_method<>
{
  public:
    void handle(Event event) override
    {
        auto is_finisher{[](auto event) {
            return std::holds_alternative<Finisher>(event);
        }};

        if (is_finisher(event))
            current_state = finished{};
    }

    bool is_started() const
    {
        return is_started_flag;
    }

  protected:
    State on_start() override
    {
        is_started_flag = true;
        return idle{};
    }

  private:
    bool is_started_flag{false};
};

TEST_CASE("Cycles are handled correctly", "[cycles]")
{
    using event_finishing_first_cycle = water_full;
    using first_cycle_mock = mock_cycle<event_finishing_first_cycle>;

    using event_finishing_second_cycle = reached_20_degrees;
    using second_cycle_mock = mock_cycle<event_finishing_second_cycle>;

    auto first_cycle{std::make_shared<first_cycle_mock>()};
    auto second_cycle{std::make_shared<second_cycle_mock>()};

    SECTION("Next cycle is started after one is finished")
    {
        process p{first_cycle, second_cycle};
        p.handle(event_finishing_first_cycle{});
        REQUIRE(second_cycle->is_started());
    }

    SECTION("Whole process is finished after all cycles are finished")
    {
        process p{first_cycle, second_cycle};
        p.handle(event_finishing_first_cycle{});
        p.handle(event_finishing_second_cycle{});
        REQUIRE(p.is_finished());
    }
}
