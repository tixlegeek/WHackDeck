
#include "./wavreader.h"

#define AUDIO_BUFFER_SIZE 2048

const static char *TAG = "wav_reader";

bool is_valid_wav_data(wav_header_t* wav_header)
{
    if( memcmp( wav_header->RIFFSectionID, "RIFF", 4 ) != 0 ) { ESP_LOGE(TAG, "Invalid data - Not RIFF format"); return false; }
    if( memcmp( wav_header->RiffFormat,"WAVE", 4 ) != 0 ) { ESP_LOGE(TAG, "Invalid data - Not Wave file"); return false; }
    if( memcmp( wav_header->FormatSectionID,"fmt", 3 ) != 0 ) { ESP_LOGE(TAG, "Invalid data - No format section found"); return false; }
    if( memcmp( wav_header->DataSectionID,"data", 4 ) != 0 ) { ESP_LOGE(TAG, "Invalid data - data section not found"); return false; }
    if( wav_header->FormatID != 1 ) { ESP_LOGE(TAG, "Invalid data - format Id must be 1"); return false; }
    if( wav_header->FormatSize != 16) { ESP_LOGE(TAG, "Invalid data - format section size must be 16."); return false; }
    if( ( wav_header->NumChannels != 1 ) & ( wav_header->NumChannels != 2 ) ) { ESP_LOGE(TAG, "Invalid data - only mono or stereo permitted."); return false; }
    if( wav_header->SampleRate > 48000 ) { ESP_LOGE(TAG, "Invalid data - Sample rate cannot be greater than 48000"); return false; }
    if( ( wav_header->BitsPerSample != 8 ) & ( wav_header->BitsPerSample != 16 ) ) { ESP_LOGE(TAG, "Invalid data - Only 8 or 16 bits per sample permitted."); return false; }
    return true;
}


void wav_header_dump(wav_header_t* wav_header)
{
    if(memcmp(wav_header->RIFFSectionID,"RIFF",4)!=0) {
        ESP_LOGE(TAG, "Not a RIFF format file - ");
        ESP_LOG_BUFFER_HEXDUMP(TAG, wav_header->RIFFSectionID, 4, ESP_LOG_ERROR);
        return;
    }
    if(memcmp(wav_header->RiffFormat,"WAVE",4)!=0) {
        ESP_LOGE(TAG, "Not a WAVE file - ");
        ESP_LOG_BUFFER_HEXDUMP(TAG, wav_header->RiffFormat,4, ESP_LOG_ERROR);
        return;
    }
    if(memcmp(wav_header->FormatSectionID,"fmt",3)!=0) {
        ESP_LOGE(TAG, "fmt ID not present - ");
        ESP_LOG_BUFFER_HEXDUMP(TAG, wav_header->FormatSectionID,3, ESP_LOG_ERROR);
        return;
    }
    if(memcmp(wav_header->DataSectionID,"data",4)!=0) {
        ESP_LOGE(TAG, "data ID not present - ");
        ESP_LOG_BUFFER_HEXDUMP(TAG, wav_header->DataSectionID,4, ESP_LOG_ERROR);
        return;
    }
    // All looks good, dump the data
    ESP_LOGE(TAG, "Total size : %d", (int)wav_header->Size);
    ESP_LOGE(TAG, "Format section size : %d", (int)wav_header->FormatSize);
    ESP_LOGE(TAG, "Wave format  : %d", (int)wav_header->FormatID);
    ESP_LOGE(TAG, "Channels  : %d", (int)wav_header->NumChannels);
    ESP_LOGE(TAG, "Sample Rate  : %d", (int)wav_header->SampleRate);
    ESP_LOGE(TAG, "Byte Rate  : %d", (int)wav_header->ByteRate);
    ESP_LOGE(TAG, "Block Align  : %d", (int)wav_header->BlockAlign);
    ESP_LOGE(TAG, "Bits Per Sample  : %d", (int)wav_header->BitsPerSample);
    ESP_LOGE(TAG, "Data Size  : %d", (int)wav_header->DataSize);
}


bool wav_header_load_from_file(FILE* wav_file)
{
    wav_header_t wav_header;

    fgets((char*)&wav_header, 44, wav_file);// Read in the WAV header, which is first 44 bytes of the file.

    if( is_valid_wav_data( &wav_header ) ) {
        wav_header_dump( &wav_header ); // print the header data to serial
        return true;
    }
    ESP_LOGE(TAG, "Invalid WAV data");
    ESP_LOG_BUFFER_HEXDUMP(TAG, &wav_header, 44, ESP_LOG_ERROR);
    return false;
}


esp_err_t play_wav(td_board_t* Board, const char *fp)
{
    FILE *fh = fopen(fp, "rb");
    if (fh == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file");
        return ESP_ERR_INVALID_ARG;
    }

    wav_header_t wav_header;

    // collect wav header data
    fgets((char*)&wav_header, 44, fh);

    if( !is_valid_wav_data( &wav_header ) ) {
        ESP_LOGE(TAG, "Invalid WAV data");
        ESP_LOG_BUFFER_HEXDUMP(TAG, &wav_header, 44, ESP_LOG_ERROR);
        return ESP_ERR_INVALID_ARG;
    }

    wav_header_dump( &wav_header ); // Dump the header data to serial, optional!

    // reconfigure i2s according to the Wav type
    {
        i2s_channel_disable(Board->Speaker.dev);

        Board->Speaker.tx_cfg->slot_cfg.slot_mode      = wav_header.NumChannels==1 ? I2S_SLOT_MODE_MONO : I2S_SLOT_MODE_STEREO; // mono/stereo
        Board->Speaker.tx_cfg->slot_cfg.data_bit_width = (i2s_data_bit_width_t)wav_header.BitsPerSample;
        i2s_channel_reconfig_std_slot(Board->Speaker.dev, &Board->Speaker.tx_cfg->slot_cfg);

        Board->Speaker.tx_cfg->clk_cfg.sample_rate_hz = wav_header.SampleRate;
        i2s_channel_reconfig_std_clock(Board->Speaker.dev, &Board->Speaker.tx_cfg->clk_cfg);

        i2s_channel_enable(Board->Speaker.dev);
    }

    // skip the header...
    fseek(fh, 44, SEEK_SET);

    // create a writer buffer
    int16_t *buf = (int16_t *)calloc(AUDIO_BUFFER_SIZE, sizeof(int16_t));
    size_t bytes_read = 0;
    size_t bytes_written = 0;

    bytes_read = fread(buf, sizeof(int16_t), AUDIO_BUFFER_SIZE, fh);

    while (bytes_read > 0)
    {
        // write the buffer to the i2s
        i2s_channel_write(Board->Speaker.dev, buf, bytes_read * sizeof(int16_t), &bytes_written, portMAX_DELAY);
        bytes_read = fread(buf, sizeof(int16_t), AUDIO_BUFFER_SIZE, fh);
        ESP_LOGV(TAG, "Bytes read: %d", bytes_read);
    }

    i2s_channel_disable(Board->Speaker.dev);
    free(buf);

    return ESP_OK;
}
