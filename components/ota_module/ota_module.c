#include <stdio.h>
#include "ota_module.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"

#include "cJSON.h"

#define USE_HTTPS 1

#ifdef USE_HTTPS
    extern const uint8_t jsonbin_cacerts_pem_start[] asm("_binary_jsonbin_cacerts_pem_start");
    extern const uint8_t jsonbin_cacerts_pem_end[]   asm("_binary_jsonbin_cacerts_pem_end");
#else
    
#endif

extern double version; // Global variable to hold the firmware version

const char* TAG = "OTA_MODULE";

char *ota_url;
char rcv_buffer[2056];
char *ota_master_key; 
char *ota_access_key;
uint8_t is_master_key_set = 0; // Flag to check if master key is set
uint8_t is_access_key_set = 0; // Flag to check if access key is
char * ota_firmware_bin; // Variable to hold the firmware binary URL
double version_read = 0.0; // Variable to hold the read version from JSON


void ota_set_url(const char *url)
{
    // Set the OTA URL for the firmware update
    ota_url = url;
    
}

void ota_set_master_key(const char *key)
{
    // Set the access key for OTA authentication
    // This function can be used to set an access key if required by the OTA server
    ota_master_key = key;
    is_master_key_set = 1; // Set the flag to indicate that the master key is set
}

void ota_set_acess_key(const char *key)
{
    // Set the access key for OTA authentication
    // This function can be used to set an access key if required by the OTA server
    ota_access_key = key;
    is_access_key_set = 1; // Set the flag to indicate that the access key is set
}

/* Event handler for catching system events */


esp_err_t __http_event_handler(esp_http_client_event_t* evt) {
    static char *json_buffer = NULL;
    static int json_buffer_len = 0;

    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Grow buffer to hold accumulated data
            json_buffer = realloc(json_buffer, json_buffer_len + evt->data_len + 1);
            if (!json_buffer) {
                ESP_LOGE(TAG, "Allocation failed");
                return ESP_FAIL;
            }

            // Copy new data to the end
            memcpy(json_buffer + json_buffer_len, evt->data, evt->data_len);
            json_buffer_len += evt->data_len;
            json_buffer[json_buffer_len] = '\0'; // Always null-terminate
            break;

        case HTTP_EVENT_ON_FINISH:
            if (!json_buffer) {
                ESP_LOGW(TAG, "No data received to parse");
                break;
            }

            ESP_LOGI(TAG, "data: %s", json_buffer);

            cJSON *root = cJSON_Parse(json_buffer);

            if (!root) {
                ESP_LOGW(TAG, "Received data is NOT valid JSON, discarding");
            } else {
                cJSON *item = NULL;
                cJSON_ArrayForEach(item, root) {
                    if (item->string != NULL) {
                        if (strcmp(item->string, "version") == 0 && cJSON_IsNumber(item)) {
                            version_read = item->valuedouble;
                            ESP_LOGI(TAG, "Firmware version: %.2f", version_read);
                        } else if (strcmp(item->string, "firmware_bin") == 0 && cJSON_IsString(item)) {
                            ota_firmware_bin = item->valuestring;
                            ESP_LOGI(TAG, "Firmware Bin URL found: %s", ota_firmware_bin);
                        }
                    }
                }
                cJSON_Delete(root);
            }

            free(json_buffer);
            json_buffer = NULL;
            json_buffer_len = 0;
            break;

        default:
            break;
    }

    return ESP_OK;
}


void ota_task(void *pvParameters)
{
    esp_err_t ret;
    // Initialize the OTA process
   
    while (1)
    {
        esp_http_client_config_t config = {
            .url = ota_url,
            .event_handler = __http_event_handler, // Set the event handler for OTA events

            #if USE_HTTPS
                .cert_pem = (const char *) jsonbin_cacerts_pem_start,
            #else
                .cert_pem = NULL,
            #endif
                    
            .timeout_ms = 10000, // Set timeout for the OTA process 
        };
        esp_http_client_handle_t client = esp_http_client_init(&config);

        if (client == NULL) {
            printf("Failed to initialize HTTP client\n");
            vTaskDelay(pdMS_TO_TICKS(60000));  // Wait 60s before retry
            continue;
        }

        if(is_master_key_set == 1 && ota_master_key != NULL){
            esp_http_client_set_header(client, "X-Master-Key", ota_master_key);
        }
        
        if(is_access_key_set == 1 && ota_access_key != NULL){
            esp_http_client_set_header(client, "X-Access-Key", ota_access_key);
        }
        if(is_master_key_set == 1 || is_access_key_set == 1){
            esp_http_client_set_header(client, "X-Bin-Meta", "false");
        }
        esp_http_client_set_header(client, "Content-Type", "application/json");
        ret = esp_http_client_perform(client);

        if(ret == ESP_OK){
            ret = ESP_FAIL; // default to failure
            if (version_read > version)
            {
                ESP_LOGI(TAG, "New firmware version available: %.2f", version_read);
                ESP_LOGI(TAG, "Firmware Bin URL: %s", ota_firmware_bin);

                const esp_http_client_config_t ota_http_client_config = {
                    .url = ota_firmware_bin,
                    
                    #if USE_HTTPS
                        .cert_pem = (const char *) jsonbin_cacerts_pem_start,
                    #else
                        .cert_pem = NULL,
                    #endif
                    .timeout_ms = 10000};
                
                const esp_https_ota_config_t ota_config = {
                    .http_config = &ota_http_client_config,
                };
                ret = esp_https_ota(&ota_config);

                if (ret == ESP_OK) {
                    ESP_LOGI(TAG, "Firmware update successful, restarting device...");
                    
                } else {
                    ESP_LOGE(TAG, "Firmware update failed: %d", ret);
                }
            }

            else {
                ESP_LOGI(TAG, "No new firmware version available. Current version: %.2f", version);
            }
            
        }
        else {
            ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(ret));
        }

        esp_http_client_cleanup(client);

        if(ret == ESP_OK) {
            esp_restart(); // Restart the device if the update was successful
        }
        vTaskDelay(pdMS_TO_TICKS(600000)); // Delay for 1 second before retrying
    }
}


void ota_start(void){
    // Start the OTA task
    xTaskCreate(ota_task, "ota_task", 8192, NULL, 5, NULL);
    ESP_LOGI(TAG, "OTA task started");
}