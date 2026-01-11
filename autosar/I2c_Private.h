#ifndef I2C_PRIVATE_H
#define I2C_PRIVATE_H

#include "I2c.h"

/* Queue Status */
#define QUEUE_STATUS_FREE       0
#define QUEUE_STATUS_READY      1
#define QUEUE_STATUS_ACTIVE     2
#define QUEUE_STATUS_COMPLETED  3
#define QUEUE_STATUS_QUEUED     4

/* Hardware Status Flags */
#define I2C_HW_STATUS_NACK      0x10
#define I2C_HW_STATUS_ARB_LOST  0x08
#define I2C_HW_STATUS_BUS_ERROR 0x04
#define I2C_HW_STATUS_ERROR     0x1C

/* Interrupt Types */
#define I2C_INT_TX              0
#define I2C_INT_RX              1
#define I2C_INT_ERROR           2

/* Queue Element */
typedef struct {
    I2c_DataBufferType Buffer;
    uint8 Channel;
    uint8 SequenceId;
    uint8 Status;
} I2c_QueueElementType;

/* Channel State */
typedef struct {
    I2c_SequenceResultType SeqResult;
    boolean IsListening;
    uint8 QueuedCount;
    uint8 ActiveSequence;
    uint8 CurrentJobIndex;
    
    /* Queue */
    I2c_QueueElementType Queue[I2C_MAX_QUEUE_SIZE];
    
    /* Target mode buffers */
    I2c_DataBufferType TargetTxBuffer;
    I2c_DataBufferType TargetRxBuffer;
} I2c_ChannelStateType;

/* External variables */
extern I2c_ChannelStateType I2c_ChannelState[I2C_NUM_CHANNELS];

/* Internal Functions */
static Std_ReturnType I2c_AddToQueue(uint8 Channel, uint8 Seq, const I2c_DataBufferType* BufPtr);
static Std_ReturnType I2c_ProcessNextJob(uint8 Channel, uint8 Seq);
static void I2c_HandleTransferComplete(uint8 Channel);
static void I2c_HandleTargetRequest(uint8 Channel);
static void I2c_ProcessQueuedSequences(uint8 Channel);
static void I2c_CleanupChannel(uint8 Channel);

/* System Functions */
extern uint32 GetSystemTick(void);

#endif /* I2C_PRIVATE_H */