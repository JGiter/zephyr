/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include "sensor.h"

#define SENSOR_NODE    DT_COMPAT_GET_ANY_STATUS_OKAY(st_lsm6dsl)
#define I2C_TEST_NODE  DT_PARENT(SENSOR_NODE)
#define DEVICE_ADDRESS (uint8_t)DT_REG_ADDR(SENSOR_NODE)

static const struct device *const i2c_device = DEVICE_DT_GET(I2C_TEST_NODE);

static uint16_t read_sensor_reg(uint8_t register_address)
{
	int err;
	uint8_t response = 0;

	err = i2c_reg_read_byte(i2c_device, DEVICE_ADDRESS, register_address, &response);
	zassert_equal(err, 0, "i2c_read(%x)' failed with error: %d\n", register_address, err);
	return response;
}

static void write_sensor_reg(uint8_t register_address, int8_t value)
{
	int err;

	err = i2c_reg_write_byte(i2c_device, DEVICE_ADDRESS, register_address, value);
	zassert_equal(err, 0, "i2c_reg_write_byte(%x, %x)' failed with error: %d\n",
		      register_address, value, err);
}

ZTEST(i2c_controller_to_sensor, test_sensor_dt_config)
{
	uint8_t who_i_am = read_sensor_reg(WHO_I_AM);
	zassert_equal(who_i_am, 0x6a, "WHO_I_AM expected 0x6a, but got 0x%02x\n", who_i_am);

	/* Accellerometer ODR is 104 */
	uint8_t odr_xl_raw = ((uint8_t)read_sensor_reg(CTRL1_XL) & ODR_XL_MSK) >> ODR_XL_SHIFT;
	zassert_equal(odr_xl_raw, 0x2, "ODR_XL expected is 0x2, but got 0x%x\n", odr_xl_raw);
}

ZTEST(i2c_controller_to_sensor, test_read_temperature_from_reg)
{
	/* Temperature data is ready */
	zassert_equal(read_sensor_reg(STATUS_REG) & STATUS_REG_TDA_MSK, STATUS_REG_TDA_MSK);

	TC_PRINT("Reading temperature from register...\n");
	uint16_t temperature_sample = (((uint16_t)read_sensor_reg(OUT_TEMP_L)) |
				       ((uint16_t)read_sensor_reg(OUT_TEMP_H) << 8));
	int32_t temperature_in_celcius = calculate_temperature(temperature_sample);
	TC_PRINT("Temperature is %d\n", temperature_in_celcius);
}

ZTEST(i2c_controller_to_sensor, test_i2c_nack_handling)
{
	int err;
	uint8_t test_data;

	TC_PRINT("Device address 0x%x\n", DEVICE_ADDRESS);

	err = i2c_read(i2c_device, &test_data, 1, DEVICE_ADDRESS + 1);
	zassert_equal(err, -EIO, "Invalid device address not detected, err: %d\n", err);

	err = i2c_reg_read_byte(i2c_device, DEVICE_ADDRESS, WHO_I_AM, &test_data);
	zassert_equal(err, 0, "Failed to read device register after previous address NACK: %d\n",
		      err);
}

ZTEST(i2c_controller_to_sensor, test_i2c_bus_recovery)
{
	int err;
	uint8_t test_data;

	TC_PRINT("Device address 0x%x\n", DEVICE_ADDRESS);

	err = i2c_recover_bus(i2c_device);
	zassert_equal(err, 0, "'i2c_recover_bus' failed with error: %d\n", err);

	err = i2c_reg_read_byte(i2c_device, DEVICE_ADDRESS, WHO_I_AM, &test_data);
	zassert_equal(err, 0, "Failed to read device register after bus recovery: %d\n", err);
}

ZTEST(i2c_controller_to_sensor, test_fifo_read_fifo_mode)
{
	int err;
	uint8_t response = 0;
	uint32_t i2c_config = I2C_SPEED_SET(CONFIG_TEST_I2C_SPEED) | I2C_MODE_CONTROLLER;
	uint8_t measurements_left = MEASUREMENT_CYCLES + 1;

	TC_PRINT("Device address 0x%x\n", DEVICE_ADDRESS);
	TC_PRINT("I2C speed setting: %d\n", CONFIG_TEST_I2C_SPEED);

	err = i2c_configure(i2c_device, i2c_config);
	zassert_equal(err, 0, "i2c_configure' failed with error: %d\n", err);

	response = read_sensor_reg(WHO_I_AM);
	TC_PRINT("Chip_Id: %d\n", response);

	/* Trigger FIFO signal selection from XL/GYRO data ready */
	uint8_t master_config = read_sensor_reg(MASTER_CONFIG);
	master_config &= !DATA_VALID_SEL_FIFO_MSK;
	write_sensor_reg(MASTER_CONFIG, master_config);

	/* Disable continuous update in the data registers */
	uint8_t ctrl3_c = read_sensor_reg(CTRL3_C);
	ctrl3_c &= !BDU_MSK;
	write_sensor_reg(CTRL3_C, ctrl3_c);

	/* Set FIFO accel ODR 104Hz, gyro 52Hz */
	uint8_t fifo_ctrl3 = read_sensor_reg(FIFO_CTRL3);
	fifo_ctrl3 &= !(DEC_FIFO_GYRO_MSK | DEC_FIFO_XL_MSK);
	fifo_ctrl3 |= (DEC_FIFO_GYRO_2 << DEC_FIFO_GYRO_SHIFT | DEC_FIFO_XL_1 << DEC_FIFO_XL_SHIFT);
	write_sensor_reg(FIFO_CTRL3, fifo_ctrl3);
	/* Set FIFO accel ODR 104Hz, gyro 52Hz */
	uint8_t fifo_ctrl4 = read_sensor_reg(FIFO_CTRL4);
	fifo_ctrl4 &= !(DEC_FIFO_DS3_MSK | DEC_FIFO_DS4_MSK);
	fifo_ctrl4 |= (DEC_FIFO_DS3_0 << DEC_FIFO_DS3_SHIFT | DEC_FIFO_DS4_4 << DEC_FIFO_DS4_SHIFT);
	write_sensor_reg(FIFO_CTRL4, fifo_ctrl4);

	/* Enable temperature in 4th FIFO data set and disable step counter */
	uint8_t fifo_ctrl2 = read_sensor_reg(FIFO_CTRL2);
	fifo_ctrl2 &= !TIMER_PEDO_FIFO_EN_MSK;
	fifo_ctrl2 &= !TIMER_PEDO_FIFO_DRDY_MSK;
	fifo_ctrl2 |= FIFO_TEMP_EN_MSK;
	write_sensor_reg(FIFO_CTRL2, fifo_ctrl2);

	/* Set FIFO ODR to 104Hz and set FIFO mode to FIFO */
	uint8_t fifo_ctrl5 = read_sensor_reg(FIFO_CTRL5);
	fifo_ctrl5 &= !(ODR_FIFO_MSK | FIFO_MODE_MSK);
	fifo_ctrl5 |= (ODR_FIFO_104 << ODR_FIFO_SHIFT | FIFO_MODE_FIFO << FIFO_MODE_SHIFT);
	write_sensor_reg(FIFO_CTRL5, fifo_ctrl5);

	/* Give time to collect some data */
	k_sleep(K_MSEC(SLEEP_TIME_MS));
	uint8_t fifo_status1 = read_sensor_reg(FIFO_STATUS1);
	uint8_t fifo_status2 = read_sensor_reg(FIFO_STATUS2);
	uint16_t samples = ((uint16_t)(fifo_status2 & FIFO_DIFF_UP_MSK) << 8) | fifo_status1;
	/* first sample: gyro, accel and temp
	 * second sample: accel
	 * third sample: gyro and accel
	 * fourth sample: accel
	 */
	uint16_t samples_per_measurment = ((3 + 3 + 3) + 3 + (3 + 3) + 3);
	uint16_t samples_expected = samples_per_measurment * MEASUREMENT_CYCLES;
	zassert_true(samples > samples_expected, "Not enough samples in FIFO: %d < %d", samples,
		     samples_expected);

	while (measurements_left) {
		for (uint16_t sample = 0; sample < samples_per_measurment; ++sample) {
			uint8_t fifo_status3 = read_sensor_reg(FIFO_STATUS_3);
			uint8_t fifo_status4 = read_sensor_reg(FIFO_STATUS_4);
			uint16_t fifo_pattern =
				((uint16_t)(fifo_status4 & FIFO_STATUS_UP_MSK) << 8 | fifo_status3);
			zassert_equal(sample, fifo_pattern,
				      "FIFO pattern does not match: pattern is %d, expected %d",
				      fifo_pattern, sample);
			uint32_t fifo_out = ((uint32_t)read_sensor_reg(FIFO_DATA_OUT_H) << 8) |
					    read_sensor_reg(FIFO_DATA_OUT_L);
			if (sample == 7) {
				TC_PRINT("Temperature: %d\n", calculate_temperature(fifo_out));
			}
		}
		measurements_left--;
	}
}

void *test_setup(void)
{
	zassert_true(device_is_ready(i2c_device), "i2c device is not ready");

	/* Set ODR_XL to 104Hz */
	uint8_t ctrl1_xl = read_sensor_reg(CTRL1_XL);
	ctrl1_xl &= !ODR_XL_MSK;
	ctrl1_xl |= ODR_XL_104 << ODR_XL_SHIFT;
	write_sensor_reg(CTRL1_XL, ctrl1_xl);

	/* Set ODR_GYRO to 104Hz */
	uint8_t ctrl2_g = read_sensor_reg(CTRL2_G);
	ctrl2_g &= !ODR_G_MSK;
	ctrl2_g |= ODR_G_104 << ODR_G_SHIFT;
	write_sensor_reg(CTRL2_G, ctrl2_g);

	return NULL;
}

ZTEST_SUITE(i2c_controller_to_sensor, NULL, test_setup, NULL, NULL, NULL);
