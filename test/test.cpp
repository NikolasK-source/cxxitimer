/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "cxxitimer.hpp"

#include <csignal>
#include <cstring>
#include <iostream>
#include <sysexits.h>
#include <thread>

static int  x = 0;
static void handler(int) { ++x; }

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    int tmp       = sigaction(SIGALRM, &sa, nullptr);
    if (tmp != 0) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    sigset_t set;
    sigemptyset(&set);
    if (sigaddset(&set, SIGALRM) == -1) {
        perror("sigaddset");
        return EX_OSERR;
    }

    cxxitimer::ITimer_Real timer(2.0, 1.0);

    timer.start();

    std::this_thread::sleep_for(std::chrono::seconds(10));
    if (x != 5) {
        std::cerr << "Assertion x == 5 failed " << __FILE__ << ":" << __LINE__ << std::endl;
        return EXIT_FAILURE;
    }
}
