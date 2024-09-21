//#pragma once

#include <board.h>
#include <sdcard.h>

typedef struct wav_header_t
{
    //   RIFF Section
    char RIFFSectionID[4];      // Letters "RIFF"
    uint32_t Size;              // Size of entire file less 8
    char RiffFormat[4];         // Letters "WAVE"

    //   Format Section
    char FormatSectionID[4];    // letters "fmt"
    uint32_t FormatSize;        // Size of format section less 8
    uint16_t FormatID;          // 1=uncompressed PCM
    uint16_t NumChannels;       // 1=mono,2=stereo
    uint32_t SampleRate;        // 44100, 16000, 8000 etc.
    uint32_t ByteRate;          // =SampleRate * Channels * (BitsPerSample/8)
    uint16_t BlockAlign;        // =Channels * (BitsPerSample/8)
    uint16_t BitsPerSample;     // 8,16,24 or 32

    // Data Section
    char DataSectionID[4];      // The letters "data"
    uint32_t DataSize;          // Size of the data that follows
} wav_header_t;


bool is_valid_wav_data(wav_header_t* Wav);
void wav_header_dump(wav_header_t* Wav);
bool wav_header_load_from_file(FILE* wavFile);
void i2s_reconfig(td_board_t* Board, wav_header_t *wav_header)
esp_err_t play_wav_file(td_board_t* Board, const char *fp);
esp_err_t play_wav_data(td_board_t* Board, const unsigned char *data, size_t data_len);

