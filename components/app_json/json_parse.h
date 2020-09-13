#ifdef __cplusplus
extern "C" {
#endif

#ifndef JASON_PARSE_H
#define JASON_PARSE_H

int json_parse_color(const char* const string, uint8_t *hue, uint8_t *saturation, uint8_t *value);

#endif //JASON_PARSE_H

#ifdef __cplusplus
}
#endif