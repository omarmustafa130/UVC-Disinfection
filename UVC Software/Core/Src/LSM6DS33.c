/*
 * LSM6DS33.c
 *
 *  Created on: FEB 25, 2025
 *      Author: OM
 */

#include "LSM6DS33.h"

void LSM6DS33_init(void){

	uint8_t data;

	// ODR = 1000 (1.66 kHz (high performance)); FS_XL = 00 (+/-2 g full scale)
	data = 0x80; // 0x80 = 0b10000000
	HAL_I2C_Mem_Write	(			&hi2c1,
							(0x6A)<<1,
							0x10,// LSMDS_CTRL1_XL (10h)
							1,
							&data,
							1,
							100
						);

	 // ODR = 1000 (1.66 kHz (high performance)); FS_G = 00 (245 dps for DS33)
	data = 0x80; // 0x80 = 0b10000000
	HAL_I2C_Mem_Write	(			&hi2c1,
							(0x6A)<<1,
							0x11,//  CTRL2_G = (11h)
							1,
							&data,
							1,
							100
						);

	data = 0x44; // 0x44 = 0b01000100
	HAL_I2C_Mem_Write	(			&hi2c1,
							(0x6A)<<1,
							0x16,//  CTRL7_G (16h)
							1,
							&data,
							1,
							100
						);



}

float read_data_gyro(int eksen){

	uint8_t cuffer[6];
	int16_t gyro_raw[3];

	  cuffer[0]=0x22;
	  HAL_I2C_Master_Transmit(&hi2c1,(0x6A)<<1,cuffer,1,HAL_MAX_DELAY);
	  HAL_I2C_Master_Receive(&hi2c1,(0x6A)<<1,cuffer,6,HAL_MAX_DELAY);

	  gyro_raw[0] = (cuffer[1] << 8 | cuffer[0]);
	  gyro_raw[1] = (cuffer[3] << 8 | cuffer[2]);
	  gyro_raw[2] = (cuffer[5] << 8 | cuffer[4]);

	  if(1 == eksen)	return gyro_raw[0];
	  if(2 == eksen)	return gyro_raw[1];
	  if(3 == eksen)	return gyro_raw[2];

}

float read_data_acc(int eksen){

	uint8_t buffer[6];
	int16_t acc_raw[3];

	  buffer[0]=0x28;
	  HAL_I2C_Master_Transmit(&hi2c1,(0x6A)<<1,buffer,1,HAL_MAX_DELAY);
	  HAL_I2C_Master_Receive(&hi2c1,(0x6A)<<1,buffer,6,HAL_MAX_DELAY);

	  acc_raw[0] = (buffer[1] << 8 | buffer[0]);
	  acc_raw[1] = (buffer[3] << 8 | buffer[2]);
	  acc_raw[2] = (buffer[5] << 8 | buffer[4]);

	  if(1 == eksen)	return acc_raw[0];
	  if(2 == eksen)	return acc_raw[1];
	  if(3 == eksen)	return acc_raw[2];

}

float acc_total_vector(void){

	return sqrt((read_data_acc(1)*read_data_acc(1))+(read_data_acc(2)*read_data_acc(2))+(read_data_acc(3)*read_data_acc(3)));

}

float angle_pitch_acc(void){

	//57.296 = 1 / (3.142 / 180)

	return asin((float)read_data_acc(1)/acc_total_vector())* 57.295779513082320876798154814105;

}
float angle_roll_acc(void){

	return asin((float)read_data_acc(2)/acc_total_vector())* 57.295779513082320876798154814105;


}

