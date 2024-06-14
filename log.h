//
// Created by pneuma on 2024-06-13.
//

#ifndef SDVX_RGB_BUTTONS_LOG_H
#define SDVX_RGB_BUTTONS_LOG_H

// Workaround to have spdlog log debug only in debug builds
#ifdef _DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

#include "spdlog/spdlog.h"

#endif //SDVX_RGB_BUTTONS_LOG_H
