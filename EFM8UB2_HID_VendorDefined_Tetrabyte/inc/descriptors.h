/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#ifndef __SILICON_LABS_DESCRIPTORS_H__
#define __SILICON_LABS_DESCRIPTORS_H__

#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include "si_toolchain.h"
#include "efm8_usb.h"

#ifdef __cplusplus
extern "C" {
#endif

// -------------------- USB Identification ------------------------------------
//
// **********
// NOTE: YOU MUST PROVIDE YOUR OWN USB VID/PID (below)
// **********
//
// Following are the definition of the USB VID and PID.  These are example
// values and are assigned to Silicon Labs.  You may not use the Silicon
// Labs VID/PID values in your product.  You must provide your own assigned
// VID and PID values.
///
// $[Vendor ID]
#define USB_VENDOR_ID                      htole16(0x4247)
// [Vendor ID]$

// $[Product ID]
#define USB_PRODUCT_ID                     htole16(0x001A)
// [Product ID]$

#define HID_VENDOR_IFC 0

extern SI_SEGMENT_VARIABLE(ReportDescriptor0[38], const uint8_t, SI_SEG_CODE);
extern SI_SEGMENT_VARIABLE(deviceDesc[], const USB_DeviceDescriptor_TypeDef, SI_SEG_CODE);
extern SI_SEGMENT_VARIABLE(configDesc[], const uint8_t, SI_SEG_CODE);
extern SI_SEGMENT_VARIABLE(initstruct, const USBD_Init_TypeDef, SI_SEG_CODE);

#ifdef __cplusplus
}
#endif

#endif  // #define __SILICON_LABS_DESCRIPTORS_H__
