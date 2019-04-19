#include "xr_api_internal.h"

namespace xrapi_internal{

//int cmd_send(int content[2]);
bool axilite_init_done = false;
int axilite_fd;
int cmd_fd;
bool cmd_fd_init_done = false;
unsigned int axilite_size;
void *map_addr;
volatile unsigned int *cmd_addr;
ITU_Fs itu_fs = ITU_Fs_256K;
BandWidth bw = BW_200kHz;

std::mutex instrMutex;

int Tuner_channel;
int itudd_channel;

const unsigned int Loop_e_arr[21] =             {17,18,19,20,21,22,23,     // fast
                                                 19,19,19,20,21,22,23,     // medium
                                                 19,20,21,22,23,24,25};    // slow
const unsigned int Loop_m_arr[21] =             {10,10,10,10,10,10,10,     // fast
                                                 4, 2, 1, 1, 1, 1, 1,      // medium
                                                 1, 1, 1, 1, 1, 1, 1};     // slow
const unsigned int Err_Scale_Limit_arr[21] =    {0,0,0,0,0,0,0,   // fast
                                                 0,0,0,0,0,0,0,   // medium
                                                 0,0,0,0,0,0,0};  // slow

typedef struct _Lite_cmd
{
    char tunerid;
    char ddcid;

    char cmd_id;

    char word_39_32;
    int  word_31_0;
}Lite_cmd;

ITU_Fs getFs(BandWidth bw)
{
    int tmp = (int)bw / 8;
    unsigned int fs = 256000 / pow(2.0, tmp);
    if (fs < 8000)
        fs = 8000;

    return (ITU_Fs)fs;
}
bool get_agc_time(AGC_SPEED aspeed,unsigned int *Loop_e,unsigned int *Loop_m,unsigned int *Err_Scale_Limit)
{
    unsigned int _itufs = itu_fs / 8000;
    unsigned int itufs_index = 0;
    while(pow(2.0, itufs_index) != _itufs)
    {
        itufs_index ++;
        if(itufs_index >= 8)
            return false;
    }
    *Loop_e = Loop_e_arr[aspeed * 7 + itufs_index];
    *Loop_m = Loop_m_arr[aspeed * 7 + itufs_index];
    *Err_Scale_Limit = Err_Scale_Limit_arr[aspeed * 7 + itufs_index];
    printf("Loop_e = %d\n",*Loop_e);
    printf("Loop_m = %d\n",*Loop_m);
    printf("Err_Scale_Limit = %d\n",*Err_Scale_Limit);
    return true;
}

int sendblockcmd(int *data,unsigned int size)
{
    //    read(fd_blockirq,&flag, sizeof(flag));
    int rc = write(cmd_fd,data,size);
    if(rc != size)
    {
        printf("send blockcmd failed %d\n",rc);
        return -1;
    }
    else
    {
        printf("send blockcmd success %d\n",rc);
        return 0;
    }
}

xrStatus openDevice()
{
    Tuner_channel = TUNER_NUMBER;
    itudd_channel = ITUDDC_CHANNEL_NUMBER;

    unsigned int len = 64;
    if (axilite_init_done)
    {
        printf("device has been init\n");
        return xrNoError;
    }

    // Initialize the AXI LITE device
    axilite_fd = open("/dev/uio0", O_RDWR | O_NONBLOCK);
    if(axilite_fd < 0)
    {
        fprintf(stderr, "open axi lite device error");
        axilite_init_done = false;
        return xrDeviceNotOpenErr;
    }

    cmd_fd = open("/dev/xillybus_channel_2",O_WRONLY);
    if(cmd_fd < 0)
    {
        fprintf(stderr, "open axi lite device error");
        cmd_fd_init_done = false;
        return xrDeviceNotOpenErr;
    }
    else
    {
        cmd_fd_init_done = true;
    }

    axilite_size = len;

    map_addr = mmap( NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, axilite_fd, 0);

    if(map_addr == MAP_FAILED)
    {
        fprintf(stderr, "Uable to map memory for axi lite device.");
        return xrDeviceNotConfigureErr;
    }

    cmd_addr = (unsigned int *)map_addr;
    axilite_init_done = true;

    printf("axilite_init done!\n");
    return xrNoError;
}

xrStatus closeDevice()
{
    if (axilite_init_done) {
        //axidma_destroy(paxidma_dev);
        munmap(map_addr, axilite_size);
        close(axilite_fd);
        axilite_init_done = false;

        return xrNoError;
    }

    if(cmd_fd_init_done)
    {
        close(cmd_fd);

        cmd_fd_init_done = false;
    }


    fprintf(stderr, "axilite_dev has not been initialized.\n");
}

xrStatus cmd_send(int content[2])
{

    if (!cmd_fd_init_done)
        return xrDeviceNotOpenErr;

    bool flag = false;

    instrMutex.lock();
    // 发送指令串到PL

    unsigned int *cmd_content = (unsigned int *)content;
    cmd_addr[0] = cmd_content[0];
    cmd_addr[1] = cmd_content[1];

    usleep(50000);
    int i = 1;

    //----------------------------------------
    //    read(axilite_fd, &i, sizeof(i));
    //----------------------------------------

    if(i >= 1)
    {
        flag = true;
    }
    else
    {
        flag = false;
    }

    instrMutex.unlock();

    if(flag)
        return xrNoError;
    else
        return xrDeviceNotConfigureErr;
}

//Reset(0x00000000) :
xrStatus reset()
{
    int word[2];
    word[0] = 0x00000001;
    word[1] = 0x00000000;

    if (cmd_send(word) == 0) {
        printf("reset done!\n");
        return xrNoError;
    }

    return xrRestErr;
}

//Packet_en(0x00000100) :
xrStatus setDataEnabled(bool sta,int tunerid,int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;

    int word[2];

    if (sta) {
        word[0] = 0x00000001;
    }
    else {
        word[0] = 0x00000000;
    }

    word[1] = 0x00000100;
    int tunerid_tmp = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid_tmp | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("set[tuner %d ddcid %d]DataEnabled: %d!\n",tunerid,ddcid,sta);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//WorkMode(0x00002000) : 0--FFM; 1--PSCAN; 2--F Search; 3--M Search

xrStatus setWorkMode(WORK_MODE workmode, int tunerid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(!(workmode >= WORK_MODE_FFM && workmode <= WORK_MODE_MSEARCH))
        return xrWorkModeErr;

    unsigned int mode = workmode;
    int word[2];

    word[0] = mode;
    word[1] = 0x00002000;
    tunerid = tunerid << 24;
    word[1]= tunerid | word[1];

    if (cmd_send(word) == 0) {
        printf("setWorkMode: %d!\n", workmode);
        return xrNoError;
    };

    return xrDeviceNotConfigureErr;
}

//Antenna(0x00002100):


//RecvMode(0x00002200) : 0--regular; 1--Low noise; 2--Large SFDR

xrStatus setRecvMode(RECV_MODE recvmode, int tunerid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(!(recvmode >= RECV_MODE_Regular && recvmode <= RECV_MODE_LargeSFDR))
        return xrRecvModeErr;

    int mode = int(recvmode);
    int word[2];

    word[0] = mode ;
    word[1] = 0x00002200;
    tunerid = tunerid << 24;
    word[1]= tunerid | word[1];

    if (cmd_send(word) == 0) {
        printf("setRecvMode: %d!\n", recvmode);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//Preselector(0x00002300):

//RF_Attenuation(0x00002400): 0x00002400 : manual(0~60dB);   0x00002401 : auto(0~10s)
xrStatus setRFAtten(RF_ATTEN_MODE RF_mode, double val, int tunerid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(!(RF_mode >= RF_ATTEN_MANUAL && RF_mode <= RF_ATTEN_MANUAL))
        return xrRfAttenModeErr;

    int mode;
    switch (RF_mode)
    {
    case 	RF_ATTEN_MANUAL:
        mode = CMDID_RFATTEN;
        break;
    case 	RF_ATTEN_AUTO:
        mode = CMDID_RFATTEN | 0x01;
        val  = val*102400000;
        break;
    }

    int word[2];
    word[0] = val;
    word[1] = mode;
    tunerid = tunerid << 24;
    word[1]= tunerid | word[1];

    if (cmd_send(word) == 0) {
        printf("setRFAtten: %d(mode); %f(val)!\n", RF_mode, val);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//Freq:(unit) fc--Hz; offset--MHz
xrStatus setTunerFreq(double fc,int tunerid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(!(fc >= TUNER_FREQMIN && fc <= TUNER_FREQMAX))
        return xrTunerFreqErr;

    int word[2];
    word[0] = fc;
    word[1] = CMDID_TUNER_FREQ;
    tunerid = tunerid << 24;
    word[1]= tunerid | word[1];

    int sta = cmd_send(word);

    if (sta == 0) {
        printf("set Freq %f MHz for tunner %d!\n", fc*1.0e-6, tunerid);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

xrStatus setDDCFreq(unsigned int  _type, double offset,int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if((_type != RECEIVER_DB && _type != RECEIVER_CDB))
        return xrRecvModeErr;

    int word[2];
    double IF;
    double temp;
    if(ddcid == 0)
    {
#ifdef AD_FS_51p2
        {
            IF = 9.8;  // 51.2(ad fs) - 41.4 = 9.8


            if (_type == RECEIVER_CDB)
                IF = 18.8;      // 70(IF) - 51.2 = 18.8
            temp = (IF+offset)/51.2;

        }
#else
        {
            IF = 41.4;  //

            if (_type == RECEIVER_CDB)
                IF = 32.4;      //
            temp = (IF-offset)/102.4;
        }
#endif
        word[0] = temp*pow(2.0,16);
    }
    else if(ddcid == 1)
    {
        temp = offset / 6.4;
        word[0] = temp*pow(2.0,24);

    }
    else if(ddcid >= 2)
    {
        temp = offset / 6.4;
        word[0] = temp*pow(2.0,24);
    }

    word[1] = CMDID_DDC;

    tunerid = tunerid << 24;
    ddcid = ddcid << 16;

    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    int sta = cmd_send(word);

    if (sta == 0) {
        printf("setDDCFreq %f Hz!\n", offset*1e6);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

xrStatus setBBDDCFreq(double offset, int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid != BBDDC_ID)
        return xrDDCidErr;

    int word[2];
    double IF;
    double temp;

    double offset_MHz = offset * 1e-6;

#ifdef RECIVER_TYPE_CDB
    IF = 18.8;      // 70(IF) - 51.2 = 18.8
#else
    IF = 9.8;  // 51.2(ad fs) - 41.4 = 9.8
#endif
    temp = (IF+offset_MHz)/51.2;


    word[0] = temp*pow(2.0,16);


    word[1] = CMDID_DDC;

    tunerid = tunerid << 24;
    ddcid = ddcid << 16;

    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    int sta = cmd_send(word);

    if (sta == 0) {
        printf("setDDCFreq %f Hz!\n", offset_MHz);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

xrStatus setPSDDCFreq(double offset, int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid != PSDDC_ID)
        return xrDDCidErr;

    int word[2];
    double temp;

    double offset_MHz = offset * 1e-6;

    temp = offset_MHz / 6.4;
    word[0] = temp*pow(2.0,24);

    word[1] = CMDID_DDC;

    tunerid = tunerid << 24;
    ddcid = ddcid << 16;

    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    int sta = cmd_send(word);

    if (sta == 0) {
        printf("setDDCFreq %f Hz!\n", offset_MHz);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}



xrStatus setITUDDCFreq(double offset, int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= ITUDDC_CHANNEL_NUMBER)
        return xrDDCidErr;

    int word[2];
    double temp;

    double offset_MHz = offset * 1e-6;

    temp = offset_MHz / 6.4;
    word[0] = temp*pow(2.0,24);

    word[1] = CMDID_DDC;

    tunerid = tunerid << 24;
    ddcid = ddcid << 16;

    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    int sta = cmd_send(word);

    if (sta == 0) {
        printf("setDDCFreq %f Hz!\n", offset_MHz);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//ITU_Bandwidth(0x00026100) :lookup table

xrStatus setITUBandwidth(BandWidth bandwidth, int tunerid, int ddcid)
{
    int bw_index = (int)bandwidth;

    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(bandwidth >= BW_200kHz && bandwidth <= BW_100Hz))
        return xrBandwidthErr;

    int word[2];
    word[0] = bw_index;
    word[1] = CMDID_DDC_BW;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setITUBandwidth: %d!!\n", bw_index);
        bw = bandwidth;
        itu_fs = getFs(bw);
        printf("itu_fs = %d\n",itu_fs);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

extern xrStatus setPSBandwidth(PSBandWidth bandwidth, int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(bandwidth >= PSBW_5MHz && bandwidth <= PSBW_50KHz))
        return xrPSBandwidthErr;

    int bw = (int)bandwidth;

    int word[2];
    word[0] = bw;
    word[1] = 0x00006100;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setITUBandwidth: %d!!\n", bw);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//ITU_Demod(0x00026200): 0--Amp; 1--Phase

xrStatus setThirdDataType(ITU_DataType_3rd datatype, int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(datatype >= ITU_DataType_3rd_Amp && datatype <= ITU_DataType_3rd_Phase))
        return xrItuDataTypeErr;


    int type;
    switch (datatype){
    case	ITU_DataType_3rd_Amp:
        type=0;
        break;
    case	ITU_DataType_3rd_Phase:
        type=1;
        break;
    }
    int word[2];
    word[0] = type;
    word[1] = CMDID_ITUTHIRDDATATYPE;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setThirdDataType: %d!\n", datatype);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//ITU_SilencingLevel(0x00026300): Unit???
xrStatus setSquelchThreshold(double level,int tunerid,int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(level >= SQUELCH_THRESHOLD_MIN && level <= SQUELCH_THRESHOLD_MAX))
        return xrSquelchThresholdErr;


    int word[2];
    word[0] = level;
    word[1] = CMDID_ITUSILENCINGLEVEL;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setSquelchThreshold done!\n");
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//PSD_WindowType(0x00026400): 0--Rectangular; 1--Blackman; 2--Flat top; 3--Hanning

xrStatus setWindowsType(WINDOWS_TYPE window,int tunerid,int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(window >= WINDOWS_Rectangular && window <= WINDOWS_Hanning))
        return xrWindowsTypeErr;

    int type = int(window);
    int word[2];

    word[0] = type;
    word[1] = CMDID_PSD_FFTWINDOWS_TYPE;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setWindowsType: %d!\n", window);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//PSD_FFTLength(0x00026500): 7~12 --2^7(128)~2^12(4096)

xrStatus setFFTLength(FFT_LENGTH length,int tunerid,int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(length >= LENGTH_128 && length <= LENGTH_4096))
        return xrFFTLengthErr;

    int len;
    switch (length){
    case 	LENGTH_128:
        len = 7;
        break;
    case 	LENGTH_256:
        len = 8;
        break;
    case 	LENGTH_512:
        len = 9;
        break;
    case 	LENGTH_1024:
        len = 10;
        break;
    case 	LENGTH_2048:
        len = 11;
        break;
    case 	LENGTH_4096:
        len = 12;
        break;
    }
    int word[2];
    word[0] = len;
    word[1] = CMDID_PSD_FFTLENGTH;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setFFTLength: %d!\n", length);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//PSD_ProcessType(0x00026600):
xrStatus setDetector(DETECTION ProcType,int tunerid,int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(ProcType >= REAL_TIME && ProcType <= MINIMUM))
        return xrDETECTIONErr;

    int type;
    switch (ProcType){
    case	REAL_TIME:
        type=0;
        break;
    case	AVERAGE:
        type=1;
        break;
    case	MAXIMUM:
        type=2;
        break;
    case	MINIMUM:
        type=3;
        break;
    }

    int word[2];
    word[0] = type;
    word[1] = CMDID_PSD_PROCESSTYPE;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setDetector: %d!\n", ProcType);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//PSD_ProcessFrame(0x00026700): range 1 to 40000.
xrStatus setFrame(unsigned int frame,int tunerid,int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(frame >= 1 && frame <= PSD_PROCESSFRAME_MAX))
        return xrPSDFrameErr;

    int word[2];
    word[0] = frame;
    word[1] = CMDID_PSD_PROCESSFRAME;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setFrame done!\n");
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//PSD_ProcessDecimation(0x00026800): rang 0 to 255.
xrStatus setDecimation(unsigned int dec,int tunerid,int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(dec >= 0 && dec <= PSD_PROCESSDECIMATION_MAX))
        return xrPSDDecimationErr;


    int word[2];
    word[0] = dec;
    word[1] = CMDID_PSD_PROCESSDECIMATION;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setDecimation: %d!\n", dec);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}


//ITU_LevelType(0x00026900):
xrStatus setLevelType(LevelType LevType,int tunerid,int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(LevType >= LevelType_Fast && LevType <= LevelType_MIN))
        return xrLeverTypeErr;

    int type;
    switch (LevType){
    case	LevelType_Fast:
        type=0;
        break;
    case	LevelType_Average:
        type=1;
        break;
    case	LevelType_RMS:
        type=2;
        break;
    case	LevelType_MAX:
        type=3;
        break;
    case	LevelType_MIN:
        type=4;
        break;
    }
    int word[2];
    word[0] = type;
    word[1] = CMDID_ITU_LEVELTYPE;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setLevelType: %d!\n", LevType);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}
//ITU_LevelTime(0x00026A00):
xrStatus setLevelTime(unsigned int LevTime,bool LevAuto,int tunerid,int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(LevTime >= LEVELTIME_MIN && LevTime <= LEVELTIME_MAX))
        return xrLevelTimeErr;


    int type = LevTime | (unsigned int)LevAuto << 31;

    printf("type = %x\n",type);

    int word[2];
    word[0] = type;
    word[1] = CMDID_ITU_LEVELTIME;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setLevelTime: %d setLevelAuto: %d!\n", LevTime,LevAuto);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}




//ljj change word[2] type

//ITU_DwellTime(0x00026B00): ???UNIT???
xrStatus setDwellTime(unsigned long long time, int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;

    int word[2];
    word[0] = time & 0xffffffff;
    word[1] = CMDID_ITU_DWELLTIME | (time >> 32);
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setDwellTime done!\n");
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//ITU_HoldTime(??????0x00026C00): ???UNIT???
xrStatus setHoldTime(unsigned long long time, int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;

    int word[2];
    word[0] = time & 0xffffffff;
    word[1] = CMDID_ITU_HOLDTIME | (time >> 32);
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setHoldTime done!\n");
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

xrStatus setDgc(DGC_MODE dgc_mode, int gain,AGC_SPEED agc_speed , int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(dgc_mode >= 0 && dgc_mode <= 2))
        return xrDGCModeErr;
    else if(!(gain >= MGC_GAIN_MIN && gain <= MGC_GAIN_MAX))
        return xrMGCGainErr;
    else if(!(agc_speed >= AGC_FAST && agc_speed <= AGC_SLOW))
        return xrAGCSpeedErr;

    unsigned int Loop_e;
    unsigned int Loop_m;
    unsigned int Err_SCALE_LIMIT;
    int word[2];
    if(dgc_mode == 0)
    {
        word[0] = 0x00000701;
    }
    else if(dgc_mode == 1)
    {
        unsigned int G_COARSE = floor(gain/6);
        //        unsigned int G_FINE=floor((10^((gain-6*G_COARSE)/20))*128);

        unsigned int G_FINE=floor(pow(10,((gain-6*(int)G_COARSE)/20.0)) * 128);

        word[0] = G_FINE | (G_COARSE << 8);

    }
    else if(dgc_mode == 2)
    {
        get_agc_time(agc_speed,&Loop_e,&Loop_m,&Err_SCALE_LIMIT);
        word[0] = Loop_m | (Loop_e << 8);
        word[0] = word[0] | (Err_SCALE_LIMIT << 16);
    }
    word[1] = CMDID_PSD_TRUNCATION_BITS;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];
    word[1]= (dgc_mode << 6) | word [1];
    word[1]= (agc_speed << 4) | word [1];

    //    printf("word[0] = %x word[1] = %x\n",word[0],word[1]);

    if (cmd_send(word) == 0) {
        printf("setDgc: dgc_mode = %d,gain = %d,agc_speed = %d!\n", dgc_mode,gain,agc_speed);
        return xrNoError;
    }
    return xrDeviceNotConfigureErr;
}

xrStatus setSquelch(bool Squelch_en,int squelch_threshold,int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(squelch_threshold >= SQUELCH_THRESHOLD_MIN && squelch_threshold <= SQUELCH_THRESHOLD_MAX))
        return xrSquelchThresholdErr;

    int word[2];

    word[0] = squelch_threshold;
    word[1] = CMDID_ITU_SQUELCH;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];
    word[1]= ((unsigned int )Squelch_en << 6) | word [1];

    printf("word[0] = %x word[1] = %x\n",word[0],word[1]);
    if (cmd_send(word) == 0) {
        if(Squelch_en)
            printf("setSquelch: %d!\n", squelch_threshold);
        else
            printf("setSquelch: Squelch_en = 0!\n");
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

xrStatus setDemod(DEMOD_TYPE demod_type,int BFO,int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(demod_type >= DEMOD_AM && demod_type <= DEMOD_ISB))
        return xrDemodTypeErr;
    else if(!(BFO >= -BFO_MAX && BFO <= BFO_MAX))
        return xrBFOErr;

    int word[2];
    int bw_arr = bwarr[bw];
    word[1] = CMDID_ITU_DEMOD;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];
    word[1]= demod_type | word [1];
    if(demod_type == 0x04)
    {
        if(BFO < 0)
        {
            word[0] = (unsigned long)1 << 31;
            word[0] = (unsigned int)BFO * 65536 / itu_fs;
        }
        else
        {
            word[0] = (unsigned int)BFO * 65536 / itu_fs;
        }
    }
    else if(demod_type == 0x03 || demod_type == 0X02)
    {
        word[0] = (unsigned int)65536 * (bw_arr/4) / itu_fs;
    }
    else if(demod_type == 0x05)
    {
        word[0] = (unsigned int)65536 * (bw_arr/4 - 75) / itu_fs;
    }
    //    printf("word[0] = %x word[1] = %x\n",word[0],word[1]);
    if (cmd_send(word) == 0) {
        printf("setDemod: demod_type = %d,BFO = %d!\n", demod_type,BFO);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

xrStatus setAudioFilter(bool aduiofilter_en, int tunerid, int ddcid)
{
    unsigned char fir_index = 0;
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;

    int word[2];

    word[0] = fir_index;
    word[1] = CMDID_ITU_AUDIOFILTER;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];
    word[1]= ((unsigned int)aduiofilter_en << 7) | word [1];

    //    printf("word[0] = %x word[1] = %x\n",word[0],word[1]);

    if (cmd_send(word) == 0) {
        printf("setAudioFilter: aduiofilter_en = %d! fir_index = %d\n", aduiofilter_en,fir_index);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}


xrStatus setAudioVolume(unsigned int volume, int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(ddcid >= itudd_channel || ddcid < 0)
        return xrDDCidErr;
    else if(!(volume >= 0 && volume <= VOLUME_MAX))
        return xrVolumeErr;

    int word[2];
    unsigned int temp = pow(10,volume / 20.0);

    word[0] = temp;
    word[1] = 0x00007600;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    //    printf("word[0] = %x word[1] = %x\n",word[0],word[1]);

    if (cmd_send(word) == 0) {
        printf("setAudioVolume: volume = %d!\n", volume);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//Scan_start/end(0x00004000): 0--end; 1--begin
xrStatus startScan(bool sta,int tunerid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(!(sta >= 0 && sta <= 1))
        return xrStartScanErr;


    int word[2];
    word[0] = sta;
    word[1] = 0x00004000;
    tunerid = tunerid << 28;
    word[1]= tunerid | word[1];


    if (cmd_send(word) == 0) {
        printf("startScan done!\n");
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//Scan_continue/pause(0x00004100): 0--continue; 1--stop
xrStatus continueScan(bool sta, int tunerid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(!(sta >= 0 && sta <= 1))
        return xrContinueScanErr;


    int word[2];
    word[0] = sta;
    word[1] = 0x00004100;
    tunerid = tunerid << 28;
    word[1]= tunerid | word[1];

    if (cmd_send(word) == 0) {
        printf("continueScan done!\n");
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//Blockcmd_flush(0x00004200):
xrStatus flushBlockCMDs(int tunerid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;


    int word[2];
    word[0] = 0x00000001;
    word[1] = 0x00004200;
    tunerid = tunerid << 28;
    word[1]= tunerid | word[1];

    if (cmd_send(word) == 0) {
        printf("flushBlockCMDs done!\n");
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

//PSD_Truncation_bits(0x00027000):range 0 to 16
xrStatus SetTruncBit(unsigned int trunc_bit, int tunerid, int ddcid)
{
    if (!axilite_init_done)
        return xrDeviceNotOpenErr;
    else if(tunerid >= Tuner_channel || tunerid < 0)
        return xrTuneridErr;
    else if(!(trunc_bit >= 0 && trunc_bit <= 16))
        return xrPSDTruncBitErr;

    int word[2];
    word[0] = (int)trunc_bit;
    word[1] = CMDID_PSD_TRUNCATION_BITS;
    tunerid = tunerid << 24;
    ddcid = ddcid << 16;
    word[1]= tunerid | word[1];
    word[1]= ddcid | word [1];

    if (cmd_send(word) == 0) {
        printf("setTruncBits: %d!\n", trunc_bit);
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}


xrStatus setFSearch(double start_freq, double stop_freq, double step_freq, FSearchParameter parameter)
{
    if(!cmd_fd_init_done)
        return xrDeviceNotOpenErr;
    else if(start_freq < 0 || stop_freq < 0 || step_freq < 0 ||
            start_freq > stop_freq)
        return xrFSearchParameterErr;

    int *cmd_data;
    int size;

    cmd_data = (int *)malloc(size);

    //-----------FSearch--------------------------------





    //-----------FSearch--------------------------------

    free(cmd_data);

    int rc = sendblockcmd(cmd_data,size);

    if(rc != size)
    {
        return xrDeviceNotConfigureErr;
    }
    return xrNoError;
}

xrStatus setMSearch(MSearchParameter *parameter, int channel_number)
{
    if(!cmd_fd_init_done)
        return xrDeviceNotOpenErr;
    else if(channel_number < 0)
        return xrMSearchParameterErr;

    int *cmd_data;
    int size;

    cmd_data = (int *)malloc(size);

    //-----------MSearch--------------------------------





    //-----------MSearch--------------------------------
    free(cmd_data);

    int rc = sendblockcmd(cmd_data,size);

    if(rc != size)
    {
        return xrDeviceNotConfigureErr;
    }
    return xrNoError;
}

xrStatus setPSScan(double start_freq, double stop_freq, PSPSD_Resolution resolution)
{
    if(!cmd_fd_init_done)
        return xrDeviceNotOpenErr;
    else if(start_freq < 0 || stop_freq < 0 ||
            start_freq > stop_freq)
        return xrPSScanParameterErr;
    else if(!(resolution >= Resolution_100KHz && resolution <= Resolution_125Hz))
        return xrPSScanResolutionErr;

    int *cmd_data;
    int size;

    cmd_data = (int *)malloc(size);

    //-----------PSScan--------------------------------





    //-----------PSScan--------------------------------
    free(cmd_data);

    int rc = sendblockcmd(cmd_data,size);

    if(rc != size)
    {
        return xrDeviceNotConfigureErr;
    }
    return xrNoError;
}


template<typename T>
bool array_cmp(T *a1, T *a2, int len)
{
    bool sta = true;

    for (int n = 0; n < len; ++n) {
        if (a1[n] != a2[n]) {
            sta = false;
            break;
        }
    }

    return sta;
}

} //using namespace


