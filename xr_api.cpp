#include "xr_api.h"
#include "xr_api_internal.h"
using namespace xrapi_internal;

struct XrSettings
{
    XrSettings() {}
    Hz rx_freq[RX_NUM_CHAN];
    Hz rx_itu_freq[RX_NUM_ITU_CHAN][RX_NUM_CHAN];
    bool rf_agc_enabled;
    bool itu_agc_enabled;
    double rf_gain[RX_NUM_CHAN];
    double itu_gain[RX_NUM_ITU_CHAN][RX_NUM_CHAN];
    std::string rx_gain_profile;

} settings;

XR_API size_t get_rx_num_channels()
{
    return RX_NUM_CHAN;
}

XR_API size_t get_rx_num_itu_channels()
{
    return RX_NUM_ITU_CHAN;
}

// Frequency
XR_API xrStatus set_rx_freq (Hz freq, size_t chan)
{
    Hz tunerFreq = round(freq/RX_TUNER_STEP) * RX_TUNER_STEP;
    Hz offset = freq - tunerFreq;
    xrStatus ret1 = setTunerFreq(tunerFreq, chan);
    xrStatus ret2 = setBBDDCFreq(offset, chan, BB_DDC_ID);

    if (ret1 == xrNoError && ret2 == xrNoError) {
        settings.rx_freq[chan] = freq;
        return xrNoError;
    }

    return xrDeviceNotConfigureErr;
}

XR_API Hz get_rx_freq (size_t chan)
{
    return settings.rx_freq[chan];
}

XR_API xrStatus set_rx_freq_analy (Hz freq, size_t subchan, size_t chan)
{
    xrStatus sta = setITUDDCFreq(freq, chan, subchan);

    if (sta == xrNoError) {
        settings.rx_itu_freq[subchan][chan] = freq;
        return xrNoError;
    }

    return sta;
}

XR_API Hz get_rx_freq_analy (size_t subchan, size_t chan)
{
    return settings.rx_itu_freq[subchan][chan];
}

XR_API void get_rx_freq_range (Hz &min, Hz &max, size_t chan)
{
    min = XR_MIN_FREQ;
    max = XR_MAX_FREQ;
}

// Gain
XR_API xrStatus set_rx_rf_gain (double gain, size_t chan)
{
    if (gain < XR_MIN_RF_GAIN || gain > XR_MAX_RF_GAIN) {
        return xrRFGainRangeErr;
    }

    if (chan < 0 || chan > RX_NUM_CHAN-1) {
        return xrTuneridErr;
    }

    if (!settings.rf_agc_enabled) {
        return setRFAtten(RF_ATTEN_MANUAL, XR_MAX_RF_GAIN-gain, chan);
    } else {
        return xrDeviceNotConfigureErr;
    }
}

XR_API xrStatus get_rx_rf_gain (double &gain, size_t chan)
{
    if (chan < 0 || chan > RX_NUM_CHAN-1) {
        return xrTuneridErr;
    }

    gain = settings.rf_gain[chan];
    return xrNoError;
}

XR_API xrStatus set_rx_itu_gain (double gain, size_t subchan, size_t chan)
{
    if (chan < 0 || chan > RX_NUM_CHAN-1) {
        return xrTuneridErr;
    }

    if (subchan < 0 || subchan > RX_NUM_ITU_CHAN-1) {
        return xrDDCidErr;
    }

    xrStatus sta;

    if (!settings.itu_agc_enabled) {
//              xrStatus      sta = setDgc(, gain, chan);
        if (sta == xrNoError)
            settings.itu_gain[subchan][chan] = gain;
    } else {
        return xrDeviceNotConfigureErr;
    }

    return sta;
}

XR_API xrStatus get_rx_itu_gain (double &gain, size_t subchan, size_t chan)
{
    if (chan < 0 || chan > RX_NUM_CHAN-1) {
        return xrTuneridErr;
    }

    if (subchan < 0 || subchan > RX_NUM_ITU_CHAN-1) {
        return xrDDCidErr;
    }

    gain = settings.itu_gain[subchan][chan];
    return xrNoError;
}

XR_API void get_rx_rf_gain_range (double &min, double &max)
{
    min = XR_MIN_RF_GAIN;
    max = XR_MAX_RF_GAIN;
}

XR_API void get_rx_itu_gain_range (double &min, double &max)
{
    min = XR_MIN_ITU_GAIN;
    max = XR_MAX_ITU_GAIN;
}

XR_API xrStatus set_rx_gain_profile (const std::string &profile, const size_t chan)
{
    xrStatus sta;

    if (profile == "low noise") {

    } else if (profile == "low distortion") {

    } else if (profile == "normal") {

    }
    else {
        return xrDeviceNotConfigureErr;
    }
}

XR_API xrStatus get_rx_gain_profile (std::string &profile, const size_t chan)
{
    if (chan < 0 || chan > RX_NUM_CHAN-1) {
        return xrTuneridErr;
    }

    profile = settings.rx_gain_profile;
    return xrNoError;
}

XR_API xrStatus set_rx_rf_agc (bool enable, size_t chan)
{
    if (chan < 0 || chan > RX_NUM_CHAN-1) {
        return xrTuneridErr;
    }

    //    setRFAtten(enable, settings.rf_gain[chan], chan);

    return xrNoError;
}

//// Rate
//XR_API xrStatus 	set_rx_rate (Hz rate, size_t chan=ALL_CHANS);
//XR_API Hz        	get_rx_rate (size_t chan=0);

//// RF frontend
//XR_API xrStatus 	set_rx_bandwidth (Hz bandwidth, size_t chan=0);  // Set the RX bandwidth on the frontend
//XR_API Hz        	get_rx_bandwidth (size_t chan=0);
