#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>  // for socket

#include "esp_err.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "esp_random.h"

#include "esp_log.h"

static const char *TAG = "MQTT-AWS";

// Number of attempts to try connect wifi network
int retry_num=0;
char strftime_buf[64];

// FreeRTOS event group to signal when we are connected
//	We need it to signal the WiFi connection establishment 
//	to our main program
static EventGroupHandle_t s_wifi_event_group;
// The event group allows multiple bits for each event, but we only care about two events:
// - we are connected to the AP with an IP
// - we failed to connect after the maximum amount of retries 
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// AWS MQTT server config
const char* host = "mqtts://d089317216m59ztpwfadr-ats.iot.eu-west-1.amazonaws.com:8883";

// xxxxxxxxxx-certificate.pem.crt
const char* certificate_pem_crt = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUWnVmKCvUHx0PzJfo5mlHoGJSg+IwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MTAyNjE2MzQw
M1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPXYo+HpHRQANeGwWeE8
cYWy/OTpQta2rSRPvngkY9mKC7POBoN10EHuGHD0cPpguUovpSALGDh8kaYjVaZk
pKwE7cxPUv+GAVk6HQulhfygQI+xSkUc3SDS8PHVE0RcQM1ayK7AT9VA8rd9GnjB
dVaD0IXlg23tnsGA/c74gkdNNNu1D84CISMDifEJPoutSh5FEyWMet1/0+NbdmRA
whb2HMa5LeUsVV1ojJT4X1g4RghED/QODs9Ux6bxi6ZHrvD2eGBGChG3eINJAfib
YYQ5ZfI5n8YJqQgcm5za9ga0fFlHzbu+cBRbeGNsFsEbWghjBku12d32yGCTM0pz
Nz0CAwEAAaNgMF4wHwYDVR0jBBgwFoAULjlsv9MdfKycqfP1DyHu4/4NetswHQYD
VR0OBBYEFH/wzdC4Yth5+8UPKN1/3Xm0sySfMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCujAMzCCAkP8QndrnVXe3XqvSA
G/moc+djh4vJWFOWhRQslljVxO5CxqEE6FrP44gVH7Si+VgThF6VlThPQ3xs0FK4
MZZk7V25PPLTOs1asfLKKn1gBH3JtoRh+qTQ0j+MW4q0xm1amo/ZriRBK2mBhkrp
359cRlmTVDU/9AVqSgdfy4gFabqpZSfFjQxjcexkCirWoNtKNo75g6+LQXGM5E5k
Ua+94G8zE/mUQz9sj6M5dwPusNZXjZrswxke7rDdv8w26TgoBGcia/mA/ZbjnHfd
BRTxCBrS7B2UV/aFIBbrkGF4JV7YDMiUaFFjhSi6kWZGFXJK9lDHbodrnx5o
-----END CERTIFICATE-----
)EOF";

// xxxxxxxxxx-private.pem.key
const char* private_pem_key = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEA9dij4ekdFAA14bBZ4TxxhbL85OlC1ratJE++eCRj2YoLs84G
g3XQQe4YcPRw+mC5Si+lIAsYOHyRpiNVpmSkrATtzE9S/4YBWTodC6WF/KBAj7FK
RRzdINLw8dUTRFxAzVrIrsBP1UDyt30aeMF1VoPQheWDbe2ewYD9zviCR00027UP
zgIhIwOJ8Qk+i61KHkUTJYx63X/T41t2ZEDCFvYcxrkt5SxVXWiMlPhfWDhGCEQP
9A4Oz1THpvGLpkeu8PZ4YEYKEbd4g0kB+JthhDll8jmfxgmpCBybnNr2BrR8WUfN
u75wFFt4Y2wWwRtaCGMGS7XZ3fbIYJMzSnM3PQIDAQABAoIBACnh/4F0MmT+6C8L
2TQbFQf/B0CEUMO9mV+vSjazlGMyNm4qckehIZqp0nYj3r95DPgLh+kemsC80B9q
s6PGT5uSt2RwOyaXENG/qcUbLOlt77HLNWjy0uVNqoi4kTC7OrsBdWD4GGGvT+pc
oM7jvna/vB2jjTGEIuC6dgL1qVQ2CIhSvdMJ3z36FtBWC3eNPJFkH8A+ESBwnQGI
4HFcuQy+65eeLx0dg12GzQbKh752St/U9NtsXq8STw21tcsm8+RzKT+UVBHzqoz6
zs7oQ5AHX4WMW5R2or0ij7SkFy2AQN5flcWOuz1IKlRnkD2Chkt6zEysGhxgkZx2
HdVz360CgYEA/ZptcYgZV0i1cTTUa57U37graWTrb0/a0CqPw2x3Mxdiu0awzmw0
BEBECnE0az9ENKY4JBqse70YGHTwhV2tQv8M9rrrpBmfoUZQEuEy6arFOGUGC1FY
HAyC1Ge8Vv8WWpEnGzhglOzyh+5VkO5hUeJW0oMgkMPkyhPKPJJpEFsCgYEA+Ctx
/L6oyaw7yAQdQfSl4LhOmjikf7jNc2G2/rI9kZ/ZQpRr6bnvkejiUrt49s2hp3L4
zkkEKP4VMrbnm5876HwmC51KuRo9enQVwqOX1AoWVzcJr6FsNZs/PGo0VsR+x6Jg
Y+MVMwoqmtQEASHtmmlLsnouLgMOn4L41RG+akcCgYEA68qXk4WdOiBqQxWBo4aM
Gm5cPqQMmvQ+WGoaPPMuaRyHOeTiIytLueAe4y6aNFUgj2s0q6z5ThtkueiQcQ/G
NpJ5dTYaPfttBkAf1033Tnbu5B5Z3lAFgh7HzcGdxa1rFU38dDyY4B7m6SfZ+uUJ
K18sppIwxyi8In7//A2NDh0CgYEAxcTBkvxRwF3cSjpVIpGXe20FXsuS9CHOwIt3
bKOYgyuS1Qc4tEszyuB/NBUoge4/TBVIiAQSJ6TMOd0e15rfABenrgvMV5S8t5A5
IyKBxT3ArgIzWt5WQKYWj8vHJ/pxWiBhlliKNWF9sGMTSd/C3RHafEH3+T0Ac0fJ
iusYBtMCgYEAlf+ogYwBFI72RxLnfCGoCmY26XQVFOpHPKX56XqiEOVs2YUb0eVx
dPSZy8BzEUTxTqCyNrzcGlRgnunVNqCDlbHcKOIM3BjkDRt8po6+Y9fw1RSlB4El
45PfzHernz0l8IVwxEBZcka/JwvLktc+MfwuO+SKIiNPmXlQO9qFsBY=
-----END RSA PRIVATE KEY-----
)EOF";

// The AWS IoT CA Certificate from: 
// https://docs.aws.amazon.com/iot/latest/developerguide/managing-device-certs.html#server-authentication

const char* rootCA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

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

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "/read/esp32-001", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
//        msg_id = esp_mqtt_client_publish(client, "/write/esp32-001", "data", 0, 0, 0);
//        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    int msg_id;
	// For data to send
	int temp;
	char sensorData[256];

	// For time
	time_t now;
	struct tm timeinfo;

  	const esp_mqtt_client_config_t mqtt_cfg = {
    	.broker.address.uri = host,
    	.broker.verification.certificate = rootCA,
    	.credentials = {
      		.authentication = {
        		.certificate = (const char *)certificate_pem_crt,
        		.key = (const char *)private_pem_key,
      		},
    	}
  	};

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    // The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);


	while(true) {
		time(&now);
		// Set timezone to CET
		setenv("TZ", "CET-1", 1);
		tzset();
		
		localtime_r(&now, &timeinfo);
		strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	
	
		// Prepase JSON with data
	  	temp = (int)(((double)esp_random()/4294967296.0)*10.0);
	
		sprintf(sensorData,  "{\"time\":\"%s\",\"temp\":%d}", strftime_buf, temp);
		msg_id = esp_mqtt_client_publish(client, "/write/esp32-001", sensorData, 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
		sleep(10);
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

   	mqtt_app_start();
}
