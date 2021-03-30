#ifndef _DT_BINDINGS_RST_SUNPLUS_Q645_H
#define _DT_BINDINGS_RST_SUNPLUS_Q645_H


/* mo_reset0 ~ mo_reset4 */
#define RST_SYSTEM		  0x00
#define	RST_CA55CORE0		0x01
#define RST_CA55CORE1		0x02
#define	RST_CA55CORE2		0x03
#define RST_CA55CORE3		0x04
#define RST_CA55SCUL3		0x05
#define RST_CA55		    0x06
#define RST_IOP			    0x07
#define RST_PBUS0		    0x08
#define RST_PBUS1		    0x09
#define RST_PBUS2   		0x0a
#define RST_PBUS3	    	0x0b
#define RST_BR0	      	0x0c
#define RST_CARD_CTL0		0x0d
#define RST_CARD_CTL1		0x0e
#define RST_CARD_CTL2		0x0f

#define RST_CBDMA0		      0x10
#define RST_CPIOL		        0x11
#define RST_CPIOR		        0x12
#define RST_DDR_PHY0		    0x13
#define RST_SDCTL0		      0x14
#define RST_DUMMY_MASTER0		0x15
#define RST_DUMMY_MASTER1		0x16
#define RST_DUMMY_MASTER2		0x17
#define RST_EVDN		      	0x18
#define RST_SDPROT0			    0x19
#define RST_UMCTL2			    0x1a
#define RST_GPU			        0x1b
#define RST_HSM			        0x1c
#define RST_RBUS_TOP		    0x1d
#define RST_SPACC		        0x1e
#define RST_INTERRUPT		    0x1f

#define RST_N78		      0x20
#define RST_NIC400		  0x21
#define RST_OTPRX		    0x22
#define RST_PMC		      0x23
#define RST_RBUS_L00		0x24
#define RST_RBUS_L01		0x25
#define RST_RBUS_L02		0x26
#define RST_RBUS_L03		0x27
#define RST_RTC			    0x28
#define RST_MIPZ		  	0x29
#define RST_SPIFL			  0x2a
#define RST_BCH		      0x2b
#define RST_SPIND		    0x2c
#define RST_UADMA01		  0x2d
#define RST_UADMA23		  0x2e
#define RST_UA0		      0x2f

#define RST_UA1		    0x30
#define RST_UA2		    0x31
#define RST_UA3		    0x32
#define RST_UA4		    0x33
#define RST_UA5		    0x34
#define RST_UADBG		  0x35
#define RST_UART2AXI	0x36
#define RST_GDMAUA		0x37
#define RST_UPHY0		  0x38
#define RST_USB30C0		0x39
#define RST_USB30C1		0x3a
#define RST_U3PHY0		0x3b
#define RST_U3PHY1		0x3c
#define RST_USBC0			0x3d
#define RST_VCD		    0x3e
#define RST_VCE		    0x3f

#define RST_CM4			  0x40
#define RST_STC0		  0x41
#define RST_STC_AV0		0x42
#define RST_STC_AV1		0x43
#define RST_STC_AV2		0x44
#define RST_MAILBOX		0x45

#define RST_MAX			0xA0

#endif
