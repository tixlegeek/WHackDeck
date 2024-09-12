#include <board.h>
#include <esp_err.h>
#include <esp_log.h>
#include <math.h>
#include <stdio.h>
#include <tdeck-lib.h>
// #include <board.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <st7789.h>
const static char *TAG = "WHACKDECK";

td_board_t *Board = NULL;
volatile const uint8_t sin256[] = {
    127, 130, 133, 136, 139, 142, 145, 148, 151, 154, 157, 161, 164, 166, 169,
    172, 175, 178, 181, 184, 187, 189, 192, 195, 197, 200, 202, 205, 207, 210,
    212, 214, 217, 219, 221, 223, 225, 227, 229, 231, 233, 234, 236, 237, 239,
    240, 242, 243, 244, 245, 247, 248, 249, 249, 250, 251, 252, 252, 253, 253,
    253, 254, 254, 254, 254, 254, 254, 254, 253, 253, 253, 252, 252, 251, 250,
    249, 249, 248, 247, 245, 244, 243, 242, 240, 239, 237, 236, 234, 233, 231,
    229, 227, 225, 223, 221, 219, 217, 214, 212, 210, 207, 205, 202, 200, 197,
    195, 192, 189, 187, 184, 181, 178, 175, 172, 169, 166, 164, 161, 157, 154,
    151, 148, 145, 142, 139, 136, 133, 130, 127, 124, 121, 118, 115, 112, 109,
    106, 103, 100, 97,  93,  90,  88,  85,  82,  79,  76,  73,  70,  67,  65,
    62,  59,  57,  54,  52,  49,  47,  44,  42,  40,  37,  35,  33,  31,  29,
    27,  25,  23,  21,  20,  18,  17,  15,  14,  12,  11,  10,  9,   7,   6,
    5,   5,   4,   3,   2,   2,   1,   1,   1,   0,   0,   0,   0,   0,   0,
    0,   1,   1,   1,   2,   2,   3,   4,   5,   5,   6,   7,   9,   10,  11,
    12,  14,  15,  17,  18,  20,  21,  23,  25,  27,  29,  31,  33,  35,  37,
    40,  42,  44,  47,  49,  52,  54,  57,  59,  62,  65,  67,  70,  73,  76,
    79,  82,  85,  88,  90,  93,  97,  100, 103, 106, 109, 112, 115, 118, 121,
    124, 0};
unsigned int iii; /* test variable */
/**************************************************************/

void vTaskSwirl(void *pvParameters) {
  while (true) {
    ESP_LOGI(TAG, "TASK");
    lcdDrawFinish(Board);
  }
}

void app_main(void) {
  float voltage = .0f;
  td_board_init(&Board);

  TaskHandle_t swirltask = NULL;
  xTaskCreate(vTaskSwirl, "Swirl", 4096, NULL, tskIDLE_PRIORITY, &swirltask);
  while (1) {
    td_battery_update(Board);
    ESP_LOGI(TAG, "Battery level: %.02f ", Board->Battery.voltage);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
