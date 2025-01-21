#include "defines.h"

void Playback();
void Pause();
void Stop();
void ProcessCommand(uint8_t command);
void SendString(const char* str);
void StartUSARTAudio_DMA(void);
void StopUSARTAudio_DMA(void);
void StopDACAudio_DMA(void);
void StartDACAudio_DMA(void);