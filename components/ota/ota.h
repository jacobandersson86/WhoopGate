#ifdef __cplusplus
extern "C" {
#endif

#ifndef OTA_H
#define OTA_H

void ota_init();
void ota_print_current_firmware_version();
void ota_begin_firmare_update();

#endif //OTA_H

#ifdef __cplusplus
}
#endif