set(COMPONENT_SRCS mqtt.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES freertos mqtt)

register_component()