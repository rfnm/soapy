//
// Includes
//

// stdlib
#include <cstring>

// spdlog
#include <spdlog/spdlog.h>

// RFNM
#include "rfnm.h"
#include "rfnm_constants.h"
#include "rfnm_conversion.h"

#include <SoapySDR/Formats.h>


//
// Identification API
//

namespace RFNM {
    void RFNMDevice::threadFunction(size_t thread_index) {
        auto *localbuf = (uint8_t *) malloc(RFNM_USB_HOST_BUFSIZE);

        auto &tpm = m_thread_data[thread_index];

        int r;
        while (!tpm.request_stop) {
            int transferred;
            r = libusb_bulk_transfer(m_dev_handle, (tpm.ep_id | LIBUSB_ENDPOINT_IN), localbuf, RFNM_USB_HOST_BUFSIZE,
                                     &transferred, 1000);
            if (r) {
                spdlog::error("RFNMDevice::threadFunction() -> tx failed {}", tpm.ep_id);
            } else {
                if (transferred != RFNM_USB_HOST_BUFSIZE) {
                    spdlog::error("RFNMDevice::threadFunction() ->usb not returning full packet, {}, {}", transferred,
                                  tpm.ep_id);
                }

                for (int subpacket = 0; subpacket < RFNM_USB_HOST_BUF_MULTI; subpacket++) {
                    rfnm_packet_head rfnm_packet_head{};
                    std::memcpy(&rfnm_packet_head, &localbuf[RFNM_USB_DEVICE_BUFSIZE * subpacket],
                                RFNM_PACKET_HEAD_SIZE);

                    {
                        std::lock_guard<std::mutex> lockGuard(m_status.mutex);
                        if (rfnm_packet_head.check != 0x7ab8bd6f) {
                            m_status.align_error++;
                        }
                        if (rfnm_packet_head.reader_too_slow) {
                            m_status.device_slow_report++;
                        }
                    }

                    // if max cc is not a multiple of buf we have a bug every once in a few hours
                    // TODO: unpacked without lock?

                    {

                        std::lock_guard<std::mutex> lockGuard(m_outbuf.data_mutex);

                        if (!m_outbuf.format.compare(SOAPY_SDR_CS16)) {
                            Conversion::unpack12_to_cs16(&m_outbuf.buf[(rfnm_packet_head.cc % RFNM_NUM_SUBBUFS) *
                                RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT * m_outbuf.bytes_per_sample],
                                &localbuf[RFNM_PACKET_HEAD_SIZE + (RFNM_USB_DEVICE_BUFSIZE * subpacket)],
                                RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT);
                        }
                        else if (!m_outbuf.format.compare(SOAPY_SDR_CF32)) {
                            Conversion::unpack12_to_cf32(&m_outbuf.buf[(rfnm_packet_head.cc % RFNM_NUM_SUBBUFS) *
                                RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT * m_outbuf.bytes_per_sample],
                                &localbuf[RFNM_PACKET_HEAD_SIZE + (RFNM_USB_DEVICE_BUFSIZE * subpacket)],
                                RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT);
                        }
                        else if (!m_outbuf.format.compare(SOAPY_SDR_CS8)) {
                            Conversion::unpack12_to_cs8(&m_outbuf.buf[(rfnm_packet_head.cc % RFNM_NUM_SUBBUFS) *
                                RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT * m_outbuf.bytes_per_sample],
                                &localbuf[RFNM_PACKET_HEAD_SIZE + (RFNM_USB_DEVICE_BUFSIZE * subpacket)],
                                RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT);
                        }

                    }

                    m_outbuf.mutex.lock();
                    {
                       // std::lock_guard<std::mutex> lockGuard(m_outbuf.mutex);

                        tpm.data_cnt += RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT * m_outbuf.bytes_per_sample;
                        tpm.head = rfnm_packet_head.cc % RFNM_NUM_SUBBUFS;

                        m_outbuf.subbuf_status[rfnm_packet_head.cc % RFNM_NUM_SUBBUFS] = rfnm_packet_head.cc;
                        //spdlog::info("-> {} {}", rfnm_packet_head.cc % RFNM_NUM_SUBBUFS, rfnm_packet_head.cc);
                        
                        /*if (rfnm_packet_head.cc % 16 == 0)
                            spdlog::info("{} -> {} {} {}", thread_index, rfnm_packet_head.cc % RFNM_NUM_SUBBUFS, 
                                rfnm_packet_head.cc,
                                (rfnm_packet_head.cc % RFNM_NUM_SUBBUFS) *
                                RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT * m_outbuf.bytes_per_sample);*/
                    }
                    m_outbuf.mutex.unlock();
                }
            }
        }

        tpm.finished = 1;
        free(localbuf);
    }
}
