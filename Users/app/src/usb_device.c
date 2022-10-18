#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include <string.h>

USBD_HandleTypeDef hUsbDeviceFS;
uint8_t *para_temp;
uint8_t tx_count=0;

void USB_device_received_handle(uint8_t *buf,uint16_t *length);

void usb_device_init(uint8_t *para,uint16_t buffer_count)
{
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
  if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    Error_Handler(__FILE__,__LINE__);
  }

  HAL_PWREx_EnableUSBVoltageDetector();
  set_rx_handler(USB_device_received_handle);
  para_temp=para;
  tx_count=buffer_count;
}

void USB_device_transmit_data(uint8_t *buf,uint16_t length)
{
  CDC_Transmit_FS(buf,length);
}

void USB_device_received_handle(uint8_t *buf,uint16_t *length)
{
  uint8_t len=*length;
  if(buf!=NULL)
  {
    if(buf[0]==0x55&&buf[1]==0x66)
    {
      memcpy(para_temp,buf,len);
    }

    USB_device_transmit_data(para_temp,tx_count);
  }
}