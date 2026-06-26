/**
 * @file overload.hpp
 * @author Kacper Kowalski (kacper.s.kowalski@gmail.com)
 * @brief Implements overload pattern.
 * @date 2020-05-06
 */
#ifndef __OVERLOAD_HPP__
#define __OVERLOAD_HPP__

template<class... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
overload(Ts...) -> overload<Ts...>;

#endif // __OVERLOAD_HPP__
