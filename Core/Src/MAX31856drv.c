/*  ------------------------------------------------------------------------- */
/*
 * Copyright (C) 2012 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED PRODUCTS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated Products
 * shall not be used except as stated in the Maxim Integrated Products
 * Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products retains all ownership rights.
 *
 ***************************************************************************/
/** \file max31856drv.c ******************************************************
 *
 *             Project: max31856
 *            Filename: max31856drv.c
 *         Description: This module contains the Main application for the max31856 example program.
 *
 *    Revision History:
 *\n                    23-12-15    Rev 1.0.0    TTS    Initial release.
 *\n                                             TTS    Re-release.
 *\n                    		
 *
 *  --------------------------------------------------------------------
 *
 *  This code follows the following naming conventions:
 *
 *\n    char                    	ch_pmod_value
 *\n    char (array)            s_pmod_string[16]
 *\n    float                  	 f_pmod_value
 *\n    int                    	 n_pmod_value
 *\n    int (array)             	an_pmod_value[16]
 *\n    u16                     	u_pmod_value
 *\n    u16 (array)             au_pmod_value[16]
 *\n    u8                     	 uch_pmod_value
 *\n    u8 (array)              	auch_pmod_buffer[16]
 *\n    unsigned int     	un_pmod_value
 *\n    int *                   	pun_pmod_value
 *
 *---------------------------------------------------------------*/

#include "MAX31856drv.h"

#define  SPI_CS_LOW       HAL_GPIO_WritePin( CS_1_GPIO_Port, CS_1_Pin, GPIO_PIN_RESET);
#define  SPI_CS_HIGH      HAL_GPIO_WritePin( CS_1_GPIO_Port, CS_1_Pin, GPIO_PIN_SET);

void SPI_WriteByte(uint8_t b)
{
	HAL_SPI_Transmit( &hspi1, &(b), 1, HAL_MAX_DELAY);
}

uint8_t SPI_ReadByte()
{
	uint8_t b;
	HAL_SPI_Receive( &hspi1, &(b), 1, HAL_MAX_DELAY);
	return b;
}

void SPI_Read(uint8_t *buff, uint8_t nBytes)
{
	HAL_SPI_Receive( &hspi1, buff, nBytes, HAL_MAX_DELAY);
}

uint8_t uch_cr0,uch_cr1,uch_mask;

void maxim_31856_write_register(uint8_t uch_register_address, uint8_t uch_register_value)
{
  SPI_CS_LOW;
  SPI_WriteByte(uch_register_address);
  SPI_WriteByte(uch_register_value);
  SPI_CS_HIGH; 
}

uint8_t maxim_31856_read_register(uint8_t uch_register_address)
{
  uint8_t uch_register_data;
  
  SPI_CS_LOW;
  SPI_WriteByte(uch_register_address);
  uch_register_data=SPI_ReadByte();
  SPI_CS_HIGH;
  return (uch_register_data);
  
}
void maxim_31856_read_nregisters(uint8_t uch_register_address, uint8_t *uch_buff,uint8_t uch_nBytes)
{
 SPI_CS_LOW;
 SPI_WriteByte(uch_register_address);
 SPI_Read(uch_buff,uch_nBytes);
 SPI_CS_HIGH;
}

void maxim_31856_init(void)
{
   //Enable fault detection, select 50Hz filtering, use interrupt mode as fault mode
   uch_cr0 = OC_Fault_Enable_1|NRF_50Hz| Interrupt_Mode; //Configure CR0
   //Because the single measurement mode is adopted, the output result of each measurement is set. Thermocouple selection K-type thermocouple
   uch_cr1&= Average_1_Bit;
   uch_cr1|=TC_TypeK;
   //Because the fault detection is enabled, no fault detection is shielded, and the faults that do not need to be detected can be shielded according to the requirements
   uch_mask=No_Fault_Mask;
   
   maxim_31856_write_register(0x80, uch_cr0); //Set CR0
   maxim_31856_write_register(0x81, uch_cr1); //Set CR1
   maxim_31856_write_register(0x82,uch_mask); //Set MASK
   
   //Write the cold junction fault threshold register, which can be set as required
   maxim_31856_write_register(0x83,0x7F);
   maxim_31856_write_register(0x84,0xC0);
   //Write the thermocouple fault threshold register, which can be set as required
   maxim_31856_write_register(0x85,0x7F);
   maxim_31856_write_register(0x86,0xFF);
   maxim_31856_write_register(0x87,0x80);
   maxim_31856_write_register(0x88,0x00);
   //Write the cold junction temperature offset register, which can be set as required
   maxim_31856_write_register(0x89,0x00);
   //If it is forbidden to use the internal cold junction compensation of the device, write the cold junction temperature register, which can be set as required
   //If the external cold junction temperature sensor is used to measure the cold junction temperature, it needs to be updated every time the cold junction temperature is measured
   //The cold junction temperature register.
   maxim_31856_write_register(0x8A,0x00);
   maxim_31856_write_register(0x8B,0x00);
}

/*****************************************************

CR0 Bit operation
******************************************************/
void maxim_stop_conversion(void)
{
  uch_cr0= maxim_31856_read_register(0x00);
  uch_cr0&=Stop_Conversion_Bit;
  maxim_31856_write_register(0x80, uch_cr0);
}
/****************************************************

Start_Conversion: used to start conversion.

Conversion Mode can be One_Shot_ Conversion or Automatic_Conversion

 the define of One_Shot_ Conversion or Automatic_Conversion
 please see MAX31856drv.h
*****************************************************/
void maxim_start_conversion(uint8_t uch_conversion_mode)  
{
 uch_cr0=maxim_31856_read_register(0x00);
 uch_cr0&=Stop_Conversion_Bit;
 uch_cr0|=uch_conversion_mode;
 maxim_31856_write_register(0x80, uch_cr0);
}

void maxim_disable_oc_fault_detection(void)
{
 uch_cr0= maxim_31856_read_register(0x00);
 uch_cr0&=OC_Fault_Disable_Bit;
 maxim_31856_write_register(0x80, uch_cr0);
}
/****************************************************

OC_Fault_Enable: OC_Fault_Enable1
                 OC_Fault_Enable2
                 OC_Fault_Enable3
*****************************************************/
void maxim_set_oc_fault_detection(uint8_t uch_oc_fault_enable)
{
 uch_cr0= maxim_31856_read_register(0x00);
 uch_cr0&=OC_Fault_Disable_Bit;
 uch_cr0|=uch_oc_fault_enable;
 maxim_31856_write_register(0x80, uch_cr0);
}

void maxim_clear_fault_status(void)
{
 uch_cr0= maxim_31856_read_register(0x00);
 uch_cr0|= Fault_Clear ;
 maxim_31856_write_register(0x80, uch_cr0);
}
//-------------------------------------------------------------------------/


