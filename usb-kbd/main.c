#include <stdint.h>
#include <stddef.h>

#include "../include/common.h"
#include "../include/resets.h"
#include "../include/xosc.h"
#include "../include/pll.h"
#include "../include/clocks.h"
#include "../include/usb.h"
#include "descriptors.h"

#define XOSC_FREQ 12000000
#define POSTDIV1 6
#define POSTDIV2 5
#define FBDIV 120
#define USBFREQ XOSC_FREQ*(FBDIV/POSTDIV1/POSTDIV2) //48 MHz

#define USB_DEVICE_DPRAM_EP1_BUFFER0_OFFSET 0x180

#define USBCTRL_IRQ_IRQn 14

struct{
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint8_t wValue_low; //wValue has two bytes with separate uses
    uint8_t wValue_high;
    uint16_t wIndex;
    uint16_t wLength;
} ctrl_request;

uint8_t ep1_buf[1];
uint8_t ep1_pid;

extern uint32_t _sidata, _sdata, _edata, _ebss, _sbss;

void flash_to_ram(){

    uint32_t *p_idata = &_sidata;

    for(uint32_t *p_data = &_sdata; p_data < &_edata; *p_data++ = *p_idata++);
    for(uint32_t *p_bss = &_sbss; p_bss < &_ebss; *p_bss++ = 0);
}

void clocks_init(){

    REG(XOSC_BASE + XOSC_CTRL_OFFSET) = (XOSC_CTRL_ENABLE_VALUE_ENABLE<<XOSC_CTRL_ENABLE_LSB) + XOSC_CTRL_FREQ_RANGE_VALUE_1_15MHZ;
    while(!(REG(XOSC_BASE + XOSC_STATUS_OFFSET) & XOSC_STATUS_STABLE_BITS));

    REG(RESETS_BASE + RESETS_RESET_OFFSET + REG_ALIAS_CLR_BITS) = RESETS_RESET_IO_BANK0_BITS | RESETS_RESET_PLL_USB_BITS;
    while(~(REG(RESETS_BASE + RESETS_RESET_DONE_OFFSET)) & (RESETS_RESET_DONE_IO_BANK0_BITS | RESETS_RESET_DONE_PLL_USB_BITS));

    REG(PLL_USB_BASE + PLL_FBDIV_INT_OFFSET) = FBDIV;
    REG(PLL_USB_BASE + PLL_PRIM_OFFSET) = (POSTDIV1<<PLL_PRIM_POSTDIV1_LSB | POSTDIV2<<PLL_PRIM_POSTDIV2_LSB);
    REG(PLL_USB_BASE + PLL_PWR_OFFSET + REG_ALIAS_CLR_BITS) = PLL_PWR_PD_BITS | PLL_PWR_VCOPD_BITS | PLL_PWR_POSTDIVPD_BITS;
    while(!(REG(PLL_USB_BASE + PLL_CS_OFFSET) & PLL_CS_LOCK_BITS));

    REG(CLOCKS_BASE + CLOCKS_CLK_SYS_CTRL_OFFSET) = CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB<<CLOCKS_CLK_SYS_CTRL_AUXSRC_LSB; //set system aux clock to pll_usb
    REG(CLOCKS_BASE + CLOCKS_CLK_SYS_CTRL_OFFSET) |= CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX; //set sytem clock to pll_usb
    while(REG(CLOCKS_BASE + CLOCKS_CLK_SYS_SELECTED_OFFSET) != 1<<CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX);

    REG(CLOCKS_BASE + CLOCKS_CLK_PERI_CTRL_OFFSET) = CLOCKS_CLK_PERI_CTRL_ENABLE_BITS;

    REG(CLOCKS_BASE + CLOCKS_CLK_USB_CTRL_OFFSET) = CLOCKS_CLK_USB_CTRL_ENABLE_BITS;
}

void usb_init(){  //datasheet 4.1.3.2.1

    REG(RESETS_BASE + RESETS_RESET_OFFSET + REG_ALIAS_CLR_BITS) = RESETS_RESET_USBCTRL_BITS;
    while(~(REG(RESETS_BASE + RESETS_RESET_DONE_OFFSET)) & (RESETS_RESET_DONE_USBCTRL_BITS));

    REG(USBCTRL_REGS_BASE + USB_MAIN_CTRL_OFFSET + REG_ALIAS_CLR_BITS) = USB_MAIN_CTRL_PHY_ISO_BITS;

    REG(USBCTRL_REGS_BASE + USB_USB_MUXING_OFFSET) = USB_USB_MUXING_TO_PHY_BITS;
    REG(USBCTRL_REGS_BASE + USB_USB_PWR_OFFSET) = USB_USB_PWR_VBUS_DETECT_BITS | USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS; //datasheet 4.1.2.8
    REG(USBCTRL_REGS_BASE + USB_MAIN_CTRL_OFFSET) = USB_MAIN_CTRL_CONTROLLER_EN_BITS;
    REG(USBCTRL_REGS_BASE + USB_SIE_CTRL_OFFSET) = USB_SIE_CTRL_EP0_INT_1BUF_BITS;
    REG(USBCTRL_REGS_BASE + USB_INTE_OFFSET) = USB_INTE_BUFF_STATUS_BITS | USB_INTE_BUS_RESET_BITS | USB_INTE_SETUP_REQ_BITS;
    REG(USBCTRL_REGS_BASE + USB_SIE_CTRL_OFFSET + REG_ALIAS_SET_BITS) = USB_SIE_CTRL_PULLUP_EN_BITS;

    REG(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP0_OUT_BUFFER_CONTROL_OFFSET) = USB_DEVICE_DPRAM_EP0_OUT_BUFFER_CONTROL_AVAILABLE_0_BITS;

    REG(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP1_IN_CONTROL_OFFSET) = USB_DEVICE_DPRAM_EP1_IN_CONTROL_ENABLE_BITS |
                                                                       USB_DEVICE_DPRAM_EP1_IN_CONTROL_INTERRUPT_PER_BUFF_BITS |
                                                                       USB_DEVICE_DPRAM_EP1_IN_CONTROL_ENDPOINT_TYPE_VALUE_INTERRUPT<<USB_DEVICE_DPRAM_EP1_IN_CONTROL_ENDPOINT_TYPE_LSB |
                                                                       USB_DEVICE_DPRAM_EP1_BUFFER0_OFFSET;
}

void handle_reset(){

    REG(USBCTRL_REGS_BASE + USB_SIE_STATUS_OFFSET) = USB_SIE_STATUS_BUS_RESET_BITS;
}

void dpram_to_buffer(uint8_t * dpram_buf, uint8_t* buf, uint8_t len){

    for(uint8_t i = 0; i < len; i++)
        buf[i] = dpram_buf[i];
}

void buffer_to_dpram(uint8_t ep, uint8_t pid, uint8_t * dpram_buf, uint8_t* buf, uint8_t len){

    for(uint8_t i = 0; i < len; i++)
        dpram_buf[i] = buf[i];

  REG(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP0_IN_BUFFER_CONTROL_OFFSET + 8*ep) = USB_DEVICE_DPRAM_EP0_IN_BUFFER_CONTROL_FULL_0_BITS |
                                                                              pid*USB_DEVICE_DPRAM_EP0_IN_BUFFER_CONTROL_PID_0_BITS  |
                                                                              USB_DEVICE_DPRAM_EP0_IN_BUFFER_CONTROL_AVAILABLE_0_BITS |
                                                                              len;
}

void handle_get_request(){

    switch(ctrl_request.wValue_high){
    case 1:
        buffer_to_dpram(0, 1, (uint8_t *)(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP0_BUFFER0_OFFSET), deviceDescriptor, ctrl_request.wLength);
        break;
    case 2:
        buffer_to_dpram(0, 1, (uint8_t *)(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP0_BUFFER0_OFFSET), configDescriptor, ctrl_request.wLength);
        break;
    case 3:
        if(ctrl_request.wValue_low == 0)
                buffer_to_dpram(0, 1, (uint8_t *)(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP0_BUFFER0_OFFSET), langidDescriptor, sizeof langidDescriptor);
        else if(ctrl_request.wValue_low == 1)
                buffer_to_dpram(0, 1, (uint8_t *)(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP0_BUFFER0_OFFSET), manufacturerDescriptor, sizeof manufacturerDescriptor);
        else if(ctrl_request.wValue_low == 2)
                buffer_to_dpram(0, 1, (uint8_t *)(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP0_BUFFER0_OFFSET), productDescriptor, sizeof productDescriptor);
        else if(ctrl_request.wValue_low == 3)
                buffer_to_dpram(0, 1, (uint8_t *)(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP0_BUFFER0_OFFSET), serialDescriptor, sizeof serialDescriptor);
    }
}

void handle_set_request(){

    buffer_to_dpram(0, 1, NULL, NULL, 0);
}

void handle_hid_get_request(){

    buffer_to_dpram(0, 1, (uint8_t *)(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP0_BUFFER0_OFFSET), reportDescriptor, ctrl_request.wLength);
}

void handle_ep0_setup(){

    dpram_to_buffer((uint8_t *)USBCTRL_DPRAM_BASE, (uint8_t *)&ctrl_request, 8);
    switch(ctrl_request.bmRequestType){
    case 0:
        handle_set_request();
        break;
    case 0x21:
        handle_set_request();
        break;
    case 0x80:
        handle_get_request();
        break;
    case 0x81:
        handle_hid_get_request();
    }

    REG(USBCTRL_REGS_BASE + USB_SIE_STATUS_OFFSET) = USB_SIE_STATUS_SETUP_REC_BITS;
}

void handle_ep0_in(){

    if(ctrl_request.bRequest == 5)
        REG(USBCTRL_REGS_BASE + USB_ADDR_ENDP_OFFSET) = ctrl_request.wValue_low;

    REG(USBCTRL_REGS_BASE + USB_BUFF_STATUS_OFFSET) = USB_BUFF_STATUS_EP0_IN_BITS;
}

void handle_ep0_out(){

    REG(USBCTRL_REGS_BASE + USB_BUFF_STATUS_OFFSET) = USB_BUFF_STATUS_EP0_OUT_BITS;
}

void handle_ep1_in(){

    REG(USBCTRL_REGS_BASE + USB_BUFF_STATUS_OFFSET) = USB_BUFF_STATUS_EP1_IN_BITS;
}

void usb_send(uint8_t len){

    buffer_to_dpram(1, ep1_pid, (uint8_t *)(USBCTRL_DPRAM_BASE + USB_DEVICE_DPRAM_EP1_BUFFER0_OFFSET), ep1_buf, len);
    ep1_pid ^= 1;
}

void USBCTRL_IRQ_Handler() __attribute__((interrupt)) __attribute__((aligned(32)));

void USBCTRL_IRQ_Handler(){

    switch(REG(USBCTRL_REGS_BASE + USB_INTS_OFFSET) & (USB_INTS_BUFF_STATUS_BITS | USB_INTS_BUS_RESET_BITS | USB_INTS_SETUP_REQ_BITS)){
        case USB_INTS_BUS_RESET_BITS:
            handle_reset();
            break;
        case USB_INTS_SETUP_REQ_BITS:
            handle_ep0_setup();
            break;
        case USB_INTS_BUFF_STATUS_BITS:
            if(REG(USBCTRL_REGS_BASE + USB_BUFF_STATUS_OFFSET) & USB_BUFF_STATUS_EP0_IN_BITS)
                handle_ep0_in();
            else if(REG(USBCTRL_REGS_BASE + USB_BUFF_STATUS_OFFSET) & USB_BUFF_STATUS_EP0_OUT_BITS)
                handle_ep0_out();
            else if(REG(USBCTRL_REGS_BASE + USB_BUFF_STATUS_OFFSET) & USB_BUFF_STATUS_EP1_IN_BITS)
                handle_ep1_in();
    }
}

void delay(uint32_t t){

    for(uint32_t i = t; i; i--)
        __asm__ volatile("nop");
}

void prog(){

    flash_to_ram();
    clocks_init();
    usb_init();

    while(1){
        ep1_buf[0] = 0x5; // 'press' 'b' key
        usb_send(1);
        delay(100000);
        ep1_buf[0] = 0; // 'release' key
        usb_send(1);
        delay(1000000);
    }
}
