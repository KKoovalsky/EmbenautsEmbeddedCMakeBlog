/**
 * @file	process.hpp
 * @brief	High level process definition which handles cycles.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <vector>

#include "cycle.hpp"
#include "events.hpp"

/**
 * \brief Executes cycles one after another
 * \todo Might be also a cycle (composite pattern might be used) - but can't see benefits at this stage.
 *       It could because both this class and cycle have handle(Event) and is_finished() methods.
 */
class process
{
  public:
    explicit process(std::vector<std::shared_ptr<cycle>> cycles) : cycles{std::move(cycles)}
    {
        start();
    }

    explicit process(std::initializer_list<std::shared_ptr<cycle>> cycles) : cycles{cycles}
    {
        start();
    }

    void handle(Event event)
    {
        if (is_finished())
            throw std::runtime_error("Can't handle event when process is finished");

        auto start_next_cycle{[&]() {
            auto& next_cycle{cycles[cycle_index + 1]};
            next_cycle->start();
        }};

        auto& current_cycle{cycles[cycle_index]};
        current_cycle->handle(std::move(event));
        if (current_cycle->is_finished())
        {
            if (is_not_last_cycle())
            {
                start_next_cycle();
            }
            ++cycle_index;
        }
    }

    bool is_finished() const
    {
        return cycle_index >= cycles.size();
    }

  private:
    void start()
    {
        if (is_finished())
            return;

        this->cycles[cycle_index]->start();
    }

    bool is_not_last_cycle() const
    {
        return cycle_index != cycles.size() - 1;
    }

    std::vector<std::shared_ptr<cycle>> cycles;
    unsigned cycle_index{0};
};

#endif /* PROCESS_HPP */
