#include "adc_wheels.h"

static ADC_HandleTypeDef *adc_host;

void adc_wheels_set_adc_host(ADC_HandleTypeDef *host)
{
    adc_host = host;
}

int adc_wheels_read(uint32_t channel, uint16_t *value)
{
    ADC_ChannelConfTypeDef config = {0};
    HAL_StatusTypeDef status;

    if (adc_host == 0 || value == 0) {
        return -1;
    }

    config.Channel = channel;
    config.Rank = 1;
    config.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;

    if (HAL_ADC_ConfigChannel(adc_host, &config) != HAL_OK) {
        return -1;
    }

    status = HAL_ADC_Start(adc_host);
    if (status != HAL_OK) {
        return -1;
    }

    status = HAL_ADC_PollForConversion(adc_host, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        HAL_ADC_Stop(adc_host);
        return -1;
    }

    *value = (uint16_t)HAL_ADC_GetValue(adc_host);
    HAL_ADC_Stop(adc_host);

    return 0;
}

int adc_wheels_read_axis(uint8_t axis, uint16_t *value)
{
    static const uint32_t channels[] = {
        ADC_CHANNEL_0,
        ADC_CHANNEL_1,
    };

    if (axis >= (sizeof(channels) / sizeof(channels[0]))) {
        return -1;
    }

    return adc_wheels_read(channels[axis], value);
}
