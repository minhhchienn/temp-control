idf_component_register(SRCS "project1.c"
                    INCLUDE_DIRS ".")

# Add the 'index.html' file to the SPIFFS partition
spiffs_create_partition_image(storage ../spiffs_data FLASH_IN_PROJECT)