#pragma once

//
// Includes
//

// RFNM
#include "rfnm_types.h"


//
// Public
//

#define RFNM_PACKET_HEAD_SIZE sizeof (struct rfnm_packet_head)
#define RFNM_USB_DEVICE_BUFSIZE (4096*32)
#define RFNM_USB_DEVICE_BUFSIZE_NOHEAD (RFNM_USB_DEVICE_BUFSIZE - RFNM_PACKET_HEAD_SIZE)
#define RFNM_USB_HOST_BUF_MULTI (4)
#define RFNM_USB_HOST_BUFSIZE (RFNM_USB_DEVICE_BUFSIZE * RFNM_USB_HOST_BUF_MULTI)
#define RFNM_USB_HOST_BUFSIZE_NOHEAD (RFNM_USB_DEVICE_BUFSIZE_NOHEAD * RFNM_USB_HOST_BUF_MULTI)

#define RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT (RFNM_USB_DEVICE_BUFSIZE_NOHEAD / 3)
#define RFNM_USB_HOST_BUFSIZE_ELEM_CNT (RFNM_USB_DEVICE_BUFSIZE_ELEM_CNT * RFNM_USB_HOST_BUF_MULTI)

//#define RFNM_MAX_OUT_BYTES_PER_ELE (8)

#define RFNM_OUT_BUF_MULTI (128)
//#define RFNM_OUT_BUFSIZE  (RFNM_MAX_OUT_BYTES_PER_ELE * RFNM_USB_HOST_BUFSIZE_ELEM_CNT * RFNM_OUT_BUF_MULTI)
#define RFNM_OUT_BUFSIZE  ((RFNM_USB_HOST_BUFSIZE_NOHEAD * RFNM_OUT_BUF_MULTI * 4) / 3)

#define RFNM_NUM_SUBBUFS (RFNM_USB_HOST_BUF_MULTI * RFNM_OUT_BUF_MULTI)
#define RFNM_CLEAR_CC_BUFFERS_REQUIRED_FOR_DATA_READY 25
#define RFNM_THREAD_COUNT (4)

#define RFNM_USB_VID (0x5522)
#define RFNM_USB_PID (0x1199)
