#ifndef GLOBALS_H
#define GLOBALS_H

#define BB_DDC_ID                    0
#define PSD_DDC_ID                   1
#define RX_NUM_CHAN 1
#define RX_NUM_ITU_CHAN 8
#define RX_TUNER_STEP 1000
#define XR_MIN_FREQ            30.0e6
#define XR_MAX_FREQ            1500.0e6
#define XR_MAX_SPAN            (XR_MAX_FREQ - XR_MIN_FREQ)
#define XR_MIN_RF_GAIN         0
#define XR_MAX_RF_GAIN         60
#define XR_RF_GAIN_STEP        1
#define XR_MIN_ITU_GAIN        0
#define XR_MAX_ITU_GAIN        30
#define SQUELCH_THRESHOLD_MAX       140
#define SQUELCH_THRESHOLD_MIN       0
#define LEVEL_MEAS_TIME_MS_MAX         1000
#define LEVEL_MEAS_TIME_MS_MIN         10
#define PSD_FRAMES_PREPROCESSED_MAX        40000
#define PSD_DECIMATION_MAX   255
#define BFO_MAX                     4000
#define NUM_AUDIO_FIRS              4
#define VOLUME_MAX                  40
#define NUM_BANDWIDTHS              58
#define RECIVER_CDB

typedef enum _xrStatus{
    // Configuration Errors
    xrRFGainRangeErr                = -113,
    xrInvalidModeErr                = -112,
    xrReferenceLevelErr             = -111,
    xrInvalidSweepTimeErr           = -107,
    xrBandwidthErr                  = -106,
    xrInvalidGainErr                = -105,
    xrAttenuationErr                = -104,
    xrWorkModeErr                   = -103,
    xrRecvModeErr                   = -102,
    xrRfAttenModeErr                = -101,
    xrItuDataTypeErr                = -100,
    xrWindowsTypeErr                = -99,
    xrFFTLengthErr                  = -98,
    xrDETECTIONErr                  = -97,
    xrLeverTypeErr                  = -96,
    xrTunerFreqErr                  = -95,
    xrSquelchThresholdErr           = -94,
    xrPSDFrameErr                   = -93,
    xrPSDDecimationErr              = -92,
    xrLevelTimeErr                  = -91,
    xrDemodTypeErr                  = -90,
    xrBFOErr                        = -89,
    xrITUFsErr                      = -88,
    xrAudioFilterErr                = -87,
    xrVolumeErr                     = -86,
    xrStartScanErr                  = -85,
    xrContinueScanErr               = -84,
    xrPSDTruncBitErr                = -83,
    xrDGCModeErr                    = -82,
    xrMGCGainErr                    = -81,
    xrAGCSpeedErr                   = -80,


    // General Errors

    xrDDCidErr                   = -6,
    xrTuneridErr                 = -5,
    xrDeviceNotConfigureErr      = -4,
    xrRestErr                    = -3,
    xrDeviceMmapErr              = -2,
    xrDeviceNotOpenErr           = -1,

    // No Error
    xrNoError                    = 0,

    // Warnings/Messages

}xrStatus;

#endif // GLOBALS_H
