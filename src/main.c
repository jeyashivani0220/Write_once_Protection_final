#include "sdk_project_config.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define EEPROM_SIZE 512U
#define EEPROM_ADDR (0xA0U >>1) //0x50
#define PAGE_SIZE       8U
#define total_pages    (EEPROM_SIZE / PAGE_SIZE)

#define NVM_REQ_OK      0U
#define NVM_REQ_NOT_OK  1U

uint8_t WriteOnceEnabled[total_pages];
uint8_t BlockLocked[total_pages];
lpi2c_master_state_t lpi2c1MasterState;
void UART_SendString(const char *str)
{
    int len = (int)strlen(str);
    LPUART_DRV_SendDataBlocking(INST_LPUART_1, (uint8_t*)str, len, 1000);
}
uint8_t EEPROM_Write(uint16_t memAddr,uint8_t *data,uint8_t len)

{
    uint8_t slaveAddr;
    uint8_t wordAddr;
    uint8_t buf[16];
    status_t status;
    if ((data == NULL) || (len == 0U) ||
        (len > 15U) ||
        ((uint32_t)memAddr + len > EEPROM_SIZE) ||
        (((memAddr % 16U) + len) > 16U))
    {
        return NVM_REQ_NOT_OK;
    }
    slaveAddr = EEPROM_ADDR | ((memAddr >> 7U) & 0x02U);
    wordAddr = (uint8_t)(memAddr & 0xFFU);
    buf[0] = wordAddr;
    for (uint8_t i = 0U; i < len; i++)
    {
        buf[i + 1U] = data[i];
    }
    LPI2C_DRV_MasterSetSlaveAddr(0, slaveAddr, false);
    status = LPI2C_DRV_MasterSendDataBlocking(0, buf, (uint32_t)len + 1U, true, 100U);
    if (status != STATUS_SUCCESS)
    {
        return NVM_REQ_NOT_OK;
    }
    OSIF_TimeDelay(10U);
    return NVM_REQ_OK;
}
uint8_t EEPROM_Read(uint16_t memAddr,uint8_t *data,uint8_t len)
{
    uint8_t slaveAddr;
    uint8_t wordAddr;
    status_t status;
    if ((data == NULL) || (len == 0U) ||
        ((uint32_t)memAddr + len > EEPROM_SIZE))
    {
        return NVM_REQ_NOT_OK;
    }
    slaveAddr = EEPROM_ADDR | ((memAddr >> 7U) & 0x02U);
    wordAddr = (uint8_t)(memAddr & 0xFFU);
    LPI2C_DRV_MasterSetSlaveAddr(0, slaveAddr, false);
    status = LPI2C_DRV_MasterSendDataBlocking(0, &wordAddr, 1U, false, 100U);
    if (status != STATUS_SUCCESS)
    {
        return NVM_REQ_NOT_OK;
    }
    status = LPI2C_DRV_MasterReceiveDataBlocking(0, data, len, true, 100U);
    if (status != STATUS_SUCCESS)
    {
        return NVM_REQ_NOT_OK;
    }
    return NVM_REQ_OK;
}
void NvM_InitWriteOnce(void)
{
    uint16_t i;

    for (i = 0U; i < total_pages; i++)
    {
        WriteOnceEnabled[i] = 1U;
        BlockLocked[i] = 0U;
    }
    UART_SendString("NvM initialized.\r\n");
}
uint8_t NvM_CheckWriteProtection(uint16_t blockId)
{
    if (blockId >= total_pages)
    {
    	UART_SendString("Reason: Write-Once Protected.\r\n");
        return NVM_REQ_NOT_OK;
    }

    if ((WriteOnceEnabled[blockId] == 1U) &&
        (BlockLocked[blockId] == 1U))
    {
    	UART_SendString("Reason: Write-Once Protected.\r\n");
        return NVM_REQ_NOT_OK;
    }

    return NVM_REQ_OK;
}
void NvM_LockBlock(uint16_t blockId)
{
    if ((blockId < total_pages) &&
        (WriteOnceEnabled[blockId] == 1U))
    {
    	UART_SendString("Block locked.\r\n");
        BlockLocked[blockId] = 1U;
    }
}
uint8_t NvM_WriteBlock(uint16_t blockId, uint8_t *data)
{
    uint16_t startAddress;
    uint8_t verifyData[PAGE_SIZE];


    if ((blockId >= total_pages) || (data == NULL))
    {
        return NVM_REQ_NOT_OK;
    }

    if (NvM_CheckWriteProtection(blockId) != NVM_REQ_OK)
    {
        return NVM_REQ_NOT_OK;
    }
    startAddress = blockId * PAGE_SIZE;

    if (EEPROM_Write(startAddress, data, PAGE_SIZE) != NVM_REQ_OK)
    {
        return NVM_REQ_NOT_OK;
    }


    if (EEPROM_Read(startAddress, verifyData, PAGE_SIZE) != NVM_REQ_OK)
    {
        return NVM_REQ_NOT_OK;
    }


    if (memcmp(data, verifyData, PAGE_SIZE) != 0)
    {
        return NVM_REQ_NOT_OK;
    }


    NvM_LockBlock(blockId);
    return NVM_REQ_OK;
}
void NvM_TestWriteOnce(void)
{
    uint8_t data1[PAGE_SIZE] ={10U, 20U, 30U, 40U, 50U, 60U, 70U, 80U};
    uint8_t data2[PAGE_SIZE] ={11U, 22U, 33U, 44U, 55U, 66U, 77U, 88U};
    uint8_t result;
    uint8_t readData[PAGE_SIZE];
    uint8_t i;
    uint16_t blockId = 0U;


    result = NvM_WriteBlock(blockId, data1);

    if (result == NVM_REQ_OK)
    {
    	 UART_SendString("First write: NVM_REQ_OK\r\n");
    }
    else
    {
    	 UART_SendString("First write: NVM_REQ_NOT_OK\r\n");
    }

    result = NvM_WriteBlock(blockId, data2);
    if (result == NVM_REQ_NOT_OK)
    {
    	 UART_SendString("Second write: NVM_REQ_NOT_OK\r\n");
    	 UART_SendString("Reason: Write-Once Protected.\r\n");
    }
    else
    {
    	 UART_SendString("Second write: NVM_REQ_OK\r\n");
    }

    if (EEPROM_Read(blockId * PAGE_SIZE,readData, PAGE_SIZE) == NVM_REQ_OK)
    {
    	char buffer[32];
    	 UART_SendString("Stored data: ");

        for (i = 0U; i < PAGE_SIZE; i++)
        {
        	 sprintf(buffer,"%u ", readData[i]);
        	 UART_SendString(buffer);
        }

        UART_SendString("\r\n");
    }
}
int main(void)
{
  CLOCK_DRV_Init(&clockMan1_InitConfig0);
  PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
  LPI2C_DRV_MasterInit(0,&lpi2c0_MasterConfig0,&lpi2c1MasterState);
  LPUART_DRV_Init(INST_LPUART_1, &lpUartState0, &lpuart_0_InitConfig0);
  NvM_InitWriteOnce();
  NvM_TestWriteOnce();
  while(1)
  {

  }
}
