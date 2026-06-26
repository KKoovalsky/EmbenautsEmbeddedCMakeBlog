/**
 * @file	states.hpp
 * @brief	List of possible states.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef STATES_HPP
#define STATES_HPP

struct error
{
};

struct idle
{
};

struct pouring_water
{
};

struct waiting_drum_gets_cool
{
};

struct heating
{
};

struct rotating
{
};

struct drainage
{
};

struct finished
{
};

#endif /* STATES_HPP */
