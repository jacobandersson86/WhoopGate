set(COMPONENT_SRCS ota.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_EMBED_TXTFILES "../../cert/GlobalSign.pem")
set(COMPONENT_REQUIRES esp_http_client esp_https_ota app_update)

register_component()
