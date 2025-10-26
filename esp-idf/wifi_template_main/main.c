#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>  // for socket

#include "esp_err.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "esp_log.h"

int retry_num=0;

static const char *TAG = "MQTT-AWS";


// FreeRTOS event group to signal when we are connected
//	We need it to signal the WiFi connection establishment 
//	to our main program
static EventGroupHandle_t s_wifi_event_group;
// The event group allows multiple bits for each event, but we only care about two events:
// - we are connected to the AP with an IP
// - we failed to connect after the maximum amount of retries 
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,void *event_data) {
	if(event_id == WIFI_EVENT_STA_START) {
  		printf("WIFI CONNECTING....\n");
	} else if (event_id == WIFI_EVENT_STA_CONNECTED) {
  		printf("WiFi CONNECTED\n");
	} else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
  		printf("WiFi lost connection\n");
  		if(retry_num<5) {
			esp_wifi_connect();
			retry_num++;
			printf("Retrying to Connect...\n");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
	} else if (event_id == IP_EVENT_STA_GOT_IP) {
  		printf("Wifi got IP...\n\n");
 		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("My IP:" IPSTR "\n", IP2STR(&event->ip_info.ip));
		// Send the signal the WiFi connection is OK
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

void setup(void) {
	printf("WiFi setup start\n");
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init()); // ESP network interface init
    ESP_ERROR_CHECK(esp_event_loop_create_default());     // event loop  
    esp_netif_create_default_wifi_sta(); // Prepare TCP/IP stack structure for WiFi client station

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // Default WiFi configuration structure
	ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // Init default values

	// Setup SSID and PASS for our WiFi AP
	wifi_config_t wifi_config = {
        .sta = {
            .ssid = "iotagh",
            .password = "IoTiISW123",
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	esp_err_t status = esp_wifi_connect();
	if (status == ESP_OK) {
		printf("WiFI connected\n");
	} else {
		printf("WiFI connection failed: %s\n", esp_err_to_name(status));
	}	
}

void app_main(void)
{
//Initialize NVS to store WiFi AP configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      nvs_flash_erase();
      nvs_flash_init();
    }
	ESP_ERROR_CHECK(ret);

	// Setup WiFi connection in STA mode
	setup();
	
	
    // Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
    // number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) 
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    // xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
    // happened. 
    if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGE(TAG, "WiFi connection ready to use");
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to WiFi");
		return;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
		return;
    }

	ESP_LOGI(TAG, "Try to connect server\n");

    while (true) {
		sleep(10);
    }
}
