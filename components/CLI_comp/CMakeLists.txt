#Created by VisualGDB. Right-click on the component in Solution Explorer to edit properties using convenient GUI.

set(COMPONENT_SRCS CLI.c cmd_system.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")

set(COMPONENT_REQUIRES fatfs nvs_flash console flash_config_comp)

#register_component()
