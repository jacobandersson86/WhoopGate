#include <string.h>
#include "cJSON.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_types.h"
#include "json_parse.h"

static const char* TAG = "json_parse";

int json_parse_color(const char* const string, uint8_t *hue, uint8_t *saturation, uint8_t *value)
{
    const cJSON *object = NULL;
    const cJSON *colorObject = NULL;
    const cJSON *hueItem = NULL;
    const cJSON *saturationItem = NULL;
    const cJSON *valueItem = NULL;
    int status = 0;

    object = cJSON_Parse(string);
    if (object == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        status = 0;
        goto end;
    }
    colorObject = cJSON_GetObjectItemCaseSensitive(object, "color");
    if(!cJSON_IsObject(colorObject))
    {
        status = 0;
        goto end;
    }

    hueItem = cJSON_GetObjectItemCaseSensitive(colorObject, "hue");
    saturationItem = cJSON_GetObjectItemCaseSensitive(colorObject, "saturation");
    valueItem = cJSON_GetObjectItemCaseSensitive(colorObject, "value");

    if( !cJSON_IsNumber(hueItem) || !cJSON_IsNumber(saturationItem) || !cJSON_IsNumber(valueItem))
    {
        status = 0;
        goto end;
    }
    *hue = hueItem->valueint;
    *saturation = saturationItem->valueint;
    *value = valueItem->valueint;
    status = 1;
    goto end;

end:
    cJSON_Delete(object);
    return status;
}