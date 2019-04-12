#ifndef XR_API_H
#define XR_API_H

#include <stddef.h>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
    #ifdef XR_EXPORTS
        #define XR_API __declspec(dllexport)
    #else
        #define XR_API __declspec(dllimport)
    #endif
#else // Linux
    #define XR_API
#endif

typedef double Hz;
static const size_t ALL_CHANS = size_t(~0);

#ifdef __cplusplus
extern "C" {
#endif

XR_API size_t get_rx_num_channels();
XR_API size_t get_rx_num_itu_channels();

// Frequency
XR_API xrStatus set_rx_freq (Hz freq, size_t chan=0);
XR_API Hz       get_rx_freq (size_t chan=0);
XR_API xrStatus set_rx_freq_analy (Hz freq, size_t subchan=0, size_t chan=0);
XR_API Hz       get_rx_freq_analy (size_t subchan=0, size_t chan=0);
XR_API void     get_rx_freq_range (Hz &min, Hz &max, size_t chan=0);

// Gain
XR_API xrStatus     set_rx_rf_gain (double gain, size_t chan=0);
XR_API xrStatus     get_rx_rf_gain (double &gain, size_t chan=0);
XR_API xrStatus     set_rx_itu_gain (double gain, size_t subchan=0, size_t chan=0);
XR_API xrStatus     get_rx_itu_gain (double &gain, size_t subchan=0, size_t chan=0);
XR_API void         get_rx_rf_gain_range (double &min, double &max);
XR_API void         get_rx_itu_gain_range (double &min, double &max);
XR_API xrStatus     set_rx_gain_profile (const std::string &profile, const size_t chan=0);
XR_API xrStatus     get_rx_gain_profile (std::string &profile, const size_t chan=0);
XR_API xrStatus     set_rx_rf_agc (bool enable=true, size_t chan=0);
XR_API xrStatus     set_rx_itu_agc (bool enable=true, size_t chan=0);

// Rate
XR_API xrStatus 	set_rx_rate (Hz rate, size_t chan=ALL_CHANS);
XR_API Hz        	get_rx_rate (size_t chan=0);

// RF frontend
XR_API xrStatus 	set_rx_bandwidth (Hz bandwidth, size_t chan=0);  // Set the RX bandwidth on the frontend
XR_API Hz        	get_rx_bandwidth (size_t chan=0);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // XR_API_H
