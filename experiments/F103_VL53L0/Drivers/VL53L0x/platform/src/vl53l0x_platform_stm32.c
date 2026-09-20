#define VL53L0X_MAX_I2C_XFER_SIZE   64 /* Maximum buffer size to be used in i2c */

#include <string.h>
#include "main.h"
#include "vl53l0x_platform.h"
#include "vl53l0x_i2c_platform.h"
#include "vl53l0x_api.h"

#define LOG_FUNCTION_START(fmt, ... )           _LOG_FUNCTION_START(TRACE_MODULE_PLATFORM, fmt, ##__VA_ARGS__)
#define LOG_FUNCTION_END(status, ... )          _LOG_FUNCTION_END(TRACE_MODULE_PLATFORM, status, ##__VA_ARGS__)
#define LOG_FUNCTION_END_FMT(status, fmt, ... ) _LOG_FUNCTION_END_FMT(TRACE_MODULE_PLATFORM, status, fmt, ##__VA_ARGS__)

#define VL53L0X_I2C_USER_VAR         /* none but could be for a flag var to get/pass to mutex interruptible  return flags and try again */
#define VL53L0X_GetI2CAccess(Dev)    /* todo mutex acquire */
#define VL53L0X_DoneI2CAcces(Dev)    /* todo mutex release */

#define STATUS_OK              0x00
#define STATUS_FAIL            0x01


extern I2C_HandleTypeDef hi2c2;
//
//VL53L0X_Error VL53L0X_LockSequenceAccess(VL53L0X_DEV Dev);
//
//
//VL53L0X_Error VL53L0X_UnlockSequenceAccess(VL53L0X_DEV Dev);
//


VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    int32_t status_int = 0;
	uint8_t deviceAddress;

    if (count>=VL53L0X_MAX_I2C_XFER_SIZE){
        Status = VL53L0X_ERROR_INVALID_PARAMS;
    }

	deviceAddress = Dev->I2cDevAddr;

	status_int = HAL_I2C_Mem_Write(&hi2c2, deviceAddress, index, sizeof(index), pdata, count, 100);
//	status_int = VL53L0X_write_multi(deviceAddress, index, pdata, count);

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;

    return Status;
}


VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)
{
    VL53L0X_I2C_USER_VAR
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    int32_t status_int;
	uint8_t deviceAddress;

    if (count>=VL53L0X_MAX_I2C_XFER_SIZE){
        Status = VL53L0X_ERROR_INVALID_PARAMS;
    }

    deviceAddress = Dev->I2cDevAddr;

//    status_int = HAL_I2C_Mem_Read(&hi2c2, deviceAddress, index, sizeof(index), pdata, count, 100);
	status_int = VL53L0X_read_multi(deviceAddress, index, pdata, count);

    // 디버그
//    if (index == 0x14) {
//        printf(">>>>> st:%ld :", status_int);
//        for (uint32_t i = 0; i < count; i++)
//        	printf(" %02X", pdata[i]);
//        printf("\r\n");
//    }

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;

    return Status;
}

// ======

VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    int32_t status_int;
	uint8_t deviceAddress;

    deviceAddress = Dev->I2cDevAddr;

	status_int = VL53L0X_write_byte(deviceAddress, index, data);
//	status_int = HAL_I2C_Mem_Write(&hi2c2, deviceAddress, index, sizeof(index), &data, sizeof(uint8_t), 100);

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;

    return Status;
}

VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data){
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    int32_t status_int;
    uint8_t deviceAddress;

    deviceAddress = Dev->I2cDevAddr;

//    status_int = HAL_I2C_Mem_Read(&hi2c2, deviceAddress, index, sizeof(index), data, sizeof(uint8_t), 100);
    status_int = VL53L0X_read_byte(deviceAddress, index, data);

    if (status_int != 0)
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;

    return Status;
}

VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev){
    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    LOG_FUNCTION_START("");

    HAL_Delay(10);

    LOG_FUNCTION_END(status);
    return status;
}

VL53L0X_Error  VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data){
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    int32_t status_int;
    uint8_t deviceAddress;

    deviceAddress = Dev->I2cDevAddr;

    status_int = VL53L0X_read_dword(deviceAddress, index, data);

    if (status_int != 0)
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;

    return Status;
}

int32_t VL53L0X_read_dword(uint8_t address, uint8_t index, uint32_t *pdata)
{
    int32_t status = STATUS_OK;
	uint8_t  buffer[BYTES_PER_DWORD];

    status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_DWORD);
//    status = HAL_I2C_Mem_Read(&hi2c2, address, index, sizeof(index), buffer, BYTES_PER_DWORD, 100);
    *pdata = ((uint32_t)buffer[0]<<24) + ((uint32_t)buffer[1]<<16) + ((uint32_t)buffer[2]<<8) + (uint32_t)buffer[3];

    return status;

}



VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data){
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    int32_t status_int;
    uint8_t deviceAddress;

    deviceAddress = Dev->I2cDevAddr;

    status_int = VL53L0X_read_word(deviceAddress, index, data);

    if (status_int != 0)
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;

    return Status;
}

int32_t VL53L0X_read_word(uint8_t address, uint8_t index, uint16_t *pdata)
{
    int32_t  status = STATUS_OK;
	uint8_t  buffer[BYTES_PER_WORD];

    status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_WORD);
//    status = HAL_I2C_Mem_Read(&hi2c2, address, index, sizeof(index), buffer, BYTES_PER_WORD, 100);
	*pdata = ((uint16_t)buffer[0]<<8) + (uint16_t)buffer[1];

    return status;

}


VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData){
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    int32_t status_int;
    uint8_t deviceAddress;
    uint8_t data;

    deviceAddress = Dev->I2cDevAddr;

    status_int = VL53L0X_read_byte(deviceAddress, index, &data);

    if (status_int != 0)
        Status = VL53L0X_ERROR_CONTROL_INTERFACE;

    if (Status == VL53L0X_ERROR_NONE) {
        data = (data & AndData) | OrData;
        status_int = VL53L0X_write_byte(deviceAddress, index, data);

        if (status_int != 0)
            Status = VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    return Status;
}

int32_t VL53L0X_read_byte(uint8_t address, uint8_t index, uint8_t *pdata)
{
    int32_t status = STATUS_OK;
    int32_t cbyte_count = 1;

    status = VL53L0X_read_multi(address, index, pdata, cbyte_count);
//    status = HAL_I2C_Mem_Read(&hi2c2, address, index, sizeof(index), pdata, sizeof(uint8_t), 100);

#ifdef VL53L0X_LOG_ENABLE
    trace_print(TRACE_LEVEL_INFO,"Read reg : 0x%02X, Val : 0x%02X\n", index, *pdata);
#endif

    return status;

}

int32_t VL53L0X_write_byte(uint8_t address, uint8_t index, uint8_t data)
{
    int32_t status = STATUS_OK;
    const int32_t cbyte_count = 1;

#ifdef VL53L0X_LOG_ENABLE
    trace_print(TRACE_LEVEL_INFO,"Write reg : 0x%02X, Val : 0x%02X\n", index, data);
#endif

//    status = VL53L0X_write_multi(address, index, &data, cbyte_count);
    status = HAL_I2C_Mem_Write(&hi2c2, address, index, sizeof(index), &data, cbyte_count, 100);

    return status;
}

VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data){
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    int32_t status_int;
	uint8_t deviceAddress;

    deviceAddress = Dev->I2cDevAddr;

	status_int = VL53L0X_write_word(deviceAddress, index, data);

	if (status_int != 0)
		Status = VL53L0X_ERROR_CONTROL_INTERFACE;

    return Status;
}

int32_t VL53L0X_write_word(uint8_t address, uint8_t index, uint16_t data)
{
    int32_t status = STATUS_OK;

    uint8_t  buffer[BYTES_PER_WORD];

    // Split 16-bit word into MS and LS uint8_t
    buffer[0] = (uint8_t)(data >> 8);
    buffer[1] = (uint8_t)(data &  0x00FF);

    if(index%2 == 1)
    {
//        status = VL53L0X_write_multi(address, index, &buffer[0], 1);
        status = HAL_I2C_Mem_Write(&hi2c2, address, index, sizeof(index), &buffer[0], sizeof(buffer[0]), 100);

//        status = VL53L0X_write_multi(address, index + 1, &buffer[1], 1);
        status = HAL_I2C_Mem_Write(&hi2c2, address, index + 1, sizeof(index), &buffer[1], sizeof(buffer[0]), 100);
        // serial comms cannot handle word writes to non 2-byte aligned registers.
    }
    else
    {
//        status = VL53L0X_write_multi(address, index, buffer, BYTES_PER_WORD);
        status = HAL_I2C_Mem_Write(&hi2c2, address, index, sizeof(index), buffer, BYTES_PER_WORD, 100);
    }

    return status;

}

int32_t VL53L0X_read_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{
	if(count < 1 || count > VL53L0X_MAX_I2C_XFER_SIZE)
		return -1;

    uint8_t tmp[VL53L0X_MAX_I2C_XFER_SIZE + 1];
    uint32_t res = HAL_I2C_Mem_Read(&hi2c2, address, index, 1, tmp, count + 1, 100);
    if(res == HAL_OK)
    	memcpy(pdata, tmp, count);

    return res;
}



//VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data);
//
//
//VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data);
//
//
//VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data);
//
//

//VL53L0X_Error VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data);
//
//
