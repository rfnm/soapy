#pragma once

//
// Packed structure
//

#if defined(__GNUC__)
    #define RFNM_PACKED_STRUCT( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#elif defined(_MSC_VER)
    #define RFNM_PACKED_STRUCT( __Declaration__ ) __pragma(pack(push,1)) __Declaration__ __pragma(pack(pop))
#endif
