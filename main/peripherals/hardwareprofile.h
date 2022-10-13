#ifndef HARDWAREPROFILE_H_INCLUDED
#define HARDWAREPROFILE_H_INCLUDED

/*
 * Definizioni dei pin da utilizzare
 */

#define HAP_ASP GPIO_NUM_26
#define HAP_SOFF GPIO_NUM_25
#define HAP_INT0 GPIO_NUM_35

#define HAP_TEMP1 ADC2_CHANNEL_5
#define HAP_TEMP2 ADC1_CHANNEL_6

#define HAP_LIVELLO1_H20 ADC1_CHANNEL_4
#define HAP_LIVELLO2_H20 ADC1_CHANNEL_5

#define HAP_PWM_LIVELLI GPIO_NUM_2

#endif