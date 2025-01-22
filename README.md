STM32F429:
Only LL used;
2 UARTs to communicate with PC:
  1st for controlling state of MCU
  2nd for audio streaming
DMA+DAC
(to do: change to USB, add stereo support, increase bit depth)

PC:
PyQt5 frontend(requires FFmpeg)
