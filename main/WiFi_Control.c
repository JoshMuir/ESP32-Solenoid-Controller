#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

// CHANGE THIS TO MATCH YOUR OWN NETWORK SETTINGS
#include "secrets.h"
#define WIFI_SSID _SSID
#define WIFI_PASS _PASSWORD

// Add macro definition for MIN
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

// Define GPIO pins for outputs
#define NUM_OUTPUTS 8
#define RELAY_OUTPUT_1 32
#define RELAY_OUTPUT_2 33
#define RELAY_OUTPUT_3 25
#define RELAY_OUTPUT_4 26
#define RELAY_OUTPUT_5 27
#define RELAY_OUTPUT_6 14
#define RELAY_OUTPUT_7 12
#define RELAY_OUTPUT_8 13

static const int output_pins[NUM_OUTPUTS] = {
    RELAY_OUTPUT_1,  // Output 1
    RELAY_OUTPUT_2,  // Output 2
    RELAY_OUTPUT_3, // Output 3
    RELAY_OUTPUT_4, // Output 4
    RELAY_OUTPUT_5, // Output 5
    RELAY_OUTPUT_6, // Output 6
    RELAY_OUTPUT_7, // Output 7
    RELAY_OUTPUT_8  // Output 8
};

static const char *TAG = "gpio_control";
static httpd_handle_t server = NULL;

// Initialize GPIO outputs
static void init_gpio(void)
{
    for (int i = 0; i < NUM_OUTPUTS; i++) {
        gpio_reset_pin(output_pins[i]);
        gpio_set_direction(output_pins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(output_pins[i], 0); // Initialize all outputs to LOW
    }
}

// HTTP GET handler for reading output states
static esp_err_t get_outputs_handler(httpd_req_t *req)
{
    char response[128];
    int len = 0;
    
    len += sprintf(response + len, "{\"outputs\":[");
    for (int i = 0; i < NUM_OUTPUTS; i++) {
        len += sprintf(response + len, "%d%s", 
                      gpio_get_level(output_pins[i]),
                      (i < NUM_OUTPUTS-1) ? "," : "");
    }
    len += sprintf(response + len, "]}");
    
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response, len);
}

// HTTP POST handler for setting output states
static esp_err_t set_output_handler(httpd_req_t *req)
{
    char content[32];
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        return ESP_FAIL;
    }
    content[recv_size] = '\0';

    // Parse output number and state
    int output_num, state;
    if (sscanf(content, "{\"output\":%d,\"state\":%d}", &output_num, &state) == 2) {
        if (output_num >= 0 && output_num < NUM_OUTPUTS) {
            gpio_set_level(output_pins[output_num], state ? 1 : 0);
            const char *resp = "{\"status\":\"ok\"}";
            httpd_resp_set_type(req, "application/json");
            return httpd_resp_send(req, resp, strlen(resp));
        }
    }

    const char *resp = "{\"status\":\"error\",\"message\":\"invalid request\"}";
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, resp, strlen(resp));
}

// HTTP server configuration
static httpd_uri_t uri_get = {
    .uri      = "/outputs",
    .method   = HTTP_GET,
    .handler  = get_outputs_handler,
    .user_ctx = NULL
};

static httpd_uri_t uri_post = {
    .uri      = "/output",
    .method   = HTTP_POST,
    .handler  = set_output_handler,
    .user_ctx = NULL
};

// Start HTTP server
static void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
        ESP_LOGI(TAG, "Web server started");
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi station started, attempting connection...");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WiFi connected!");
                // Start the web server after WiFi connection
                start_webserver();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WiFi disconnected, attempting reconnection...");
                esp_wifi_connect();
                break;
        }
    }
}

void wifi_init_sta(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, 
                                             ESP_EVENT_ANY_ID, 
                                             &wifi_event_handler,
                                             NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void)
{
    // Initialize GPIO pins
    init_gpio();
    
    // Initialize WiFi station
    wifi_init_sta();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
