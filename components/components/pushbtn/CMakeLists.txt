idf_component_register(SRCS "src/pushbtn.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES "driver" 
                    REQUIRES "esp_timer")

