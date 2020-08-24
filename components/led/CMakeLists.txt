set(COMPONENT_SRCS led.cpp)
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES FastLED-idf spi_flash)

register_component()
