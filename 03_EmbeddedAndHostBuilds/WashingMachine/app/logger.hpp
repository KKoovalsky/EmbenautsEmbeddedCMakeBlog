/**
 * @file	logger.hpp
 * @brief	A simple logger for native platform.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <mutex>

class logger
{
  public:
    template<typename T>
    logger& operator<<(T&& t)
    {
        std::lock_guard g{cout_mux};
        std::cout << std::forward<T>(t);
        return *this;
    }

  private:
    static inline std::mutex cout_mux;
};

#endif /* LOGGER_HPP */
