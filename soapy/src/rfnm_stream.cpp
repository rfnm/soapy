//
// Includes
//

// stdlib
#include <cstring>

// spdlog
#include <spdlog/spdlog.h>

// libusb
#include <libusb-1.0/libusb.h>

// RFNM
#include "rfnm.h"

#include <SoapySDR/Formats.h>

//
// Stream API
//

namespace RFNM {
    int RFNMDevice::activateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs,
                                   const size_t numElems) {
        spdlog::info("RFNMDevice::activateStream()");

        m_status.clear();
        m_outbuf.clear();
        m_status.countdown_to_data_ready = RFNM_CLEAR_CC_BUFFERS_REQUIRED_FOR_DATA_READY;


        m_outbuf.bufsize = m_outbuf.bytes_per_sample * RFNM_USB_HOST_BUFSIZE_ELEM_CNT * RFNM_OUT_BUF_MULTI;

        m_outbuf.buf = (uint8_t *) malloc(m_outbuf.bufsize);
        m_outbuf.subbuf_status = (uint32_t *) malloc(RFNM_NUM_SUBBUFS * sizeof(uint32_t));
        spdlog::info("RFNMDevice::activateStream() -> Allocating {} bytes for the output buffer", m_outbuf.bufsize);

        int r;
#if LIBUSB_API_VERSION >= 0x0100010A
        r = libusb_init_context(nullptr, nullptr, 0);
#else
        r= libusb_init(nullptr);
#endif
        if (r < 0) {
            spdlog::error("RFNMDevice::activateStream() -> failed to initialize libusb");
            return r;
        }

        m_dev_handle = libusb_open_device_with_vid_pid(nullptr, RFNM_USB_VID, RFNM_USB_PID);
        if (!m_dev_handle) {
            errno = ENODEV;
            spdlog::error("RFNMDevice::activateStream() -> couldn't find RFNM device");
            return -1;
        }

        r = libusb_claim_interface(m_dev_handle, 0);
        if (r < 0) {
            spdlog::error("RFNMDevice::activateStream() -> failed to claim interface, {}, {}", r, libusb_strerror(r));
            return -1;
        }

        spdlog::info("RFNMDevice::activateStream() -> RFNM device was found");
        for (int8_t i = 0; i < RFNM_THREAD_COUNT; i++) {
            m_thread_data[i].ep_id = i + 1;
            m_thread_data[i].head = -1;
        }

        for (int i = 0; i < RFNM_THREAD_COUNT; i++) {
            m_thread[i] = std::thread(&RFNMDevice::threadFunction, this, i);
        }

        m_start_time = std::chrono::system_clock::now();
        m_previous_time = m_start_time;

        return 0;
    }

    int RFNMDevice::deactivateStream(SoapySDR::Stream *stream, const int flags0, const long long int timeNs) {
        spdlog::info("RFNMDevice::deactivateStream()");

        for (auto &i: m_thread_data) {
            i.request_stop = 1;
        }

        for (auto &i: m_thread) {
            i.join();
        }

        free(m_outbuf.buf);
        free(m_outbuf.subbuf_status);

        return 0;
    }


    SoapySDR::Stream* RFNMDevice::setupStream(const int direction, const std::string& format, const std::vector<size_t>& channels, const SoapySDR::Kwargs& args) {


        if (!format.compare(SOAPY_SDR_CF32)) {
            m_outbuf.format = format;
            m_outbuf.bytes_per_sample = SoapySDR_formatToSize(SOAPY_SDR_CF32);
        } else if (!format.compare(SOAPY_SDR_CS16)) {
            m_outbuf.format = format;
            m_outbuf.bytes_per_sample = SoapySDR_formatToSize(SOAPY_SDR_CS16);
        } else if (!format.compare(SOAPY_SDR_CS8)) {
            m_outbuf.format = format;
            m_outbuf.bytes_per_sample = SoapySDR_formatToSize(SOAPY_SDR_CS8);
        }
        else {
            throw std::runtime_error("setupStream invalid format " + format);
        }

        return (SoapySDR::Stream*) this;
    }

    void RFNMDevice::closeStream(SoapySDR::Stream* stream) {
        spdlog::info("RFNMDevice::closeStream() -> Closing stream");
    }




    int RFNMDevice::readStream(SoapySDR::Stream *stream, void *const *buffs, const size_t numElems, int &flags,
                               long long int &timeNs, const long timeoutUs) {
        //spdlog::info("RFNMDevice::readStream()");

        long total_data, data_diff;
        double time_diff, m_readstream_time_diff;

        std::chrono::time_point<std::chrono::system_clock> m_readstream_start_time = std::chrono::system_clock::now();

keep_waiting: 

        total_data = 0;
        for (const auto& thread_data : m_thread_data) {
            total_data += thread_data.data_cnt;
        }

        data_diff = total_data - m_previous_data;
        m_this_time = std::chrono::system_clock::now();
        m_time_diff = m_this_time - m_previous_time;
        time_diff = std::chrono::duration<double>(m_time_diff).count();

        int32_t virtual_head_max = -1;
        int32_t virtual_head_min = RFNM_NUM_SUBBUFS + 2;

repeat_min_search:

        m_outbuf.mutex.lock();

        for (auto &thread_data: m_thread_data) {
            if (thread_data.head < virtual_head_min) {
                virtual_head_min = thread_data.head;
            }
            if (thread_data.head > virtual_head_max) {
                virtual_head_max = thread_data.head;
            }
        }

        // were all buffers inited?
        if (virtual_head_min > 0) {
            // are the buffers overlapping at the end?
            if (virtual_head_min < (RFNM_NUM_SUBBUFS / 5) &&
                virtual_head_max > (RFNM_NUM_SUBBUFS - (RFNM_NUM_SUBBUFS / 5))) {
                // repeat search finding highest min after half buffer
                virtual_head_min = (RFNM_NUM_SUBBUFS / 2);

                m_outbuf.mutex.unlock();

                //TODO: really?
                goto repeat_min_search;
            }

            // if the stored virtual head is not the same as our local param I should
            // be able to advance it and find a matching device head

            while (m_outbuf.virtual_head != virtual_head_min) {
                if (++m_outbuf.virtual_head == RFNM_NUM_SUBBUFS) {
                    m_outbuf.virtual_head = 0;
                }

                if (++m_outbuf.device_head != m_outbuf.subbuf_status[m_outbuf.virtual_head]) {

                    {
                        std::lock_guard<std::mutex> lockGuard(m_status.mutex);

                        if (m_outbuf.device_head + RFNM_NUM_SUBBUFS == m_outbuf.subbuf_status[m_outbuf.virtual_head]) {
                            m_status.read_slow++;
                        }
                        else {
                            m_status.continuity_error++;
                        }
                        
                        if (m_status.countdown_to_data_ready) {
                            m_status.countdown_to_data_ready = RFNM_CLEAR_CC_BUFFERS_REQUIRED_FOR_DATA_READY;
                        }

                        //spdlog::error("continuity_error should {} is {} id {} heads {} {}", m_outbuf.device_head, m_outbuf.subbuf_status[m_outbuf.virtual_head], m_outbuf.virtual_head, virtual_head_min, virtual_head_max);
                    }

                    m_outbuf.device_head = m_outbuf.subbuf_status[m_outbuf.virtual_head];
                }
                else if (m_status.countdown_to_data_ready && --m_status.countdown_to_data_ready == 0) {
                    
                    m_outbuf.tail = m_outbuf.virtual_head * RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT * m_outbuf.bytes_per_sample;
                    //spdlog::info("setting tail to {} {}", m_outbuf.virtual_head, m_outbuf.tail);
                    
                    
                }

                
            }
        }

        if (m_status.countdown_to_data_ready) {
            //spdlog::info("not ready {}", m_status.countdown_to_data_ready);
            m_outbuf.mutex.unlock();
            //spdlog::info("NO");

            m_readstream_time_diff = std::chrono::duration<double>(std::chrono::system_clock::now() - m_readstream_start_time).count();
            if (m_readstream_time_diff < ((double)timeoutUs) / 1000000.0) {
                goto keep_waiting;
            }


            return 0;
        }


        m_outbuf.mutex.unlock();

        

        if (time_diff > 1) {
            {
                std::lock_guard<std::mutex> lockGuard(m_status.mutex);
                if (m_status.device_slow_report || m_status.align_error || m_status.continuity_error || m_status.read_slow) {
                    spdlog::info("RFNMDevice::readStream()-> device slow {}, soapy slow {}, align {}, cc {}, type {}, read {}",
                        m_status.device_slow_report, m_status.read_slow, m_status.align_error, m_status.continuity_error,
                        m_outbuf.format, numElems);
                    m_status.device_slow_report = 0;
                    m_status.align_error = 0;
                    m_status.continuity_error = 0;
                    m_status.read_slow = 0;
                }
            }
            m_previous_data = total_data;
            m_previous_time = std::chrono::system_clock::now();
        }

        uint32_t head = m_outbuf.virtual_head * RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT * m_outbuf.bytes_per_sample;
        int32_t readable = head - m_outbuf.tail;

        if (readable < 0) {
            readable += m_outbuf.bufsize;
        }

        if (readable < numElems * m_outbuf.bytes_per_sample) {
            
            m_readstream_time_diff = std::chrono::duration<double>(std::chrono::system_clock::now() - m_readstream_start_time).count();
            if (m_readstream_time_diff < ((double)timeoutUs) / 1000000.0) {
                goto keep_waiting;
            }

            //spdlog::info("NO {}", m_readstream_time_diff);
            return 0;
        }

        if (m_outbuf.tail + numElems * m_outbuf.bytes_per_sample > m_outbuf.bufsize) {

            std::lock_guard<std::mutex> lockGuard(m_outbuf.data_mutex);

            int first_read_size = m_outbuf.bufsize - m_outbuf.tail;

            std::memcpy(buffs[0], &m_outbuf.buf[m_outbuf.tail], first_read_size);
            std::memcpy(((uint8_t *) buffs[0]) + first_read_size, m_outbuf.buf, (numElems * m_outbuf.bytes_per_sample) - first_read_size);
            //spdlog::info("over cnt {} tail {} first_read {} bufsize {}", numElems * m_outbuf.bytes_per_sample, m_outbuf.tail,
            //    first_read_size, m_outbuf.bufsize);
        } else {

            std::lock_guard<std::mutex> lockGuard(m_outbuf.data_mutex);

            //memset(&m_outbuf.buf[m_outbuf.tail], 0xf0, numElems * m_outbuf.bytes_per_sample);

            //m_outbuf.buf[m_outbuf.tail] = 0xff;

            //printf("%02x %02x %02x %02x\n", m_outbuf.buf[m_outbuf.tail + 0], m_outbuf.buf[m_outbuf.tail + 1], m_outbuf.buf[m_outbuf.tail + 2], m_outbuf.buf[m_outbuf.tail + 3]);

            std::memcpy(buffs[0], &m_outbuf.buf[m_outbuf.tail], numElems * m_outbuf.bytes_per_sample);

           /*if (m_outbuf.tail % 64 == 0)
                spdlog::info("normal bytes {} tail {} head {} virtual {} min {} max {}", numElems * m_outbuf.bytes_per_sample, 
                    m_outbuf.tail, head, m_outbuf.virtual_head,
                    virtual_head_min, virtual_head_max
                );*/

            /*uint8_t* b = (uint8_t*)buffs[0];
            int fail = 1;
            int z;
            //*(b + 3) = 4;
            for (z = 0; z < numElems * m_outbuf.bytes_per_sample; z++) {
                if (*(b + z) != 0x0) {
                    fail = 0;
                }
            }
            if (fail && numElems > 100) {
                spdlog::info("empty bytes {} tail {} head {} virtual {}  {} {} {} {}", 
                    numElems* m_outbuf.bytes_per_sample,
                    m_outbuf.tail, head, m_outbuf.virtual_head,
                    m_outbuf.buf[m_outbuf.tail + 1], 
                    m_outbuf.buf[m_outbuf.tail + 2], 
                    m_outbuf.buf[m_outbuf.tail + 3],
                    m_outbuf.buf[m_outbuf.tail + 4]);
            }*/
        }

        m_outbuf.tail += numElems * m_outbuf.bytes_per_sample;

        if (m_outbuf.tail > m_outbuf.bufsize) {
            m_outbuf.tail -= m_outbuf.bufsize;
        }

        //spdlog::info("read {} elems", numElems);

        return numElems;
    }
}
