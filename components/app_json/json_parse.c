#include <string.h>
#include "cJSON.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_types.h"
#include "json_parse.h"
#include "led.h"

static const char* TAG = "json_parse";



static int json_parse_color(cJSON *object);


static int json_parse_color(cJSON *object)
{
    const cJSON *hueItem = NULL;
    const cJSON *saturationItem = NULL;
    const cJSON *valueItem = NULL;
    int status = 0;

    hueItem = cJSON_GetObjectItemCaseSensitive(object, "hue");
    saturationItem = cJSON_GetObjectItemCaseSensitive(object, "saturation");
    valueItem = cJSON_GetObjectItemCaseSensitive(object, "value");

    if( !cJSON_IsNumber(hueItem) || !cJSON_IsNumber(saturationItem) || !cJSON_IsNumber(valueItem))
    {
        status = 0;
        goto end;
    }
    ledSetColorHSV(hueItem->valueint, saturationItem->valueint, valueItem->valueint);
    status = 1;
    goto end;

end:
    return status;
}

int json_parse(const char* const string)
{
    const cJSON *object = NULL;
    const cJSON *colorObject = NULL;
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
    if(cJSON_IsObject(colorObject))
    {
        status = json_parse_color(colorObject);
    }
    else
    {
        status = 0;
        goto end;
    }

end:
    cJSON_Delete(object);
    return status;
}