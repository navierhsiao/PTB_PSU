#ifndef __USB_DEVICE__H__
#define __USB_DEVICE__H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "usbd_def.h"

#include "../../BSP/system.h"

void usb_device_init(uint8_t *para,uint16_t buffer_count);
void USB_device_transmit_data(uint8_t *buf,uint16_t length);


#ifdef __cplusplus
}
#endif

#endif /* __USB_DEVICE__H__ */
