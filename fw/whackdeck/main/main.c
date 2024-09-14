#include <board.h>
#include <esp_err.h>
#include <esp_log.h>
#include <math.h>
#include <stdio.h>
#include <tdeck-lib.h>
// #include <board.h>
#include "driver/i2c_master.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <st7789.h>
const static char *TAG = "WHACKDECK";

td_board_t *Board = NULL;

/**************************************************************/

void vTaskMain(void *ctx) {
  td_board_t *Board = (td_board_t *)ctx;
  esp_err_t err = ESP_OK;

  while (true) {
    uint8_t key;
    err = td_keyboard_poll(Board, &key);
    if (err == ESP_OK) {
      ESP_LOGI(TAG, "Key: %c", key);
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void app_main(void) {
  float voltage = .0f;
  td_board_init(&Board);

  TaskHandle_t maintask = NULL;
  xTaskCreate(vTaskMain, "Main", 2048, Board, tskIDLE_PRIORITY, &maintask);
  while (1) {
    td_battery_update(Board);
    ESP_LOGI(TAG, "Battery level: %.02f ", Board->Battery.voltage);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
  
}
