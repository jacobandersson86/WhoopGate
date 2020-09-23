#ifdef __cplusplus
extern "C" {
#endif

#ifndef JASON_PARSE_H
#define JASON_PARSE_H

/**
 * @brief Takes a string as an argument. Returns 0 if the parsing was unsucessfull and 1 or more
 * if a recognized object was found.
 * 
 * @param string 
 * @return int 
 */
int json_parse(const char* const string);

void json_parse_init();

#endif //JASON_PARSE_H

#ifdef __cplusplus
}
#endif