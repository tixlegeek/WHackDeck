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
#include <jsonconfig.h>

#include <audio.h>
#include <st7789.h>

#include <dirent.h>
const static char *TAG = "WHACKDECK";

/**************************************************************/
typedef enum {
    AUDIO_PLAYER_STATE_IDLE,
    AUDIO_PLAYER_STATE_PLAYING,
    AUDIO_PLAYER_STATE_PAUSE,
    AUDIO_PLAYER_STATE_SHUTDOWN
} audio_player_state_t;

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

/*
extern const int16_t pcm_start[] asm("_binary_drama_pcm_start");
extern const int16_t pcm_end[] asm("_binary_drama_pcm_end");
*/

static int x = 160;
static int y = 120;
//static int oldx = 0;
//static int oldy = 0;

// TOBOZO
void turtle(void*ctx, int dirx, int diry)
{
  td_board_t *Board = (td_board_t *)ctx;
  //oldx = x;
  //oldy = y;
  x += dirx;
  y += diry;

  if( x > 319 ) x = 319;
  if( x <= 0 )  x = 0;

  if( y > 239 ) y = 239;
  if( y <= 0 )  y = 0;

  //lcdDrawPixel(Board, oldx, oldy, 0x0000);
  lcdDrawPixel(Board, x,    y,    0xffff);
  lcdDrawFinish(Board);
}

void app_main(void) {
  float voltage = .0f;
  td_board_t *Board = NULL;

  esp_err_t err = td_board_init(&Board, INIT_SDCARD|INIT_DISPLAY|INIT_KEYBOARD|INIT_TRACKBALL|INIT_SPEAKER|INIT_BATTERY|INIT_GPS);
  if(err != ESP_OK){
    ESP_LOGE(TAG, "Initialisation failed.");
    while(1){
      ESP_LOGE(TAG, "x");
      vTaskDelay(100);
    }
  }

  TaskHandle_t maintask = NULL;
  TaskHandle_t trackballtask = NULL;
  //TaskHandle_t gpsTask = NULL;
  //TaskHandle_t nmeaTask = NULL;

  ESP_ERROR_CHECK(i2s_channel_enable(Board->Speaker->dev));
  ESP_LOGI(TAG, "Battery level: %.02f ", Board->Battery->voltage);

  td_battery_update(Board);
  xTaskCreate(vTaskMain, "Main", 2048, Board, tskIDLE_PRIORITY, &maintask);
  td_trackball_set_cb(turtle) ;
  xTaskCreate(td_trackball_task, "TrackBall", 2048, Board, tskIDLE_PRIORITY, &trackballtask);

  ESP_ERROR_CHECK(td_audio_init(Board));

  struct dirent *entity;
 //BOARD_SDCARD_MOUNT_POINT
  DIR *dir = opendir(BOARD_SDCARD_MOUNT_POINT);
  if(dir){
    while((entity = readdir(dir))!= NULL){
      switch (entity->d_type) {
        case DT_DIR: // If entity points to a directory
          ESP_LOGW(TAG, "+ %s", entity->d_name);
          break;
        case DT_REG: // If entity points to a file?
          ESP_LOGW(TAG, "- %s", entity->d_name);
          break;
        default:
          ESP_LOGW(TAG, "? %s", entity->d_name);
          break;
      }
    }
    closedir(dir);
  }
  else
  {
    ESP_LOGE(TAG, "Could not open dir \"%s\"", BOARD_SDCARD_MOUNT_POINT);
  }
  td_config_t *config = malloc(sizeof(td_config_t));
  ESP_ERROR_CHECK(td_load( BOARD_SDCARD_MOUNT_POINT"/CONFIG~1.JSO", config));


  while (1) {
    td_battery_update(Board);
    ESP_LOGI(TAG, "Battery level: %.02f ", Board->Battery->voltage);
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    /*
    td_gps_sendcmd(Board, "$PCAS03,0,0,0,0,0,0,0,0,0,0,,,0,0");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    td_gps_sendcmd(Board, "$PCAS06,0");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    */
  }

}
