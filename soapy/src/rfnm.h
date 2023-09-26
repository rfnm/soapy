#pragma once

//
// Includes
//

// stdlib
#include <array>
#include <chrono>
#include <thread>
#include <string>

// libusb
#include <libusb-1.0/libusb.h>

// SoapySDR
#include "SoapySDR/Device.hpp"

// RFNM
#include "rfnm_constants.h"
#include "rfnm_types.h"



//
// Public
//

namespace RFNM {
    class RFNMDevice : public SoapySDR::Device {
        // ctor (rfnm_construction.cpp)
    public:
        explicit RFNMDevice(const SoapySDR::Kwargs &args);

        ~RFNMDevice() override;

        // identification API (rfnm_identification.cpp)
    public:
        [[nodiscard]] std::string getDriverKey() const override;

        [[nodiscard]] std::string getHardwareKey() const override;

        [[nodiscard]] SoapySDR::Kwargs getHardwareInfo() const override;

        // stream API (rfnm_stream.cpp)
    public:
        SoapySDR::Stream* setupStream(const int direction,
            const std::string& format,
            const std::vector<size_t>& channels = std::vector<size_t>(),
            const SoapySDR::Kwargs& args = SoapySDR::Kwargs());
        void closeStream(SoapySDR::Stream* stream);

        int activateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs, const size_t numElems) override;

        int deactivateStream(SoapySDR::Stream *stream, const int flags0, const long long timeNs) override;

        int readStream(SoapySDR::Stream *stream,void * const *buffs,const size_t numElems,int &flags,long long &timeNs,const long timeoutUs);

        size_t getStreamMTU(SoapySDR::Stream* stream) const;

        size_t getNumChannels(const int direction) const;

        std::vector<double> listSampleRates(const int direction, const size_t channel) const;

        std::string getNativeStreamFormat(const int direction, const size_t channel, double& fullScale) const;

        std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const;

        // device thread (rfnm_thread.cpp)
    private:
        void threadFunction(size_t thread_index);

    private:
        std::array<std::thread, RFNM_THREAD_COUNT> m_thread{};
        rfnm_thread_params m_thread_data[RFNM_THREAD_COUNT]{};


        // private fields
    private:
        rfnm_status m_status{};
        rfnm_outbuf m_outbuf{};
        libusb_device_handle* m_dev_handle{};
        std::chrono::time_point<std::chrono::system_clock> m_previous_time{};
        std::chrono::time_point<std::chrono::system_clock> m_start_time{};
        std::chrono::time_point<std::chrono::system_clock> m_this_time{};
        std::chrono::duration<double> m_time_diff{};
        long m_previous_data{};
    };
}
