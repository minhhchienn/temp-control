#include <stdio.h>
#include <esp_http_server.h>
#include <http_server.h>
#include <esp_err.h>
#include <esp_log.h>
#include <ds18b20.h>
#include <relay.h>
#include <stdint.h>


esp_err_t serve_index_html(httpd_req_t *req) {
    // Mở tệp "index.html" từ SPIFFS
    const char *path = "/spiffs/index.html";
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        // Xử lý lỗi khi mở tệp
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to open file");
        return ESP_FAIL;
    }

    // Gửi nội dung của tệp "index.html" về máy khách
    char buffer[1024];
    size_t read_bytes;
    do {
        read_bytes = fread(buffer, 1, sizeof(buffer), file);
        if (read_bytes > 0) {
            httpd_resp_send_chunk(req, buffer, read_bytes);
        }
    } while (read_bytes > 0);

    // Đóng tệp
    fclose(file);

    // Kết thúc gửi nội dung
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/* Xử lý yêu cầu GET cho endpoint "/temperature" */
esp_err_t temperature_get_handler(httpd_req_t *req)
{
    char response_data[50];
    float temp = ds18b20_get_temp();
    snprintf(response_data, sizeof(response_data), "{\"temperature\": %.2f}", temp);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_data, strlen(response_data));
    return ESP_OK;
}

/* Xử lý yêu cầu GET cho endpoint "/relay1" */
esp_err_t relay1_get_handler(httpd_req_t *req)
{
    char response_data[20];
    snprintf(response_data, sizeof(response_data), "{\"relay1\": %s}", relay1_state ? "true" : "false");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_data, strlen(response_data));
    return ESP_OK;
}

/* Xử lý yêu cầu GET cho endpoint "/relay2" */
esp_err_t relay2_get_handler(httpd_req_t *req)
{
    char response_data[20];
    snprintf(response_data, sizeof(response_data), "{\"relay2\": %s}", relay2_state ? "true" : "false");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_data, strlen(response_data));
    return ESP_OK;
}

/* Xử lý yêu cầu POST cho endpoint "/toggle-relay1" */
esp_err_t toggle_relay1_post_handler(httpd_req_t *req)
{
    // Đảo trạng thái của Relay 1
    toggle_relay(1);

    // Gửi trạng thái mới của Relay 1 dưới dạng JSON
    char response_data[20];
    snprintf(response_data, sizeof(response_data), "{\"relay1\": %s}", relay1_state ? "true" : "false");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_data, strlen(response_data));
    return ESP_OK;
}

/* Xử lý yêu cầu POST cho endpoint "/toggle-relay2" */
esp_err_t toggle_relay2_post_handler(httpd_req_t *req)
{
    // Đảo trạng thái của Relay 2
    toggle_relay(2);

    // Gửi trạng thái mới của Relay 2 dưới dạng JSON
    char response_data[20];
    snprintf(response_data, sizeof(response_data), "{\"relay2\": %s}", relay2_state ? "true" : "false");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_data, strlen(response_data));
    return ESP_OK;
}

void start_http_server(){
    // Khởi tạo máy chủ HTTP
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    if (httpd_start(&server, &config) == ESP_OK) {
        // Đăng ký xử lý yêu cầu GET cho tệp "index.html"
        httpd_uri_t uri_get = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = serve_index_html,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &uri_get);

        /* Đăng ký xử lý yêu cầu GET cho endpoint "/temperature" */
        httpd_uri_t temperature_uri = {
            .uri = "/temperature",
            .method = HTTP_GET,
            .handler = temperature_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &temperature_uri);

        httpd_uri_t relay1_uri = {
            .uri = "/relay1",
            .method = HTTP_GET,
            .handler = relay1_get_handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(server, &relay1_uri);

        httpd_uri_t relay2_uri = {
            .uri = "/relay2",
            .method = HTTP_GET,
            .handler = relay2_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &relay2_uri);

        /* Đăng ký xử lý yêu cầu POST cho các endpoint */
        httpd_uri_t toggle_relay1_uri = {
            .uri = "/toggle-relay1",
            .method = HTTP_POST,
            .handler = toggle_relay1_post_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &toggle_relay1_uri);

        httpd_uri_t toggle_relay2_uri = {
            .uri = "/toggle-relay2",
            .method = HTTP_POST,
            .handler = toggle_relay2_post_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &toggle_relay2_uri);
    }
}