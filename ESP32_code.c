/* Try to send data with netconn with LED color
/ Benjamin IOLLER, 2017/12/14
*/
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "mdns.h"

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/netdb.h"                              //add more lib.
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/dns.h"

#include <driver/adc.h>

#define ADC1_ABS_CHANNEL (ADC1_CHANNEL_7)
#define ADC1_COND_CHANNEL (ADC1_CHANNEL_4)
#define ADC1_WLEVEL_CHANNEL (ADC1_CHANNEL_6)

#define DEVICE_IP          "192.168.137.101"
#define DEVICE_GW          "192.168.137.1"
#define DEVICE_NETMASK     "255.255.255.0"
#define MESSAGE "YOLO TCP MESSAGE TO SERVER"
#define TCPServerIP "192.168.111.11"
#define TCPServerPORT 6666

TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;
TaskHandle_t TaskHandle_3;

SemaphoreHandle_t xSemaphore = NULL;
SemaphoreHandle_t wifideadlineSemaphore = NULL;

// Event group for inter-task communication
static EventGroupHandle_t event_group;
const int WIFI_CONNECTED_BIT = BIT0;
//TAG
static const char *TAG="tcp_client";

// Global variable
float absorbance=0.0;
float conductivity=0.0;
float wlevel=0.;
int measureID=0;

// Wifi event handler
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
		
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
		gpio_set_level(CONFIG_LED_G,1);
		gpio_set_level(CONFIG_LED_R,0);
        break;
    
	case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(event_group, WIFI_CONNECTED_BIT);
        break;
    
	case SYSTEM_EVENT_STA_DISCONNECTED:
		xEventGroupClearBits(event_group, WIFI_CONNECTED_BIT);
		gpio_set_level(CONFIG_LED_G,0);
        break;
    
	default:
        break;
    }  
	return ESP_OK;
}

//WiFi delay handler
void Wifi_delay()
{
	//Delay (30s)
	vTaskDelay(5000/portTICK_PERIOD_MS);
	//Try to get the wifideadlineSemaphore
	if(xSemaphoreTake(wifideadlineSemaphore,(TickType_t)40) == pdTRUE)
		{
			xSemaphoreGive(wifideadlineSemaphore);
			vTaskDelete(TaskHandle_3);
		}
		else
		{
			ESP_LOGE(TAG, "Wifi connection time exceeded \n");
			gpio_set_level(CONFIG_LED_G,0);
			gpio_set_level(CONFIG_LED_R,1);
			vTaskDelay(1000/portTICK_PERIOD_MS);
			vTaskDelete(TaskHandle_3);
		}
}

// Measurement
void absorbance_measurment()
{
	float mean=0.0;
	int i=0;
	//Turn measurement LED on (blue)
	gpio_set_level(CONFIG_LED_B,1);
	//small delay
	vTaskDelay(1000/portTICK_PERIOD_MS);
	//turn on LED
	gpio_set_level(CONFIG_LED_ABS,1);
	//measurement (average and set the variable)
	for(i=0; i<100; i++)
	{
		mean=mean+adc1_get_voltage(ADC1_ABS_CHANNEL);
		//printf("Step: %d --> %f\n",i,mean);
	}
	absorbance=(float)mean/i;
	printf("absorbance = %f\n",absorbance);
	//turn off LED
	vTaskDelay(1000/portTICK_PERIOD_MS);
	gpio_set_level(CONFIG_LED_ABS,0);
	//Turn measurement LED OFF (blue)
	gpio_set_level(CONFIG_LED_B,0);
	
}

void conductivity_measurment()
{
	float mean=0.0;
	int i=0;
	//small delay
	vTaskDelay(1000/portTICK_PERIOD_MS);
	//Turn measurement LED on (blue)
	gpio_set_level(CONFIG_LED_B,1);
	//apply tension
	gpio_set_level(CONFIG_33_COND,1);
	//measurement (average and set the variable)
	for(i=0; i<100; i++)
	{
		mean=mean+adc1_get_voltage(ADC1_COND_CHANNEL);
		//printf("Step: %d --> %f\n",i,mean);
	}
	//remove tension
	gpio_set_level(CONFIG_33_COND,0);
	//print result
	conductivity=(float)mean/i;
	printf("Conductivity = %f\n",conductivity);
	//Turn measurement LED OFF (blue)
	gpio_set_level(CONFIG_LED_B,0);
	measureID++;
}

char* messageC(char* buf, size_t max_len )
{
	//Creation of the TCP message
	char id_data[] = "*";
	char absobance_data[] = ":";  
	char A[20];
	char conductivity_data[] = ":";
	char C[20];
	char message[250];
	memset(message,0,251);
	strcat(message, id_data);
	sprintf(message,"%d",measureID);
	strcat(message,absobance_data);
	sprintf(A, "%g", absorbance);
	strcat(message, A);
	strcat(message, conductivity_data);
	sprintf(C, "%g", conductivity);		
	strcat(message,C);
	//printf("message in function: %s\n",message);
	strncpy(buf, message, max_len);
	printf("buf?: %s\n",buf);
	return buf;
}

void wifi_setup()
{
	
	event_group = xEventGroupCreate();
		
	tcpip_adapter_init();

	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	//Fixed the IP:
	tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA); // Don't run a DHCP client
	tcpip_adapter_ip_info_t ipInfo;
	inet_pton(AF_INET, DEVICE_IP, &ipInfo.ip);
	inet_pton(AF_INET, DEVICE_GW, &ipInfo.gw);
	inet_pton(AF_INET, DEVICE_NETMASK, &ipInfo.netmask);
	tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
}

void gpio_setup()
{
	// configure pin as GPIO, output
	gpio_pad_select_gpio(CONFIG_LED_R);
    gpio_set_direction(CONFIG_LED_R, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(CONFIG_LED_G);
    gpio_set_direction(CONFIG_LED_G, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(CONFIG_LED_B);
    gpio_set_direction(CONFIG_LED_B, GPIO_MODE_OUTPUT);
	
	gpio_pad_select_gpio(CONFIG_LED_ABS);
	gpio_set_direction(CONFIG_LED_ABS, GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(CONFIG_33_COND);
	gpio_set_direction(CONFIG_33_COND, GPIO_MODE_OUTPUT);

	// set initial status = OFF
	gpio_set_level(CONFIG_LED_R, 0);
	gpio_set_level(CONFIG_LED_G, 0);
	gpio_set_level(CONFIG_LED_B, 0);
	gpio_set_level(CONFIG_LED_ABS, 0);
	gpio_set_level(CONFIG_33_COND, 0);
	
    // initialize ADC
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_ABS_CHANNEL, ADC_ATTEN_11db);
	adc1_config_channel_atten(ADC1_COND_CHANNEL, ADC_ATTEN_11db);
}

void tcp_client(void *pvParam)
{
	//Take the semaphore :) (en premier)
	xSemaphoreTake(xSemaphore,(TickType_t)10);
	
	//LED handler
	gpio_set_level(CONFIG_LED_B,0);
	
	// Connection to the network:
	ESP_ERROR_CHECK(esp_wifi_start());

	// wait for connection
	xTaskCreate(&Wifi_delay,"Wifi_delay",2048,NULL,5,TaskHandle_3);
	xSemaphoreTake(wifideadlineSemaphore,(TickType_t)10);
	printf("Waiting for connection to the wifi network...\n ");
	xEventGroupWaitBits(event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
	xSemaphoreGive(wifideadlineSemaphore);
	printf("Connected\n\n");
	
	// print the local IP address
	tcpip_adapter_ip_info_t ip_info;
	ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
	printf("IP Address:  %s\n", ip4addr_ntoa(&ip_info.ip));
	printf("Subnet mask: %s\n", ip4addr_ntoa(&ip_info.netmask));
	printf("Gateway:     %s\n", ip4addr_ntoa(&ip_info.gw));	
	
	//Create the socket 
    struct sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_addr.s_addr = inet_addr(TCPServerIP);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons( TCPServerPORT );                           //port number
    int s, r;
    char recv_buf[1024];
	printf("Servor Ip: %s\n",TCPServerIP);
	printf("Servor port: %d\n",TCPServerPORT);
	
    while(1){
		//Connecting
        xEventGroupWaitBits(event_group,WIFI_CONNECTED_BIT,false,true,portMAX_DELAY);
        s = socket(AF_INET, SOCK_STREAM, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.\n");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
			//LED handler
			gpio_set_level(CONFIG_LED_G,0);
			gpio_set_level(CONFIG_LED_R,1);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket\n");
         if(connect(s, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d \n", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
			//LED handler
			gpio_set_level(CONFIG_LED_G,0);
			gpio_set_level(CONFIG_LED_R,1);
            continue;
        }
        ESP_LOGI(TAG, "... connected \n");
		
		//Measurements 
		absorbance_measurment();
		conductivity_measurment();
		
		//Formating the message
		char message[251];
		memset(message,0,251);
		messageC(message,250);
		printf("message: %s",message);
		
        if( write(s , message , strlen(message)) < 0)
        {
            ESP_LOGE(TAG, "... Send failed \n");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");
		ESP_LOGI(TAG, "Waiting for the reply ...");
		//Check if the server received [...]
        bzero(recv_buf, sizeof(recv_buf));
        r = read(s, recv_buf, sizeof(recv_buf)-1);
		printf("Server reply: %s\n", recv_buf);
		if(r<0){
			ESP_LOGI(TAG,"error from server side")
		}
        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);
        vTaskDelay(8000 / portTICK_PERIOD_MS);
		//Release semaphore and die :(
		xSemaphoreGive(xSemaphore);
		//Cut wifi:
		ESP_ERROR_CHECK(esp_wifi_stop());
		ESP_LOGI(TAG, "...tcp_client task closed\n");
		vTaskDelete(TaskHandle_2);
    }
}

void check_input(void *pvParameter)
{
	bool ledState=false;
	while(1)
	{
		printf("Check Input :p\n");
		vTaskDelay(300/portTICK_PERIOD_MS);
		wlevel=((adc1_get_voltage(ADC1_WLEVEL_CHANNEL)*0.985)+217.2)*(3.3/4095);
		printf("Water level (in Volt): %f\n",wlevel);
		//blink the LED
		gpio_set_level(CONFIG_LED_B, !ledState);
		printf("LED test\n");
		vTaskDelay(1000/portTICK_PERIOD_MS);
		ledState=!ledState;
		
		if(wlevel>1.0)
		{
			printf("start the measurement:\n");
			//Release the semaphore / start the task / wait / wait until semaphore is free
			xTaskCreate(&tcp_client,"tcp_client",4096,NULL,5,TaskHandle_2);
			vTaskDelay(500/portTICK_PERIOD_MS);
			if(xSemaphore !=NULL)
			{	
				bool done=false;
				while(done!=true)
				{
					if(xSemaphoreTake(xSemaphore,(TickType_t)40) == pdTRUE)
					{
						done=true;
					}
					else
					{
						printf("Measurement still going\n");
						vTaskDelay(2000/portTICK_PERIOD_MS);
					}
				}
				xSemaphoreGive(xSemaphore);
			}
		}
		
	}
}

void app_main()
{
	// disable the default wifi logging
	esp_log_level_set("wifi", ESP_LOG_NONE);
	
	//Create the semaphore:
	vSemaphoreCreateBinary( xSemaphore );
	vSemaphoreCreateBinary( wifideadlineSemaphore );
	
	//Initialization
	wifi_setup();
	nvs_flash_init();
	gpio_setup();
	
    xTaskCreate(&check_input,"check_input",4048,NULL,5,TaskHandle_1);
}
