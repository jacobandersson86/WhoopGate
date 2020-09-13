#ifdef __cplusplus
extern "C" {
#endif

#ifndef JSON_CREATE_H
#define JSON_CREATE_H
#include "cJSON.h"

/**
 * @brief Deletes a json object previously created by json_create_xx
 * 
 * @param object 
 */
void json_create_delete(cJSON *object);

/**
 * @brief Creates a '\0' terminated string. The allocatied string must
 * be deallocated by application after use. The json object passed to 
 * this function will be deallocated. 
 * 
 * @param object 
 * @return char* 
 */
char *json_create_string(cJSON * object);

/**
 * @brief Creates a color object. Takes hue, saturation and value as arguments.
 * Returns an allocated cJSON object on succes. Deallocation must be handled by
 * applicaiton. All the values must be in the range of 0-255.
 * 
 * @param hue 
 * @param saturation 
 * @param value 
 * @return cJSON* 
 */
cJSON *json_create_color(uint8_t hue, uint8_t saturation, uint8_t value);

#endif //JSON_CREATE_H

#ifdef __cplusplus
}
#endif