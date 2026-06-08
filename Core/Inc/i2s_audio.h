#ifndef __I2S_AUDIO_H__
#define __I2S_AUDIO_H__

#ifndef SPI_I2S_SUPPORT
#define SPI_I2S_SUPPORT
#endif

#include "stm32f1xx_hal.h"

#ifndef IS_I2S_ALL_INSTANCE
#define IS_I2S_ALL_INSTANCE(INSTANCE) (((INSTANCE) == SPI2) || ((INSTANCE) == SPI3))
#endif

#define I2S_AUDIO_SAMPLE_RATE  I2S_AUDIOFREQ_8K
#define I2S_AUDIO_BUF_SIZE     256

void I2S_Audio_Init(void);
void I2S_Audio_PlayADPCM(const uint8_t *adpcm_data, uint32_t adpcm_size, uint32_t num_samples);
void I2S_Audio_TestTone(void);
void I2S_Audio_Stop(void);

extern I2S_HandleTypeDef hi2s2;

#endif
