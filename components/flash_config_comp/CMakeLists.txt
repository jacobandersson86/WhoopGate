#Created by VisualGDB. Right-click on the component in Solution Explorer to edit properties using convenient GUI.

set(COMPONENT_SRCS flash_config.c cmd_flash_config.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES libfixmath_comp nvs_flash freertos console fatfs)


register_component()
