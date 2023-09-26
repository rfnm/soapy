//
// Includes
//

// spdlog
#include <spdlog/spdlog.h>

// RFNM
#include "rfnm_conversion.h"



//
// Implementation
//

namespace RFNM {
    namespace Conversion {
        bool unpack12_to_cs16(uint8_t* dest, uint8_t* src, size_t sample_cnt) {
            uint64_t buf{};
            uint64_t r0{};
            uint64_t* dest_64{};
            uint64_t* src_64{};
            //uint32_t input_bytes_cnt;
            //input_bytes_cnt = sample_cnt * 3;

            if (sample_cnt % 2) {
                spdlog::error("RFNM::Conversion::unpack12to16() -> sample_cnt {} is not divisible by 2", sample_cnt);
                return false;
            }

            // process two samples at a time
            sample_cnt = sample_cnt / 2;
            for (size_t c = 0; c < sample_cnt; c++) {
                src_64 = (uint64_t*)((uint8_t*)src + (c * 6));
                buf = *(src_64); //unaligned read?
                r0 = 0;
                r0 |= (buf & (0xfffll << 0)) << 4;
                r0 |= (buf & (0xfffll << 12)) << 8;
                r0 |= (buf & (0xfffll << 24)) << 12;
                r0 |= (buf & (0xfffll << 36)) << 16;

                dest_64 = (uint64_t*)(dest + (c * 8));
                *dest_64 = r0;
            }
            return true;
        }


        bool unpack12_to_cf32(uint8_t* dest, uint8_t* src, size_t sample_cnt) {
            uint64_t buf{};
            uint64_t r0{};
            uint64_t* dest_64{};
            uint64_t* src_64{};
            //uint32_t input_bytes_cnt;
            //input_bytes_cnt = sample_cnt * 3;

            if (sample_cnt % 2) {
                spdlog::error("RFNM::Conversion::unpack12to16() -> sample_cnt {} is not divisible by 2", sample_cnt);
                return false;
            }

            // process two samples at a time
            sample_cnt = sample_cnt / 2;
            for (size_t c = 0; c < sample_cnt; c++) {
                src_64 = (uint64_t*)((uint8_t*)src + (c * 6));
                buf = *(src_64); //unaligned read?

                float *i1, * i2, * q1, * q2;

                i1 = (float*)((uint8_t*)dest + (c * 16) + 0);
                q1 = (float*)((uint8_t*)dest + (c * 16) + 4);
                i2 = (float*)((uint8_t*)dest + (c * 16) + 8);
                q2 = (float*)((uint8_t*)dest + (c * 16) + 12);

                
                *i1 = ((int16_t) ((buf & (0xfffll << 0)) << 4)) / 32767.0f;
                *q1 = ((int16_t) ((buf & (0xfffll << 12)) >> 8)) / 32767.0f;
                *i2 = ((int16_t) ((buf & (0xfffll << 24)) >> 20)) / 32767.0f;
                *q2 = ((int16_t) ((buf & (0xfffll << 36)) >> 32)) / 32767.0f;


                /*
                *i1 = ((int16_t) ((buf & (0x7ffll << 0)) << 4)) / 32768.0f;
                *q1 = ((int16_t) ((buf & (0x7ffll << 12)) >> 8)) / 32768.0f;
                *i2 = ((int16_t) ((buf & (0x7ffll << 24)) >> 20)) / 32768.0f;
                *q2 = ((int16_t) ((buf & (0x7ffll << 36)) >> 32)) / 32768.0f;


                if (buf & (0x800ll << 0)) {
                    *i1 = *i1 * -1;
                }
                if (buf & (0x800ll << 12)) {
                    *q1 = *q1 * -1;
                }
                if (buf & (0x800ll << 24)) {
                    *i2 = *i2 * -1;
                }
                if (buf & (0x800ll << 36)) {
                    *q2 = *q2 * -1;
                }
                */

                                
            }
            return true;
        }


        bool unpack12_to_cs8(uint8_t* dest, uint8_t* src, size_t sample_cnt) {
            uint64_t buf{};
            uint32_t r0{};
            uint32_t* dest_32{};
            uint64_t* src_64{};
            //uint32_t input_bytes_cnt;
            //input_bytes_cnt = sample_cnt * 3;

            if (sample_cnt % 2) {
                spdlog::error("RFNM::Conversion::unpack12to16() -> sample_cnt {} is not divisible by 2", sample_cnt);
                return false;
            }

            // process two samples at a time
            sample_cnt = sample_cnt / 2;
            for (size_t c = 0; c < sample_cnt; c++) {
                src_64 = (uint64_t*)((uint8_t*)src + (c * 6));
                buf = *(src_64);
                r0 = 0;
                r0 |= (buf & (0xffll << 4)) >> 4;
                r0 |= (buf & (0xffll << 16)) >> 8;
                r0 |= (buf & (0xffll << 28)) >> 12;
                r0 |= (buf & (0xffll << 40)) >> 16;

                dest_32 = (uint32_t*)((uint8_t*) dest + (c * 4));
                *dest_32 = r0;
            }
            return true;
        }


        



    }
}