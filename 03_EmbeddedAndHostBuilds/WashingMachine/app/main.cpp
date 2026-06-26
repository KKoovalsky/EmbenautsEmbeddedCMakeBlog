#include <active_impl.hpp>
#include <chrono>
#include <cxxopts.hpp>
#include <iostream>
#include <memory>
#include <native/message_pump.hpp>
#include <ratio>
#include <stdexcept>
#include <thread>

#include "blocking_flag.hpp"
#include "job_delayer_impl.hpp"
#include "job_repeater_impl.hpp"
#include "logger.hpp"
#include "washer.hpp"

template<typename Message>
using active = jungles::active_generic_impl<Message, std::thread, jungles::message_pump>;

static std::pair<int, bool> validate_and_parse_input_arguments(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    auto [washing_temperature, drying_enabled] = validate_and_parse_input_arguments(argc, argv);

    std::cout << "Starting with washing temperature: " << washing_temperature << " and drying "
              << (drying_enabled ? "enabled" : "disabled") << std::endl;

    logger log;
    washer<active, job_repeater_impl, job_delayer_impl, blocking_flag, logger> w{
        washing_temperature, drying_enabled, log};

    return 0;
}

static std::pair<int, bool> validate_and_parse_input_arguments(int argc, char* argv[])
{
    cxxopts::Options options("WashingMachine", "Simple washer simulation");
    options.add_options()                                                                                         //
        ("w,washing_temperature", "Temperature of the washing cycle", cxxopts::value<int>()->default_value("20")) //
        ("d,drying", "Enable drying", cxxopts::value<bool>()->default_value("false"));
    auto parsed_options{options.parse(argc, argv)};

    auto washing_temperature{parsed_options["w"].as<int>()};
    auto drying_enabled{parsed_options["d"].as<bool>()};

    if (washing_temperature != 20 && washing_temperature != 40)
    {
        throw std::runtime_error("Wrong washing temperature specified. 20 or 40 degrees options are only possible.");
    }

    return {washing_temperature, drying_enabled};
}

