set(COMPONENT_SRCS main.c cmd_app.c)
set(COMPONENT_ADD_INCLUDEDIRS "")
set(COMPONENT_REQUIRES wifi app_mqtt)

register_component()

