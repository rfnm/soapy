//
// Includes
//

// stdlib
#include <cstring>

// spdlog
#include <spdlog/spdlog.h>

// RFNM
#include "rfnm_types.h"



//
// Implementation
//

namespace RFNM {

    void rfnm_status::clear() {
        read_slow = 0;
        write_slow= 0;
        align_error= 0;
        device_slow_report= 0;
        continuity_error= 0;
    }

    void rfnm_outbuf::clear() {
        if(buf){
            free(buf);
        }
        buf = nullptr;

        if(subbuf_status){
            free(subbuf_status);
        }
        subbuf_status = nullptr;
        
        virtual_head = 0;
        device_head = 0;
        tail = 0;
        inited = 0;
    }
}

