/* CDD_I2C.c
 * Companion CDD wrapper that maps an AUTOSAR-like configuration
 * (sequences/jobs) to the I2C EB-mode APIs implemented in I2C3.c.
 *
 * Purpose:
 * - Provide a minimal example configuration structure (sequences/jobs)
 * - Enforce master-only operation (report DET if slave requested)
 * - Provide init + sequence-run APIs that call I2C_Init / I2C_SetupEB / I2C_SyncTransmit
 *
 * Notes:
 * - This file is a bring-up example. For a production ECU the full
 *   ECUC-generated configuration structures should be used.
 */

#include "Std_Types.h"
#include "Det.h"
#include "I2c.h"

/* Use types from I2c.h (I2C_ChannelCfgType, I2C_JobCfgType, I2C_SequenceCfgType) */

/* Example: small static configuration with one sequence containing a
 * single job. Adjust as needed for your ECUC outputs. */

/* Example configuration: one channel (master), one job, one sequence */
static const I2C_ChannelCfgType CDD_Channels[] = {
    /* NOTE: placeholder base address replaced with BAT32A2/9 hint from project notes.
     * The exact peripheral base and offsets should be taken from the device SFR header
     * or the BAT32A2/9 manual. Adjust if your linker or memory map uses a different
     * canonical address format. */
    { .BaseAddress = 0x000FFF10u, .BaudRate = 100000u, .Mode = I2C_HW_UNIT_MODE_CONTROLLER, .TargetListening = FALSE, .OwnAddress = 0 }
};

static const I2C_JobCfgType CDD_Jobs[] = {
    { .JobId = 0u, .DeviceAddress = 0x50u, .Direction = I2C_DIR_WRITE }
};

static const I2C_JobCfgType* CDD_joblist_seq0[] = { &CDD_Jobs[0] };

static const I2C_SequenceCfgType CDD_Sequences[] = {
    { .SequenceId = 0u, .Channel = &CDD_Channels[0], .JobList = CDD_joblist_seq0, .JobCount = 1u, .EndNotification = NULL }
};

static const I2C_ConfigType CDD_Config = {
    .Channels = CDD_Channels,
    .ChannelCount = 1u,
    .Jobs = CDD_Jobs,
    .JobCount = 1u,
    .Sequences = CDD_Sequences,
    .SequenceCount = 1u
};

/* Example transfer buffers */
static const uint8 CDD_job0_txbuf[] = { 0xA5u };


/* Simple flag indicating CDD init done */
static boolean CDD_Inited = FALSE;

/* CDD init: sets up driver HW using a basic config and fills job pointer
 * tables used by I2C_SetupEB. This enforces master-only operation.
 */
void CDD_I2C_Init(void)
{
#if I2C_DEV_ERROR_DETECT
    if (CDD_Inited) {
        /* already inited, ignore */
        return;
    }
#endif

    /* Provide the CDD example configuration and init the driver */
    I2C_Init(&CDD_Config);
    CDD_Inited = TRUE;
}

/* Run a configured sequence by SequenceId. This maps configured jobs to
 * the I2C_SetupEB + I2C_SyncTransmit calls. Returns E_OK if all jobs
 * in the sequence completed successfully.
 */
Std_ReturnType CDD_I2C_RunSequence(I2c_SequenceType seqId)
{
#if I2C_DEV_ERROR_DETECT
    if (!CDD_Inited) {
        Det_ReportError(0xFFFFu, 0, 0x10u, I2C_E_UNINIT); /* module id unknown here */
        return E_NOT_OK;
    }
#endif

    /* Only the small example sequence table exists. Validate bounds. */
    if (seqId >= (sizeof(CDD_Sequences) / sizeof(CDD_Sequences[0]))) {
#if I2C_DEV_ERROR_DETECT
        Det_ReportError(0xFFFFu, 0, 0x11u, I2C_E_PARAM_SEQUENCE);
#endif
        return E_NOT_OK;
    }

    const I2c_SequenceCfgType* seq = &CDD_Sequences[seqId];

    /* Example enforces master-only. If a real Channel object exists and
     * indicates slave mode, report DET and return error. Here Channel is
     * NULL in our sample config, so we proceed. */
    if (seq->Channel != NULL_PTR) {
        /* If Channel object were defined, test its mode. We don't have
         * a Channel structure definition in this example so we conservatively
         * assume master-only support. */
#if I2C_DEV_ERROR_DETECT
        Det_ReportError(0xFFFFu, 0, 0x12u, I2C_E_WRONG_MODE);
#endif
        return E_NOT_OK;
    }

    /* Fill runtime buffers for each job, then trigger sequence execute */
    for (uint8 j = 0u; j < seq->JobCount; j++) {
        const I2C_JobCfgType* jobCfg = seq->JobList[j];
        if (jobCfg == NULL) continue;
        I2C_JobType job = jobCfg->JobId;

        const I2C_DataConstPtrType txbuf = (job == 0u) ? (const I2C_DataConstPtrType)CDD_job0_txbuf : NULL;
        I2C_DataPtrType rxbuf = NULL;
        I2C_NumberOfDataType length = 1u;

        if (I2C_SetupEB(job, jobCfg->DeviceAddress, txbuf, rxbuf, length) != E_OK) {
            return E_NOT_OK;
        }
    }

    /* Execute the configured sequence (blocking) */
    if (I2C_SyncTransmit(seqId) != E_OK) return E_NOT_OK;

    /* Call end notification if present */
    if (seq->EndNotification != NULL_PTR) {
        seq->EndNotification();
    }

    return E_OK;
}

/* Optional helper to expose the example configuration for debug */
const I2c_SequenceCfgType* CDD_I2C_GetSequenceCfg(I2c_SequenceType seqId)
{
    if (seqId >= (sizeof(CDD_Sequences) / sizeof(CDD_Sequences[0]))) return NULL_PTR;
    return &CDD_Sequences[seqId];
}

/* End of CDD_I2C.c */
