#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <tdeck-lib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

const static char *TAG = "WHACKDECK";

td_board_t *Board = NULL;

void app_main(void)
{
  float voltage = .0f;
  td_board_init(&Board);

  while(1){
    voltage = td_battery_read(Board);
    ESP_LOGI(TAG, "Battery level: %.02f ", (float)voltage);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
