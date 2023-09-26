#pragma once

//
// Includes
//

// stdlib
#include <cstdint>
#include <mutex>

// RFNM
#include "rfnm_compiler.h"
#include "rfnm_types.h"



//
// Public
//

namespace RFNM {

    struct rfnm_status {
        uint32_t read_slow{};
        uint32_t write_slow{};
        uint32_t align_error{};
        uint32_t device_slow_report{};
        uint32_t continuity_error{};
        int16_t countdown_to_data_ready{};
        std::mutex mutex;

        void clear();
    };

    struct rfnm_outbuf {
        uint8_t *buf{};
        uint32_t *subbuf_status{};
        uint32_t virtual_head{};
        uint32_t device_head{};
        int32_t tail{};
        uint8_t inited{};
        std::string format;
        std::mutex mutex;
        std::mutex data_mutex;
        uint8_t bytes_per_sample;
        uint32_t bufsize;
        

        void clear();
    };

    struct rfnm_thread_params {
        int8_t ep_id;
        int8_t finished;
        uint64_t data_cnt;
        double time_ep;
        int32_t head;
        int8_t request_stop;
    };

    RFNM_PACKED_STRUCT(
        struct rfnm_packet_head {
            uint32_t check;
            uint32_t cc;
            uint8_t reader_too_slow;
            uint8_t padding[16 - 9 + 4 + 12];
        }
    );
}
