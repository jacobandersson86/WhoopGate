set(COMPONENT_SRCS json_create.c json_parse.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES json led)

register_component()
