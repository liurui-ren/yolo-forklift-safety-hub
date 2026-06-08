#include "jq8900.h"
#include "usart.h"

void Voice_PlayTrack(uint16_t track_num)
{
    uint8_t cmd[6];
    uint8_t checksum;

    cmd[0] = VM_FRAME_HEADER;
    cmd[1] = VM_CMD_SPECIFY;
    cmd[2] = 0x02;
    cmd[3] = (uint8_t)((track_num >> 8) & 0xFF);
    cmd[4] = (uint8_t)(track_num & 0xFF);

    checksum = (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4]) & 0xFF;
    cmd[5] = checksum;

    HAL_UART_Transmit(&huart1, cmd, 6, HAL_MAX_DELAY);
}

void Voice_SetVolume(uint8_t volume)
{
    uint8_t cmd[5];
    uint8_t checksum;

    if (volume > 30) volume = 30;

    cmd[0] = VM_FRAME_HEADER;
    cmd[1] = VM_CMD_VOLUME;
    cmd[2] = 0x01;
    cmd[3] = volume;

    checksum = (cmd[0] + cmd[1] + cmd[2] + cmd[3]) & 0xFF;
    cmd[4] = checksum;

    HAL_UART_Transmit(&huart1, cmd, 5, HAL_MAX_DELAY);
}

void Voice_Stop(void)
{
    uint8_t cmd[4];
    uint8_t checksum;

    cmd[0] = VM_FRAME_HEADER;
    cmd[1] = VM_CMD_STOP;
    cmd[2] = 0x00;

    checksum = (cmd[0] + cmd[1] + cmd[2]) & 0xFF;
    cmd[3] = checksum;

    HAL_UART_Transmit(&huart1, cmd, 4, HAL_MAX_DELAY);
}
