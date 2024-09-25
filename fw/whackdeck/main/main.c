#include <board.h>
#include <esp_err.h>
#include <esp_log.h>
#include <math.h>
#include <stdio.h>
#include <tdeck-lib.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
// #include <board.h>
#include "driver/i2c_master.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <st7789.h>
const static char *TAG = "WHACKDECK";


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

extern const int16_t pcm_start[] asm("_binary_drama_pcm_start");
extern const int16_t pcm_end[] asm("_binary_drama_pcm_end");

#define SAMPLES 1024
int16_t sndbuffer[SAMPLES] = {0};

void vTaskPlay(void *ctx) {
  td_board_t *Board = (td_board_t *)ctx;

  size_t bytes_written = 0;
  size_t total_sample_size = pcm_end - pcm_start;
  size_t cursor = 0;
  int16_t  *readaddr = NULL;
  while (1) {

    readaddr = (int16_t*)(pcm_start + cursor);

    if ((readaddr+(SAMPLES* sizeof(int16_t))) < pcm_end){
      memcpy(sndbuffer, readaddr , SAMPLES * sizeof(int16_t));
      for(uint16_t i=0;i<SAMPLES; i++){
        sndbuffer[i]=sndbuffer[i]/20;
      }
      ESP_ERROR_CHECK(i2s_channel_write(Board->Speaker->dev, sndbuffer, (SAMPLES * sizeof(int16_t)),
                        &bytes_written, portMAX_DELAY));
      ESP_LOGI(TAG, "Bytes written:%d", (int)bytes_written);
      cursor += bytes_written>>1;
      bytes_written=0;
    } else {
      break;
    }

    vTaskDelay(1);
  }
  vTaskDelete(NULL);
}

void app_main(void) {
  float voltage = .0f;
  td_board_t *Board = NULL;

  esp_err_t err = td_board_init(&Board, INIT_SDCARD|INIT_DISPLAY|INIT_KEYBOARD|INIT_TRACKBALL|INIT_SPEAKER|INIT_BATTERY);
  if(err != ESP_OK){
    ESP_LOGE(TAG, "Initialisation failed.");
    while(1){
      ESP_LOGE(TAG, "x");
      vTaskDelay(100);
    }
  }

  TaskHandle_t maintask = NULL;
  TaskHandle_t refreshtask = NULL;

  ESP_ERROR_CHECK(i2s_channel_enable(Board->Speaker->dev));
  ESP_LOGI(TAG, "Battery level: %.02f ", Board->Battery->voltage);

  td_battery_update(Board);
  xTaskCreate(vTaskPlay, "TaskPlay", 4096, Board, tskIDLE_PRIORITY, &maintask);
  xTaskCreate(vTaskMain, "Main", 2048, Board, tskIDLE_PRIORITY, &maintask);
  while (1) {
    td_battery_update(Board);
    ESP_LOGI(TAG, "Battery level: %.02f ", Board->Battery->voltage);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }

}
