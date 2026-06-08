#ifndef __VOICE_MODULE_H__
#define __VOICE_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define VM_FRAME_HEADER  0xAA
#define VM_CMD_PLAY      0x02
#define VM_CMD_STOP      0x04
#define VM_CMD_SPECIFY   0x07
#define VM_CMD_VOLUME    0x13

void Voice_PlayTrack(uint16_t track_num);
void Voice_SetVolume(uint8_t volume);
void Voice_Stop(void);

#ifdef __cplusplus
}
#endif

#endif
