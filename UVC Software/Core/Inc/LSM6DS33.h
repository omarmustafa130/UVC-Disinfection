/*
 * LSM6DS33.h
 *
 *  Created on: Feb 25, 2025
 *      Author: OM
 */

#include "stm32l0xx_hal.h"
#include "stdint.h"
#include <math.h>

extern I2C_HandleTypeDef hi2c1;

#ifndef INC_LSM6DS33_H_
#define INC_LSM6DS33_H_

// LSM6DS33 init
void LSM6DS33_init(void);

//gyro func.
float read_data_gyro(int eksen);

//acc func.
float read_data_acc(int eksen);
float angle_pitch_acc(void);
float angle_roll_acc(void);

#endif /* INC_LSM6DS33_H_ */
