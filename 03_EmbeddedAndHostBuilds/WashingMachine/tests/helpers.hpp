/**
 * @file	helpers.hpp
 * @brief	Test helpers definition
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <condition_variable>
#include <mutex>

namespace test_helpers
{

struct flag
{
    std::mutex mux;
    std::condition_variable cv;
    bool finished{false};

    void set()
    {
        {
            std::lock_guard g{mux};
            finished = true;
        }
        cv.notify_all();
    }

    void wait()
    {
        std::unique_lock lk{mux};
        cv.wait(lk, [&] { return finished; });
    }

    bool is_set() const
    {
        return finished;
    }
};

} // namespace test_helpers

#endif /* HELPERS_HPP */
