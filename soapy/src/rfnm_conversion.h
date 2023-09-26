#pragma once


//
// Includes
//

// stdlib
#include <cstddef>
#include <cstdint>



//
// Public
//

namespace RFNM {
    namespace Conversion {
        bool unpack12_to_cf32(uint8_t* dest, uint8_t* src, size_t cnt);
        bool unpack12_to_cs16(uint8_t* dest, uint8_t* src, size_t cnt);
        bool unpack12_to_cs8(uint8_t* dest, uint8_t* src, size_t cnt);
    }
}
