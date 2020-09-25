#ifdef __cplusplus
extern "C" {
#endif

#ifndef OTA_H
#define OTA_H

void ota_init();
void ota_print_current_firmware_version();
void ota_begin_firmare_update();
/**
 * @brief Obtains the present fw version.
 * The string is maximum 32 characters long.
 * 
 * @param version pointer to an 32 characters long string
 */
void ota_get_fw_version(char * version);

#endif //OTA_H

#ifdef __cplusplus
}
#endif