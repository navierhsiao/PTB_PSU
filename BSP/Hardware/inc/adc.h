#ifndef __ADC_H
#define __ADC_H

#define SAMPLE_TIMES 25

extern DMA_HandleTypeDef hdma_adc1;

void ADC_Init(void);
uint16_t get_ADC_buffer_value(int index);
uint16_t get_ADC_smoothed_data(uint8_t index);

#endif