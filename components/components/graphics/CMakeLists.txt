set(srcs "src/fonts.c" "src/graphics.c" "src/ssd1306patch.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "include" "fonts"
                       REQUIRES "driver" "esp_lcd")
