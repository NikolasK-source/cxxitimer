/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#pragma once

#include <fstream>
#include <sys/time.h>

namespace cxxitimer {

/**
 * @brief abstract class ITimer
 */
class ITimer {
private:
    //* timer value (speed factor 1.0)
    timeval timer_value;

    //* timer interval (speed factor 1.0)
    timeval timer_interval;

    //* timer type (REAL/VIRTUAL/PROF see man getitimer)
    int type;

    /**
     * @brief speed adjustment factor
     * @details
     *   - ]0;1[   -->  slower
     *   - ]1;inf[ -->  faster
     *   - 1       -->  normal speed
     */
    double speed_factor;

    //* timer running indicator
    bool running;

    //* internal use only!
    virtual void adjust_speed(double new_factor);

protected:
    //* internal use only!
    explicit ITimer(int type, const timeval &interval = {1, 0}) noexcept;

    //* internal use only!
    ITimer(int type, const timeval &interval, const timeval &value) noexcept;

    //* internal use only!
    ITimer(int type, double interval) noexcept;

    //* internal use only!
    ITimer(int type, double interval, double value) noexcept;

public:
    //! copying is not possible
    ITimer(const ITimer &other) = delete;
    //! moving is not possible
    ITimer(ITimer &&other) = delete;
    //! copying is not possible
    ITimer &operator=(const ITimer &other) = delete;
    //! moving is not possible
    ITimer &operator=(ITimer &&other) = delete;

    /**
     * @brief Destroy the timer instance
     *
     * @details
     * Timer is stopped if running, however, the timer should be stopped beforehand if possible.
     * If the timer is still running and the call of stop() fails, the process will be terminated.
     */
    virtual ~ITimer();

    /**
     * @brief start timer
     * @exception std::logic_error timer already started
     * @exception std::runtime_error invalid timer values due to to a to small speed factor
     * @exception std::system_error call of setitimer failed
     */
    void start();

    /**
     * @brief stop timer
     * @exception std::logic_error timer already stopped
     * @exception std::system_error call of setitimer failed
     */
    void stop();

    /**
     * @brief set speed factor
     * @details is applied directly, even if the timer is running
     *   - ]0;1[   -->  slower
     *   - ]1;inf[ -->  faster
     *   - 1       -->  normal speed
     * @param factor speed factor
     * @exception std::invalid_argument negative values or nan/inf
     * @exception std::system_error call of setitimer failed
     */
    void set_speed_factor(double factor);

    /**
     * @brief set interval (timeval)
     * @details
     *      only allowed if the timer is stopped!
     *      set the timer value to the same time
     * @param interval timer interval
     * @exception std::logic_error timer is started
     */
    inline void set_interval(const timeval &interval) { set_interval_value(interval, interval); }

    /**
     * @brief set interval (double)
     * @details
     *      only allowed if the timer is stopped!
     *      set the timer value to the same time
     * @param interval timer interval (seconds)
     * @exception std::logic_error timer is started
     */
    inline void set_interval(double &interval) { set_interval_value(interval, interval); }

    /**
     * @brief set interval and value
     * @details only allowed if the timer is stopped
     * @param interval timer interval
     * @param value timer value
     * @exception std::logic_error timer is started
     */
    void set_interval_value(const timeval &interval, const timeval &value);

    /**
     * @brief set interval and value
     * @details only allowed if the timer is stopped
     * @param interval timer interval (seconds)
     * @param value timer value (seconds)
     * @exception std::logic_error timer is started
     */
    void set_interval_value(double interval, double value);

    /**
     * @brief set speed to normal
     * @details like calling set_speed_factor with 1.0
     * @exception std::system_error call of setitimer failed
     */
    void set_speed_to_normal();

    /**
     * @brief write to binary file stream
     * @details
     * writes interval and value to file stream.
     * type and speed factor is not stored!
     * @param fstream file stream to write to
     * @exception std::system_error call of getitimer failed
     */
    void to_fstream(std::ofstream &fstream) const;

    /**
     * @brief read from binary filestream
     * @param fstream file stream to read from
     * @exception std::logic_error timer is running
     */
    void from_fstream(std::ifstream &fstream);

    /**
     * @brief get timer value
     * @details returns the stored timer value if the timer is stopped or the actual timer value if running
     * @return timer value
     */
    [[nodiscard]] timeval get_timer_value() const;

    /**
     * @brief check if timer is running
     * @return true timer is running
     * @return true timer is stopped
     */
    [[nodiscard]] inline bool is_running() const noexcept { return running; }
};

/** @brief class ITimer_Real
 *
 * @details
 * "This  timer counts down in real (i.e., wall clock) time.
 * At each expiration, a SIGALRM signal is generated." (man getitimer)
 */
class ITimer_Real : public ITimer {
    //* only one instance per process allowed
    static bool instance_exists;

public:
    /**
     * @brief create ITimer_Real instance
     * @details only one instance is allowed
     * @param interval timer interval
     * @exception std::logic_error instance exists
     */
    explicit ITimer_Real(const timeval &interval = {1, 0});

    /**
     * @brief create ITimer_Real instance
     * @details only one instance is allowed
     * @param interval interval at which the timer is triggered
     * @param value ime period after which the timer expires for the first time
     * @exception std::logic_error instance exists
     */
    ITimer_Real(const timeval &interval, const timeval &value);

    /**
     * @brief create ITimer_Real instance
     * @details only one instance is allowed
     * @param interval timer interval
     * @exception std::logic_error instance exists
     */
    explicit ITimer_Real(double interval);

    /**
     * @brief create ITimer_Real instance
     * @details only one instance is allowed
     * @param interval interval at which the timer is triggered
     * @param value ime period after which the timer expires for the first time
     * @exception std::logic_error instance exists
     */
    ITimer_Real(double interval, double value);

    //* destroy instance
    ~ITimer_Real() override;

    //* copying is not possible
    ITimer_Real(const ITimer_Real &other) = delete;
    //* moving is not possible
    ITimer_Real(ITimer_Real &&other) = delete;
    //* copying is not possible
    ITimer_Real &operator=(const ITimer_Real &other) = delete;
    //* moving is not possible
    ITimer_Real &operator=(ITimer_Real &&other) = delete;
};

/**
 * @brief class ITimer_Virtual
 *
 * @details
 * "This timer counts down against the  user-mode  CPU  time consumed  by the process.  (The measurement includes CPU
 * time consumed by all threads in the process.) At  each expiration, a SIGVTALRM signal is generated." (man getitimer)
 */
class ITimer_Virtual : public ITimer {
    //* only one instance per process allowed
    static bool instance_exists;

public:
    /**
     * @brief create ITimer_Virtual instance
     * @details only one instance is allowed
     * @param interval timer interval
     * @exception std::logic_error instance exists
     */
    explicit ITimer_Virtual(const timeval &interval = {1, 0});

    /**
     * @brief create ITimer_Virtual instance
     * @details only one instance is allowed
     * @param interval interval at which the timer is triggered
     * @param value ime period after which the timer expires for the first time
     * @exception std::logic_error instance exists
     */
    ITimer_Virtual(const timeval &interval, const timeval &value);

    /**
     * @brief create ITimer_Virtual instance
     * @details only one instance is allowed
     * @param interval timer interval
     * @exception std::logic_error instance exists
     */
    explicit ITimer_Virtual(double interval);

    /**
     * @brief create ITimer_Virtual instance
     * @details only one instance is allowed
     * @param interval interval at which the timer is triggered
     * @param value ime period after which the timer expires for the first time
     * @exception std::logic_error instance exists
     */
    ITimer_Virtual(double interval, double value);

    //* destroy instance
    ~ITimer_Virtual() override;

    //* copying is not possible
    ITimer_Virtual(const ITimer_Virtual &other) = delete;
    //* moving is not possible
    ITimer_Virtual(ITimer_Virtual &&other) = delete;
    //* copying is not possible
    ITimer_Virtual &operator=(const ITimer_Virtual &other) = delete;
    //* moving is not possible
    ITimer_Virtual &operator=(ITimer_Virtual &&other) = delete;
};

/**
 * @brief class ITimer_Prof
 *
 * @details
 * "This  timer  counts  down  against the total (i.e., both user and system) CPU time consumed by the process. (The
 * measurement includes CPU time consumed by all threads in the process.)  At each expiration, a SIGPROF  signal is
 * generated.
 * In  conjunction with ITIMER_VIRTUAL, this timer can be used to profile user and system CPU time consumed by the
 * process." (man getitimer)
 */
class ITimer_Prof : public ITimer {
    //* only one instance per process allowed
    static bool instance_exists;

public:
    /**
     * @brief create ITimer_Prof instance
     * @details only one instance is allowed
     * @param interval timer interval
     * @exception std::logic_error instance exists
     */
    explicit ITimer_Prof(const timeval &interval = {1, 0});

    /**
     * @brief create ITimer_Prof instance
     * @details only one instance is allowed
     * @param interval interval at which the timer is triggered
     * @param value ime period after which the timer expires for the first time
     * @exception std::logic_error instance exists
     */
    ITimer_Prof(const timeval &interval, const timeval &value);

    /**
     * @brief create ITimer_Prof instance
     * @details only one instance is allowed
     * @param interval timer interval
     * @exception std::logic_error instance exists
     */
    explicit ITimer_Prof(double interval);

    /**
     * @brief create ITimer_Prof instance
     * @details only one instance is allowed
     * @param interval interval at which the timer is triggered
     * @param value ime period after which the timer expires for the first time
     * @exception std::logic_error instance exists
     */
    ITimer_Prof(double interval, double value);

    //* destroy instance
    ~ITimer_Prof() override;

    //* copying is not possible
    ITimer_Prof(const ITimer_Prof &other) = delete;
    //* moving is not possible
    ITimer_Prof(ITimer_Prof &&other) = delete;
    //* copying is not possible
    ITimer_Prof &operator=(const ITimer_Prof &other) = delete;
    //* moving is not possible
    ITimer_Prof &operator=(ITimer_Prof &&other) = delete;
};

//* multiply timeval with double factor
timeval &operator*=(timeval &left, double right) noexcept;

//* multiply each timeval of itimerval with double factor
itimerval &operator*=(itimerval &left, double right) noexcept;

//* multiply timeval with double factor
timeval operator*(const timeval &left, double right) noexcept;

//* multiply each timeval of itimerval with double factor
itimerval operator*(const itimerval &left, double right) noexcept;

//* divide timeval by double factor
timeval &operator/=(timeval &left, double right) noexcept;

//* divide each timeval of itimerval by double factor
itimerval &operator/=(itimerval &left, double right) noexcept;

//* divide timeval by double factor
timeval operator/(const timeval &left, double right) noexcept;

//* divide each timeval of itimerval by double factor
itimerval operator/(const itimerval &left, double right) noexcept;

//* convert timeval to double (seconds)
double timeval_to_double(const timeval &time) noexcept;

//* convert double (seconds) to timeval
timeval double_to_timeval(double time) noexcept;


}  // namespace cxxitimer
