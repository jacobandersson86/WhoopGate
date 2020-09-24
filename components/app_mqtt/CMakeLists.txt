set(COMPONENT_SRCS mqtt.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES freertos mqtt nutsbolts app_json)

register_component()