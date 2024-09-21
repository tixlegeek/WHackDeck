#include <board.h>
#include <esp_err.h>
#include <esp_log.h>
#include <math.h>
#include <stdio.h>

#define SAMPLE_RATE 16000 // Sampling rate (44.1 kHz)
#define I2S_NUM I2S_NUM_0         // I2S port number
#define WAVE_FREQ_HZ 1000 // Frequency of sine wave (1 kHz)
#define PI 3.14159265
#define SAMPLES 512 // Number of samples for the sine wave buffer

#include <tdeck-lib.h>
// #include <board.h>
#include "driver/i2c_master.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <st7789.h>
const static char *TAG = "WHACKDECK";

td_board_t *Board = NULL;

/**************************************************************/
uint16_t x = BOARD_DISPLAY_HEIGHT / 2;
uint16_t y = BOARD_DISPLAY_WIDTH / 2;
uint8_t speed = 5;

void vTaskMain(void *ctx) {
  td_board_t *Board = (td_board_t *)ctx;
  esp_err_t err = ESP_OK;
  td_trackball_evt_t evt;

  while (true) {
    uint8_t key;
    err = td_keyboard_poll(Board, &key);
    if (err == ESP_OK) {
      ESP_LOGI(TAG, "Key: %c (0x%.02X)", key, key);
    }
    if (xQueueReceive(td_trackball_queue, &evt, portMAX_DELAY)) {
      // printf("GPIO[%"PRIu32"] intr, val: %d\n", evt);
      switch (evt) {
      case EVT_TRACKBALL_UP:
        x += speed;
        break;
      case EVT_TRACKBALL_DOWN:
        x -= speed;
        break;
      case EVT_TRACKBALL_LEFT:
        y -= speed;
        break;
      case EVT_TRACKBALL_RIGHT:
        y += speed;
        break;
      default:
        ESP_LOGI(TAG, "????");
        break;
      }
      lcdDrawFillCircle(Board, x, y, 10, 0xFFFF);
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void vTaskRefresh(void *ctx) {
  td_board_t *Board = (td_board_t *)ctx;
  while (1) {
    lcdDrawFinish(Board);
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}
// Generate sine wave data
void generate_sine_wave(int16_t *buffer, int num_samples, float frequency,
                        int sample_rate) {
  for (int i = 0; i < num_samples; i++) {
    float sample = sinf(2 * PI * frequency * i / sample_rate);
    buffer[i] = (int16_t)(sample * 5000);     // Left channel
  }
}

extern const int16_t pcm_start[] asm("_binary_drama_pcm_start");
extern const int16_t pcm_end[] asm("_binary_drama_pcm_end");

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

      ESP_ERROR_CHECK(i2s_channel_write(Board->Speaker.dev, sndbuffer, (SAMPLES * sizeof(int16_t)),
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
  td_board_init(&Board);
  size_t w_bytes = 0;
  uint32_t offset = 0;

  TaskHandle_t maintask = NULL;
  TaskHandle_t refreshtask = NULL;

  ESP_ERROR_CHECK(i2s_channel_enable(Board->Speaker.dev));
  // xTaskCreate(vTaskMain, "Main", 2048, Board, tskIDLE_PRIORITY, &maintask);
  // xTaskCreate(i2s_example_write_task, "i2s_example_write_task", 8192, NULL,
  // 5, NULL);
  //  xTaskCreate(vTaskRefresh, "Refresh", 2048, Board, tskIDLE_PRIORITY,
  //  &refreshtask);
  td_battery_update(Board);
  ESP_LOGI(TAG, "Battery level: %.02f ", Board->Battery.voltage);

  xTaskCreate(vTaskPlay, "TaskPlay", 2048, Board, tskIDLE_PRIORITY, &maintask);

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
