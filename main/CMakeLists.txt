idf_component_register(SRCS "config_menu.c" "config.c" "mqtt_menu.c" "jsmn.c" "rgb_led_menu.c" "wifi.c" "main.c" "prompt.c" "menu.c" "main_menu.c" "wifi_menu.c" "console.c" "filesystem.c" "console_windows.c" "ansi_term.c" "linenoise_lite.c" "ring_buffer.c" "known_networks.c" "known_networks_menu.c" "state_machine.c" "network_manager.c" "network_manager_menu.c" "datastream.c" "datastream_menu.c" "temp_sensor.c" "rgb_led.c" "terrapin.c" "mqtt.c"
                    INCLUDE_DIRS ".")

message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
message("COMPONENT_TARGET = ${COMPONENT_TARGET}")                 
target_compile_definitions(${COMPONENT_TARGET} PRIVATE PROJECT_NAME="${CMAKE_PROJECT_NAME}")
