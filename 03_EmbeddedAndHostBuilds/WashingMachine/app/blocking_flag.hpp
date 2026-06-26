/**
 * @file	blocking_flag.hpp
 * @brief	Implements a blocking flag which can be set and thread hangs on waiting for it to be set
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef BLOCKING_FLAG_HPP
#define BLOCKING_FLAG_HPP

#include <condition_variable>
#include <mutex>

class blocking_flag
{
  public:
    void set()
    {
        std::lock_guard g{mux};
        flag = true;
        cv.notify_all();
    }

    void wait()
    {
        std::unique_lock lk{mux};
        cv.wait(lk, [this]() { return flag; });
    }

  private:
    std::mutex mux;
    std::condition_variable cv;
    bool flag;
};

#endif /* BLOCKING_FLAG_HPP */
