/*
 * Copyright (C) 2024 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "cxxitimer.hpp"

namespace cxxitimer {

std::string get_lib_version() { return {PROJECT_VERSION}; }

std::string get_lib_info() { return {PROJECT_NAME " " PROJECT_VERSION " - " COMPILER_INFO " on " SYSTEM_INFO}; }

std::string get_lib_date() {
#ifdef COMPILER_CLANG
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdate-time"
#endif
    return {__DATE__ " " __TIME__};
#ifdef COMPILER_CLANG
#    pragma clang diagnostic pop
#endif
}

}  // namespace cxxitimer
