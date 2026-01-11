#include "I2c_Types.h"

/* ================= CHANNEL ================= */


typedef struct {
    uint32 BaseAddress;
    uint32 BaudRate;
    I2c_HwUnitModeType Mode;
    boolean TargetListening;
    I2c_AddressType OwnAddress;   /* dùng cho SLAVE */
    uint32 PeripheralClockHz;     /* clock supplied to peripheral (for WL/WH calc) */
    boolean UseAltPins;           /* chọn nhóm chân alternate nếu TRUE */
} I2c_ChannelType;

/* ================= CHANNEL ================= */
static const I2c_ChannelType I2c_Channels[] =
{
    /* ---------- Channel 0 : MASTER ---------- */
    {
        .BaseAddress      = 0x40066000u,   /* IICA0 */
        .BaudRate         = 100000u,
        .Mode             = I2C_HW_UNIT_MODE_CONTROLLER,
        .TargetListening  = FALSE,
        .OwnAddress       = 0
        , .PeripheralClockHz = 8000000u
        , .UseAltPins = FALSE
    },

    /* ---------- Channel 1 : SLAVE ----------- */
    {
        .BaseAddress      = 0x40067000u,   /* IICA1 */
        .BaudRate         = 100000u,
        .Mode             = I2C_HW_UNIT_MODE_TARGET,
        .TargetListening  = TRUE,
        .OwnAddress       = 0x30            /* SVA */
        , .PeripheralClockHz = 8000000u
        , .UseAltPins = TRUE
    }
};



/* ================= JOB ================= */
typedef struct {
    I2c_JobIdType JobId;
    I2c_AddressType DeviceAddress;
    I2c_JobDirectionType Direction;
} I2c_JobType;

/* ================= JOB ================= */
static const I2c_JobType I2c_Jobs[] =
{
    /* Master write to EEPROM */
    {
        .JobId         = 0,
        .DeviceAddress = 0x50,
        .Direction     = I2C_WRITE
    },

    /* Master read from RTC */
    {
        .JobId         = 1,
        .DeviceAddress = 0x68,
        .Direction     = I2C_READ
    },

    /* Slave job */
    {
        .JobId         = 2,
        .DeviceAddress = 0,          /* không dùng */
        .Direction     = I2C_SLAVE
    }
};

/* ================= SEQUENCE ================= */
typedef void (*I2c_EndNotificationType)(void);

typedef struct {
    I2c_SequenceType SequenceId;
    const I2c_ChannelType* Channel;
    const I2c_JobType** JobList;
    uint8 JobCount;
    I2c_EndNotificationType EndNotification;
} I2c_SequenceCfgType;

/* ===== CALLBACK ===== */
static void I2c_MasterSeq0_End(void) {}
static void I2c_MasterSeq1_End(void) {}
static void I2c_SlaveSeq_End(void) {}

/* ===== SEQ 0 : Master write EEPROM ===== */
static const I2c_JobType* I2c_Seq0_Jobs[] =
{
    &I2c_Jobs[0]
};

/* ===== SEQ 1 : Master write + read ===== */
static const I2c_JobType* I2c_Seq1_Jobs[] =
{
    &I2c_Jobs[0],
    &I2c_Jobs[1]
};

/* ===== SEQ 2 : Slave listen ===== */
static const I2c_JobType* I2c_Seq2_Jobs[] =
{
    &I2c_Jobs[2]
};

/* ================= SEQUENCE ================= */
static const I2c_SequenceCfgType I2c_Sequences[] =
{
    /* -------- Master Sequence 0 -------- */
    {
        .SequenceId      = 0,
        .Channel         = &I2c_Channels[0],
        .JobList         = I2c_Seq0_Jobs,
        .JobCount        = 1,
        .EndNotification = I2c_MasterSeq0_End
    },

    /* -------- Master Sequence 1 -------- */
    {
        .SequenceId      = 1,
        .Channel         = &I2c_Channels[0],
        .JobList         = I2c_Seq1_Jobs,
        .JobCount        = 2,
        .EndNotification = I2c_MasterSeq1_End
    },

    /* -------- Slave Sequence -------- */
    {
        .SequenceId      = 2,
        .Channel         = &I2c_Channels[1],
        .JobList         = I2c_Seq2_Jobs,
        .JobCount        = 1,
        .EndNotification = I2c_SlaveSeq_End
    }
};

typedef struct {
    const I2c_ChannelType* Channels;
    uint8 ChannelCount;

    const I2c_JobType* Jobs;
    uint8 JobCount;

    const I2c_SequenceCfgType* Sequences;
    uint8 SequenceCount;
} I2c_ConfigType;

/* ================= ROOT CONFIG ================= */
const I2c_ConfigType I2c_Config =
{
    .Channels      = I2c_Channels,
    .ChannelCount  = 2,

    .Jobs          = I2c_Jobs,
    .JobCount      = 3,

    .Sequences     = I2c_Sequences,
    .SequenceCount = 3
};


/* MASTER */
I2c_Init(&I2c_Config);
I2c_SetupEB(0, txBuf, NULL, len);
I2c_SyncTransmit(0);

/* SLAVE */
I2c_Init(&I2c_Config);
I2c_SetupEB(2, rxBuf, txBuf, len);
I2c_StartListening(2);


/////////////////////////////////////////////////////////
/* ================= GLOBAL STATE ================= */

typedef enum {
    I2C_UNINIT = 0,
    I2C_INIT
} I2c_DriverStateType;

typedef struct {
    const I2c_JobType* Job;
    uint8* TxBuf;
    uint8* RxBuf;
    uint32 Length;
    uint32 Index;
    I2c_AddressType NodeAddr;
    boolean IsRead;
} I2c_JobRuntimeType;

typedef struct {
    I2c_SequenceResultType Result;
    uint8 CurrentJob;
    boolean Active;
} I2c_SequenceRuntimeType;

/* ================= INTERNAL DATA ================= */

static I2c_DriverStateType I2c_State = I2C_UNINIT;
static const I2c_ConfigType* I2c_ConfigPtr = NULL;

static I2c_JobRuntimeType I2c_JobRt[10];
static I2c_SequenceRuntimeType I2c_SeqRt[10];

void I2C_Init(const I2c_ConfigType* ConfigPtr)
{
    uint8 i;

    if (ConfigPtr == NULL) return;

    I2c_ConfigPtr = ConfigPtr;

    for (i = 0; i < ConfigPtr->ChannelCount; i++) {
        I2C_Hw_Init(&ConfigPtr->Channels[i]);
    }

    for (i = 0; i < ConfigPtr->SequenceCount; i++) {
        I2c_SeqRt[i].Result = I2C_SEQ_OK;
        I2c_SeqRt[i].Active = FALSE;
    }

    I2c_State = I2C_INIT;
}

void I2C_DeInit(void)
{
    uint8 i;

    if (I2c_State != I2C_INIT) return;

    for (i = 0; i < I2c_ConfigPtr->ChannelCount; i++) {
        I2C_Hw_DeInit(&I2c_ConfigPtr->Channels[i]);
    }

    I2c_State = I2C_UNINIT;
}

Std_ReturnType I2C_SetupEB(
    I2C_JobType JobId,
    I2C_AddressType NodeAddress,
    uint8* TxBuf,
    uint8* RxBuf,
    uint32 Length)
{
    I2c_JobRuntimeType* rt;

    if (I2c_State != I2C_INIT) return E_NOT_OK;

    if ((TxBuf == NULL) && (RxBuf == NULL)) return E_NOT_OK;
    if ((TxBuf != NULL) && (RxBuf != NULL)) return E_NOT_OK;

    rt = &I2c_JobRt[JobId];

    rt->Job = &I2c_ConfigPtr->Jobs[JobId];
    rt->TxBuf = TxBuf;
    rt->RxBuf = RxBuf;
    rt->Length = Length;
    rt->Index = 0;

    rt->NodeAddr = (NodeAddress != 0) ?
                   NodeAddress : rt->Job->DeviceAddress;

    rt->IsRead = (RxBuf != NULL);

    return E_OK;
}

Std_ReturnType I2C_SyncTransmit(I2C_SequenceType SequenceId)
{
    const I2c_SequenceCfgType* seq;
    I2c_SequenceRuntimeType* rt;
    uint8 j;

    if (I2c_State != I2C_INIT) return E_NOT_OK;

    seq = &I2c_ConfigPtr->Sequences[SequenceId];
    rt = &I2c_SeqRt[SequenceId];

    if (rt->Active) return E_NOT_OK;

    rt->Active = TRUE;
    rt->Result = I2C_SEQ_PENDING;

    for (j = 0; j < seq->JobCount; j++) {
        const I2c_JobType* job = seq->JobList[j];
        I2c_JobRuntimeType* jrt = &I2c_JobRt[job->JobId];

        I2C_Hw_Start(seq->Channel, jrt->NodeAddr, jrt->IsRead);

        while (jrt->Index < jrt->Length) {
            if (jrt->IsRead)
                jrt->RxBuf[jrt->Index++] = I2C_Hw_Read(seq->Channel);
            else
                I2C_Hw_Write(seq->Channel, jrt->TxBuf[jrt->Index++]);
        }

        I2C_Hw_Stop(seq->Channel);
    }

    rt->Result = I2C_SEQ_OK;
    rt->Active = FALSE;

    if (seq->EndNotification) seq->EndNotification();

    return E_OK;
}

Std_ReturnType I2C_AsyncTransmit(I2C_SequenceType SequenceId)
{
    I2c_SequenceRuntimeType* rt = &I2c_SeqRt[SequenceId];

    if (rt->Active) return E_NOT_OK;

    rt->Active = TRUE;
    rt->Result = I2C_SEQ_PENDING;
    rt->CurrentJob = 0;

    return E_OK;
}

void I2C_MainFunction(void)
{
    uint8 s;

    for (s = 0; s < I2c_ConfigPtr->SequenceCount; s++) {
        I2c_SequenceRuntimeType* rt = &I2c_SeqRt[s];
        const I2c_SequenceCfgType* seq = &I2c_ConfigPtr->Sequences[s];

        if (!rt->Active) continue;

        const I2c_JobType* job = seq->JobList[rt->CurrentJob];
        I2c_JobRuntimeType* jrt = &I2c_JobRt[job->JobId];

        if (jrt->Index == 0)
            I2C_Hw_Start(seq->Channel, jrt->NodeAddr, jrt->IsRead);

        if (jrt->Index < jrt->Length) {
            if (jrt->IsRead)
                jrt->RxBuf[jrt->Index++] = I2C_Hw_Read(seq->Channel);
            else
                I2C_Hw_Write(seq->Channel, jrt->TxBuf[jrt->Index++]);
        } else {
            I2C_Hw_Stop(seq->Channel);
            rt->CurrentJob++;

            if (rt->CurrentJob >= seq->JobCount) {
                rt->Active = FALSE;
                rt->Result = I2C_SEQ_OK;
                if (seq->EndNotification) seq->EndNotification();
            }
        }
    }
}
I2c_SequenceResultType I2C_GetSequenceResult(I2C_SequenceType SequenceId)
{
    return I2c_SeqRt[SequenceId].Result;
}

void I2C_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
    if (VersionInfo == NULL) return;

    VersionInfo->vendorID = 1;
    VersionInfo->moduleID = 255;
    VersionInfo->sw_major_version = 24;
    VersionInfo->sw_minor_version = 11;
    VersionInfo->sw_patch_version = 0;
}

Std_ReturnType I2C_StartListening(I2C_SequenceType SequenceId)
{
    const I2c_SequenceCfgType* seq = &I2c_ConfigPtr->Sequences[SequenceId];
    I2c_SequenceRuntimeType* rt = &I2c_SeqRt[SequenceId];

    if (seq->Channel->Mode != I2C_HW_UNIT_MODE_TARGET) return E_NOT_OK;
    if (rt->Active) return E_NOT_OK;

    I2C_Hw_EnableSlave(seq->Channel, seq->Channel->OwnAddress);

    rt->Active = TRUE;
    rt->Result = I2C_SEQ_PENDING;

    return E_OK;
}

void I2C_Hw_Init(const I2c_ChannelType* Ch)
{
    volatile uint8* CTL = (uint8*)(Ch->BaseAddress + 0x00);
    *CTL = 0x80; /* enable clock */
}

void I2C_Hw_Start(const I2c_ChannelType* Ch, uint16 Addr, boolean Read)
{
    /* START + address + R/W */
}

void I2C_Hw_Write(const I2c_ChannelType* Ch, uint8 Data)
{
    volatile uint8* DR = (uint8*)(Ch->BaseAddress + 0x04);
    *DR = Data;
}

uint8 I2C_Hw_Read(const I2c_ChannelType* Ch)
{
    volatile uint8* DR = (uint8*)(Ch->BaseAddress + 0x04);
    return *DR;
}

void I2C_Hw_Stop(const I2c_ChannelType* Ch)
{
    /* STOP condition */
}
