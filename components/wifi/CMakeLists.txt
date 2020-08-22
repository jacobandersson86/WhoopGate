set(COMPONENT_SRCS wifi.c cmd_wifi.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES nvs_flash freertos)

register_component()
