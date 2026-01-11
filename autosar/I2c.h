#ifndef I2C_H
#define I2C_H

#include "Std_Types.h"

/* Module ID */
#define I2C_MODULE_ID       110U
#define I2C_VENDOR_ID       0x0040U

/* Service IDs */
#define I2C_SID_INIT               0x00U
#define I2C_SID_DEINIT             0x01U
#define I2C_SID_SETUPEB            0x02U
#define I2C_SID_ASYNCTRANSMIT      0x03U
#define I2C_SID_SYNCTRANSMIT       0x04U
#define I2C_SID_GETVERSIONINFO     0x05U
#define I2C_SID_GETSEQUENCERESULT  0x06U
#define I2C_SID_STARTLISTENING     0x07U
#define I2C_SID_MAINFUNCTION       0x08U

/* Error Codes */
#define I2C_E_PARAM_CHANNEL        0x01U
#define I2C_E_PARAM_SEQ            0x02U
#define I2C_E_PARAM_POINTER        0x03U
#define I2C_E_PARAM_DATA           0x04U
#define I2C_E_BUSY                 0x05U
#define I2C_E_NO_COMM              0x06U
#define I2C_E_TIMEOUT              0x07U

/* Sequence Result Values */
#define I2C_SEQ_OK                 0x00U
#define I2C_SEQ_PENDING            0x01U
#define I2C_SEQ_FAILED             0x02U
#define I2C_SEQ_QUEUED             0x04U

/* Buffer Type */
typedef struct {
    uint8* DataPtr;
    uint16 Length;
    uint8 JobId;
    uint8 NodeAddress;
    boolean IsWrite;
} I2c_DataBufferType;

/* Sequence Result Type */
typedef struct {
    uint8 SequenceResult;
    uint8 ErrorStatus;
} I2c_SequenceResultType;

/* Channel Config */
typedef struct {
    uint32 I2cBaudRate;
    uint32 I2cHwUnitBaseAddress;
    uint8  I2cHwUnitMode;
    boolean I2cTargetListening;
} I2c_ChannelConfigType;

/* Job Config */
typedef struct {
    uint16 I2cDeviceAddress;
    uint8  I2cJobId;
} I2c_JobConfigType;

/* Sequence Config */
typedef struct {
    void (*I2cEndNotification)(uint8 Channel, uint8 Seq);
    uint8  I2cSequenceId;
    const I2c_ChannelConfigType* I2cAssignedChannel;
    const I2c_JobConfigType* I2cAssignedJob;
    uint8  NumAssignedJobs;
} I2c_SequenceConfigType;

/* General Config */
typedef struct {
    boolean I2cDevErrorDetect;
    boolean I2cVersionInfoApi;
} I2c_GeneralType;

/* Main Config */
typedef struct {
    I2c_GeneralType I2cGeneral;
    const I2c_ChannelConfigType* I2cChannelConfig;
    const I2c_JobConfigType* I2cJobConfig;
    const I2c_SequenceConfigType* I2cSequenceConfig;
    uint8 NumChannels;
    uint8 NumJobs;
    uint8 NumSequences;
} I2c_ConfigType;

/* Function Prototypes */
void I2c_Init(const I2c_ConfigType* ConfigPtr);
void I2c_DeInit(void);
Std_ReturnType I2c_SetupEB(uint8 Channel, uint8 Seq, const I2c_DataBufferType* BufPtr);
Std_ReturnType I2c_AsyncTransmit(uint8 Channel, uint8 Seq);
Std_ReturnType I2c_SyncTransmit(uint8 Channel, uint8 Seq, uint32 Timeout);
void I2c_GetVersionInfo(Std_VersionInfoType* versioninfo);
Std_ReturnType I2c_GetSequenceResult(uint8 Channel, uint8 Seq, I2c_SequenceResultType* ResultPtr);
Std_ReturnType I2c_StartListening(uint8 Channel);
void I2c_MainFunction(void);

extern const I2c_ConfigType I2c_Config;

#endif /* I2C_H */