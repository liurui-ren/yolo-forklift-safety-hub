#include "i2s_audio.h"
#include "main.h"

I2S_HandleTypeDef hi2s2;

static const int16_t adpcm_step_table[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static const int8_t adpcm_index_table[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

static int16_t adpcm_predictor = 0;
static int8_t  adpcm_step_index = 0;

static int16_t adpcm_decode_nibble(uint8_t code)
{
    int16_t step = adpcm_step_table[adpcm_step_index];
    int16_t diff = step >> 3;
    if (code & 1) diff += step >> 2;
    if (code & 2) diff += step >> 1;
    if (code & 4) diff += step;
    if (code & 8)
        adpcm_predictor -= diff;
    else
        adpcm_predictor += diff;
    if (adpcm_predictor > 32767) adpcm_predictor = 32767;
    if (adpcm_predictor < -32768) adpcm_predictor = -32768;
    adpcm_step_index += adpcm_index_table[code];
    if (adpcm_step_index < 0) adpcm_step_index = 0;
    if (adpcm_step_index > 88) adpcm_step_index = 88;
    return adpcm_predictor;
}

void I2S_Audio_Init(void)
{
    hi2s2.Instance          = SPI2;
    hi2s2.Init.Mode         = I2S_MODE_MASTER_TX;
    hi2s2.Init.Standard     = I2S_STANDARD_PHILIPS;
    hi2s2.Init.DataFormat   = I2S_DATAFORMAT_16B_EXTENDED;
    hi2s2.Init.MCLKOutput   = I2S_MCLKOUTPUT_DISABLE;
    hi2s2.Init.AudioFreq    = I2S_AUDIO_SAMPLE_RATE;
    hi2s2.Init.CPOL         = I2S_CPOL_LOW;

    if (HAL_I2S_Init(&hi2s2) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_I2S_MspInit(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_InitStruct.Pin       = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void HAL_I2S_MspDeInit(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2)
    {
        __HAL_RCC_SPI2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15);
    }
}

void I2S_Audio_PlayADPCM(const uint8_t *adpcm_data, uint32_t adpcm_size, uint32_t num_samples)
{
    static int16_t i2s_buf[I2S_AUDIO_BUF_SIZE * 2];
    uint32_t sample_offset = 0;
    uint32_t adpcm_offset = 0;

    adpcm_predictor = 0;
    adpcm_step_index = 0;

    while (sample_offset < num_samples)
    {
        uint32_t buf_frames = 0;

        while (buf_frames < I2S_AUDIO_BUF_SIZE && sample_offset < num_samples)
        {
            if (adpcm_offset < adpcm_size)
            {
                uint8_t byte = adpcm_data[adpcm_offset++];

                int16_t s1 = adpcm_decode_nibble(byte & 0x0F);
                i2s_buf[buf_frames * 2]     = s1;
                i2s_buf[buf_frames * 2 + 1] = s1;
                buf_frames++;
                sample_offset++;

                if (sample_offset >= num_samples) break;

                int16_t s2 = adpcm_decode_nibble((byte >> 4) & 0x0F);
                i2s_buf[buf_frames * 2]     = s2;
                i2s_buf[buf_frames * 2 + 1] = s2;
                buf_frames++;
                sample_offset++;
            }
            else
            {
                i2s_buf[buf_frames * 2]     = 0;
                i2s_buf[buf_frames * 2 + 1] = 0;
                buf_frames++;
                sample_offset++;
            }
        }

        if (buf_frames > 0)
        {
            HAL_I2S_Transmit(&hi2s2, (uint16_t *)i2s_buf, buf_frames * 2, 5000);
        }
    }

    int16_t silence_buf[64];
    for (int i = 0; i < 64; i++) silence_buf[i] = 0;
    HAL_I2S_Transmit(&hi2s2, (uint16_t *)silence_buf, 64, 100);

    __HAL_I2S_DISABLE(&hi2s2);
}

void I2S_Audio_TestTone(void)
{
    static int16_t tone_buf[64];

    for (int i = 0; i < 32; i++)
    {
        int16_t val = ((i % 4) < 2) ? 20000 : -20000;
        tone_buf[i * 2]     = val;
        tone_buf[i * 2 + 1] = val;
    }

    for (int j = 0; j < 250; j++)
    {
        if (HAL_I2S_Transmit(&hi2s2, (uint16_t *)tone_buf, 64, 1000) != HAL_OK)
        {
            break;
        }
    }

    int16_t silence_buf[64];
    for (int i = 0; i < 64; i++) silence_buf[i] = 0;
    HAL_I2S_Transmit(&hi2s2, (uint16_t *)silence_buf, 64, 100);

    __HAL_I2S_DISABLE(&hi2s2);
}

void I2S_Audio_Stop(void)
{
    __HAL_I2S_DISABLE(&hi2s2);
}
