/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <zephyr/kernel.h>

#define SLEEP_TIME_MS      1000
#define MEASUREMENT_CYCLES 3
#define FIFO_BUF_SIZE      4096

#define MASTER_CONFIG             0x1A
#define DATA_VALID_SEL_FIFO_SHIFT 6
#define DATA_VALID_SEL_FIFO_MSK   1 << DATA_VALID_SEL_FIFO_SHIFT

#define WHO_I_AM 0x0F

#define CTRL1_XL     0x10
#define ODR_XL_SHIFT 4
#define ODR_XL_MSK   0xF << ODR_XL_SHIFT /* 4 bits */
#define ODR_XL_104   0x2

#define CTRL2_G     0x11
#define ODR_G_SHIFT 4
#define ODR_G_MSK   0xF << ODR_G_SHIFT /* 4 bits */
#define ODR_G_0     0x0
#define ODR_G_12_5  0x1
#define ODR_G_26    0x2
#define ODR_G_52    0x3
#define ODR_G_104   0x4

#define CTRL3_C   0x12
#define BDU_SHIFT 6
#define BDU_MSK   1 << BDU_SHIFT

#define FIFO_CTRL2                 0x07
#define TIMER_PEDO_FIFO_EN_SHIFT   7
#define TIMER_PEDO_FIFO_EN_MSK     1 << TIMER_PEDO_FIFO_EN_SHIFT
#define TIMER_PEDO_FIFO_DRDY_SHIFT 6
#define TIMER_PEDO_FIFO_DRDY_MSK   1 << TIMER_PEDO_FIFO_DRDY_SHIFT
#define FIFO_TEMP_EN_SHIFT         3
#define FIFO_TEMP_EN_MSK           1 << FIFO_TEMP_EN_SHIFT

#define FIFO_CTRL3          0x08
#define DEC_FIFO_GYRO_SHIFT 3
#define DEC_FIFO_GYRO_MSK   0x7 << DEC_FIFO_GYRO_SHIFT /* 3 bits */
#define DEC_FIFO_GYRO_0     0x0
#define DEC_FIFO_GYRO_1     0x1
#define DEC_FIFO_GYRO_2     0x2
#define DEC_FIFO_XL_SHIFT   0
#define DEC_FIFO_XL_MSK     0x7 << DEC_FIFO_XL_SHIFT /* 3 bits */
#define DEC_FIFO_XL_0       0x0
#define DEC_FIFO_XL_1       0x1
#define DEC_FIFO_XL_2       0x2

#define FIFO_CTRL4         0x09
#define DEC_FIFO_DS3_SHIFT 0
#define DEC_FIFO_DS3_MSK   0x7 << DEC_FIFO_DS3_SHIFT /* 3 bits */
#define DEC_FIFO_DS3_0     0x0
#define DEC_FIFO_DS3_1     0x1
#define DEC_FIFO_DS3_2     0x2
#define DEC_FIFO_DS3_3     0x3
#define DEC_FIFO_DS3_4     0x4
#define DEC_FIFO_DS4_SHIFT 3
#define DEC_FIFO_DS4_MSK   0x7 << DEC_FIFO_DS4_SHIFT /* 3 bits */
#define DEC_FIFO_DS4_0     0x0
#define DEC_FIFO_DS4_1     0x1
#define DEC_FIFO_DS4_2     0x2
#define DEC_FIFO_DS4_3     0x3
#define DEC_FIFO_DS4_4     0x4

#define FIFO_CTRL5       0x0A
#define ODR_FIFO_SHIFT   3
#define ODR_FIFO_MSK     0xF << ODR_FIFO_SHIFT /* 4 bits */
#define ODR_FIFO_0       0x0
#define ODR_FIFO_12_5    0x1
#define ODR_FIFO_26      0x2
#define ODR_FIRO_52      0x3
#define ODR_FIFO_104     0x4
#define ODR_FIFO_208     0x5
#define FIFO_MODE_SHIFT  0
#define FIFO_MODE_MSK    0x7 << FIFO_MODE_SHIFT /* 3 bits */
#define FIFO_MODE_BYPASS 0x0
#define FIFO_MODE_FIFO   0x1

#define FIFO_STATUS1 0x3A

#define FIFO_STATUS2       0x3B
#define FIFO_DIFF_UP_SHIFT 0
#define FIFO_DIFF_UP_MSK   0x7 << FIFO_DIFF_UP_SHIFT /* 3 bits */

#define FIFO_STATUS_3 0x3C

#define FIFO_STATUS_4        0x3D
#define FIFO_STATUS_UP_SHIFT 0
#define FIFO_STATUS_UP_MSK   0x3 << FIFO_STATUS_UP_SHIFT /* 2 bits */

#define FIFO_DATA_OUT_L 0x3E
#define FIFO_DATA_OUT_H 0x3F

#define STATUS_REG          0x1E
#define STATUS_REG_XLDA_BIT 0
#define STATUS_REG_XLDA_MSK 1 << STATUS_REG_XLDA_BIT
#define STATUS_REG_GDA_BIT  1
#define STATUS_REG_GDA_MSK  1 << STATUS_REG_GDA_BIT
#define STATUS_REG_TDA_BIT  2
#define STATUS_REG_TDA_MSK  1 << STATUS_REG_TDA_BIT

#define OUT_TEMP_L 0x20
#define OUT_TEMP_H 0x21

int16_t calculate_temperature(uint16_t raw_temp);

#endif
