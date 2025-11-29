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

#include "driver/i2c_master.h"

int retry_num=0;

static const char *TAG = "I2C";


#define I2C_MASTER_SCL_IO           22        /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21        /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_1 /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          100000	  /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0         /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0         /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       0x3000

// BMP280 register addresses
#define BMP280_I2C_ADDRESS          0x76     // BMP280 address with SD0 connected to GND

#define BMP280_REG_TEMP_MSB   		0xFA
#define BMP280_REG_TEMP_LSB   		0xFB
#define BMP280_REG_TEMP_xLSB  		0xFC
#define BMP280_REG_CTRL_MEAS  		0xF4
#define BMP280_REG_CONFIG     		0xF5
#define BMP280_REG_ID         		0xD0


#define BMP280_TEMP_REG_SIZE  		3		// Number for bytes to read

// For BMP calibration
#define dig_T1_R					0x88
#define dig_T2_R					0x8A
#define dig_T3_R					0x8C

uint16_t dig_T1;
uint16_t dig_T2;
uint16_t dig_T3;

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

/**
 * @brief i2c master initialization
 */
static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BMP280_I2C_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

static esp_err_t bmp280_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t bmp280_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

double bmp280_compensate_T_double(int32_t adc_T) {
    double var1, var2, T;
    var1 = (((double) adc_T) / 16384.0 - ((double) dig_T1) / 1024.0)
            * ((double) dig_T2);
    var2 = ((((double) adc_T) / 131072.0 - ((double) dig_T1) / 8192.0)
            * (((double) adc_T) / 131072.0 - ((double) dig_T1) / 8192.0))
            * ((double) dig_T3);
    T = (var1 + var2) / 5120.0;
    return T;
}

void app_main(void)
{
	// Data buffer
	uint8_t aRxBuffer[BMP280_TEMP_REG_SIZE];
	
	// Temp variable
	uint32_t temp_raw;

	//Initialize NVS to store WiFi AP configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      nvs_flash_erase();
      nvs_flash_init();
    }
	ESP_ERROR_CHECK(ret);

	// Setup WiFi connection in STA mode
	setup();
	
	// Setup I2C devices
	i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
    i2c_master_init(&bus_handle, &dev_handle);
    ESP_LOGI(TAG, "I2C initialized successfully");
    
	
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

	ESP_LOGI(TAG, "Ready to read the date from BMP280\n");

	// Configuration specific for BMP280
    uint8_t osrs_t = 1;             //Temperature oversampling x 1
    uint8_t osrs_p = 1;             //Pressure oversampling x 1
    uint8_t mode = 3;               //Normal mode
    uint8_t t_sb = 5;               //Tstandby 1000ms
    uint8_t filter = 0;             //Filter off
    uint8_t spi3w_en = 0;           //3-wire SPI Disable
    uint8_t ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode;
    uint8_t config_reg = (t_sb << 5) | (filter << 2) | spi3w_en;
    
    //*****************Task4********************
    ESP_ERROR_CHECK(bmp280_register_write_byte(dev_handle, BMP280_REG_CTRL_MEAS, ctrl_meas_reg));
    ESP_ERROR_CHECK(bmp280_register_write_byte(dev_handle, BMP280_REG_CONFIG, config_reg));
    // Calibration data
    ESP_ERROR_CHECK(bmp280_register_read( dev_handle, dig_T1_R, (uint8_t *)(&dig_T1), sizeof(dig_T1)));
    ESP_ERROR_CHECK(bmp280_register_read( dev_handle, dig_T2_R, (uint8_t *)(&dig_T2), sizeof(dig_T2)));
    ESP_ERROR_CHECK(bmp280_register_read( dev_handle, dig_T3_R, (uint8_t *)(&dig_T3), sizeof(dig_T3)));
    ESP_LOGI(TAG, "Calibration data: dig_T1=%d, dig_T2=%d, dig_T3=%d\n", dig_T1, dig_T2, dig_T3);

    while (true) {
		ESP_ERROR_CHECK(bmp280_register_read( dev_handle, BMP280_REG_TEMP_MSB, aRxBuffer, BMP280_TEMP_REG_SIZE));
		
		temp_raw = (aRxBuffer[0] << 12) | (aRxBuffer[1] << 4)
                | (aRxBuffer[2] >> 4);
        ESP_LOGI(TAG, "Temp regs read: %d %d %d\n", aRxBuffer[0], aRxBuffer[1],
                aRxBuffer[2]);
        ESP_LOGI(TAG, "Temp raw: %d\n", temp_raw);
        //*********************Task1**********************
        double temperature = 0.0;
        temperature = bmp280_compensate_T_double(temp_raw);
        ESP_LOGI(TAG, "Temperature [C]: %lf\n", temperature);
		sleep(1);
    }
}
