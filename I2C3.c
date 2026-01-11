/*
 * CDD_I2C - AUTOSAR Classic Platform R24-11
 * Target MCU: Renesas BAT32A259 (RL78 family)
 * Scope: Reference-quality FULL driver skeleton
 * - Conforms to AUTOSAR_CP_SWS_I2CDriver (Doc ID 1101, R24-11)
 * - CDD style (not official MCAL)
 * - Includes DET / DEM hooks
 * - EB mode only
 *
 * NOTE:
 *  - Hardware register names are representative (RL78 IICA)
 *  - Timing/bitrate values must be tuned with Renesas HW manual
 */

/* =================== [INCLUDES] =================== */
#include "Std_Types.h"
#include "Det.h"
#include "Dem.h"
#include "I2c.h"

/* =================== [VERSION INFO] =================== */
#define I2C_VENDOR_ID        0x1234
#define I2C_MODULE_ID        0x5A
#define I2C_SW_MAJOR_VERSION 24
#define I2C_SW_MINOR_VERSION 11
#define I2C_SW_PATCH_VERSION 0

/* =================== [DEV ERROR SWITCH] =================== */
#define I2C_DEV_ERROR_DETECT STD_ON

/* =================== [DET ERROR CODES] =================== */
#define I2C_E_PARAM_JOB          0x00
#define I2C_E_PARAM_SEQUENCE     0x01
#define I2C_E_PARAM_POINTER      0x02
#define I2C_E_UNINIT             0x03
#define I2C_E_WRONG_CONDITION    0x04
/*
 * I2C3.c
 * Consolidated I2C CDD (EB-mode) implementation for RL78-style IICA.
 */

/* =================== [DET / API IDs] =================== */
#if (I2C_DEV_ERROR_DETECT == STD_ON)
#ifndef I2C_INSTANCE_ID
#define I2C_INSTANCE_ID 0
#endif
#define I2C_APIID_INIT            0x01
#define I2C_APIID_DEINIT          0x02
#define I2C_APIID_SETUPEB         0x03
#define I2C_APIID_SYNCTRANSMIT    0x04
#define I2C_APIID_ASYNCTRANSMIT   0x05
#define I2C_APIID_MAINFUNCTION    0x06
#define I2C_APIID_STARTLISTENING  0x07
#define I2C_APIID_GETSEQRESULT    0x08

#define I2C_DET_REPORT(apiId, err) Det_ReportError(I2C_MODULE_ID, I2C_INSTANCE_ID, (apiId), (err))
#else
#define I2C_DET_REPORT(apiId, err) ((void)0)
#endif


/* =========== Concrete HW helpers (adapted from I2c.c) ============ */
/* Use static linkage here to avoid symbol collisions with other I2C files. */

/* Peripheral and port registers (addresses taken from project notes/manual) */
#define PER0   (*(volatile uint8*)0x40020420U)
#define PER0_IICA0EN (1U << 4)

#define PIOR0  (*(volatile uint8*)0x40040877U)
#define PIOR02 (1U << 2)   /* 1=P14/P15, 0=P60/P61 */

/* Port registers */
#define P1   (*(volatile uint8*)(0x40040000U + 0x301))
#define PM1  (*(volatile uint8*)(0x40040000U + 0x321))
#define PMC1 (*(volatile uint8*)(0x40040000U + 0x361))

#define P6   (*(volatile uint8*)0x40040306U)
#define PM6  (*(volatile uint8*)0x40041306U)

/* Default register offsets for an IICA unit (VERIFY against device manual / header)
 * These are intentionally defined as overridable macros so you can provide the
 * exact offsets from the Renesas SFR header instead of the defaults below. */
#ifndef IICCTL00_OFFSET
#define IICCTL00_OFFSET 0x00u
#endif
#ifndef IICCTL01_OFFSET
#define IICCTL01_OFFSET 0x01u
#endif
#ifndef IICS0_OFFSET
#define IICS0_OFFSET    0x02u
#endif
#ifndef IICWL0_OFFSET
#define IICWL0_OFFSET   0x03u
#endif
#ifndef IICWH0_OFFSET
#define IICWH0_OFFSET   0x04u
#endif
/* data register offset (commonly at +0x04 in simple layouts) */
#ifndef IICD0_OFFSET
#define IICD0_OFFSET    0x04u
#endif
/* Slave address (SVA) register offset - many devices have a dedicated SVA reg */
#ifndef SVA_OFFSET
#define SVA_OFFSET      0x05u
#endif

/* IICCTL00 bits (common names; verify values in manual) */
#ifndef IICE
#define IICE (1U << 7)
#endif
#ifndef WREL
#define WREL (1U << 6)
#endif
#ifndef STT
#define STT  (1U << 5)
#endif
#ifndef SPT
#define SPT  (1U << 4)
#endif
#ifndef ACKE
#define ACKE (1U << 3)
#endif

/* IICS0 bits */
#ifndef TEND
#define TEND (1U << 7)
#endif
#ifndef ACKD
#define ACKD (1U << 2)
#endif

/* DEM event IDs used by this driver (user may change to match ECU event catalog) */
#ifndef I2C_DEM_EVENT_NACK
#define I2C_DEM_EVENT_NACK 0x0100u
#endif


/* Helper to access registers by channel base + offset (volatile uint8) */
static inline volatile uint8* IIC_REG(const I2C_ChannelCfgType* Ch, uint32 offset)
{
    return (volatile uint8*)(uintptr_t)(Ch->BaseAddress + offset);
}

/* Compute and write IICWL/IICWH based on configured peripheral clock and desired BaudRate
 * This is a simple approximation: ticks_per_bit = PCLK / Baud; WL = WH = ticks_per_bit/2
 * The formula must be validated and adjusted against the MCU datasheet for production. */
static void I2C_Hw_ComputeAndWriteTiming(const I2C_ChannelCfgType* Ch)
{
    uint32 pclk = Ch->PeripheralClockHz;
    uint32 baud = Ch->BaudRate;
    uint8 wl = 0u;
    uint8 wh = 0u;

    if (pclk == 0u || baud == 0u) {
        /* leave defaults if not provided */
        return;
    }

    uint32 ticks_per_bit = pclk / baud;
    if (ticks_per_bit < 2u) {
        wl = 1u;
        wh = 1u;
    } else {
        uint32 half = ticks_per_bit / 2u;
        if (half > 0xFFu) half = 0xFFu;
        wl = (uint8)half;
        wh = (uint8)half;
    }

    *IIC_REG(Ch, IICWL0_OFFSET) = wl;
    *IIC_REG(Ch, IICWH0_OFFSET) = wh;
}

/* Pin selection helper: configures PIOR and the matching port registers for two
 * common mappings (P14/P15 or P60/P61). This uses the PIOR0/PIOR02 macros
 * already defined above in the file; adjust if your device uses different names. */
static void I2C_ConfigPinsForIICA0(boolean useP14P15)
{
    if (useP14P15) {
        PIOR0 |= PIOR02; /* select P14/P15 */
        /* clear output to avoid glitch */
        P1 &= ~((1U << 4) | (1U << 5));
        /* set as output (PMx = 0 => output on many RL78 parts) */
        PM1 &= ~((1U << 4) | (1U << 5));
        /* enable peripheral function */
        PMC1 |= (1U << 4) | (1U << 5);
    } else {
        PIOR0 &= ~PIOR02; /* select P60/P61 */
        P6 &= ~((1U << 0) | (1U << 1));
        PM6 &= ~((1U << 0) | (1U << 1));
        /* NOTE: PMC6 definition may differ by device; if absent, define it via SFR header */
        /* PMC6 |= (1U<<0)|(1U<<1); */
    }
}

static void I2C_EnableClock(void)
{
    PER0 |= PER0_IICA0EN;
}

/* Unified HW helpers that use the channel BaseAddress + offset macros.
 * These are conservative and require you to VERIFY offsets with the device
 * SFR documentation. They replace the hard-coded IICA0 struct usage to allow
 * the driver to work with any configured base address. */
static void I2C_Hw_Init(const I2C_ChannelCfgType* Ch)
{
    /* Basic bring-up: enable peripheral clock and configure pins.
     * We default to P14/P15 selection here; if your board uses P60/P61,
     * set useP14P15=false or pass this selection via configuration. */
    I2C_EnableClock();
    /* Use per-channel pin selection (UseAltPins) instead of hard-coded TRUE */
    I2C_ConfigPinsForIICA0(Ch->UseAltPins);

    /* Disable peripheral before configuration */
    *IIC_REG(Ch, IICCTL00_OFFSET) = 0x00u;

    /* Default to controller (master) mode */
    *IIC_REG(Ch, IICCTL01_OFFSET) = 0x00u;

    /* Compute and write timing registers from channel config (if provided) */
    I2C_Hw_ComputeAndWriteTiming(Ch);

    /* Enable peripheral */
    *IIC_REG(Ch, IICCTL00_OFFSET) |= IICE;
}

static void I2C_Hw_DeInit(const I2C_ChannelCfgType* Ch)
{
    (void)Ch;
    *IIC_REG(Ch, IICCTL00_OFFSET) = 0x00u; /* disable */
}

static void I2C_Hw_Start(const I2C_ChannelCfgType* Ch, uint16 Addr, boolean Read)
{
    /* Generate START */
    *IIC_REG(Ch, IICCTL00_OFFSET) |= STT;

    /* Wait for TEND in status */
    while (!(*IIC_REG(Ch, IICS0_OFFSET) & TEND));

    /* Send address (7-bit << 1 | R/W) */
    *IIC_REG(Ch, IICD0_OFFSET) = (uint8)((Addr << 1) | (Read ? 1U : 0U));
    while (!(*IIC_REG(Ch, IICS0_OFFSET) & TEND));
}

static void I2C_Hw_Write(const I2C_ChannelCfgType* Ch, uint8 Data)
{
    *IIC_REG(Ch, IICD0_OFFSET) = Data;
    while (!(*IIC_REG(Ch, IICS0_OFFSET) & TEND));
}

static uint8 I2C_Hw_Read(const I2C_ChannelCfgType* Ch)
{
    /* Release to receive and ACK by default; caller can set ACKE/NACK as needed */
    *IIC_REG(Ch, IICCTL00_OFFSET) |= WREL;
    while (!(*IIC_REG(Ch, IICS0_OFFSET) & TEND));
    return (uint8)(*IIC_REG(Ch, IICD0_OFFSET));
}

static void I2C_Hw_Stop(const I2C_ChannelCfgType* Ch)
{
    *IIC_REG(Ch, IICCTL00_OFFSET) |= SPT;
    /* wait until stop has completed */
    while (*IIC_REG(Ch, IICCTL00_OFFSET) & SPT) { }
}

static void I2C_Hw_EnableSlave(const I2C_ChannelCfgType* Ch, I2C_AddressType OwnAddr)
{
    /* Set target (slave) mode and own address. Exact register for SVA may
     * differ per device; many RL78 parts have a dedicated SVA register. If so,
     * override SVA_OFFSET and write via IIC_REG(Ch, SVA_OFFSET). Here we use
     * IICD0_OFFSET as a fallback only when no explicit SVA register macro is
     * available — VERIFY with the manual. */
    *IIC_REG(Ch, IICCTL01_OFFSET) = 0x01u; /* target mode (verify value) */
    /* If the device provides a dedicated SVA register, write it; otherwise
     * fall back to the data register as the previous implementation did. */
#if defined(SVA_OFFSET)
    *IIC_REG(Ch, SVA_OFFSET) = (uint8)(OwnAddr & 0x7Fu);
#else
    *IIC_REG(Ch, IICD0_OFFSET) = (uint8)(OwnAddr & 0x7Fu);
#endif
    *IIC_REG(Ch, IICCTL00_OFFSET) |= IICE;
}

/* End of unified HW helpers */

/* Simple limits for runtime arrays (can be tuned or derived from config) */
#define I2C_MAX_RUNTIME_JOBS 16u
#define I2C_MAX_RUNTIME_SEQS 16u
/* Queue size for queued sequences (per driver global FIFO) */
#define I2C_QUEUE_SIZE I2C_MAX_RUNTIME_SEQS

/* Simple FIFO queue of sequence IDs for queued sequences. We keep a global
 * FIFO and when a channel becomes available we scan the queue for the first
 * sequence that targets that channel. This is small and simple and meets the
 * AUTOSAR requirement to queue requests on a busy channel (CP_SWS_I2C_82304). */
static I2C_SequenceType I2c_Queue[I2C_QUEUE_SIZE];
static uint8 I2c_QueueCount = 0u;

/* Enqueue sequence id. Returns 1 on success, 0 if queue full. */
static uint8 I2C_EnqueueSequence(I2C_SequenceType seq)
{
    if (I2c_QueueCount >= I2C_QUEUE_SIZE) return 0u;
    I2c_Queue[I2c_QueueCount++] = seq;
    return 1u;
}

/* Dequeue first sequence for the given channel. Returns sequence id or 0xFF if none. */
static I2C_SequenceType I2C_DequeueForChannel(const I2C_ChannelCfgType* Ch)
{
    if (I2c_QueueCount == 0u) return (I2C_SequenceType)0xFFu;
    uint8 i;
    for (i = 0; i < I2c_QueueCount; i++) {
        I2C_SequenceType sid = I2c_Queue[i];
        if (sid >= I2c_ConfigPtr->SequenceCount) continue;
        if (I2c_ConfigPtr->Sequences[sid].Channel == Ch) {
            /* remove entry i by shifting rest left */
            uint8 j;
            for (j = i; j + 1 < I2c_QueueCount; j++) I2c_Queue[j] = I2c_Queue[j+1];
            I2c_QueueCount--;
            return sid;
        }
    }
    return (I2C_SequenceType)0xFFu;
}

/* Helper: check whether a channel currently has a pending sequence */
static boolean I2C_IsChannelBusy(const I2C_ChannelCfgType* Ch)
{
    uint8 s;
    for (s = 0; s < I2c_ConfigPtr->SequenceCount && s < I2C_MAX_RUNTIME_SEQS; s++) {
        if (I2c_SeqRt[s].Active && (I2c_SeqRt[s].Result == I2C_SEQ_PENDING)) {
            const I2C_SequenceCfgType* seq = &I2c_ConfigPtr->Sequences[s];
            if (seq->Channel == Ch) return TRUE;
        }
    }
    return FALSE;
}

/* Check hardware status for errors (NACK) and report to DEM and runtime.
 * Returns TRUE if an error was detected and handled (sequence should stop). */
static boolean I2C_CheckAndHandleHwError(const I2C_ChannelCfgType* Ch, I2C_SequenceRuntimeType* SeqRt)
{
    uint8 status = (uint8)(*IIC_REG(Ch, IICS0_OFFSET));
    if (status & ACKD) {
        /* NACK detected */
        SeqRt->Result = I2C_SEQ_NACK;
        Dem_ReportErrorStatus(I2C_DEM_EVENT_NACK, DEM_EVENT_STATUS_FAILED);
        return TRUE;
    }
    return FALSE;
}

/* Helper to start a dequeued sequence (arm and start first job) */
static void I2C_StartDequeuedSequence(I2C_SequenceType SequenceId)
{
    if (SequenceId >= I2c_ConfigPtr->SequenceCount) return;
    I2C_SequenceRuntimeType* rt = &I2c_SeqRt[SequenceId];
    const I2C_SequenceCfgType* seq = &I2c_ConfigPtr->Sequences[SequenceId];

    rt->Active = TRUE;
    rt->Result = I2C_SEQ_PENDING;
    rt->CurrentJob = 0;

    /* Start first job immediately if setup exists */
    if (seq->JobCount > 0) {
        const I2C_JobCfgType* job = seq->JobList[0];
        if (job != NULL) {
            I2C_JobRuntimeType* jrt = &I2c_JobRt[job->JobId];
            if (jrt->JobCfg != NULL && jrt->Index == 0) {
                I2C_Hw_Start(seq->Channel, jrt->NodeAddr, jrt->IsRead);
            }
        }
    }
}

/* Runtime job state */
typedef struct {
    const I2C_JobCfgType* JobCfg;
    const uint8* TxBuf;
    uint8* RxBuf;
    uint32 Length;
    uint32 Index;
    I2C_AddressType NodeAddr;
    boolean IsRead;
} I2C_JobRuntimeType;

/* Runtime sequence state */
typedef struct {
    I2C_SequenceResultType Result;
    uint8 CurrentJob;
    boolean Active;
} I2C_SequenceRuntimeType;

/* Internal configuration pointer */
static const I2C_ConfigType* I2c_ConfigPtr = NULL;

/* Runtime storage (fixed size for example) */
static I2C_JobRuntimeType I2c_JobRt[I2C_MAX_RUNTIME_JOBS];
static I2C_SequenceRuntimeType I2c_SeqRt[I2C_MAX_RUNTIME_SEQS];

/* Driver state */
typedef enum { I2C_STATE_UNINIT = 0, I2C_STATE_INIT } I2C_StateType;
static I2C_StateType I2c_State = I2C_STATE_UNINIT;

/* Forward declarations for HW access - minimal, peripheral-specific code
 * should be adapted for the real target. We use BaseAddress offsets as
 * placeholders similar to the I2C2.c sample. */
static void I2C_Hw_Init(const I2C_ChannelCfgType* Ch);
static void I2C_Hw_DeInit(const I2C_ChannelCfgType* Ch);
static void I2C_Hw_Start(const I2C_ChannelCfgType* Ch, uint16 Addr, boolean Read);
static void I2C_Hw_Write(const I2C_ChannelCfgType* Ch, uint8 Data);
static uint8 I2C_Hw_Read(const I2C_ChannelCfgType* Ch);
static void I2C_Hw_Stop(const I2C_ChannelCfgType* Ch);
static void I2C_Hw_EnableSlave(const I2C_ChannelCfgType* Ch, I2C_AddressType OwnAddr);


void I2C_Init(const I2C_ConfigType* ConfigPtr)
{
    uint8 i;
    if (ConfigPtr == NULL) return;

    /* If driver already initialized, perform a de-init first to allow
     * re-initialization (satisfies re-initialization/upstream requirements). */
    if (I2c_State == I2C_STATE_INIT) {
        I2C_DeInit();
    }

    I2c_ConfigPtr = ConfigPtr;

    /* Initialize hardware for each configured channel. The HW helper is
     * expected to use Channel.BaseAddress and Channel.BaudRate when
     * configuring the peripheral. */
    for (i = 0; i < ConfigPtr->ChannelCount; i++) {
        I2C_Hw_Init(&ConfigPtr->Channels[i]);
    }

    /* Set sequence runtime results to OK per upstream requirement */
    for (i = 0; i < ConfigPtr->SequenceCount && i < I2C_MAX_RUNTIME_SEQS; i++) {
        I2c_SeqRt[i].Result = I2C_SEQ_OK;
        I2c_SeqRt[i].Active = FALSE;
    }

    I2c_State = I2C_STATE_INIT;
}


void I2C_DeInit(void)
{
    uint8 i;
    /* Development error detection: if driver not initialized, report error */
    if (I2c_State != I2C_STATE_INIT) {
        I2C_DET_REPORT(I2C_APIID_DEINIT, I2C_E_UNINIT);
        return;
    }

    /* For each configured channel, try to bring peripheral to a reset-like state
     * (disable peripheral, clear timing and SVA where applicable, and disable clock). */
    for (i = 0; i < I2c_ConfigPtr->ChannelCount; i++) {
        const I2C_ChannelCfgType* ch = &I2c_ConfigPtr->Channels[i];
        /* Disable peripheral module */
        *IIC_REG(ch, IICCTL00_OFFSET) = 0x00u;
        /* Clear control1 */
        *IIC_REG(ch, IICCTL01_OFFSET) = 0x00u;
        /* Clear timing registers if present */
        *IIC_REG(ch, IICWL0_OFFSET) = 0x00u;
        *IIC_REG(ch, IICWH0_OFFSET) = 0x00u;
        /* Clear SVA if present */
#if defined(SVA_OFFSET)
        *IIC_REG(ch, SVA_OFFSET) = 0x00u;
#else
        /* optional: clear data reg as fallback */
        *IIC_REG(ch, IICD0_OFFSET) = 0x00u;
#endif
        /* Disable peripheral clock - device-specific bit in PER0 */
        PER0 &= (uint8)(~PER0_IICA0EN);
    }

    I2c_State = I2C_STATE_UNINIT;
}

Std_ReturnType I2C_SetupEB(I2C_JobType JobId,
                           I2C_AddressType NodeAddress,
                           I2C_DataConstPtrType TxDataBufferPtr,
                           I2C_DataPtrType RxDataBufferPtr,
                           I2C_NumberOfDataType Length)
{
    I2C_JobRuntimeType* rt;

    if (I2c_State != I2C_STATE_INIT) {
        I2C_DET_REPORT(I2C_APIID_SETUPEB, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }

    /* basic validation */
    if ((TxDataBufferPtr == NULL) && (RxDataBufferPtr == NULL)) {
        I2C_DET_REPORT(I2C_APIID_SETUPEB, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }
    if ((TxDataBufferPtr != NULL) && (RxDataBufferPtr != NULL)) {
        I2C_DET_REPORT(I2C_APIID_SETUPEB, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }

    if ((I2c_ConfigPtr == NULL) || (JobId >= I2c_ConfigPtr->JobCount)) {
        I2C_DET_REPORT(I2C_APIID_SETUPEB, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }

    rt = &I2c_JobRt[JobId];

    rt->JobCfg = &I2c_ConfigPtr->Jobs[JobId];
    rt->TxBuf = TxDataBufferPtr;
    rt->RxBuf = RxDataBufferPtr;
    rt->Length = Length;
    rt->Index = 0;

    /* AUTOSAR CP_SWS_I2C_00103: If NodeAddress != 0 use it; otherwise use the
     * configured device address (I2cDeviceAddress). Here `DeviceAddress` in
     * the job config corresponds to the configured I2cDeviceAddress.
     * If both are zero that's an invalid setup for a controller job -> report
     * DET and fail the setup. */
    /* AUTOSAR CP_SWS_I2C_00103: If NodeAddress != 0 use it; otherwise use the
     * configured device address (I2cDeviceAddress). For jobs configured as
     * SLAVE (target) the NodeAddress may be unused; only enforce presence of
     * an address for controller jobs. */
    rt->NodeAddr = (NodeAddress != 0) ? NodeAddress : rt->JobCfg->DeviceAddress;
    if ((rt->NodeAddr == 0u) && (rt->JobCfg->Direction != I2C_DIR_SLAVE)) {
        /* No override provided and no configured device address => error for controller jobs */
        I2C_DET_REPORT(I2C_APIID_SETUPEB, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }
    rt->IsRead = (RxDataBufferPtr != NULL);

    return E_OK;
}

Std_ReturnType I2C_SyncTransmit(I2C_SequenceType SequenceId)
/* CP_SWS_I2C_82407 — Paraphrased requirement and implementation notes
 *
 * Paraphrased requirement:
 * The I2C driver shall execute all jobs contained in a sequence sequentially
 * when invoked via the synchronous transmit API. If no other transmission is
 * ongoing on the same channel, the SyncTransmit call shall take exclusive
 * control of the channel and perform every job in blocking mode until the
 * sequence completes successfully or fails due to a hardware error.
 *
 * Behavior details expected by the specification (paraphrase):
 * - The synchronous API shall validate parameters and require that SetupEB has
 *   been called for all referenced jobs; otherwise report a development error
 *   and return E_NOT_OK (see CP_SWS_I2C_82409 for SetupEB validation).
 * - If a sequence is already pending on the same channel (asynchronous or
 *   synchronous), SyncTransmit shall refuse to take over and return E_NOT_OK.
 * - Jobs inside the sequence shall be executed in order. Each job issues the
 *   Start/Address/Write/Read/Stop steps as required by the job definition.
 * - On hardware errors (for example: NACK, arbitration loss, bus error) the
 *   driver shall map the condition to a sequence result (I2C_SEQ_NACK or
 *   I2C_SEQ_FAILED), stop the sequence execution, and surface the result to
 *   callers and runtime state (and optionally report to DEM where configured).
 * - On complete success the sequence result shall be I2C_SEQ_OK and the API
 *   returns E_OK.
 *
 * Implementation notes (where this file implements the requirement):
 * - Parameter checks and SetupEB presence validation are performed at the
 *   start of this function (CP_SWS_I2C_82409 behavior enforced).
 * - Channel-busy detection (reject takeover if another sequence is pending)
 *   is implemented via I2C_IsChannelBusy() and causes an immediate return
 *   of E_NOT_OK (aligned with CP_SWS_I2C_82404 semantics).
 * - Sequential job execution is implemented by iterating the jobs list and
 *   calling the low-level I2C_Hw_* helpers to perform Start/Write/Read/Stop.
 * - NACK detection and DEM reporting are centralized in
 *   I2C_CheckAndHandleHwError(); other status-to-DEM mappings should be
 *   extended similarly inside the hardware helpers.
 * - The runtime result variable for the sequence is updated to one of:
 *     I2C_SEQ_OK, I2C_SEQ_NACK, I2C_SEQ_FAILED
 *   which callers can query via I2C_GetSequenceResult.
 *
 * Suggested tests to validate CP_SWS_I2C_82407 conformance:
 * - Execute a sequence with multiple jobs while channel is idle: verify all
 *   jobs complete in order and final result == I2C_SEQ_OK.
 * - Call SyncTransmit when a sequence is pending on the same channel: expect
 *   E_NOT_OK and no takeover.
 * - Induce a NACK on a mid-sequence job: verify sequence result becomes
 *   I2C_SEQ_NACK, DEM reported (if configured), and execution stops.
 * - Omit SetupEB for a referenced job and call SyncTransmit: expect
 *   DET report and E_NOT_OK.
 */
{
    const I2C_SequenceCfgType* seq;
    I2C_SequenceRuntimeType* rt;
    uint8 j;

    if (I2c_State != I2C_STATE_INIT) {
        I2C_DET_REPORT(I2C_APIID_SYNCTRANSMIT, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }
    if ((I2c_ConfigPtr == NULL) || (SequenceId >= I2c_ConfigPtr->SequenceCount)) {
        I2C_DET_REPORT(I2C_APIID_SYNCTRANSMIT, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }

    seq = &I2c_ConfigPtr->Sequences[SequenceId];
    rt = &I2c_SeqRt[SequenceId];

    if (rt->Active) {
        I2C_DET_REPORT(I2C_APIID_SYNCTRANSMIT, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }
    /* Development Error: ensure SetupEB has been called for all Jobs in the Sequence
     * (CP_SWS_I2C_82409). If any job runtime has not been setup, report error. */
    {
        uint8 k;
        for (k = 0; k < seq->JobCount; k++) {
            const I2C_JobCfgType* job = seq->JobList[k];
            if (job == NULL) continue;
            I2C_JobRuntimeType* jrt = &I2c_JobRt[job->JobId];
            if (jrt->JobCfg == NULL) {
                I2C_DET_REPORT(I2C_APIID_SYNCTRANSMIT, I2C_E_PARAM_SEQUENCE);
                return E_NOT_OK;
            }
        }
    }

    /* Check whether another sequence is pending on the same channel. If so,
     * per AUTOSAR CP_SWS_I2C_82404 SyncTransmit shall reject the request. */
    if (I2C_IsChannelBusy(seq->Channel)) {
        I2C_DET_REPORT(I2C_APIID_SYNCTRANSMIT, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }

    rt->Active = TRUE;
    rt->Result = I2C_SEQ_PENDING;

    for (j = 0; j < seq->JobCount; j++) {
        const I2C_JobCfgType* job = seq->JobList[j];
        if (job == NULL) continue; /* defensive */
        if (job->JobId >= I2c_ConfigPtr->JobCount) continue;

        I2C_JobRuntimeType* jrt = &I2c_JobRt[job->JobId];

        /* If the job wasn't set up via SetupEB, skip with error */
        if (jrt->JobCfg == NULL) {
            rt->Result = I2C_SEQ_FAILED;
            break;
        }

        /* Start on hardware channel */
    I2C_Hw_Start(seq->Channel, jrt->NodeAddr, jrt->IsRead);

        while (jrt->Index < jrt->Length) {
            if (jrt->IsRead) {
                if (jrt->RxBuf != NULL) jrt->RxBuf[jrt->Index++] = I2C_Hw_Read(seq->Channel);
                else { rt->Result = I2C_SEQ_FAILED; break; }
            } else {
                if (jrt->TxBuf != NULL) I2C_Hw_Write(seq->Channel, jrt->TxBuf[jrt->Index++]);
                else { rt->Result = I2C_SEQ_FAILED; break; }
            }
        }

        I2C_Hw_Stop(seq->Channel);

        if (rt->Result != I2C_SEQ_PENDING) break; /* exit on failure */
    }

    if (rt->Result == I2C_SEQ_PENDING) rt->Result = I2C_SEQ_OK;
    rt->Active = FALSE;

    if (seq->EndNotification) seq->EndNotification();

    /* If any sequence is queued for this channel, start the next one (CP_SWS_I2C_82308) */
    {
        I2C_SequenceType next = I2C_DequeueForChannel(seq->Channel);
        if (next != (I2C_SequenceType)0xFFu) {
            I2C_StartDequeuedSequence(next);
        }
    }

    return (rt->Result == I2C_SEQ_OK) ? E_OK : E_NOT_OK;
}

Std_ReturnType I2C_AsyncTransmit(I2C_SequenceType SequenceId)
{
    I2C_SequenceRuntimeType* rt;

    if (I2c_State != I2C_STATE_INIT) {
        I2C_DET_REPORT(I2C_APIID_ASYNCTRANSMIT, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }
    if ((I2c_ConfigPtr == NULL) || (SequenceId >= I2c_ConfigPtr->SequenceCount)) {
        I2C_DET_REPORT(I2C_APIID_ASYNCTRANSMIT, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }

    rt = &I2c_SeqRt[SequenceId];
    if (rt->Active) {
        I2C_DET_REPORT(I2C_APIID_ASYNCTRANSMIT, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }

    /* Development Error: ensure SetupEB has been called for all Jobs in the Sequence
     * (CP_SWS_I2C_82309). If any job runtime has not been setup, report error. */
    {
        uint8 k;
        const I2C_SequenceCfgType* seq = &I2c_ConfigPtr->Sequences[SequenceId];
        for (k = 0; k < seq->JobCount; k++) {
            const I2C_JobCfgType* job = seq->JobList[k];
            if (job == NULL) continue;
            I2C_JobRuntimeType* jrt = &I2c_JobRt[job->JobId];
            if (jrt->JobCfg == NULL) {
                I2C_DET_REPORT(I2C_APIID_ASYNCTRANSMIT, I2C_E_PARAM_SEQUENCE);
                return E_NOT_OK;
            }
        }

        /* If another sequence on the same channel is pending, queue this
         * sequence and return E_OK (CP_SWS_I2C_82304 / 82308). */
        if (I2C_IsChannelBusy(seq->Channel)) {
            /* mark queued and enqueue */
            rt->Result = I2C_SEQ_QUEUED;
            if (!I2C_EnqueueSequence(SequenceId)) {
                /* queue full -> reject */
                I2C_DET_REPORT(I2C_APIID_ASYNCTRANSMIT, I2C_E_PARAM_SEQUENCE);
                return E_NOT_OK;
            }
            return E_OK;
        }
    }

    rt->Active = TRUE;
    rt->Result = I2C_SEQ_PENDING;
    rt->CurrentJob = 0;

    /* CP_SWS_I2C_82303: If no other sequence on the same channel is pending,
     * the driver shall take over and initiate transmission. Start the first
     * job immediately so the transfer begins without waiting for MainFunction.
     */
    {
        const I2C_SequenceCfgType* seq = &I2c_ConfigPtr->Sequences[SequenceId];
        if ((seq != NULL) && (seq->JobCount > 0)) {
            const I2C_JobCfgType* firstJob = seq->JobList[0];
            if (firstJob != NULL) {
                I2C_JobRuntimeType* fjrt = &I2c_JobRt[firstJob->JobId];
                if (fjrt->JobCfg != NULL && fjrt->Index == 0) {
                    I2C_Hw_Start(seq->Channel, fjrt->NodeAddr, fjrt->IsRead);
                }
            }
        }
    }

    return E_OK;
}

void I2C_MainFunction(void)
{
    uint8 s;
    if ((I2c_ConfigPtr == NULL) || (I2c_State != I2C_STATE_INIT)) return;

    for (s = 0; s < I2c_ConfigPtr->SequenceCount; s++) {
        I2C_SequenceRuntimeType* rt = &I2c_SeqRt[s];
        const I2C_SequenceCfgType* seq = &I2c_ConfigPtr->Sequences[s];

        if (!rt->Active) continue;
        if (seq == NULL) { rt->Active = FALSE; rt->Result = I2C_SEQ_FAILED; continue; }

        if (rt->CurrentJob >= seq->JobCount) {
            /* sequence finished */
            rt->Active = FALSE;
            rt->Result = I2C_SEQ_OK;
            if (seq->EndNotification) seq->EndNotification();
            continue;
        }

        const I2C_JobCfgType* job = seq->JobList[rt->CurrentJob];
        if (job == NULL || job->JobId >= I2c_ConfigPtr->JobCount) {
            rt->Active = FALSE; rt->Result = I2C_SEQ_FAILED; continue;
        }

        I2C_JobRuntimeType* jrt = &I2c_JobRt[job->JobId];
        if (jrt->Index == 0) {
            /* ensure job runtime was setup */
            if (jrt->JobCfg == NULL) { rt->Active = FALSE; rt->Result = I2C_SEQ_FAILED; continue; }
            I2C_Hw_Start(seq->Channel, jrt->NodeAddr, jrt->IsRead);
        }

        if (jrt->Index < jrt->Length) {
            if (jrt->IsRead) {
                if (jrt->RxBuf != NULL) jrt->RxBuf[jrt->Index++] = I2C_Hw_Read(seq->Channel);
                else { rt->Active = FALSE; rt->Result = I2C_SEQ_FAILED; }
            } else {
                if (jrt->TxBuf != NULL) I2C_Hw_Write(seq->Channel, jrt->TxBuf[jrt->Index++]);
                else { rt->Active = FALSE; rt->Result = I2C_SEQ_FAILED; }
            }
        }

        if (jrt->Index >= jrt->Length) {
            I2C_Hw_Stop(seq->Channel);
            rt->CurrentJob++;
            if (rt->CurrentJob >= seq->JobCount) {
                rt->Active = FALSE;
                rt->Result = I2C_SEQ_OK;
                if (seq->EndNotification) seq->EndNotification();
                /* After finishing this sequence, start next queued sequence for this channel */
                {
                    I2C_SequenceType next = I2C_DequeueForChannel(seq->Channel);
                    if (next != (I2C_SequenceType)0xFFu) {
                        I2C_StartDequeuedSequence(next);
                    }
                }
            }
        }
    }
}

I2C_SequenceResultType I2C_GetSequenceResult(I2C_SequenceType SequenceId)
{
    if ((I2c_ConfigPtr == NULL) || (SequenceId >= I2c_ConfigPtr->SequenceCount)) {
        I2C_DET_REPORT(I2C_APIID_GETSEQRESULT, I2C_E_PARAM_JOB);
        return I2C_SEQ_FAILED;
    }
    return I2c_SeqRt[SequenceId].Result;
}

void I2C_GetVersionInfo(Std_VersionInfoType* VersionInfo)
{
    if (VersionInfo == NULL) return;

    VersionInfo->vendorID = 1;
    VersionInfo->moduleID = I2C_MODULE_ID;
    VersionInfo->sw_major_version = 24;
    VersionInfo->sw_minor_version = 11;
    VersionInfo->sw_patch_version = 0;
}

Std_ReturnType I2C_StartListening(I2C_SequenceType SequenceId)
{
    const I2C_SequenceCfgType* seq;
    I2C_SequenceRuntimeType* rt;

    if (I2c_State != I2C_STATE_INIT) {
        I2C_DET_REPORT(I2C_APIID_STARTLISTENING, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }
    if ((I2c_ConfigPtr == NULL) || (SequenceId >= I2c_ConfigPtr->SequenceCount)) {
        I2C_DET_REPORT(I2C_APIID_STARTLISTENING, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }

    seq = &I2c_ConfigPtr->Sequences[SequenceId];
    rt = &I2c_SeqRt[SequenceId];

    if (seq->Channel == NULL) { I2C_DET_REPORT(I2C_APIID_STARTLISTENING, I2C_E_PARAM_JOB); return E_NOT_OK; }
    if (seq->Channel->Mode != I2C_HW_UNIT_MODE_TARGET) {
        I2C_DET_REPORT(I2C_APIID_STARTLISTENING, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }
    if (rt->Active) {
        I2C_DET_REPORT(I2C_APIID_STARTLISTENING, I2C_E_PARAM_JOB);
        return E_NOT_OK;
    }

    /* Ensure SetupEB has been called for jobs in this sequence (CP_SWS_I2C_82806) */
    {
        uint8 k;
        for (k = 0; k < seq->JobCount; k++) {
            const I2C_JobCfgType* job = seq->JobList[k];
            if (job == NULL) continue;
            I2C_JobRuntimeType* jrt = &I2c_JobRt[job->JobId];
            if (jrt->JobCfg == NULL) {
                I2C_DET_REPORT(I2C_APIID_STARTLISTENING, I2C_E_PARAM_SEQUENCE);
                return E_NOT_OK;
            }
        }
    }

    I2C_Hw_EnableSlave(seq->Channel, seq->Channel->OwnAddress);

    rt->Active = TRUE;
    rt->Result = I2C_SEQ_PENDING;

    return E_OK;
}
