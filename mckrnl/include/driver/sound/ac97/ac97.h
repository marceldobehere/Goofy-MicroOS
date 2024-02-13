#pragma once
#include <driver/sound_driver.h>
#include <driver/pci/pci.h>
#include <driver/pci/pci_bar.h>

sound_driver_t* get_ac97_driver(pci_device_header_t header, uint16_t bus, uint16_t device, uint16_t function);
void ac97_pci_found(pci_device_header_t header, uint16_t bus, uint16_t device, uint16_t function);

#define AC97_OUTPUT_BUFFER_PAGES 32
#define AC97_NUM_BUFFER_DESCRIPTORS 32

typedef enum BusRegisters {
    NABM_PCM_IN	= 0x00u,
    NABM_PCM_OUT   = 0x10u,
    NABM_MIC	   = 0x20u,
    GLOBAL_CONTROL = 0x2Cu,
    GLOBAL_STATUS  = 0x30u
} BusRegisters;

typedef enum MixerRegisters {
    RESET		 = 0x00u, //word
    MASTER_VOLUME = 0x02u, //word
    MIC_VOLUME	= 0x0Eu, //word
    PCM_VOLUME	= 0x18u, //word
    INPUT_DEVICE  = 0x1Au, //word
    INPUT_GAIN	= 0x1Cu, //word
    MIC_GAIN	  = 0x1Eu, //word
    SAMPLE_RATE   = 0x2Cu //word
} MixerRegisters;

typedef enum GlobalControl {
    INTERRUPT_ENABLE = 0b00000001,
    COLD_RESET	   = 0b00000010
} GlobalControl;

typedef enum ChannelRegisters {
    BUFFER_LIST_ADDR  = 0x00u, //dword
    CURRENT_INDEX	 = 0x04u, //byte
    LAST_VALID_INDEX  = 0x05u, //byte
    STATUS			= 0x06u, //word
    BUFFER_POSITION   = 0x08u, //word
    PREFETCHED_INDEX  = 0x0Au, //byte
    CONTROL		   = 0x0Bu, //byte
} ChannelRegisters;

typedef enum ControlFlags {
    PAUSE_BUS_MASTER	 = 0b00000001,
    RESET_REGISTERS	  = 0b00000010,
    ERROR_INTERRUPT	  = 0b00001000,
    COMPLETION_INTERRUPT = 0b00010000
} ControlFlags;

typedef struct BufferDescriptor {
    uint32_t data_addr;
    uint16_t num_samples;
    struct {
        uint16_t : 14;
        bool is_last_entry : 1;
        bool interrupt_on_completion : 1;
    }  __attribute__((packed)) flags;
} __attribute__((packed)) BufferDescriptor;

typedef union BufferStatus {
    struct {
        bool is_halted : 1;
        bool is_last_valid : 1;
        bool last_valid_interrupt : 1;
        bool completion_interrupt_status : 1;
        bool fifo_error : 1;
        uint16_t : 11;
    };
    uint16_t value;
} BufferStatus;

typedef struct AC97Driver 
{
    sound_driver_t driver;

    BufferDescriptor bufferDescriptor;

    BufferStatus bufferStatus;
    
    ///Audio::BasicAudioDestination* audioDestination;
    void* PCIBaseAddress;     
    pci_bar_t m_mixer_type, m_bus_type;
    BufferDescriptor* m_output_buffer_descriptors;

    int DEF_SAMPLE_COUNT;/// = 4096;
    //bool lastDone;
    bool needManualRestart;
    bool doCheck;
    bool dataReady;


    bool QuickCheck;/// = false;
    
    //~AC97Driver();


    uint64_t writeBufferCount;

    
    uint64_t lastCheckTime;/// = 0;

    pci_device_header_t GOOFY_header;
    uint16_t GOOFY_bus;
    uint16_t GOOFY_device;
    uint16_t GOOFY_function;


    uint16_t /*m_mixer_address, m_bus_address, */m_output_channel;
    uint8_t* m_output_buffer_region; // kstd::Arc<VMRegion>
    uint8_t* m_output_buffer_descriptor_region;

    uint32_t m_current_output_buffer_page;/// = 0;
    uint32_t m_current_buffer_descriptor;/// = 0;
    bool m_output_dma_enabled;/// = false;
    bool m_blocker; //BooleanBlocker
    uint32_t m_sample_rate;   
    uint8_t irqId;/// = 0;
} AC97Driver;

AC97Driver* NEW_AC97Driver (pci_device_header_t header, uint16_t bus, uint16_t device, uint16_t function);
bool AC97_DoQuickCheck();
void AC97_reset_output();
void AC97_set_sample_rate(uint32_t sample_rate);
bool AC97_handle_irq();
uint64_t AC97_writeBuffer(uint64_t offset, uint8_t* buffer, uint64_t count);
bool AC97_CheckMusic();
void HandleIRQ();

#include <utils/io.h>

inline void write_word(uint64_t address, pci_bar_t type, uint16_t field, uint16_t value)
{
    if (type.type == MMIO32 || type.type == MMIO64)
        *(uint16_t*)((uint32_t)type.mem_address + field) = value;
    else if (type.type == IO)
        outw(type.io_address + field, value);
}

inline void write_mixer(AC97Driver* driver, MixerRegisters reg, uint16_t val) {
    write_word((uint64_t)driver->PCIBaseAddress, driver->m_mixer_type, reg, val);//outw(m_mixer_address + reg, val);
    io_wait(10);
}

