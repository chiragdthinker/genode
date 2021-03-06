SRC_C   += $(addprefix net/usb/, usbnet.c smsc95xx.c)
SRC_C   += usb/host/ehci-omap.c

include $(REP_DIR)/lib/mk/usb.inc
include $(REP_DIR)/lib/mk/armv7/usb.inc

CC_OPT  += -DCONFIG_USB_EHCI_HCD_OMAP -DCONFIG_USB_EHCI_TT_NEWSCHED -DVERBOSE_DEBUG
SRC_CC  += platform.cc

vpath platform.cc $(LIB_DIR)/arm/platform_panda
