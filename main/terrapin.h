/**
 * terrapin.h
 * 
 * SPDX-FileCopyrightText: Copyright © 2025 Honulanding Software <dev@honulanding.com>
 * SPDX-License-Identifier: Apache-2.0
 * 
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "terrapin.def"

/**
 * @brief datastream identifiers
 */
typedef enum {
    #define X(NAME, TOPIC, UNITS, PRECISION) NAME,
    DATASTREAM_LIST
    #undef X
    TERRAPIN_DATASTREAM_IDX_MAX
} DATASTREAM_ID_T;

/**
 * @brief project-specific initialization
 */
bool terrapin_init(void);
