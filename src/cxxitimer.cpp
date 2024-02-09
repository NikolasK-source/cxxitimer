/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "cxxitimer.hpp"

#include <cmath>
#include <iostream>
#include <sysexits.h>


namespace cxxitimer {

//* timeval to stop timer
static constexpr itimerval STOP_TIMER = {{0, 0}, {0, 0}};

//* number of usec per second
constexpr auto USEC_PER_SEC = static_cast<double>(1000000);


bool ITimer_Real::instance_exists    = false;
bool ITimer_Virtual::instance_exists = false;
bool ITimer_Prof::instance_exists    = false;

ITimer::ITimer(int type, const timeval &interval) noexcept
    : timer_value(interval),
      timer_interval(interval),
      type(type),
      speed_factor(1.0),  // normal speed
      running(false)      // not running
{}

ITimer::ITimer(int type, double interval) noexcept
    : timer_value(double_to_timeval(interval)),
      timer_interval(double_to_timeval(interval)),
      type(type),
      speed_factor(1.0),  // normal speed
      running(false)      // not running
{}

ITimer::ITimer(int type, const timeval &interval, const timeval &value) noexcept
    : timer_value(value),
      timer_interval(interval),
      type(type),
      speed_factor(1.0),  // normal speed
      running(false)      // not running
{}

ITimer::ITimer(int type, double interval, double value) noexcept
    : timer_value(double_to_timeval(value)),
      timer_interval(double_to_timeval(interval)),
      type(type),
      speed_factor(1.0),  // normal speed
      running(false)      // not running
{}


ITimer::~ITimer() {
    // stop timer if running
    if (running) {
        try {
            stop();
        } catch (const std::exception &e) {
            std::cerr << "Exception in destructor (" << __PRETTY_FUNCTION__ << "): " << e.what() << std::endl;
            exit(EX_SOFTWARE);
        }
    }
}

void ITimer::adjust_speed(double new_factor) {
    if (!running) throw std::runtime_error("timer not running");

    itimerval val {};
    int       tmp = setitimer(type, &STOP_TIMER, &val);
    if (tmp) throw std::system_error(errno, std::generic_category(), "call of setitimer failed");

    // set timer interval
    val.it_interval = timer_interval / new_factor;

    // scale timer value
    val.it_value *= speed_factor / new_factor;

    // set new timer value
    tmp = setitimer(type, &val, nullptr);
    if (tmp) throw std::system_error(errno, std::generic_category(), "call of setitimer failed");

    // save speed factor
    speed_factor = new_factor;
}

void ITimer::start() {
    if (running) throw std::logic_error("timer already started");

    // create scaled timer value
    itimerval timer_val {timer_interval / speed_factor, timer_value / speed_factor};

    if (timer_val.it_interval.tv_sec < 0) throw std::runtime_error("timer interval is negative");

    if (timer_val.it_value.tv_sec < 0) throw std::runtime_error("timer value is negative");

    if (timer_val.it_interval.tv_sec == 0 && timer_val.it_interval.tv_usec == 0)
        throw std::runtime_error("invalid timer values due to to a to small speed factor");

    // start timer;
    int tmp = setitimer(type, &timer_val, nullptr);
    if (tmp < 0) throw std::system_error(errno, std::generic_category(), "call of setitimer failed");

    running = true;
}

void ITimer::stop() {
    if (!running) throw std::runtime_error("timer already stopped");

    // stop timer and save value
    itimerval timer_val {};
    int       tmp = setitimer(type, &STOP_TIMER, &timer_val);
    if (tmp < 0) throw std::system_error(errno, std::generic_category(), "call of setitimer failed");

    // normalize value
    timer_value = timer_val.it_value * speed_factor;

    running = false;
}

void ITimer::set_speed_factor(double factor) {
    // check speed_factor
    if (factor <= 0.0) throw std::invalid_argument("negative values not allowed");

    if (std::isnan(factor) || std::isinf(factor)) throw std::invalid_argument("invalid double value");

    if (running) adjust_speed(factor);
    else
        speed_factor = factor;
}

void ITimer::set_interval_value(const timeval &interval, const timeval &value) {
    if (running) throw std::logic_error("cannot set interval/value if timer is running");

    this->timer_interval = interval;
    this->timer_value    = value;
}

void ITimer::set_interval_value(double interval, double value) {
    set_interval_value(double_to_timeval(interval), double_to_timeval(value));
}

void ITimer::set_speed_to_normal() {
    // adjust speed if running
    if (running) adjust_speed(1.0);
    else
        speed_factor = 1.0;
}

void ITimer::to_fstream(std::ofstream &fstream) const {
    itimerval val {};
    if (running) {
        int tmp = getitimer(type, &val);
        if (tmp < 0) throw std::system_error(errno, std::generic_category(), "call of getitimer failed");

        val.it_value *= speed_factor;
    } else {
        val.it_value = timer_value;
    }

    val.it_interval = timer_interval;

    fstream.write(reinterpret_cast<char *>(&val), sizeof(val));
}

void ITimer::from_fstream(std::ifstream &fstream) {
    if (running) throw std::logic_error("timer is running");

    itimerval val {};
    fstream.read(reinterpret_cast<char *>(&val), sizeof(val));
    timer_interval = val.it_interval;
    timer_value    = val.it_value;
}

timeval ITimer::get_timer_value() const {
    if (running) {
        itimerval temp {};
        int       tmp = getitimer(type, &temp);
        if (tmp < 0) throw std::system_error(errno, std::generic_category(), "call of getitimer failed");
        return temp.it_value;
    } else
        return timer_value;
}

ITimer_Real::ITimer_Real(const timeval &interval) : ITimer(ITIMER_REAL, interval) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Real::ITimer_Real(const timeval &interval, const timeval &value) : ITimer(ITIMER_REAL, interval, value) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Real::ITimer_Real(double interval) : ITimer(ITIMER_REAL, interval) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Real::ITimer_Real(double interval, double value) : ITimer(ITIMER_REAL, interval, value) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Real::~ITimer_Real() {
    // allow new instance
    instance_exists = false;
}

ITimer_Virtual::ITimer_Virtual(const timeval &interval) : ITimer(ITIMER_VIRTUAL, interval) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Virtual::ITimer_Virtual(const timeval &interval, const timeval &value)
    : ITimer(ITIMER_VIRTUAL, interval, value) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Virtual::ITimer_Virtual(double interval) : ITimer(ITIMER_VIRTUAL, interval) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Virtual::ITimer_Virtual(double interval, double value) : ITimer(ITIMER_VIRTUAL, interval, value) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Virtual::~ITimer_Virtual() {
    // allow new instance
    instance_exists = false;
}

ITimer_Prof::ITimer_Prof(const timeval &interval) : ITimer(ITIMER_PROF, interval) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Prof::ITimer_Prof(const timeval &interval, const timeval &value) : ITimer(ITIMER_PROF, interval, value) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Prof::ITimer_Prof(double interval) : ITimer(ITIMER_PROF, interval) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Prof::ITimer_Prof(double interval, double value) : ITimer(ITIMER_PROF, interval, value) {
    // prevent multiple instances
    if (instance_exists) throw std::logic_error("instance exists");
    instance_exists = true;
}

ITimer_Prof::~ITimer_Prof() {
    // allow new instance
    instance_exists = false;
}

timeval &operator*=(timeval &left, double right) noexcept {
    double timer_value = timeval_to_double(left) * right;
    left               = double_to_timeval(timer_value);
    return left;
}

itimerval &operator*=(itimerval &left, double right) noexcept {
    left.it_interval *= right;
    left.it_value *= right;
    return left;
}

timeval operator*(const timeval &left, double right) noexcept {
    auto ret_val = left;
    ret_val *= right;
    return ret_val;
}

itimerval operator*(const itimerval &left, double right) noexcept {
    auto ret_val = left;
    ret_val *= right;
    return ret_val;
}

timeval &operator/=(timeval &left, double right) noexcept {
    double timer_value = timeval_to_double(left) / right;
    left               = double_to_timeval(timer_value);
    return left;
}

itimerval &operator/=(itimerval &left, double right) noexcept {
    left.it_interval /= right;
    left.it_value /= right;
    return left;
}

timeval operator/(const timeval &left, double right) noexcept {
    auto ret_val = left;
    ret_val /= right;
    return ret_val;
}

itimerval operator/(const itimerval &left, double right) noexcept {
    auto ret_val = left;
    ret_val /= right;
    return ret_val;
}

double timeval_to_double(const timeval &time) noexcept {
    double ret_val = static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_usec) / USEC_PER_SEC;
    return ret_val;
}

timeval double_to_timeval(const double time) noexcept {
    timeval ret_val {static_cast<time_t>(time), static_cast<suseconds_t>(fmod(time, 1.0) * USEC_PER_SEC)};
    return ret_val;
}

}  // namespace cxxitimer
