
idf_component_register(SRCS "ota_module.c"
                       EMBED_TXTFILES "jsonbin_cacerts.pem" "github_cacerts.pem"
                       INCLUDE_DIRS "include"
                       REQUIRES "esp_https_ota" "freertos" "esp_http_client" "json" "esp_event")
