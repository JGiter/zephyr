/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sensor.h"
#include "zephyr/tc_util.h"

int16_t calculate_temperature(uint16_t raw_temp)
{
	int16_t temperature;
	int32_t zero_level = 25;
	/* 256 per Celcius */
	int32_t sensitivity = 8;
	uint8_t is_negative = raw_temp >> 15;
	if (is_negative) {
		raw_temp = ~raw_temp + 1;
	}
	/* For fractional part is discarded */
	uint16_t temp_integer = raw_temp >> sensitivity;
	if (is_negative) {
		temperature = zero_level - temp_integer;
	} else {
		temperature = zero_level + temp_integer;
	}

	return temperature;
}
