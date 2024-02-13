#pragma once
#include <stdint.h>

typedef struct attrStruct {
    bool io_space : 1;
    bool mem_space : 1;
    bool bus_master : 1;
    bool special_cycles : 1;
    bool mem_write_invalidate_enable : 1;
    bool VGA_palette_snoop : 1;
    bool parity_error_response : 1;
    bool : 1;
    bool serr_enable : 1;
    bool fast_back_to_back_enable : 1;
    bool interrupt_disable : 1;
    uint8_t reserved2 : 5;
} __attribute((packed)) attrStruct;

typedef union Command {
    attrStruct attrs;
	uint16_t value;
} Command;

//PORTS
#define PCI_ADDRESS_PORT 0xCF8
#define PCI_DATA_PORT 0xCFC

// Fields
#define PCI_VENDOR_ID 0x0 //16
#define PCI_DEVICE_ID 0x2 //16
#define PCI_COMMAND 0x4 //16
#define PCI_STATUS 0x6 //16
#define PCI_REVISION_ID 0x8 //8
#define PCI_PROG_IF 0x9 //8
#define PCI_SUBCLASS 0xa //8
#define PCI_CLASS 0xb //8
#define PCI_CACHE_LINE_SIZE 0xc //8
#define PCI_LATENCY_TIMER 0xd //8
#define PCI_HEADER_TYPE 0xe //8
#define PCI_BIST 0xf //8
#define PCI_BAR0 0x10 //32
#define PCI_BAR1 0x14 //32
#define PCI_BAR2 0x18 //32
#define PCI_BAR3 0x1C //32
#define PCI_BAR4 0x20 //32
#define PCI_BAR5 0x24 //32
#define PCI_PRIMARY_BUS 0x18 //8
#define PCI_SECONDARY_BUS 0x19 //8
#define PCI_INTERRUPT_LINE 0x3c //8
#define PCI_INTERRUPT_PIN 0x3d //8

typedef union IOAddress {
    struct __attribute((packed)) attrs {
        uint8_t field: 8;
        uint8_t function: 3;
        uint8_t slot: 5;
        uint8_t bus: 8;
        uint8_t : 7;
        bool enable: 1;
    } attrs;
    uint32_t value;
} IOAddress;


