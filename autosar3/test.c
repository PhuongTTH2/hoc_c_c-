/******************************************************************************
* File: App_I2cExample.c
* Description: Ví dụ sử dụng I2C Driver
******************************************************************************/

#include "I2c.h"

/* Buffer dữ liệu */
uint8 TxBuffer[10] = {0x01, 0x02, 0x03, 0x04, 0x05};
uint8 RxBuffer[10];

/******************************************************************************
* Callback: I2C_SeqEndNotification0
* Mô tả: Callback khi Sequence 0 hoàn thành
******************************************************************************/
void I2C_SeqEndNotification0(I2C_SequenceType SequenceId, 
                            I2C_SequenceResultType Result)
{
    if (Result == I2C_SEQ_OK)
    {
        /* Xử lý khi truyền thành công */
    }
    else
    {
        /* Xử lý lỗi */
    }
}

/******************************************************************************
* Hàm: Main_I2C_Example
* Mô tả: Ví dụ sử dụng I2C Driver
******************************************************************************/
void Main_I2C_Example(void)
{
    Std_ReturnType Ret;
    
    /* 1. Khởi tạo I2C Driver */
    I2C_Init(&I2C_ConfigSet[0]);
    
    /* 2. Cấu hình Job để ghi dữ liệu */
    Ret = I2C_SetupEB(
        0,              /* JobId = 0 */
        0x50,           /* Địa chỉ slave */
        TxBuffer,       /* Buffer truyền */
        NULL_PTR,       /* Không đọc */
        5               /* 5 bytes */
    );
    
    if (Ret != E_OK)
    {
        /* Xử lý lỗi */
    }
    
    /* 3. Truyền dữ liệu bất đồng bộ */
    Ret = I2C_AsyncTransmit(0); /* SequenceId = 0 */
    
    if (Ret != E_OK)
    {
        /* Xử lý lỗi */
    }
    
    /* 4. Kiểm tra trạng thái */
    while (I2C_GetSequenceResult(0) == I2C_SEQ_PENDING)
    {
        /* Chờ hoàn thành */
        I2C_MainFunction();
    }
    
    /* 5. Cấu hình Job để đọc dữ liệu */
    Ret = I2C_SetupEB(
        1,              /* JobId = 1 */
        0x76,           /* Địa chỉ sensor */
        NULL_PTR,       /* Không ghi */
        RxBuffer,       /* Buffer nhận */
        8               /* 8 bytes */
    );
    
    /* 6. Truyền dữ liệu đồng bộ */
    Ret = I2C_SyncTransmit(1); /* SequenceId = 1 */
    
    /* 7. Lấy thông tin phiên bản */
    Std_VersionInfoType Version;
    I2C_GetVersionInfo(&Version);
    
    /* 8. Hủy khởi tạo khi không cần */
    I2C_DeInit();
}