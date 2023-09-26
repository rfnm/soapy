//
// Includes
//

// spdlog
#include <spdlog/spdlog.h>

// RFNM
#include "rfnm.h"

#include <SoapySDR/Formats.hpp>


//
// Identification API
//

namespace RFNM {
    std::string RFNMDevice::getDriverKey() const {
        spdlog::info("RFNMDevice::getDriverKey()");
        return {"RFNM"};
    }

    std::string RFNMDevice::getHardwareKey() const {
        spdlog::info("RFNMDevice::getHardwareKey()");
        return {"RFNM"};
    }

    SoapySDR::Kwargs RFNMDevice::getHardwareInfo() const {
        spdlog::info("RFNMDevice::getHardwareInfo()");
        return {};
    }

    size_t RFNMDevice::getStreamMTU(SoapySDR::Stream* stream) const {
        return RFNM_USB_HOST_BUFSIZE_NOHEAD;
    }

    size_t RFNMDevice::getNumChannels(const int direction) const {
        return direction;
    }

    std::vector<double> RFNMDevice::listSampleRates(const int direction, const size_t channel) const {
        std::vector<double> rates;
        rates.push_back(122880000);
        return rates;
    }

    std::string RFNMDevice::getNativeStreamFormat(const int direction, const size_t /*channel*/, double& fullScale) const {
       
        fullScale = 32768;
        return SOAPY_SDR_CS16;
    }

    std::vector<std::string> RFNMDevice::getStreamFormats(const int direction, const size_t channel) const {
        std::vector<std::string> formats;
        formats.push_back(SOAPY_SDR_CS16);
        formats.push_back(SOAPY_SDR_CF32);
        formats.push_back(SOAPY_SDR_CS8);
        return formats;
    }



}
