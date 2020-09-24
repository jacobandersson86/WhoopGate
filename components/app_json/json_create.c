#include <string.h>
#include "cJSON.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_types.h"
#include "json_create.h"
#include "nutsbolts.h"

static const char* TAG = "json_create";

#define JSON_GOTO_END_CHECK(x) if(x == NULL) goto end

void json_create_delete(cJSON *object)
{
    cJSON_Delete(object);
}

char *json_create_string(cJSON * object)
{
    char *string = cJSON_Print(object);
    if(string == NULL)
    {
        ESP_LOGE(TAG, "Failed to create string from object");
    }
    json_create_delete(object);
    return string;
}


cJSON *json_create_color(uint8_t hue, uint8_t saturation, uint8_t value)
{
    cJSON *object = NULL;
    cJSON *colorObject = NULL;
    cJSON *hueItem = NULL;
    cJSON *saturationItem = NULL;
    cJSON *valueItem = NULL;

    object = cJSON_CreateObject();
    JSON_GOTO_END_CHECK(object);

    colorObject = cJSON_CreateObject();
    JSON_GOTO_END_CHECK(colorObject);
    cJSON_AddItemToObject(object, "color", colorObject);

    hueItem = cJSON_CreateNumber(hue);
    JSON_GOTO_END_CHECK(hueItem);
    cJSON_AddItemToObject(colorObject, "hue", hueItem);

    saturationItem = cJSON_CreateNumber(saturation);
    JSON_GOTO_END_CHECK(saturationItem);
    cJSON_AddItemToObject(colorObject, "saturation", saturationItem);

    valueItem = cJSON_CreateNumber(value);
    JSON_GOTO_END_CHECK(valueItem);
    cJSON_AddItemToObject(colorObject, "value", valueItem);

    return object;

end:
    cJSON_Delete(object);
    return NULL;
}

cJSON *json_create_identity()
{
    cJSON *object = NULL;
    cJSON *gateObject = NULL;
    cJSON *idItem = NULL;
    cJSON *colorObjectParent = NULL;
    cJSON *colorObject = NULL;

    object = cJSON_CreateObject();
    JSON_GOTO_END_CHECK(object);

    gateObject = cJSON_CreateObject();
    JSON_GOTO_END_CHECK(gateObject);
    cJSON_AddItemToObject(object, "gate", gateObject);

    uint8_t id[2];
    nutsbolts_get_id(id);
    char id_str[32];
    sprintf(id_str, "gate%02x%02x",id[0],id[1]);
    idItem = cJSON_CreateString(id_str);
    JSON_GOTO_END_CHECK(idItem);
    cJSON_AddItemToObject(gateObject, "id", idItem);

    colorObjectParent = json_create_color(180, 120, 20); // TODO:Might have memory leak!
    colorObject = cJSON_GetObjectItemCaseSensitive(colorObjectParent, "color");
    JSON_GOTO_END_CHECK(colorObject);
    cJSON_AddItemToObject(gateObject, "color", colorObject);

    return object; 

end:
    cJSON_Delete(object);
    return NULL;
}