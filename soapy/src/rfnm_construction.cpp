//
// Includes
//

// spdlog
#include <spdlog/spdlog.h>

// SoapySDR
#include <SoapySDR/Registry.hpp>

// RFNM
#include "rfnm.h"
#include "rfnm_constants.h"



//
// Implementation
//

namespace RFNM {
    RFNMDevice::RFNMDevice(const SoapySDR::Kwargs &args) {
        spdlog::info("RFNMDevice::RFNMDevice()");
    }

    RFNMDevice::~RFNMDevice() {
        spdlog::info("RFNMDevice::~RFNMDevice()");
    }
}



//
// Registration and probing
//

SoapySDR::Device *rfnm_device_create(const SoapySDR::Kwargs &args)
{
    spdlog::info("rfnm_device_create()");
    return new RFNM::RFNMDevice(args);
}

SoapySDR::KwargsList rfnm_device_find(const SoapySDR::Kwargs &args) {
    spdlog::info("rfnm_device_find()");

#if LIBUSB_API_VERSION >= 0x0100010A
    libusb_init_context(nullptr, nullptr, 0);
#else
    libusb_init(nullptr);
#endif

    auto *devh = libusb_open_device_with_vid_pid(nullptr, RFNM_USB_VID, RFNM_USB_PID);
    if (!devh) {
        return {};
    }
    libusb_close(devh);

    SoapySDR::Kwargs deviceInfo;
    deviceInfo["device_id"] = "whatEvenIsADeviceID";
    deviceInfo["label"] = "RFNM PROTOTYPE [00001]";
    deviceInfo["serial"] = "00001";

    return {deviceInfo};
}

[[maybe_unused]] static SoapySDR::Registry rfnm_module_registration("RFNM", &rfnm_device_find, &rfnm_device_create, SOAPY_SDR_ABI_VERSION);
