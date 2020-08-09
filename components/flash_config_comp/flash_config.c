//TODO: add code for your component here

#include "flash_config.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "errno.h"
#include "stdio.h"
#include "fix16.h"


#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

static const char *TAG = "flash_config";

//Related to NVS storage
nvs_handle_t flash_config_nvs_handle;

// Related to FAT system and wear leveling
static int file_system_mounted = 0;
static const char *file_system_base_path = "/spiflash";
static const char *file_system_config_file = "/spiflash/config.bin";
static const char *file_system_cli_file = "/spiflash/cli.bin";
static const char *file_system_angle_cal_file = "/spiflash/anglecal.bin";
static const char *file_system_partition_name = "storage";
static wl_handle_t wear_leveling_handle = WL_INVALID_HANDLE;

static pmstep_config_t current_config;
static SemaphoreHandle_t config_has_changed;
static pmstep_config_callback_handle_t callback_handles[PMSTEP_CONFIG_MAX_REGISTERED_CALLBACKS];

static esp_err_t write_pmstep_config_file(pmstep_config_t *config);

bool flash_config_initiated = false;

/*
	PRIVATE FUNCTION DECLARATIONS
*/

/**
 * @brief Help function to store a PID in memory. The correct namespace must be 
 * opened before calling this function and a call to nvs_close must be made
 * after calling this function.  
 * 
 * @param kp 
 * @param ki 
 * @param kd 
 * @return esp_err_t 
 */
static esp_err_t s_flash_config_set_pid(float kp, float ki, float kd);

/**
 * @brief Help function to get a PID from memory. The correct namespace must be 
 * opened before calling this function and a call to nvs_close must be made
 * after calling this function.  
 * 
 * @param kp 
 * @param ki 
 * @param kd 
 * @return esp_err_t 
 */
static esp_err_t s_flash_config_get_pid(float *kp, float *ki, float *kd);



/*
	PRIVATE FUNCTIONS
 */
static void invoke_callbacks()
{
	for (int i = 0; i < PMSTEP_CONFIG_MAX_REGISTERED_CALLBACKS; i++)
	{
		if (callback_handles[i].enabled)
		{
			callback_handles[i].callback();
		}
	}
}

//Rename this function so that it is clear that is relates to pmstep config file??
//Or remove it and put the write_pmstep_config_file() and invoke_callbacks() inside their respective public functions below? Could maybe be more clearer.
static void after_set()
{

	xSemaphoreGive(config_has_changed);
}
static esp_err_t write_pmstep_config_file(pmstep_config_t *config)
{

	// If the filesystem isn't mounted we can't write
	if (!file_system_mounted)
	{
		ESP_LOGE(TAG, "write failed: Filesystem not mounted");
		return ESP_FAIL;
	}

	// Open the config file. Create if missing
	FILE *config_file = fopen(file_system_config_file, "wb");

	if (config_file == NULL)
	{
		ESP_LOGE(TAG, "Failed to open configuration file!");
		return ESP_FAIL;
	}

	// Write the config to the file
	//size_t written = fwrite(config, 1, sizeof(pmstep_config_t), config_file);
	size_t written = fwrite(config, sizeof(pmstep_config_t), 1, config_file);
	fflush(config_file);
	if (!written)
	{
		ESP_LOGE(TAG, "Failed to write to config file!");
		fclose(config_file);
		return ESP_FAIL;
	}
	/*
	else if (written < sizeof(pmstep_config_t)){
		ESP_LOGE(TAG, "fwrite did not write all data. Written: %d, expected: %d",written,sizeof(pmstep_config_t));
		fclose(config_file);
		return ESP_FAIL;
	}
	*/
	// Success!
	fclose(config_file);

	return ESP_OK;
}
static esp_err_t write_default_configuration()
{
	pmstep_config_t default_config;

	// Almost all motors are 200 steps/rev (1.8 deg/step) => Nr = 50
	// default_config.Nr = 50;
	// default_config.R = 2.12;
	// default_config.L = 0.0033;
	// default_config.km = 0.2;

	// No hold torque.
	default_config.hold_torque = 0;

	// Maximum torque to (hopefully) allow some movement
	// with limited risk of driving out of control on an unknown motor.
	default_config.max_torque = 0.15f;

	// Pretty low max speed to avoid accidental damage
	default_config.max_speed = 2.0f;

	// Set all motor controller PIDs to 0 for now
	default_config.kd_pos = 0;
	default_config.kp_pos = 0;
	default_config.ki_pos = 0;
	default_config.kd_spd = 0;
	default_config.kp_spd = 0;
	default_config.ki_spd = 0;

	// Set hbridge voltage divider to the value it "should" be
	default_config.voltage_divider_quotient = 47.0f / 1047.0f;

	esp_err_t err = write_pmstep_config_file(&default_config);
	if (err == ESP_OK)
	{
		ESP_LOGI(__func__, "Wrote default config to flash (0x%x: %s)", err, esp_err_to_name(err));
		pmstep_config_dump(&default_config);
	}
	else
	{
		ESP_LOGE(__func__, "Failed to write default config (0x%x: %s)", err, esp_err_to_name(err));
	}
	return err;
}
static esp_err_t read_pmstep_config_file(pmstep_config_t *config)
{
	esp_err_t err;
	// If the filesystem isn't mounted we can't write
	if (!file_system_mounted)
	{
		ESP_LOGE(TAG, "read failed: Filesystem not mounted");
		return ESP_FAIL;
	}

	FILE *config_file = fopen(file_system_config_file, "rb");

	if (config_file == NULL)
	{

		if (errno == ENOENT)
		{
			ESP_LOGW(TAG, "Config file does not exist. Creating with default values...");
			if (write_default_configuration() == ESP_OK)
			{
				config_file = fopen(file_system_config_file, "rb");
			}
		}
		if (config_file == NULL)
		{ // Still NULL
			ESP_LOGE(TAG, "Failed to open configuration file!");
			return ESP_FAIL;
		}
	}

	size_t elements_read = fread(config, sizeof(pmstep_config_t), 1, config_file);
	if (elements_read == 0)
	{
		ESP_LOGE(TAG, "Failed to read config file!");
		fclose(config_file);
		return ESP_FAIL;
	}

	// success!
	fclose(config_file);

	//Read from NVS
	// Motor params
	float km, L, R, i;
	int Nr;
	err = flash_config_get_motor_params(&km, &L, &R, &i, &Nr);
	if(ESP_ERR_NVS_NOT_FOUND == err)
	{
		err = flash_config_set_motor_params(0.0, 0.0, 0.0, 0.0, 0);
		if(ESP_OK == err)
			err = flash_config_get_motor_params(&km, &L, &R, &i, &Nr);
		ESP_ERROR_CHECK(err);
	}
	else
	{
		ESP_ERROR_CHECK(err);
	}
	current_config.km = km;
	current_config.L = L;
	current_config.R = R;
	current_config.i = i;
	current_config.Nr = Nr;

	// Position PID
	float kp, ki, kd;
	err = nvs_open("position", NVS_READWRITE, &flash_config_nvs_handle);
	ESP_ERROR_CHECK_WITHOUT_ABORT(err);
	err = s_flash_config_get_pid(&kp, &ki, &kd);
	if (err != ESP_OK) // There is uninitialized values, we must write to them in order for the key-value pair to exist.
	{	
		kp = 0;
		kd = 0;
		ki = 0;
		err = s_flash_config_set_pid(kp, ki, kd);
		ESP_ERROR_CHECK_WITHOUT_ABORT(err);
	}
	nvs_close(flash_config_nvs_handle);
	current_config.kp_pos = kp;
	current_config.ki_pos = ki;
	current_config.kd_pos = kd;
	// Speed PID
	err = nvs_open("speed", NVS_READWRITE, &flash_config_nvs_handle);
	ESP_ERROR_CHECK_WITHOUT_ABORT(err);
	err = s_flash_config_get_pid(&kp, &ki, &kd);
	if (err != ESP_OK) // There is uninitialized values, we must write to them in order for the key-value pair to exist.
	{	
		kp = 0;
		kd = 0;
		ki = 0;
		err = s_flash_config_set_pid(kp, ki, kd);
		ESP_ERROR_CHECK_WITHOUT_ABORT(err);
	}
	nvs_close(flash_config_nvs_handle);
	current_config.kp_spd = kp;
	current_config.ki_spd = ki;
	current_config.kd_spd = kd;

	// Torque PID
	err = nvs_open("torque", NVS_READWRITE, &flash_config_nvs_handle);
	ESP_ERROR_CHECK_WITHOUT_ABORT(err);	
	err = s_flash_config_get_pid(&kp, &ki, &kd);
	if (err != ESP_OK) // There is uninitialized values, we must write to them in order for the key-value pair to exist.
	{	
		kp = 0;
		kd = 0;
		ki = 0;
		err = s_flash_config_set_pid(kp, ki, kd);
		ESP_ERROR_CHECK_WITHOUT_ABORT(err);
	}
	nvs_close(flash_config_nvs_handle);
	current_config.kp_trq = kp;
	current_config.ki_trq = ki;
	current_config.kd_trq = kd;

	return err;
}

static esp_err_t write_angle_calib_file(float *lookup, int size_floats)
{

	// If the filesystem isn't mounted we can't write
	if (!file_system_mounted)
	{
		ESP_LOGE(TAG, "write failed: Filesystem not mounted");
		return ESP_FAIL;
	}

	// Open the config file. Create if missing. Deletes if exists.
	FILE *angle_cal_file = fopen(file_system_angle_cal_file, "wb");

	if (angle_cal_file == NULL)
	{
		ESP_LOGE(TAG, "Failed to open configuration file!");
		return ESP_FAIL;
	}

	// Write the config to the file
	//size_t written = fwrite(lookup, 1, (size * sizeof(float)), angle_cal_file);
	size_t written = fwrite(lookup, sizeof(float), size_floats, angle_cal_file);
	fflush(angle_cal_file);
	if (!written)
	{
		ESP_LOGE(TAG, "Failed to write to config file!");
		fclose(angle_cal_file);
		return ESP_FAIL;
	}
	// Success!
	fclose(angle_cal_file);

	return ESP_OK;
}

static void config_manager_task(void *p)
{
	while (1)
	{
		if (xSemaphoreTake(config_has_changed, portMAX_DELAY))
		{

			// Write after 0.1 second in case multiple separate sets are made.
			vTaskDelay(100 / portTICK_PERIOD_MS);
			xSemaphoreTake(config_has_changed, 0); // Make sure the semaphore is cleared here, in case someone used a "set" during the 100 ms wait.
			//write_pmstep_config(&current_config);
			write_pmstep_config_file(&current_config);
			invoke_callbacks();
		}
	}
}

static void initialize_filesystem()
{
	ESP_LOGI(TAG, "Initializing NVS storage");
	// Initialize NVS
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);



	ESP_LOGI(TAG, "Mounting filesystem");

	const esp_vfs_fat_mount_config_t mount_config = {
		.max_files = 4,
		.format_if_mount_failed = true,
		.allocation_unit_size = CONFIG_WL_SECTOR_SIZE};
	err = esp_vfs_fat_spiflash_mount(file_system_base_path, file_system_partition_name, &mount_config, &wear_leveling_handle);
	if (err == ESP_ERR_NOT_FOUND)
	{
		ESP_LOGE(TAG, "Failed to mount file system: Partition does not exists (0x%x: %s)", err, esp_err_to_name(err));
		ESP_LOGE(TAG, "Make sure the partition table is configured correctly using \"idf.py menuconfig\"");
		return;
	}
	else if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to mount file system! (0x%x: %s)", err, esp_err_to_name(err));
		return;
	}

	file_system_mounted = 1;
	ESP_LOGI(TAG, "Filesystem mounted successfully");
}

/*
	PUBLIC FUNCTIONS
 */

esp_err_t flash_config_flush()
{

	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}

	xSemaphoreTake(config_has_changed, 0);	   // Clear the semaphore
	write_pmstep_config_file(&current_config); // Write to flash

	return ESP_OK;
}

static esp_err_t s_flash_config_nvs_set_float(const char *key, float value)
{
	esp_err_t err = ESP_OK;
	fix16_t ival = fix16_from_float(value);
	err = nvs_set_i32(flash_config_nvs_handle, key, (int32_t)ival);
	return err;
}

static esp_err_t s_flash_config_nvs_get_float(const char *key, float *out_value)
{
	esp_err_t err = ESP_OK;
	fix16_t ival = 0;
	err = nvs_get_i32(flash_config_nvs_handle, key, &ival);
	*out_value = fix16_to_float(ival);
	return err;
}

static esp_err_t s_flash_config_nvs_set_int(const char *key, int32_t value)
{
	return nvs_set_i32(flash_config_nvs_handle, key, value);
}

static esp_err_t s_flash_config_nvs_get_int(const char *key, int32_t *out_value)
{
	return nvs_get_i32(flash_config_nvs_handle, key, out_value);
}

esp_err_t flash_config_init()
{

	if (flash_config_initiated)
	{
		ESP_LOGD(TAG, "flash_config already initiated!");
		return ESP_OK;
	}

	if (!file_system_mounted)
	{
		initialize_filesystem();
	}

	config_has_changed = xSemaphoreCreateBinary();

	flash_config_initiated = true;
	read_pmstep_config_file(&current_config); //Check if there is a config file and loads the data into current_config struct.
											  //If not, it creates a file with default values and loads them into current_config struct.

	xTaskCreatePinnedToCore(config_manager_task, "flash_config", 8192, NULL, 10, NULL, 0);

	ESP_LOGI(TAG, "flash_config initiated!");
	return ESP_OK;
}
esp_err_t flash_config_write_angle_cal(float *lookup, int size_floats)
{

	//size is in number of floats
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}

	if (write_angle_calib_file(lookup, size_floats) != ESP_OK)
	{
		ESP_LOGE(TAG, "Calibration file failed to be written to flash");
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Calibration file successfully written to flash");
	return ESP_OK;
}

esp_err_t flash_config_get_angle_cal_size(int *size_floats)
{
	// If the filesystem isn't mounted we can't write
	if (!file_system_mounted)
	{
		ESP_LOGE(TAG, "read failed: Filesystem not mounted");
		return ESP_FAIL;
	}

	long cal_size_bytes;
	FILE *angle_cal_file = fopen(file_system_angle_cal_file, "rb");
	int errnum;

	if (angle_cal_file == NULL)
	{
		errnum = errno;
		if (errno == ENOENT)
		{
			ESP_LOGW(TAG, "Angle calibration file does not exist. Run sensor_calibration_calibrate_angle command to get a valid angle calibration");
			return ESP_ERR_NOT_FOUND;
		}
		else
		{
			ESP_LOGE(TAG, "Error opening angle calibration file: %s", strerror(errnum));
			return ESP_FAIL;
		}
	}
	else
	{
		fseek(angle_cal_file, 0, SEEK_END);		// seek to end of file
		cal_size_bytes = ftell(angle_cal_file); // get current file pointer
		fseek(angle_cal_file, 0, SEEK_SET);		// seek back to beginning of file

		// success!
		fclose(angle_cal_file);

		*size_floats = (int)(cal_size_bytes / sizeof(float));
		ESP_LOGD(TAG, "Calibration file number of elements: %d, size (bytes): %ld", *size_floats, cal_size_bytes);
		return ESP_OK;
	}
}

esp_err_t flash_config_get_angle_cal(float *lookup, int size_floats)
{
	esp_err_t ret;

	// If the filesystem isn't mounted we can't write
	if (!file_system_mounted)
	{
		ESP_LOGE(TAG, "read failed: Filesystem not mounted");
		return ESP_FAIL;
	}

	FILE *angle_cal_file = fopen(file_system_angle_cal_file, "rb");
	int errnum;

	if (angle_cal_file == NULL)
	{
		errnum = errno;
		if (errno == ENOENT)
		{
			ESP_LOGW(TAG, "Angle calibration file does not exist. Run sensor_calibration_calibrate_angle command to get a valid angle calibration");
			return ESP_ERR_NOT_FOUND;
		}
		else
		{
			ESP_LOGE(TAG, "Error opening angle calibration file: %s", strerror(errnum));
			return ESP_FAIL;
		}
	}
	else
	{
		size_t elements_read = fread(lookup, sizeof(float), size_floats, angle_cal_file);
		if (elements_read == 0)
		{
			ESP_LOGE(TAG, "Failed to read config file!");
			fclose(angle_cal_file);
			return ESP_FAIL;
		}
		// success!
		fclose(angle_cal_file);
		ESP_LOGI(TAG, "Calibration file successfully read to flash");
		return ESP_OK;
	}
}


esp_err_t flash_config_set_motor_params(float km, float L, float R, float i, int Nr)
{

	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	esp_err_t err = ESP_OK;
	err = nvs_open("motor", NVS_READWRITE, &flash_config_nvs_handle);
	ESP_ERROR_CHECK(err);
	err = s_flash_config_nvs_set_float("km", km);
	ESP_ERROR_CHECK(err);
	err = s_flash_config_nvs_set_float("L", L);
	ESP_ERROR_CHECK(err);
	err = s_flash_config_nvs_set_float("R", R);
	ESP_ERROR_CHECK(err);
	err = s_flash_config_nvs_set_float("i", i);
	ESP_ERROR_CHECK(err);
	err = s_flash_config_nvs_set_int("Nr",Nr);
	ESP_ERROR_CHECK(err);
	err = nvs_commit(flash_config_nvs_handle);
	ESP_ERROR_CHECK(err);
	nvs_close(flash_config_nvs_handle);

	current_config.km = km;
	current_config.L = L;
	current_config.R = R;
	current_config.i = i;
	current_config.Nr = Nr;

	invoke_callbacks();
	return ESP_OK;
}

esp_err_t flash_config_get_motor_params(float *km, float *L, float *R, float *i, int *Nr)
{

	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	esp_err_t err;
	err = nvs_open("motor", NVS_READONLY, &flash_config_nvs_handle);
	if(ESP_ERR_NVS_NOT_FOUND == err)
		return err;
	else
		ESP_ERROR_CHECK(err);
	if (km != NULL)
	{
		err = s_flash_config_nvs_get_float("km",km);
		if(err == ESP_ERR_NVS_NOT_FOUND)
			return err;
		else 
			ESP_ERROR_CHECK(err);
	}
	if (L != NULL)
	{
		err = s_flash_config_nvs_get_float("L",L);
		if(err == ESP_ERR_NVS_NOT_FOUND)
			return err;
		else 
			ESP_ERROR_CHECK(err);
	}
	if (R != NULL)
	{
		err = s_flash_config_nvs_get_float("R",R);
		if(err == ESP_ERR_NVS_NOT_FOUND)
			return err;
		else 
			ESP_ERROR_CHECK(err);
	}
	if (i != NULL)
	{
		err = s_flash_config_nvs_get_float("i",i);
		if(err == ESP_ERR_NVS_NOT_FOUND)
			return err;
		else 
			ESP_ERROR_CHECK(err);
	}
	if (Nr != NULL)
	{
		err = s_flash_config_nvs_get_int("Nr",Nr);
		if(err == ESP_ERR_NVS_NOT_FOUND)
			return err;
		else 
			ESP_ERROR_CHECK(err);
	}
	nvs_close(flash_config_nvs_handle);
	return err;
}

static esp_err_t s_flash_config_set_pid(float kp, float ki, float kd)
{
	fix16_t kp_fixed = fix16_from_float(kp);
	fix16_t ki_fixed = fix16_from_float(ki);
	fix16_t kd_fixed = fix16_from_float(kd);
	esp_err_t err;
	err = nvs_set_i32(flash_config_nvs_handle, "kp", (int32_t)kp_fixed);
	ESP_ERROR_CHECK(err);
	err = nvs_set_i32(flash_config_nvs_handle, "ki", (int32_t)ki_fixed);
	ESP_ERROR_CHECK(err);
	err = nvs_set_i32(flash_config_nvs_handle, "kd", (int32_t)kd_fixed);
	ESP_ERROR_CHECK(err);
	err = nvs_commit(flash_config_nvs_handle);
	ESP_ERROR_CHECK(err);
	return err;
}

static esp_err_t s_flash_config_get_pid(float *kp, float *ki, float *kd)
{
	fix16_t out_val = 0;

	esp_err_t err = ESP_OK;
	if (kp != NULL)
	{
		err = nvs_get_i32(flash_config_nvs_handle, "kp", (int32_t*) &out_val);
		if(err == ESP_ERR_NVS_NOT_FOUND)
			return err;
		else 
		{
			ESP_ERROR_CHECK(err);
			*kp = fix16_to_float(out_val);
		}
	}
	if (ki != NULL)
	{
		err = nvs_get_i32(flash_config_nvs_handle, "ki", (int32_t*) &out_val);
		if(err == ESP_ERR_NVS_NOT_FOUND)
			return err;
		else 
		{
			ESP_ERROR_CHECK(err);
			*ki = fix16_to_float(out_val);
		}
	}
	if (kd != NULL)
	{
		err = nvs_get_i32(flash_config_nvs_handle, "kd", (int32_t*) &out_val);
		if(err == ESP_ERR_NVS_NOT_FOUND)
			return err;
		else 
		{		
			ESP_ERROR_CHECK(err);
			*kd = fix16_to_float(out_val);
		}
	}
	return err;
}


esp_err_t flash_config_set_position_pid(float kp, float ki, float kd)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	esp_err_t err;

	err = nvs_open("position", NVS_READWRITE, &flash_config_nvs_handle);
	ESP_ERROR_CHECK(err);
	s_flash_config_set_pid(kp,ki,kd);
	nvs_close(flash_config_nvs_handle);

	current_config.kp_pos = kp;
	current_config.ki_pos = ki;
	current_config.kd_pos = kd;

	invoke_callbacks();
	return ESP_OK;
}
esp_err_t flash_config_get_position_pid(float *kp, float *ki, float *kd)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	esp_err_t err = ESP_OK;
	err = nvs_open("position", NVS_READONLY, &flash_config_nvs_handle);
	ESP_ERROR_CHECK(err);
	err = s_flash_config_get_pid(kp, ki, kd);
	nvs_close(flash_config_nvs_handle);

	return err;
}

esp_err_t flash_config_set_speed_pid(float kp, float ki, float kd)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	esp_err_t err;

	err = nvs_open("speed", NVS_READWRITE, &flash_config_nvs_handle);
	ESP_ERROR_CHECK(err);
	s_flash_config_set_pid(kp,ki,kd);
	nvs_close(flash_config_nvs_handle);

	current_config.kp_spd = kp;
	current_config.ki_spd = ki;
	current_config.kd_spd = kd;

	invoke_callbacks();

	return ESP_OK;
}
esp_err_t flash_config_get_speed_pid(float *kp, float *ki, float *kd)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	esp_err_t err = ESP_OK;
	err = nvs_open("speed", NVS_READONLY, &flash_config_nvs_handle);
	ESP_ERROR_CHECK(err);
	err = s_flash_config_get_pid(kp, ki, kd);
	nvs_close(flash_config_nvs_handle);

	return ESP_OK;
}

esp_err_t flash_config_set_torque_pid(float kp, float ki, float kd)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	esp_err_t err;

	err = nvs_open("torque", NVS_READWRITE, &flash_config_nvs_handle);
	ESP_ERROR_CHECK(err);
	s_flash_config_set_pid(kp,ki,kd);
	nvs_close(flash_config_nvs_handle);

	current_config.kp_trq = kp;
	current_config.ki_trq = ki;
	current_config.kd_trq = kd;

	invoke_callbacks();

	return ESP_OK;
}

esp_err_t flash_config_get_torque_pid(float *kp, float *ki, float *kd)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	esp_err_t err = ESP_OK;
	err = nvs_open("torque", NVS_READONLY, &flash_config_nvs_handle);
	ESP_ERROR_CHECK(err);
	err = s_flash_config_get_pid(kp, ki, kd);
	nvs_close(flash_config_nvs_handle);

	return ESP_OK;
}


esp_err_t flash_config_set_voltage_divider_quotient(float quotient)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	current_config.voltage_divider_quotient = quotient;

	after_set();

	return ESP_OK;
}
esp_err_t flash_config_get_voltage_divider_quotient(float *quotient)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	if (quotient == NULL)
		return;
	*quotient = current_config.voltage_divider_quotient;
	if (*quotient == 0.0)
	{ // Qutient not initiated, set a default.
		flash_config_set_voltage_divider_quotient(VOLTAGE_DIVIDER_QUOTIENT);
		*quotient = current_config.voltage_divider_quotient;
	}
	return ESP_OK;
}
esp_err_t flash_config_set_speed_and_torque(float hold_torque, float max_torque, float max_speed)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	current_config.hold_torque = hold_torque;
	current_config.max_torque = max_torque;
	current_config.max_speed = max_speed;

	after_set();

	return ESP_OK;
}
esp_err_t flash_config_get_speed_and_torque(float *hold_torque, float *max_torque, float *max_speed)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	if (hold_torque != NULL)
		*hold_torque = current_config.hold_torque;
	if (max_torque != NULL)
		*max_torque = current_config.max_torque;
	if (max_speed != NULL)
		*max_speed = current_config.max_speed;

	return ESP_OK;
}
int flash_config_register_callback(pmstep_config_changed_callback_t callback)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	for (int i = 0; i < PMSTEP_CONFIG_MAX_REGISTERED_CALLBACKS; i++)
	{
		if (!callback_handles[i].enabled)
		{
			callback_handles[i].callback = callback;
			callback_handles[i].enabled = 1;
			return i;
		}
	}

	return -1; // Max num callbacks already registered
}
esp_err_t flash_config_unregister_callback(int callback_id)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	if (callback_id >= 0 && callback_id < PMSTEP_CONFIG_MAX_REGISTERED_CALLBACKS)
	{
		callback_handles[callback_id].enabled = 0;
	}
	else
	{
		ESP_LOGE(__func__, "Invalid callback ID");
	}

	return ESP_OK;
}
esp_err_t flash_config_set_cli_flag(int *flag)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}
	// If the filesystem isn't mounted we can't write
	if (!file_system_mounted)
	{
		ESP_LOGE(TAG, "write failed: Filesystem not mounted");
		return ESP_FAIL;
	}

	FILE *cli_file = fopen(file_system_cli_file, "wb");

	if (cli_file == NULL)
	{
		ESP_LOGE(TAG, "Failed to open cli file!");
		return ESP_FAIL;
	}

	// Write the config to the file
	size_t written = fwrite(flag, 1, sizeof(int), cli_file);
	fflush(cli_file);
	if (!written)
	{
		ESP_LOGE(TAG, "Failed to write to cli file!");
		fclose(cli_file);
		return ESP_FAIL;
	}
	else if (written < sizeof(int))
	{
		ESP_LOGE(TAG, "fwrite did not write all data. Written: %d, expected: %d", written, sizeof(int));
		fclose(cli_file);
		return ESP_FAIL;
	}

	// Success!
	fclose(cli_file);
	return ESP_OK;
}
esp_err_t flash_config_get_cli_flag(int *flag)
{
	if (!flash_config_initiated)
	{
		ESP_LOGE(TAG, "flash_config not initiated! Init first");
		return ESP_FAIL;
	}

	// If the filesystem isn't mounted we can't write
	if (!file_system_mounted)
	{
		ESP_LOGE(TAG, "write failed: Filesystem not mounted");
		return ESP_FAIL;
	}

	FILE *cli_file = fopen(file_system_cli_file, "rb");

	if (cli_file == NULL)
	{

		if (errno == ENOENT)
		{
			ESP_LOGW(TAG, "CLI file does not exist. Creating with default values...");
			*flag = 0;
			flash_config_set_cli_flag(flag);
		}
		else
		{
			return ESP_OK;
		}
	}
	else
	{

		//No fopen here?? flash_config_set_cli_flag() closes the file!
		/*
		size_t bytes_read = fread(flag, 1, sizeof(int), cli_file);
		
		if (bytes_read == 0){
			ESP_LOGE(TAG,"Failed to read cli file!");
			*flag = 0;
			fclose(cli_file);
			return ESP_FAIL;
		}
		else if (bytes_read < sizeof(int)){
			ESP_LOGE(TAG, "fread did not read all data. Written: %d, expected: %d",bytes_read,sizeof(int));
			fclose(cli_file);
			return ESP_FAIL;
		}
		*/

		size_t elements_read = fread(flag, sizeof(int), 1, cli_file);

		if (elements_read == 0)
		{
			ESP_LOGE(TAG, "Failed to read cli file!");
			*flag = 0;
			fclose(cli_file);
			return ESP_FAIL;
		}

		// success!
		fclose(cli_file);
	}
	return ESP_OK;
}

esp_err_t flash_config_nvs_set_float(const char *namespace, const char *key, float val)
{
	esp_err_t err;
	err = nvs_open(namespace, NVS_READWRITE, &flash_config_nvs_handle);
	if(ESP_OK != err) {return err;} 
	err = s_flash_config_nvs_set_float(key, val);
	if(ESP_OK != err) {nvs_close(flash_config_nvs_handle); return err;} 
	err = nvs_commit(flash_config_nvs_handle);
	nvs_close(flash_config_nvs_handle);
	return err;
}

esp_err_t flash_config_nvs_get_float(const char *namespace, const char *key, float *out_value)
{
	esp_err_t err;
	err = nvs_open(namespace, NVS_READONLY, &flash_config_nvs_handle);
	if(ESP_OK != err) {return err;} 
	err = s_flash_config_nvs_get_float(key, out_value);
	nvs_close(flash_config_nvs_handle);
	return err;
}

esp_err_t flash_config_nvs_set_int(const char *namespace, const char *key, int32_t val)
{
	esp_err_t err;
	err = nvs_open(namespace, NVS_READWRITE, &flash_config_nvs_handle);
	if(ESP_OK != err) {return err;} 
	err = s_flash_config_nvs_set_int(key, val);
	if(ESP_OK != err) {nvs_close(flash_config_nvs_handle); return err;} 
	err = nvs_commit(flash_config_nvs_handle);
	nvs_close(flash_config_nvs_handle);
	return err;
}

esp_err_t flash_config_nvs_get_int(const char *namespace, const char *key, int32_t *out_value)
{
	esp_err_t err;
	err = nvs_open(namespace, NVS_READONLY, &flash_config_nvs_handle);
	if(ESP_OK != err) {return err;} 
	err = s_flash_config_nvs_get_int(key, out_value);
	nvs_close(flash_config_nvs_handle);
	return err;
}

esp_err_t flash_config_nvs_set_str(const char *namespace, const char *key, const char *str)
{
	esp_err_t err;
	err = nvs_open(namespace, NVS_READWRITE, &flash_config_nvs_handle);
	if(ESP_OK != err) {return err;}
	err = nvs_set_str(flash_config_nvs_handle, key, str);
	nvs_close(flash_config_nvs_handle);
	return err;
}

esp_err_t flash_config_nvs_get_str(const char *namespace, const char *key, const char *out_str, size_t *length)
{
	esp_err_t err;
	err = nvs_open(namespace, NVS_READONLY, &flash_config_nvs_handle);
	if(ESP_OK != err) {return err;}
	err = nvs_get_str(flash_config_nvs_handle, key, out_str, length);
	nvs_close(flash_config_nvs_handle);
	return err;
}

esp_err_t pmstep_config_dump(pmstep_config_t *c)
{

	if (c == NULL)
	{
		c = &current_config;
	}

	printf("==== Motor Params ====\n");
	printf("km = %f\nR = %f\nL = %f\nNr = %d\n", c->km, c->R, c->L, c->Nr);
	printf("==== Motor Limtis ====\n");
	printf("HoldTorque = %fNm\nMaxTorque = %fNm\nMaxSpeed = %frad/s\n", c->hold_torque, c->max_torque, c->max_speed);
	printf("==== PID Parameters ====\n");
	printf(" == Poistion ==\n");
	printf(" kp = %f\n ki = %f\n kd = %f\n", c->kp_pos, c->ki_pos, c->kd_pos);
	printf(" == Speed ==\n");
	printf(" kp = %f\n ki = %f\n kd = %f\n", c->kp_spd, c->ki_spd, c->kd_spd);
	printf("==== HBridge ====\n");
	printf("Voltage divider quotient = %f\n", c->voltage_divider_quotient);

	return ESP_OK;
}
