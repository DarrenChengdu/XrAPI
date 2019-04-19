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



#endif // GLOBALS_H
