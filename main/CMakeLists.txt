set(COMPONENT_SRCS main.cpp cmd_app.c)
set(COMPONENT_REQUIRES wifi app_mqtt led ota nutsbolts app_json)

register_component()

