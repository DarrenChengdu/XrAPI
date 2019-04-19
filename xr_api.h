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

XR_API size_t       get_rx_num_channels();
XR_API size_t       get_rx_num_itu_channels();

// Frequency
XR_API xrStatus     set_rx_freq (Hz freq, size_t chan=0);
XR_API Hz           get_rx_freq (size_t chan=0);
XR_API xrStatus     set_rx_freq_analy (Hz freq, size_t subchan=0, size_t chan=0);
XR_API Hz           get_rx_freq_analy (size_t subchan=0, size_t chan=0);
XR_API void         get_rx_freq_range (Hz &min, Hz &max, size_t chan=0);

// Gain
XR_API xrStatus     set_rx_gain (double gain, size_t chan=ALL_CHANS);
XR_API xrStatus     get_rx_gain (double &gain, size_t chan=0);
XR_API xrStatus     set_rx_itu_gain (double gain, size_t subchan=0, size_t chan=0);
XR_API xrStatus     get_rx_itu_gain (double &gain, size_t subchan=0, size_t chan=0);
XR_API void         get_rx_gain_range (double &min, double &max);
XR_API void         get_rx_itu_gain_range (double &min, double &max);
XR_API xrStatus     set_rx_gain_profile (const std::string &profile, const size_t chan=ALL_CHANS);
XR_API xrStatus     get_rx_gain_profile (std::string &profile, const size_t chan=0);
XR_API xrStatus     set_rx_agc (bool enable=true, size_t chan=0);
XR_API xrStatus     set_rx_itu_agc (bool enable=true, size_t chan=0);

// Rate
XR_API xrStatus 	set_rx_rate (double rate, size_t chan=ALL_CHANS);
XR_API double     	get_rx_rate (size_t chan=0);
XR_API xrStatus 	set_rx_bandwidth (Hz bw, size_t chan=ALL_CHANS);
XR_API Hz        	get_rx_bandwidth (size_t chan=0);
XR_API xrStatus 	set_rx_itu_rate (double rate, size_t subchan=0, size_t chan=0);
XR_API double      	get_rx_itu_rate (size_t subchan=0, size_t chan=0);
XR_API xrStatus 	set_rx_itu_bandwidth (Hz bw, size_t subchan=0, size_t chan=0);
XR_API Hz        	get_rx_itu_bandwidth (size_t subchan=0, size_t chan=0);

// Demodulation
XR_API xrStatus 	set_rx_itu_demod (std::string type, double param, size_t subchan=0, size_t chan=0);
XR_API xrStatus 	get_rx_itu_demod (std::string &type, double &param, size_t subchan=0, size_t chan=0);

// Audio
XR_API xrStatus 	set_rx_itu_squelch (bool enabled=true, double threshold, size_t subchan=0, size_t chan=0);
XR_API xrStatus 	get_rx_itu_squelch (bool &enabled, double &threshold, size_t subchan=0, size_t chan=0);
XR_API xrStatus     set_rx_itu_filter_enabled(bool enabled=true, size_t subchan=0, size_t chan=0);
XR_API bool         get_rx_itu_filter_enabled(size_t subchan=0, size_t chan=0);
XR_API xrStatus     set_rx_itu_volume(int volume, size_t subchan=0, size_t chan=0);
XR_API int          get_rx_itu_volume(size_t subchan=0, size_t chan=0);

// P-Scan

// RF frontend
XR_API xrStatus 	set_rx_if_bandwidth (Hz bw, size_t chan=ALL_CHANS);  // Set the RX bandwidth on the frontend
XR_API Hz        	get_rx_if_bandwidth (size_t chan=0);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // XR_API_H
