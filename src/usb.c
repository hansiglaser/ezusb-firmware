/***************************************************************************
 *   Copyright (C) 2011 by Martin Schmoelzer                               *
 *   <martin.schmoelzer@student.tuwien.ac.at>                              *
 *   Copyright (C) 2012 by Johann Glaser <Johann.Glaser@gmx.at>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/**
 * @file Defines USB descriptors, interrupt routines and helper functions.
 * To minimize code size, we make the following assumptions:
 *  - the device has exactly one configuration
 *  - and exactly one alternate setting
 *
 * Therefore, we do not have to support the Set Configuration USB request.
 */

#include "usb.h"
#include "common.h"
#include "delay.h"
#include "io.h"

/// USB idVendor value
#define ID_VENDOR   0xFFF0
/// USB idProduct value
#define ID_PRODUCT  0x0002
/// USB bcdDevice value, Release Number (in BCD)
#define BCD_DEVICE  0x0100

/* Also update external declarations in "include/usb.h" if making changes to
 * these variables! */
volatile bool Semaphore_Command = 0;
volatile bool Semaphore_EP2_out = 0;
volatile bool Semaphore_EP2_in  = 0;

volatile __xdata __at 0x7FE8 struct setup_data setup_data;

/* Define number of endpoints (except Control Endpoint 0) in a central place.
 * Be sure to include the neccessary endpoint descriptors! */
#define NUM_ENDPOINTS  2

/*
 * Normally, we would initialize the descriptor structures in C99 style:
 *
 * __code usb_device_descriptor_t device_descriptor = {
 *   .bLength = foo,
 *   .bDescriptorType = bar,
 *   .bcdUSB = 0xABCD,
 *   ...
 * };
 *
 * But SDCC currently does not support this, so we have to do it the
 * old-fashioned way...
 */

__code struct usb_device_descriptor device_descriptor = {
  /* .bLength = */             sizeof(struct usb_device_descriptor),
  /* .bDescriptorType = */     USB_DESCRIPTOR_TYPE_DEVICE,
  /* .bcdUSB = */              0x0110, /* BCD: 01.10 (Version 1.1 USB spec) */
  /* .bDeviceClass = */        USB_CLASS_VENDOR_SPEC,
  /* .bDeviceSubClass = */     USB_CLASS_VENDOR_SPEC,
  /* .bDeviceProtocol = */     USB_PROTOCOL_VENDOR_SPEC,
  /* .bMaxPacketSize0 = */     64,
  /* .idVendor = */            ID_VENDOR,
  /* .idProduct = */           ID_PRODUCT,
  /* .bcdDevice = */           BCD_DEVICE,
  /* .iManufacturer = */       1,
  /* .iProduct = */            2,
  /* .iSerialNumber = */       3,
  /* .bNumConfigurations = */  1
};

/* WARNING: ALL config, interface and endpoint descriptors MUST be adjacent! */

__code struct usb_config_descriptor config_descriptor = {
  /* .bLength = */             sizeof(struct usb_config_descriptor),
  /* .bDescriptorType = */     USB_DESCRIPTOR_TYPE_CONFIGURATION,
  /* .wTotalLength = */        sizeof(struct usb_config_descriptor) +
                               sizeof(struct usb_interface_descriptor) +
                               (NUM_ENDPOINTS *
                               sizeof(struct usb_endpoint_descriptor)),
  /* .bNumInterfaces = */      1,
  /* .bConfigurationValue = */ 1,
  /* .iConfiguration = */      4,     /* String describing this configuration */
  /* .bmAttributes = */        USB_CONFIG_ATTRIB_RESERVED,  /* Only MSB set according to USB spec */
  /* .MaxPower = */            50     /* 50*2 = 100 mA */
};

__code struct usb_interface_descriptor interface_descriptor00 = {
  /* .bLength = */             sizeof(struct usb_interface_descriptor),
  /* .bDescriptorType = */     USB_DESCRIPTOR_TYPE_INTERFACE,
  /* .bInterfaceNumber = */    0,
  /* .bAlternateSetting = */   0,
  /* .bNumEndpoints = */       NUM_ENDPOINTS,
  /* .bInterfaceClass = */     USB_CLASS_VENDOR_SPEC,
  /* .bInterfaceSubclass = */  USB_CLASS_VENDOR_SPEC,
  /* .bInterfaceProtocol = */  USB_PROTOCOL_VENDOR_SPEC,
  /* .iInterface = */          5
};

__code struct usb_endpoint_descriptor Bulk_EP2_IN_Endpoint_Descriptor = {
  /* .bLength = */             sizeof(struct usb_endpoint_descriptor),
  /* .bDescriptorType = */     USB_DESCRIPTOR_TYPE_ENDPOINT,
  /* .bEndpointAddress = */    2 | USB_DIR_IN,
  /* .bmAttributes = */        USB_ENDPOINT_TYPE_BULK,
  /* .wMaxPacketSize = */      64,
  /* .bInterval = */           0
};

__code struct usb_endpoint_descriptor Bulk_EP2_OUT_Endpoint_Descriptor = {
  /* .bLength = */             sizeof(struct usb_endpoint_descriptor),
  /* .bDescriptorType = */     USB_DESCRIPTOR_TYPE_ENDPOINT,
  /* .bEndpointAddress = */    2 | USB_DIR_OUT,
  /* .bmAttributes = */        USB_ENDPOINT_TYPE_BULK,
  /* .wMaxPacketSize = */      64,
  /* .bInterval = */           0
};

__code struct usb_language_descriptor language_descriptor = {
  /* .bLength =  */            4,
  /* .bDescriptorType = */     USB_DESCRIPTOR_TYPE_STRING,
  /* .wLANGID = */             {USB_LANG_ENGLISH_US}
};

/* String Descriptors */

__code struct usb_string_descriptor strManufacturer  = STR_DESCR(13, 'J','o','h','a','n','n',' ','G','l','a','s','e','r');

__code struct usb_string_descriptor strProduct       = STR_DESCR(15, 'E','Z','-','U','S','B',' ','F','i','r','m','w','a','r','e');

__code struct usb_string_descriptor strSerialNumber  = STR_DESCR( 6, '0','0','0','0','0','1');

__code struct usb_string_descriptor strConfigDescr   = STR_DESCR( 8, 'M','y','C','o','n','f','i','g');

__code struct usb_string_descriptor strInterface     = STR_DESCR(11, 'M','y','I','n','t','e','r','f','a','c','e');

/* Table containing pointers to string descriptors */
__code struct usb_string_descriptor* __code en_string_descriptors[5] = {
  &strManufacturer,
  &strProduct,
  &strSerialNumber,
  &strConfigDescr,
  &strInterface
};

static void usb_handle_setup_data(void);

void sudav_isr(void) __interrupt SUDAV_ISR {
  CLEAR_IRQ();

  usb_handle_setup_data();

  USBIRQ = SUDAVIR;
  EP0CS |= HSNAK;
}

void sof_isr(void)      __interrupt SOF_ISR      { }
void sutok_isr(void)    __interrupt SUTOK_ISR    { }
void suspend_isr(void)  __interrupt SUSPEND_ISR  { }
void usbreset_isr(void) __interrupt USBRESET_ISR { }
void ibn_isr(void)      __interrupt IBN_ISR      { }

void ep0in_isr(void)    __interrupt EP0IN_ISR    { }
void ep0out_isr(void)   __interrupt EP0OUT_ISR   { }
void ep1in_isr(void)    __interrupt EP1IN_ISR    { }
void ep1out_isr(void)   __interrupt EP1OUT_ISR   { }

/**
 * EP2 IN: called after the transfer from uC->Host has finished: we sent data
 */
void ep2in_isr(void)    __interrupt EP2IN_ISR { 
  Semaphore_EP2_in = 1;

  CLEAR_IRQ();
  IN07IRQ = IN2IR;     // Clear OUT2 IRQ
}

/**
 * EP2 OUT: called after the transfer from Host->uC has finished: we got data
 */
void ep2out_isr(void)   __interrupt EP2OUT_ISR {
  Semaphore_EP2_out = 1;

  CLEAR_IRQ();
  OUT07IRQ = OUT2IR;    // Clear OUT2 IRQ
}

void ep3in_isr(void)    __interrupt EP3IN_ISR    { }
void ep3out_isr(void)   __interrupt EP3OUT_ISR   { }
void ep4in_isr(void)    __interrupt EP4IN_ISR    { }
void ep4out_isr(void)   __interrupt EP4OUT_ISR   { }
void ep5in_isr(void)    __interrupt EP5IN_ISR    { }
void ep5out_isr(void)   __interrupt EP5OUT_ISR   { }
void ep6in_isr(void)    __interrupt EP6IN_ISR    { }
void ep6out_isr(void)   __interrupt EP6OUT_ISR   { }
void ep7in_isr(void)    __interrupt EP7IN_ISR    { }
void ep7out_isr(void)   __interrupt EP7OUT_ISR   { }

/**
 * Return the control/status register for an endpoint
 *
 * @param ep endpoint address
 * @return on success: pointer to Control & Status register for endpoint
 *  specified in \a ep
 * @return on failure: NULL
 */
static __xdata uint8_t* usb_get_endpoint_cs_reg(uint8_t ep) {
  /* Mask direction bit */
  uint8_t ep_num = ep & USB_ENDPOINT_ADDRESS_MASK;

  switch (ep_num) {
  case 0:
    return &EP0CS;
    break;
  case 1:
    return ep & USB_ENDPOINT_DIR_MASK ? &IN1CS : &OUT1CS;
    break;
  case 2:
    return ep & USB_ENDPOINT_DIR_MASK ? &IN2CS : &OUT2CS;
    break;
  case 3:
    return ep & USB_ENDPOINT_DIR_MASK ? &IN3CS : &OUT3CS;
    break;
  case 4:
    return ep & USB_ENDPOINT_DIR_MASK ? &IN4CS : &OUT4CS;
    break;
  case 5:
    return ep & USB_ENDPOINT_DIR_MASK ? &IN5CS : &OUT5CS;
    break;
  case 6:
    return ep & USB_ENDPOINT_DIR_MASK ? &IN6CS : &OUT6CS;
    break;
  case 7:
    return ep & USB_ENDPOINT_DIR_MASK ? &IN7CS : &OUT7CS;
    break;
  }

  return NULL;
}

static void usb_reset_data_toggle(uint8_t ep) {
  /* TOGCTL register:
     +----+-----+-----+------+-----+-------+-------+-------+
     | Q  |  S  |  R  |  IO  |  0  |  EP2  |  EP1  |  EP0  |
     +----+-----+-----+------+-----+-------+-------+-------+

     To reset data toggle bits, we have to write the endpoint direction (IN/OUT)
     to the IO bit and the endpoint number to the EP2..EP0 bits. Then, in a
     separate write cycle, the R bit needs to be set.
  */
  uint8_t togctl_value = (ep & 0x80 >> 3) | (ep & 0x7);

  /* First step: Write EP number and direction bit */
  TOGCTL = togctl_value;

  /* Second step: Set R bit */
  togctl_value |= TOG_R;
  TOGCTL = togctl_value;
}

/**
 * Handle GET_STATUS request.
 *
 * @return on success: true
 * @return on failure: false
 */
static bool usb_handle_get_status(void) {
  uint8_t *ep_cs;

  switch (setup_data.bmRequestType) {
  case USB_RECIP_GS_DEVICE:
    /* Two byte response: Byte 0, Bit 0 = self-powered, Bit 1 = remote wakeup.
     *                    Byte 1: reserved, reset to zero */
    IN0BUF[0] = 0;
    IN0BUF[1] = 0;

    /* Send response */
    IN0BC = 2;
    break;
  case USB_RECIP_GS_INTERFACE:
    /* Always return two zero bytes according to USB 1.1 spec, p. 191 */
    IN0BUF[0] = 0;
    IN0BUF[1] = 0;

    /* Send response */
    IN0BC = 2;
    break;
  case USB_RECIP_GS_ENDPOINT:
    /* Get stall bit for endpoint specified in low byte of wIndex */
    ep_cs = usb_get_endpoint_cs_reg(setup_data.wIndex & 0xff);

    if (*ep_cs & EPSTALL) {
      IN0BUF[0] = 0x01;
    }
    else {
      IN0BUF[0] = 0x00;
    }

    /* Second byte sent has to be always zero */
    IN0BUF[1] = 0;

    /* Send response */
    IN0BC = 2;
    break;
  default:
    return false;
    break;
  }

  return true;
}

/**
 * Handle CLEAR_FEATURE request.
 *
 * @return on success: true
 * @return on failure: false
 */
static bool usb_handle_clear_feature(void) {
  __xdata uint8_t *ep_cs;

  switch (setup_data.bmRequestType) {
  case USB_RECIP_CF_DEVICE:
    /* Clear remote wakeup not supported: stall EP0 */
    STALL_EP0();
    break;
  case USB_RECIP_CF_ENDPOINT:
    if (setup_data.wValue == 0) {
      /* Unstall the endpoint specified in wIndex */
      ep_cs = usb_get_endpoint_cs_reg(setup_data.wIndex);
      if (!ep_cs) {
        return false;
      }
      *ep_cs &= ~EPSTALL;
    }
    else {
      /* Unsupported feature, stall EP0 */
      STALL_EP0();
    }
    break;
  default:
    /* Vendor commands... */
  }

  return true;
}

/**
 * Handle SET_FEATURE request.
 *
 * @return on success: true
 * @return on failure: false
 */
static bool usb_handle_set_feature(void) {
  __xdata uint8_t *ep_cs;

  switch (setup_data.bmRequestType) {
  case USB_RECIP_SF_DEVICE:
    if (setup_data.wValue == 2) {
      return true;
    }
    break;
  case USB_RECIP_SF_ENDPOINT:
    if (setup_data.wValue == 0) {
      /* Stall the endpoint specified in wIndex */
      ep_cs = usb_get_endpoint_cs_reg(setup_data.wIndex);
      if (!ep_cs) {
        return false;
      }
      *ep_cs |= EPSTALL;
    }
    else {
      /* Unsupported endpoint feature */
      return false;
    }
    break;
  default:
    /* Vendor commands... */
    break;
  }

  return true;
}

/**
 * Handle GET_DESCRIPTOR request.
 *
 * @return on success: true
 * @return on failure: false
 */
static bool usb_handle_get_descriptor(void) {
  __xdata uint8_t descriptor_type;
  __xdata uint8_t descriptor_index;

  descriptor_type = (setup_data.wValue & 0xff00) >> 8;
  descriptor_index = setup_data.wValue & 0x00ff;

  switch (descriptor_type) {
  case USB_DESCRIPTOR_TYPE_DEVICE:
    SUDPTRH = HI8(&device_descriptor);
    SUDPTRL = LO8(&device_descriptor);
    break;
  case USB_DESCRIPTOR_TYPE_CONFIGURATION:
    SUDPTRH = HI8(&config_descriptor);
    SUDPTRL = LO8(&config_descriptor);
    break;
  case USB_DESCRIPTOR_TYPE_STRING:
    if (setup_data.wIndex == 0) {
      /* Supply language descriptor */
      SUDPTRH = HI8(&language_descriptor);
      SUDPTRL = LO8(&language_descriptor);
    }
    else if (setup_data.wIndex == USB_LANG_ENGLISH_US) {
      /* Supply string descriptor */
      SUDPTRH = HI8(en_string_descriptors[descriptor_index - 1]);
      SUDPTRL = LO8(en_string_descriptors[descriptor_index - 1]);
    }
    else {
      return false;
    }
    break;
  default:
    /* Unsupported descriptor type */
    return false;
    break;
  }

  return true;
}

/**
 * Handle SET_INTERFACE request.
 */
static void usb_handle_set_interface(void) {
  /* Reset Data Toggle */
  usb_reset_data_toggle(USB_DIR_IN  | 2);
  usb_reset_data_toggle(USB_DIR_OUT | 2);

  /* Unstall & clear busy flag of all valid IN endpoints */
  IN2CS = 0 | EPBSY;
  
  /* Unstall all valid OUT endpoints, reset bytecounts */
  OUT2CS = 0;
  OUT2BC = 0;
}

/**
 * Handle the arrival of a USB Control Setup Packet.
 */
static void usb_handle_setup_data(void) {
  switch (setup_data.bRequest) {
    case USB_REQ_GET_STATUS:
      if (!usb_handle_get_status()) {
        STALL_EP0();
      }
      break;
    case USB_REQ_CLEAR_FEATURE:
      if (!usb_handle_clear_feature()) {
        STALL_EP0();
      }
      break;
    case 2: case 4:
      /* Reserved values */
      STALL_EP0();
      break;
    case USB_REQ_SET_FEATURE:
      if (!usb_handle_set_feature()) {
        STALL_EP0();
      }
      break;
    case USB_REQ_SET_ADDRESS:
      /* Handled by USB core */
      break;
    case USB_REQ_SET_DESCRIPTOR:
      /* Set Descriptor not supported. */
      STALL_EP0();
      break;
    case USB_REQ_GET_DESCRIPTOR:
      if (!usb_handle_get_descriptor()) {
        STALL_EP0();
      }
      break;
    case USB_REQ_GET_CONFIGURATION:
      /* we have only one configuration, return its index */
      IN0BUF[0] = config_descriptor.bConfigurationValue;
      IN0BC = 1;
      break;
    case USB_REQ_SET_CONFIGURATION:
      /* we have only one configuration -> nothing to do */
      break;
    case USB_REQ_GET_INTERFACE:
      /* we have only one interface, return its number */
      IN0BUF[0] = interface_descriptor00.bInterfaceNumber;
      IN0BC = 1;
      break;
    case USB_REQ_SET_INTERFACE:
      usb_handle_set_interface();
      break;
    case USB_REQ_SYNCH_FRAME:
      /* Isochronous endpoints not used -> nothing to do */
      break;
    default:
      /* Any other requests: notify listener */
      Semaphore_Command = 1;
      break;
  }
}

/**
 * USB initialization. Configures USB interrupts, endpoints and performs
 * ReNumeration.
 */
void usb_init(void) {
  /* Mark endpoint 2 IN & OUT as valid */
  IN07VAL  = IN2VAL;
  OUT07VAL = OUT2VAL;

  /* Make sure no isochronous endpoints are marked valid */
  INISOVAL  = 0;
  OUTISOVAL = 0;

  /* Disable isochronous endpoints. This makes the isochronous data buffers
   * available as 8051 XDATA memory at address 0x2000 - 0x27FF */
  ISOCTL = ISODISAB;

  /* Enable USB Autovectoring */
  USBBAV |= AVEN;
  
  /* Enable SUDAV interrupt */
  USBIEN |= SUDAVIE;

  /* Enable EP2 OUT & IN interrupts */
  OUT07IEN = OUT2IEN;
  IN07IEN  = IN2IEN;

  /* Enable USB interrupt (EIE register) */
  EUSB = 1;

  /* Perform ReNumeration */
  USBCS = DISCON | RENUM;
  delay_ms(200);
  USBCS = DISCOE | RENUM;
}

