#include <stdio.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <esp_err.h>
#include <ds18b20.h>
#include <http_server.h>
#include <wifi_ap.h>
#include <string.h>
#include <relay.h>

static const char *TAG = "SPIFFS Example";

//Khởi tạo bộ nhớ flash
void spiffs_init(){
    esp_err_t ret;

    //Khởi tạo SPIFFS
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount or format SPIFFS");
    }
}

void app_main(void) {
    spiffs_init();
    init_relay();
    wifi_ap_start();
    start_http_server();
}
