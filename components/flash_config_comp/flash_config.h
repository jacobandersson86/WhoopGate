#pragma once

#include "esp_err.h"
#include "nvs.h" //Needed for error codes

#define STORAGE_NAMESPACE "storage"
#define CONFIGURATION_KEY "pmconf"





#define PMSTEP_CONFIG_MAX_REGISTERED_CALLBACKS 32

// Default values (to be used when uninitiated)
#define VOLTAGE_DIVIDER_QUOTIENT 47.0/1047.0

typedef struct {
	float km, L, R, i;
	int Nr;
	float kp_pos, ki_pos, kd_pos;
	float kp_spd, ki_spd, kd_spd;
	float kp_trq, ki_trq, kd_trq;
	float voltage_divider_quotient;	
	float hold_torque, max_torque, max_speed;
} pmstep_config_t;

typedef void(*pmstep_config_changed_callback_t)(void);
typedef struct {
	int enabled;
	pmstep_config_changed_callback_t callback;
} pmstep_config_callback_handle_t;

esp_err_t flash_config_flush();
esp_err_t flash_config_init();

esp_err_t flash_config_write_angle_cal(float *lookup, int size);
esp_err_t flash_config_get_angle_cal(float *lookup, int size_floats);
esp_err_t flash_config_get_angle_cal_size(int *size_floats);

esp_err_t flash_config_set_motor_params(float km, float L, float R, float i, int Nr);
esp_err_t flash_config_get_motor_params(float *km, float *L, float *R, float *i, int *Nr);

esp_err_t flash_config_set_position_pid(float kp, float ki, float kd);
esp_err_t flash_config_get_position_pid(float * kp, float * ki, float * kd);

esp_err_t flash_config_set_speed_pid(float kp, float ki, float kd);
esp_err_t flash_config_get_speed_pid(float * kp, float * ki, float * kd);

esp_err_t flash_config_set_torque_pid(float kp, float ki, float kd);
esp_err_t flash_config_get_torque_pid(float *kp, float *ki, float *kd);

esp_err_t flash_config_set_voltage_divider_quotient(float quotient);
esp_err_t flash_config_get_voltage_divider_quotient(float* quotient);

esp_err_t flash_config_set_speed_and_torque(float hold_torque, float max_torque, float max_speed);
esp_err_t flash_config_get_speed_and_torque(float * hold_torque, float * max_torque, float * max_speed);

esp_err_t flash_config_set_cli_flag(int * flag);
esp_err_t flash_config_get_cli_flag(int * flag);

esp_err_t flash_config_nvs_set_float(const char *namespace, const char *key, float val);
esp_err_t flash_config_nvs_get_float(const char *namespace, const char *key, float *out_value);

esp_err_t flash_config_nvs_set_int(const char *namespace, const char *key, int32_t val);
esp_err_t flash_config_nvs_get_int(const char *namespace, const char *key, int32_t *out_value);

esp_err_t flash_config_nvs_set_str(const char *namespace, const char *key, const char *str);
esp_err_t flash_config_nvs_get_str(const char *namespace, const char *key, const char *out_str, size_t *length);

int flash_config_register_callback(pmstep_config_changed_callback_t callback);
esp_err_t flash_config_unregister_callback(int callback_id);


esp_err_t pmstep_config_dump(pmstep_config_t * c);

