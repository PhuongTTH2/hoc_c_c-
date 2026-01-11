## I2C CDD — AUTOSAR CP_SWS_I2C Requirements Coverage

This document maps CP_SWS_I2C_xxxx requirements (AUTOSAR CP R24-11) to the
current implementation in this workspace and notes gaps / follow-ups.

Files touched during implementation
- `I2c.h` — public API and types
- `I2C3.c` — EB-mode driver implementation (hardware helpers, runtime, queuing)
- backups: `I2c.h.bak`, `I2C2.c.bak`

Legend
- Done: requirement implemented and basic validation present
- Partial: implemented but needs vendor-specific values or extra work (timing, exact SFR values, DEM mapping)
- Missing: not implemented

---

### 8.3.1 I2C_Init (CP_SWS_I2C_00820 / CP_SWS_I2C_82002)
- Status: Done (basic)
- Location: `I2C3.c` — `I2C_Init`
- Notes:
  - Initializes hardware per configured channel by calling `I2C_Hw_Init`.
  - Sets all sequence runtime results to `I2C_SEQ_OK`.
  - Re-initialization triggers `I2C_DeInit` first (implemented).
  - Remaining: `I2C_Hw_Init` uses per-channel fields (BaseAddress, BaudRate) but exact WL/WH computation uses an approximation and must be tuned with vendor datasheet.

### 8.3.2 I2C_DeInit (CP_SWS_I2C_00821 / CP_SWS_I2C_82105 / CP_SWS_I2C_82108)
- Status: Partial
- Location: `I2C3.c` — `I2C_DeInit`
- Notes:
  - Reports DET `I2C_E_UNINIT` when driver not initialized (CP_SWS_I2C_82108) — Done.
  - Attempts to de-initialize channels to a reset-like state (clears control, timing, SVA/data fallback, disables PER0 bit) — implemented generically but needs vendor reset values for exact POR state — Partial.

### 8.3.3 I2C_SetupEB (CP_SWS_I2C_00822 + CP_SWS_I2C_00101/102/103/104)
- Status: Done (with controller/slave nuance)
- Location: `I2C3.c` — `I2C_SetupEB`
- Notes:
  - Validates pointers: both NULL or both non-NULL -> DET `I2C_E_PARAM_JOB` / E_NOT_OK — Done (CP_SWS_I2C_00101).
  - Determines read vs write by which buffer is NULL — Done (CP_SWS_I2C_00102/00105/00106 semantics supported by runtime usage).
  - NodeAddress override: if NodeAddress != 0 use it; else use job's DeviceAddress. For controller jobs, if both zero => DET & fail per CP_SWS_I2C_00103 — Done.
  - JobId validation (CP_SWS_I2C_00104) — basic check implemented (DET on invalid JobId) — Done.

### 8.3.4 I2C_AsyncTransmit (CP_SWS_I2C_00823 and 82303/82304/82305/82307/82308/82309)
- Status: Done (major items) / Partial (error mapping)
- Location: `I2C3.c` — `I2C_AsyncTransmit`, helpers for queue
- Notes:
  - CP_SWS_I2C_82303 (No transmission ongoing): when channel is free AsyncTransmit arms the sequence, sets `I2C_SEQ_PENDING` and initiates the first job immediately — Done.
  - CP_SWS_I2C_82304 (Another transmission ongoing): implemented FIFO queueing (global FIFO) — queued sequences receive `I2C_SEQ_QUEUED` and AsyncTransmit returns E_OK — Done.
  - CP_SWS_I2C_82305 (Same transmission ongoing): if the requested Sequence is already PENDING, AsyncTransmit returns E_NOT_OK — logic ensures Active check and DET — Done.
  - CP_SWS_I2C_82307 (Multiple Jobs): driver executes the jobs in order — runtime loop supports multiple jobs — Done.
  - CP_SWS_I2C_82308 (Continuation with queued elements): when a sequence finishes we dequeue the next queued sequence for the same channel and start it — Done.
  - CP_SWS_I2C_82309 (DET presence of SetupEB): AsyncTransmit now checks that SetupEB has been called for all jobs in the sequence and reports `I2C_E_PARAM_SEQUENCE` if not — Done.
  - Partial: NACK mapping and DEM reporting for runtime HW errors (I2C_SEQ_NACK vs I2C_SEQ_FAILED) are partially handled at higher level; explicit mapping from hardware status bits to sequence result and reporting to DEM should be reviewed and extended in `I2C_Hw_*` helpers.
 - CP_SWS_I2C_00310
   - Status: Done (validation + queueing)
   - Location: `I2C3.c` — `I2C_AsyncTransmit`
   - Notes:
     - Paraphrase: The driver shall validate sequence and job parameters for
       asynchronous transmission requests and apply scheduling rules when the
       channel is busy. If the request is valid and the channel is busy the
       driver shall queue the request and return E_OK; if the queue is full
       the driver shall report a development error and return E_NOT_OK.
     - Implementation: `I2C_AsyncTransmit` verifies `SetupEB` was called for
       all jobs, checks channel-busy state via `I2C_IsChannelBusy()`, sets the
       runtime to `I2C_SEQ_QUEUED` and enqueues the sequence. Queue-full is
       detected and reported via DET — this satisfies the scheduling and
       validation aspects of CP_SWS_I2C_00310. NACK/error mapping remains
       partial and is noted elsewhere.
### 8.3.5 I2C_SyncTransmit (CP_SWS_I2C_00824 and 82403/82404/82407/82409)
- Status: Done
- Location: `I2C3.c` — `I2C_SyncTransmit`
- Notes:
  - CP_SWS_I2C_82403: when no sequence on same channel is pending, SyncTransmit takes over and executes jobs blocking — Done.
  - CP_SWS_I2C_82404: if another asynchronous sequence on same channel is PENDING, SyncTransmit rejects (E_NOT_OK) — Done.
  - CP_SWS_I2C_82407: multiple jobs executed sequentially — Done.
  - CP_SWS_I2C_00410
    - Status: Done (behavioral semantics)
    - Location: `I2C3.c` — `I2C_SyncTransmit`
    - Notes:
      - Paraphrase: The synchronous transmit API shall execute all jobs of a
        sequence in-order and in blocking mode when the channel is available.
        The function shall validate parameters and setup (DET on missing
        SetupEB), reject takeover when the channel is occupied by another
        sequence, and map hardware errors to sequence results.
      - Implementation: `I2C_SyncTransmit` validates SetupEB presence,
        enforces channel-busy rejection, performs sequential job execution
        using the low-level `I2C_Hw_*` helpers, and updates the runtime
        result to `I2C_SEQ_OK`, `I2C_SEQ_NACK` or `I2C_SEQ_FAILED` as
        appropriate. Additional DEM mapping for hardware errors is partially
        implemented and recommended for completion.
  - CP_SWS_I2C_82409: DET if SetupEB not called for all jobs — implemented — Done.
 - CP_SWS_I2C_00410
### 8.3.6 I2C_GetVersionInfo (CP_SWS_I2C_00827 / 82601)
- Status: Done
- Location: `I2C3.c` — `I2C_GetVersionInfo`
- Notes: DET if VersionInfo == NULL is checked in other implementations; here function checks NULL and fills version values — consider adding DET per CP_SWS_I2C_82601 if required.

### 8.3.7 I2C_GetSequenceResult (CP_SWS_I2C_00828 / 80701 / 80702)
- Status: Done
- Location: `I2C3.c` — `I2C_GetSequenceResult`
- Notes:
  - Returns I2C_SEQ_OK / PENDING / FAILED / NACK / QUEUED as recorded in runtime arrays — Done.
  - DET on invalid SequenceId implemented — Done.

### 8.3.8 I2C_StartListening (CP_SWS_I2C_00835 and 80801..80806 / 80804 / 80805)
- Status: Done (basic)
- Location: `I2C3.c` — `I2C_StartListening`
- Notes:
  - Validates sequence/channel mode == TARGET and that SetupEB was called for jobs (CP_SWS_I2C_82806) — Done.
  - Sets sequence runtime to PENDING and calls `I2C_Hw_EnableSlave` — Done.
  - Runtime behavior on incoming messages (copying into Rx/Tx buffers, setting result to NACK/FAILED/OK) requires proper hardware IRQ/poll mapping and is partially implemented in HW helpers; verify on target.

---

Remaining / Recommended work
- Verify and replace all SFR offsets and bit masks in `I2C3.c` with vendor `iodefine` header values (IICCTL00_OFFSET, IICD0_OFFSET, SVA_OFFSET, IICE, STT, SPT, TEND, ACKD, etc.). Many control/status bit values in the driver are placeholders and must match the MCU manual.
- Replace timing approximation in `I2C_Hw_ComputeAndWriteTiming` with the exact formula from Renesas documentation for IICA WL/WH and test on hardware.
- Improve error mapping: detect NACK/Arbitration/Bus errors from IICS or status registers and set sequence result to `I2C_SEQ_NACK` or `I2C_SEQ_FAILED` and call `Dem_ReportErrorStatus` where appropriate (SRS -> DEM mapping). Add DEM IDs.
- Consider moving from a global FIFO to per-channel queues if strict per-channel isolation or priorities are needed.
- Add unit tests / example scenarios to `I2C2.c` or a new test file demonstrating queueing and error handling.

If you want, I can now:
 - implement precise WL/WH formula if you paste the peripheral clock and Renesas formula,
 - replace SFR offsets/bit masks if you provide the vendor SFR header or the block map,
 - add DEM mapping for NACK/ARB failure (report IDs and call Dem_ReportErrorStatus).

---

Generated by the refactor on: 2026-01-11
