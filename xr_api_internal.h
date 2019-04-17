#ifndef XRAPI_INTERNAL_H
#define XRAPI_INTERNAL_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>              // Flags for open()
#include <sys/stat.h>           // Open() system call
//#include <sys/types.h>          // Types for open()
#include <sys/mman.h>           // Mmap system call
#include <sys/ioctl.h>          // IOCTL system call
#include <unistd.h>             // Close() system call
#include <time.h>
#include <getopt.h>             // Option parsing
#include <errno.h>              // Error codes
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <mutex>
#include "xr_api.h"
#include "globas.h"

namespace xrapi_internal{

extern std::mutex instrMutex;
#define INSTR_SIZE (8)          // 8 Bytes per instruction

//---------------------------------------------------------

#define TUNER_NUMBER                1
#define BBDDC_ID                    0
#define PSDDC_ID                    1
#define ITUDDC_CHANNEL_NUMBER       8
#define TUNER_FREQMAX               1500000000
#define TUNER_FREQMIN               30000000
#define SQUELCH_THRESHOLD_MAX       140
#define SQUELCH_THRESHOLD_MIN       0
#define LEVELTIME_MAX               102400000
#define LEVELTIME_MIN               1024000
#define PSD_PROCESSFRAME_MAX        40000
#define PSD_PROCESSDECIMATION_MAX   255
#define BFO_MAX                     4000
#define FIR_INDEX_MAX               4
#define VOLUME_MAX                  40
#define BANDWIDTH_MAX               58

#define MGC_GAIN_MIN                -50
#define MGC_GAIN_MAX                140

#define RECIVER_TYPE_CDB

//---------------------------------------------------------


#define CMDID_RFATTEN                       0x00002400
#define CMDID_RFBW                          0x00002500
#define CMDID_TUNER_FREQ                    0x00002600
#define CMDID_DDC                           0x00006000
#define CMDID_DDC_BW                        0x00006100
#define CMDID_ITUTHIRDDATATYPE              0x00006200
#define CMDID_ITUSILENCINGLEVEL             0x00006300
#define CMDID_PSD_FFTWINDOWS_TYPE           0x00006400
#define CMDID_PSD_FFTLENGTH                 0x00006500
#define CMDID_PSD_PROCESSTYPE               0x00006600
#define CMDID_PSD_PROCESSFRAME              0x00006700
#define CMDID_PSD_PROCESSDECIMATION         0x00006800
#define CMDID_ITU_LEVELTYPE                 0x00006900
#define CMDID_ITU_LEVELTIME                 0x00006A00
#define CMDID_ITU_DWELLTIME                 0x00006B00
#define CMDID_ITU_HOLDTIME                  0x00006C00
#define CMDID_PSD_TRUNCATION_BITS           0x00007000
#define CMDID_ITU_DGC                       0x00007100
#define CMDID_ITU_SQUELCH                   0x00007200
#define CMDID_ITU_DEMOD                     0x00007300
#define CMDID_ITU_AUDIOFILTER               0x00007400


//---------------------------------------------------------
enum WORK_MODE {
    WORK_MODE_FFM     = 0,
    WORK_MODE_PSSCAN  = 1,
    WORK_MODE_FSEARCH = 2,
    WORK_MODE_MSEARCH
};
enum ITU_Fs {
    ITU_Fs_8K         = 8000,
    ITU_Fs_16K        = 16000,
    ITU_Fs_32K        = 32000,
    ITU_Fs_64K        = 64000,
    ITU_Fs_128K       = 128000,
    ITU_Fs_256K       = 256000
//    ITU_Fs_512K       = 512000
};


enum RECV_MODE {
    RECV_MODE_Regular     = 0,
    RECV_MODE_LowNoise    = 1,
    RECV_MODE_LargeSFDR   = 2
};
enum RF_ATTEN_MODE {
    RF_ATTEN_MANUAL = 0,
    RF_ATTEN_AUTO
};
enum BandWidth {
    BW_200kHz=0,    BW_192kHz=1,    BW_162p5kHz=2,      BW_150kHz=3,    BW_144kHz=4,    BW_125kHz=5,    BW_120kHz=6,    BW_112p5kHz=7,
    BW_100kHz=8,    BW_96kHz=9,     BW_81p25kHz=10,     BW_75kHz=11,    BW_72kHz=12,    BW_62p5kHz=13,  BW_60kHz=14,    BW_56p25kHz=15,
    BW_50kHz=16,    BW_48kHz=17,    BW_40p625kHz=18,    BW_37p5kHz=19,  BW_36kHz=20,    BW_31p25kHz=21, BW_30kHz=22,    BW_28p125kHz=23,
    BW_25kHz=24,    BW_24kHz=25,    BW_20p3125kHz=26,   BW_18p75kHz=27, BW_18kHz=28,    BW_15p625kHz=29,BW_15kHz=30,    BW_14p0625kHz=31,
    BW_12p5kHz=32,  BW_12kHz=33,    BW_10p15625kHz=34,  BW_9p375kHz=35, BW_9kHz=36,     BW7p8175kHz=37, BW_7p5kHz=38,   BW_7p03125kHz=39,
    BW_6p25kHz=40,  BW_6kHz=41,     BW_5p078125kHz=42,  BW_4p8kHz=43,   BW_4p5kHz=44,   BW_4kHz=45,     BW_3p75kHz=46,  BW_3p515625kHz=47,
    BW_3000Hz=48,   BW_2700Hz=49,   BW_2400Hz=50,       BW_2100Hz=51,   BW_1500Hz=52,   BW_1000Hz=53,   BW_600Hz=54,    BW_300Hz=55,
    BW_150Hz=56,    BW_100Hz=57
};

enum ITU_DataType_3rd{
    ITU_DataType_3rd_Amp   = 0,
    ITU_DataType_3rd_Phase = 1
};
enum WINDOWS_TYPE{
    WINDOWS_Rectangular = 0,
    WINDOWS_Blackman    = 1,
    WINDOWS_Flattop     = 2,
    WINDOWS_Hanning     = 3
};
enum FFT_LENGTH {
    LENGTH_128  = 0,
    LENGTH_256  = 1,
    LENGTH_512  = 2,
    LENGTH_1024 = 3,
    LENGTH_2048 = 4,
    LENGTH_4096 = 5
};
enum DETECTION {
    REAL_TIME = 0,
    AVERAGE = 1,
    MAXIMUM = 2,
    MINIMUM
};
enum LevelType{
    LevelType_Fast 	   = 0,
    LevelType_Average  = 1,
    LevelType_RMS      = 2,
    LevelType_MAX      = 3,
    LevelType_MIN      = 4
};

enum DEMOD_TYPE {
    DEMOD_AM = 0,
    DEMOD_FM = 1,
    DEMOD_USB = 2,
    DEMOD_LSB = 3,
    DEMOD_CW =  4,
    DEMOD_WFM = 5,
    DEMOD_ISB = 6
};
enum RECEIVER_TYPE {
    RECEIVER_DB = 0,
    RECEIVER_CDB
};

enum AGC_SPEED {
    AGC_FAST = 0,
    AGC_MEDIUM = 1,
    AGC_SLOW
};

enum DGC_MODE{
    DGC_DISABLE = 0,
    MGC_MODE    = 1,
    AGC_MODE
};


static float bwarr[BANDWIDTH_MAX]={
                        200000, 192000, 162500, 150000, 144000, 125000, 120000, 112500,
                        100000, 96000,  81250,  75000,  72000,  62500,  60000,  56250,
                        50000,  48000,  40625,  37500,  36000,  31250,  30000,  28125,
                        25000,  24000,  20312,  187500, 18000,  15625,  15000,  14062,
                        12500,  12000,  10156,  9375,   9000,   7817,   7500,   7031,
                        6250,   6000,   5078,   4687,   4500,   3906,   3750,   3515,
                        3000,   2700,   2400,   2100,   1500,   1000,   600,    300,
                        150,    100
                       };

extern const unsigned int Loop_e_arr[21];
extern const unsigned int Loop_m_arr[21];
extern const unsigned int Err_Scale_Limit_arr[21];

//---------------------------------------------------------


enum PSBandWidth {
    PSBW_5MHz=0,    PSBW_2p5MHz=1,PSBW_1MHz=2,  PSBW_500KHz=3,
    PSBW_250KMHz=4, PSBW_200KHz=5,PSBW_100KHz=6,PSBW_50KHz=7
};

enum PSPSD_Resolution {
    Resolution_100KHz=0,Resolution_50KHz=1,Resolution_25KHz=2,Resolution_12p5KHz=3,
    Resolution_6p25KHz=4,Resolution_3p125KHz=5,Resolution_2p5KHz=6,Resolution_1p25KHz=7,
    Resolution_625Hz=8,Resolution_500Hz=9,Resolution_250Hz=10,Resolution_125Hz=11,
};

typedef struct _FSearchParameter {
    unsigned char       SearchMode;
    short               squenchValue;
    unsigned int        bwIndex;
    unsigned short      demodIndex;
    short               bfoValue;
    unsigned char       gainIndex;
    short               mgcValue;
    unsigned char       agcSpeed;
    unsigned short      lingeringTime;
    unsigned short      keepTime;
}FSearchParameter;

typedef struct _MSearchParameter {
    unsigned char       recoverIndex;
    short               squenchValue;
    unsigned int        bwIndex;
    unsigned short      demodIndex;
    short               bfoValue;
    unsigned char       gainIndex;
    short               mgcValue;
    unsigned char       agcSpeed;
    unsigned short      lingeringTime;
    unsigned short      keepTime;
    unsigned long long  frequnecy;
}MSearchParameter;


//---------------------------------------------------------

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

    xrPSBandwidthErr                = -79,

    xrFSearchParameterErr           = -78,
    xrMSearchParameterErr           = -77,
    xrPSScanParameterErr            = -76,

    xrPSScanResolutionErr            = -76,

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


extern xrStatus cmd_send(int content[2]);
extern xrStatus reset();
extern xrStatus setWorkMode(WORK_MODE workmode, int tunerid);
extern xrStatus setRecvMode(RECV_MODE recvmode, int tunerid);
extern xrStatus setRFAtten(RF_ATTEN_MODE mode, double val, int tunerid);
extern xrStatus setTunerFreq(double fc, int tunerid);
extern xrStatus setBBDDCFreq(double offset, int tunerid, int ddcid);
extern xrStatus setPSDDCFreq(double offset, int tunerid, int ddcid);
extern xrStatus setITUDDCFreq(double offset, int tunerid, int ddcid);
extern xrStatus setITUBandwidth(BandWidth bandwidth, int tunerid, int ddcid);

extern xrStatus setPSBandwidth(PSBandWidth bandwidth, int tunerid, int ddcid);

extern xrStatus setWindowsType(WINDOWS_TYPE window, int tunerid, int ddcid);
extern xrStatus setFFTLength(FFT_LENGTH length, int tunerid, int ddcid);
extern xrStatus setDetector(DETECTION ProcType, int tunerid, int ddcid);
extern xrStatus setFrame(unsigned int frame, int tunerid, int ddcid);
extern xrStatus setDecimation(unsigned int dec, int tunerid, int ddcid);
extern xrStatus setLevelType(LevelType LevType, int tunerid, int ddcid);
extern xrStatus setLevelTime(unsigned int LevTime, bool LevAuto, int tunerid, int ddcid);
extern xrStatus setDwellTime(unsigned long long time, int tunerid, int ddcid);
extern xrStatus setHoldTime(unsigned long long time, int tunerid, int ddcid);
extern xrStatus startScan(bool sta, int tunerid);
extern xrStatus continueScan(bool sta, int tunerid);
extern xrStatus flushBlockCMDs(int tunerid);
extern xrStatus setDgc(DGC_MODE dgc_mode, int gain, AGC_SPEED agc_speed, int tunerid, int ddcid);
extern xrStatus setSquelch(bool Squelch_en,int squelch_threshold,int tunerid, int ddcid);
extern xrStatus setDemod(DEMOD_TYPE demod_type, int BFO, int tunerid, int ddcid);
extern xrStatus setAudioFilter(bool aduiofilter_en, int tunerid, int ddcid);
extern xrStatus setAudioVolume(unsigned int volume,int tunerid,int ddcid);
extern xrStatus openDevice();
extern xrStatus closeDevice();

////////////////////////////////////////////////

extern xrStatus setFSearch(double start_freq, double stop_freq, double step_freq,
                           FSearchParameter parameter);
extern xrStatus setMSearch(MSearchParameter *parameter, int channel_number);
extern xrStatus setPSScan(double start_freq, double stop_freq, PSPSD_Resolution resolution);


////////////////////////////////////////////////

extern xrStatus setDataEnabled(bool sta, int tunerid, int ddcid);
extern xrStatus setDDCFreq(unsigned int _type, double offset, int tunerid, int ddcid);
extern xrStatus setThirdDataType(ITU_DataType_3rd datatype, int tunerid, int ddcid);
extern xrStatus setSquelchThreshold(double level, int tunerid, int ddcid);
extern xrStatus SetTruncBit(unsigned int trunc_bit, int tunerid, int ddcid);
extern xrStatus setRfBw(int bw, int tunerid, int ddcid);
}
#endif // XRAPI_INTERNAL_H

