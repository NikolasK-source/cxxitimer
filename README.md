# cxxitimer

A C++ Library to handle linux interval timer

## Interval Timer Types

Each timer type can exist only once per application.

- **ITimer_Real** counts down in real time.
- **ITimer_Virtual** counts down against the user-mode CPU time consumed by the process.
- **ITimer_Prof** counts down against the total CPU time consumed by the process.

## Usage

In the following replace ```ITimer_XXX``` with the desired timer type.

### Create Interval Timer

```c++
const timeval interval = {1, 0};
cxxitimer::ITimer_XXX itimer(interval);
```

```c++
const double interval = 1.0;
cxxitimer::ITimer_XXX itimer(interval);
```

```c++
const timeval initial_interval = {2, 0};
const timeval interval = {1, 0};
cxxitimer::ITimer_XXX itimer(interval, initial_interval);
```

```c++
const double initial_interval = 2.0;
const double interval = 1.0;
cxxitimer::ITimer_XXX itimer(interval, initial_interval);
```

### Start/Stop Interval Timer

```c++
itimer.start();
```

```c++
itimer.stop();
```

### Change interval

> **Note**: Timer must be stopped.

```c++
const timeval interval = {1, 500'000};
itimer.set_interval(interval);
```

```c++
const double interval = 2.5;
itimer.set_interval(interval);
```

### Manipulate Timer Speed

> **Note**: can be applied it the timer is running

```c++
// slow down timer
itmer.set_speed_factor(0.75);
```

```c++
// speed up timer
itmer.set_speed_factor(1.25);
```

```c++
// reset timer to original interval
itimer.set_speed_to_normal();
```
