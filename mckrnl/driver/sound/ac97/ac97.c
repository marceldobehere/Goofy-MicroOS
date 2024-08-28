#include "driver/pci/pci_bar.h"
#include "driver/sound_driver.h"
#include <driver/sound/ac97/ac97.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <memory/vmm.h>
#include <driver/pci/pci_io.h>
#include <utils/io.h>
#include <interrupts/interrupts.h>
#include <driver/timer_driver.h>
#include <assert.h>

sound_driver_t* get_ac97_driver(pci_device_header_t header, uint16_t bus, uint16_t device, uint16_t function)
{
    return (sound_driver_t*)NEW_AC97Driver(header, bus, device, function);
}

void ac97_pci_found(pci_device_header_t header, uint16_t bus, uint16_t device, uint16_t function) 
{
	register_driver((driver_t*) get_ac97_driver(header, bus, device, function));
}

void ac97_goofy_irq(cpu_registers_t* regs, void* ac97V)
{
    AC97Driver* ac97 = (AC97Driver*)ac97V;
    debugf("AC97 IRQ !!!");
    todo();
}

void ac97_init(driver_t* driver)
{
    // DO INIT
    AC97Driver* ac97Driver = (AC97Driver*)driver;
    debugf("AC97 Drive Init");

    // PCIBaseAddress = pciBaseAddress;
    uint64_t address = (uint64_t)ac97Driver->GOOFY_function << 12 | (uint64_t)ac97Driver->GOOFY_device << 15 | (uint64_t)ac97Driver->GOOFY_bus << 20 | 0x80000000;
    // PrintMsgStartLayer("AC97Driver");

    // PrintMsg("> AC97 Driver Init");
    // osData.debugTerminalWindow->Log("YOOO AC97 YES");


    //PrintMsg("> Disabled PCI IO SPACE");
    //PCI::disable_io_space((uint64_t)PCIBaseAddress);
    disable_io_space(address);
    io_wait(5000);
    //PrintMsg("> Disabled PCI MEM SPACE");
    //PCI::disable_mem_space((uint64_t)PCIBaseAddress);
    disable_mem_space(address);
    //io_wait(5000);

    //Enable bus mastering and interrupts
    io_wait(500);
    enable_interrupt(address);
    //PrintMsg("> Enabled PCI Interrupts");
    io_wait(500);
    enable_bus_mastering(address);
    //PrintMsg("> Enabled PCI Bus Mastering");
    //Println();

    // osData.debugTerminalWindow->renderer->Println("> REV: {}", to_string(((PCI::PCIDeviceHeader*)PCIBaseAddress)->Revision_ID), Colors.bgreen);

    // for (int i = 0; i < 6; i++)
    // {
    //     osData.debugTerminalWindow->renderer->Print("> BAR {}: ", to_string(i), Colors.orange);
    //     osData.debugTerminalWindow->renderer->Println("{}", ConvertHexToString(*(((uint32_t*)&((PCI::PCIHeader0*)PCIBaseAddress)->BAR0) + i)), Colors.orange);
    //     io_wait(1000);
    // }

    

    bool driverOkay = true;
    if (ac97Driver->GOOFY_header.BAR1 == NULL)
    {
        abortf("AC97 Driver only uses BAR0 and is not supported");
        // PrintMsg("> AC97 Driver only uses BAR0 and is not supported");
        // osData.debugTerminalWindow->Log("INVALID AC97");
        return;
    }


    // if (osData.ac97Driver != NULL)
    // {
    //     PrintMsg("> AC97 Driver NVM");
    //     osData.debugTerminalWindow->Log("NVM AC97");
    //     return;
    // }


    // PrintMsg("> Enabled PCI IO SPACE");
    enable_io_space(address);
    io_wait(500);
    // PrintMsg("> Enabled PCI MEM SPACE");
    enable_mem_space(address);
    io_wait(500);

    debugf("AC97 MAP ADDR");
    pci_bar_t addrBar = pci_get_bar(&ac97Driver->GOOFY_header.BAR0, 0, ac97Driver->GOOFY_bus, ac97Driver->GOOFY_device, ac97Driver->GOOFY_function);
    debugf("AC97 MEM: %x. IO: %x, SIZE: %x, TYPE: %d", (uint32_t)addrBar.mem_address, addrBar.io_address, addrBar.size, addrBar.type);

    io_wait(500);


    // uint8_t irqId1 = io_read_byte(address, PCI_INTERRUPT_LINE);
    uint8_t irqId2 = ac97Driver->GOOFY_header.interrupt_line;

    ac97Driver->irqId = irqId2 + 0x20;

    // //Panic("AC97 IRQ: {}", to_string(irqId), true);
    // {
    //     IRQHandlerCallbackFuncs[irqId] = (void*)&HandleIRQ;
    //     IRQHandlerCallbackHelpers[irqId] = (void*)this;
    // }
    register_interrupt_handler(ac97Driver->irqId, (void*)&ac97_goofy_irq, (void*)ac97Driver);

    //debugf("YOOOOOOOOOOOOOO");
    //while (1);

    // // m_address = address;
    // //m_mixer_address = PCI::io_read_word(address, PCI_BAR0) & ~1;
    pci_device_header_t hdr = ac97Driver->GOOFY_header;
    ac97Driver->m_mixer_type = pci_get_bar(&hdr.BAR0, 0, ac97Driver->GOOFY_bus, ac97Driver->GOOFY_device, ac97Driver->GOOFY_function);
    // //PrintMsgCol("> Mixer Address: {}", ConvertHexToString(m_mixer_address), Colors.yellow);
    // //PrintMsgCol("> Mixer Address (2): {}", ConvertHexToString(((PCI::PCIHeader0*)address)->BAR0), Colors.yellow);
    if (ac97Driver->m_mixer_type.type == MMIO64)
    {
        //osData.debugTerminalWindow->Log("> BUS TYPE USING BAR2", Colors.orange);
        ac97Driver->m_bus_type =  pci_get_bar(&hdr.BAR0, 2, ac97Driver->GOOFY_bus, ac97Driver->GOOFY_device, ac97Driver->GOOFY_function);;//pci_get_bar((PCI::PCIHeader0*)address, 2);
    }
    else
    {
        //osData.debugTerminalWindow->Log("> BUS TYPE USING BAR1", Colors.orange);
        ac97Driver->m_bus_type = pci_get_bar(&hdr.BAR0, 1, ac97Driver->GOOFY_bus, ac97Driver->GOOFY_device, ac97Driver->GOOFY_function);// pci_get_bar((PCI::PCIHeader0*)address, 1);
    }
    // //PrintMsgCol("> Mixer Address (3): {}", ConvertHexToString(m_mixer_address), Colors.yellow);

    // //PrintMsgCol("> Bus Address: {}", ConvertHexToString(m_bus_address), Colors.yellow);
    // //PrintMsgCol("> Bus Address (2): {}", ConvertHexToString(((PCI::PCIHeader0*)address)->BAR1), Colors.yellow);
    ac97Driver->m_output_channel = /*m_bus_address +*/ NABM_PCM_OUT;
    // PrintMsgCol("> Output Channel: {}", ConvertHexToString(m_output_channel), Colors.yellow);
    // Println();

    // osData.debugTerminalWindow->Log("TYPES: NONE, M64, M32, IO ", Colors.bgreen);

    // osData.debugTerminalWindow->Log("AC97 MIXER TYPE: {}", to_string(m_mixer_type.type), Colors.bgreen);
    // osData.debugTerminalWindow->Log("AC97 MIXER IO BASE: {}", ConvertHexToString(m_mixer_type.io_address), Colors.bgreen);
    // osData.debugTerminalWindow->Log("AC97 MIXER MEM BASE: {}", ConvertHexToString(m_mixer_type.mem_address), Colors.bgreen);

    // osData.debugTerminalWindow->Log("AC97 BUS TYPE: {}", to_string(m_bus_type.type), Colors.bgreen);
    // osData.debugTerminalWindow->Log("AC97 BUS IO BASE: {}", ConvertHexToString(m_bus_type.io_address), Colors.bgreen);
    // osData.debugTerminalWindow->Log("AC97 BUS MEM BASE: {}", ConvertHexToString(m_bus_type.mem_address), Colors.bgreen);

    // //m_output_buffer_region = MM.alloc_dma_region(0x1000 * AC97_OUTPUT_BUFFER_PAGES);
    // //m_output_buffer_descriptor_region = MM.alloc_dma_region(sizeof(BufferDescriptor) * AC97_NUM_BUFFER_DESCRIPTORS);

    // //m_output_buffer_region = (uint8_t*)_Malloc(0x1000 * AC97_OUTPUT_BUFFER_PAGES);
    // //m_output_buffer_descriptor_region = (uint8_t*)_Malloc(sizeof(BufferDescriptor) * AC97_NUM_BUFFER_DESCRIPTORS);

    

    ac97Driver->m_output_buffer_region = (uint8_t*)vmm_alloc(AC97_OUTPUT_BUFFER_PAGES);// GlobalAllocator->RequestPages(AC97_OUTPUT_BUFFER_PAGES);
    //GlobalPageTableManager.MapMemories((void*)ac97Driver->m_output_buffer_region, (void*)ac97Driver->m_output_buffer_region, AC97_OUTPUT_BUFFER_PAGES, true);
    memset(ac97Driver->m_output_buffer_region, 0, 0x1000 * AC97_OUTPUT_BUFFER_PAGES);
    //PrintMsgCol("> Output Buffer Region: {}", ConvertHexToString((uint64_t)m_output_buffer_region), Colors.yellow);
    
    //Panic("BRUH {}", ConvertHexToString((uint64_t)m_output_buffer_region), true);
    ac97Driver->m_output_buffer_descriptor_region = vmm_alloc(1);//(uint8_t*)GlobalAllocator->RequestPages(1);//(uint8_t*)_Malloc(sizeof(BufferDescriptor) * AC97_NUM_BUFFER_DESCRIPTORS);
    //GlobalPageTableManager.MapMemories((void*)ac97Driver->m_output_buffer_descriptor_region, (void*)ac97Driver->m_output_buffer_descriptor_region, 1, true);
    memset(ac97Driver->m_output_buffer_descriptor_region, 0, sizeof(BufferDescriptor) * AC97_NUM_BUFFER_DESCRIPTORS);
    //PrintMsgCol("> Output Buffer Descriptor Region: {}", ConvertHexToString((uint64_t)m_output_buffer_descriptor_region), Colors.yellow);
    //Println();
    ac97Driver->writeBufferCount = 0;

    ac97Driver->m_output_buffer_descriptors = (BufferDescriptor*) ac97Driver->m_output_buffer_descriptor_region;


    //Initialize the card with cold reset of bus and mixer, enable interrupts
    int control = read_dword(ac97Driver->m_bus_type.mem_address, ac97Driver->m_bus_type, GLOBAL_CONTROL);//inl(m_bus_address + BusRegisters::GLOBAL_CONTROL);
    control |= COLD_RESET | INTERRUPT_ENABLE;
    control &= ~(0b11 << 22);
    control |=   (0b00 << 22);
    write_dword(address, ac97Driver->m_bus_type, GLOBAL_CONTROL, control);//outl(m_bus_address + BusRegisters::GLOBAL_CONTROL, control);
    write_mixer(ac97Driver, RESET, 1);
    //PrintMsg("> Initialized Card");
    //Println();
    debugf("AC97 Drive BLEH");


    //TODO: Verify version?

    //Set master volume and reset pcm out channel
    write_mixer(ac97Driver, MASTER_VOLUME, 0);
    //PrintMsg("> Set Master Volume");
    write_mixer(ac97Driver, PCM_VOLUME, 0x0808);
    //PrintMsg("> Set PCM Volume");
    AC97_reset_output(ac97Driver);

    const int wantedSampleRate = 48000;

    //PrintMsg("> Setting Sample Rate to {}", to_string(wantedSampleRate));
    int tries = 10;
    while (tries-- > 0)
    {
        AC97_set_sample_rate(ac97Driver, wantedSampleRate);
        if (ac97Driver->m_sample_rate == wantedSampleRate)
            break;
        io_wait(5000);
    }
    //PrintMsg("> Card Sample Rate: {}", to_string((int)>m_sample_rate));

    if (ac97Driver->m_sample_rate != wantedSampleRate)
        debugf("> SAMPLE RATE IS BORKED (%d != %d)", ac97Driver->m_sample_rate, wantedSampleRate)// PrintMsg("> Sample rate is borked!");//Panic("AC97: Failed to set sample rate! GOT: {}", to_string((int)>m_sample_rate), true);
    else
        debugf("> SAMPLE RATE OK!")


    AC97_reset_output(ac97Driver);
    // PrintMsg("> Reset Output");
    ac97Driver->dataReady = false;
    ac97Driver->QuickCheck = false;
    ac97Driver->lastCheckTime =  global_timer_driver->time_ms(global_timer_driver);//PIT::TimeSinceBootMS();

    ac97Driver->DEF_SAMPLE_COUNT = ac97Driver->m_sample_rate / 47;

    // --------------------------

    // audioDestination = new Audio::BasicAudioDestination(
    //     //Audio::AudioBuffer::Create16Bit48KHzStereoBuffer(DEF_SAMPLE_COUNT)
    //     Audio::AudioBuffer::Create16Bit48KHzStereoBuffer(DEF_SAMPLE_COUNT)
    // );
    //audioDestination->buffer->sampleCount = audioDestination->buffer->totalSampleCount;
    //lastDone = true;
    ac97Driver->needManualRestart = true;
    ac97Driver->doCheck = true;

    // if (osData.ac97Driver == NULL)
    // {
    //     osData.ac97Driver = this;
    //     Audio::AudioOutputDevice* dev = new Audio::AudioOutputDevice("AC97 Device", audioDestination);
    //     osData.audioOutputDevices.Add(dev);
    //     osData.defaultAudioOutputDevice = dev;
    //     PrintMsg("> Set as main AC97 Driver");
    // }

    // PrintMsgEndLayer("AC97Driver");

    debugf("AC97 Drive Init Done");

    global_sound_driver = &ac97Driver->driver;
}

char* ac97_get_device_name(driver_t* driver)
{
    return "AC97";
}

bool ac97_is_device_present(driver_t* driver)
{
    return true;
}

AC97Driver* NEW_AC97Driver (pci_device_header_t header, uint16_t bus, uint16_t device, uint16_t function)
{   
    AC97Driver* driver = (AC97Driver*)vmm_alloc(PAGES_OF(AC97Driver));
    driver->driver.driver.init = ac97_init;
    driver->driver.driver.get_device_name = ac97_get_device_name;
    driver->driver.driver.is_device_present = ac97_is_device_present;

    driver->GOOFY_header = header;
    driver->GOOFY_bus = bus;
    driver->GOOFY_device = device;
    driver->GOOFY_function = function;

    return driver;
}

inline int min(int a, int b)
{
    if (a <= b)
        return a;
    return b;
}

inline int max(int a, int b)
{
    if (a >= b)
        return a;
    return b;
}

uint64_t AC97_writeBuffer(AC97Driver* driver, uint64_t offset, uint8_t* buffer, uint64_t count)
{
    buffer += offset;
    count -= offset;

    uint64_t n_written = 0;
    while(count > 0) 
    {
        // //Wait until we have a free buffer to write to
        // do 
        // {
        //     //Read the status, current index, and last valid index
        //     //TaskManager::ScopedCritical critical;
        //     auto status_byte = inw(m_output_channel + ChannelRegisters::STATUS);
        //     BufferStatus status = {.value = status_byte};
        //     auto current_index = inb(m_output_channel + ChannelRegisters::CURRENT_INDEX);
        //     auto last_valid_index = inb(m_output_channel + ChannelRegisters::LAST_VALID_INDEX);
        //     auto num_buffers_left = last_valid_index >= current_index ? last_valid_index - current_index : AC97_NUM_BUFFER_DESCRIPTORS - (current_index - last_valid_index);
        //     if(!status.is_halted)
        //         num_buffers_left++;
        //     if(num_buffers_left < AC97_NUM_BUFFER_DESCRIPTORS)
        //         break;
        //     //critical.exit();
        //     m_blocker = false;
        //     //TaskManager::current_thread()->block(m_blocker);
        //     // if(m_blocker.was_interrupted())
        //     //     return 0;
        // } while(m_output_dma_enabled);

        //If the output DMA is not currently enabled, reset the PCM channel to be sure
        if(!driver->m_output_dma_enabled)
            AC97_reset_output(driver);

        //reset_output();

        //Copy as much data as is applicable to the current output buffer
        uint32_t* output_buffer = (uint32_t*)(driver->m_output_buffer_region + 0x1000 * driver->m_current_output_buffer_page);
        size_t num_bytes = min(count, 0x1000);
        //buffer.read((uint8_t*) output_buffer, n_written, num_bytes); // buff offset count
        memcpy(output_buffer, (uint8_t*) buffer + n_written, num_bytes);
        count -= num_bytes;
        n_written += num_bytes;

        //Create the buffer descriptor
        BufferDescriptor* descriptor = &driver->m_output_buffer_descriptors[driver->m_current_buffer_descriptor];
        descriptor->data_addr = (uint32_t)(((uint64_t)driver->m_output_buffer_region) + 0x1000 * driver->m_current_output_buffer_page);//m_output_buffer_region->object()->physical_page(m_current_output_buffer_page).paddr();
        descriptor->num_samples = num_bytes / sizeof(uint16_t);
        //descriptor->flags = {false, true};
        descriptor->flags.is_last_entry = false;
        descriptor->flags.interrupt_on_completion = true;

        //Set the buffer descriptor list address and last valid index in the channel registers
        write_dword(0, driver->m_bus_type, driver->m_output_channel + BUFFER_LIST_ADDR, (uint32_t)(uint64_t)driver->m_output_buffer_descriptor_region);//outl(m_output_channel + ChannelRegisters::BUFFER_LIST_ADDR, (uint32_t)(uint64_t)m_output_buffer_descriptor_region);//m_output_buffer_descriptor_region->object()->physical_page(0).paddr());
        write_byte(0, driver->m_bus_type, driver->m_output_channel + LAST_VALID_INDEX, driver->m_current_buffer_descriptor);//outb(m_output_channel + ChannelRegisters::LAST_VALID_INDEX, m_current_buffer_descriptor);

        //If the output DMA is not enabled already, enable it
        if(!driver->m_output_dma_enabled) {
            uint8_t ctrl = read_byte(0, driver->m_bus_type, driver->m_output_channel + CONTROL);//inb(m_output_channel + ChannelRegisters::CONTROL);
            ctrl |= PAUSE_BUS_MASTER | ERROR_INTERRUPT | COMPLETION_INTERRUPT;
            write_byte(0, driver->m_bus_type, driver->m_output_channel + CONTROL, ctrl);//outb(m_output_channel + ChannelRegisters::CONTROL, ctrl);
            driver->m_output_dma_enabled = true;
        }

        //Increment buffer page and buffer descriptor index
        driver->m_current_output_buffer_page++;
        driver->m_current_output_buffer_page %= AC97_OUTPUT_BUFFER_PAGES;
        driver->m_current_buffer_descriptor++;
        driver->m_current_buffer_descriptor %= AC97_NUM_BUFFER_DESCRIPTORS;
    }

    return n_written;
}


void AC97_reset_output(AC97Driver* driver)
{
    int timeOut = 200;
    do
    {
        write_byte(0, driver->m_bus_type, driver->m_output_channel + CONTROL, RESET_REGISTERS);//outb(m_output_channel + ChannelRegisters::CONTROL, ControlFlags::RESET_REGISTERS);
        io_wait(10);
    } while((timeOut-- > 0) && read_byte(0, driver->m_bus_type, driver->m_output_channel + CONTROL) & RESET_REGISTERS); //inb(m_output_channel + ChannelRegisters::CONTROL)

    driver->m_output_dma_enabled = false;
    driver->m_current_buffer_descriptor = 0;
    driver->writeBufferCount = 0;
}

void AC97_set_sample_rate(AC97Driver* driver, uint32_t sample_rate)
{
    io_wait(1000);
    write_word(0, driver->m_mixer_type, SAMPLE_RATE, sample_rate);//outw(m_mixer_address + MixerRegisters::SAMPLE_RATE, sample_rate);
    io_wait(1000);
    driver->m_sample_rate = read_word(0, driver->m_mixer_type, SAMPLE_RATE);//inw(m_mixer_address + MixerRegisters::SAMPLE_RATE);
}

bool AC97_DoQuickCheck(AC97Driver* driver)
{
    if (driver->QuickCheck)
    {
        return false;
    }


    driver->QuickCheck = true;
    int samplesReady = 0;

    // TEST
    int size = 2 * 6000;
    uint16_t bruhus[size];
    for (int i = 0; i < size; i++)
    {
        bruhus[i] = ((i / 200) & 1) * 5000;
    }
    samplesReady = size;



    if (samplesReady < 1)
    {
        // TODO
        // audioDestination->buffer->sampleCount = audioDestination->buffer->totalSampleCount;
        // TODO
        // samplesReady = audioDestination->RequestBuffers();
        // TODO
        // AudioDeviceStuff::reqMoreData(audioDestination);
    }

    if (samplesReady > 0 && 
        driver != NULL)
    {
        
        uint64_t tCount = 0;
        //Serial::Writeln("> Writing {} bytes", to_string(audioDestination->buffer->byteCount));
        // tCount = osData.ac97Driver->writeBuffer(0, 
        //     (uint8_t*)(audioDestination->buffer->data), 
        //     audioDestination->buffer->byteCount);

        //samplesReady = audioDestination->buffer->totalSampleCount;


        // TODO
        int byteCount = samplesReady * 2;// samplesReady * (audioDestination->buffer->bitsPerSample / 8) * audioDestination->buffer->channelCount; // 16 bit, stereo
        uint8_t* data = bruhus;//(uint8_t*)(audioDestination->buffer->data);

        tCount = AC97_writeBuffer(driver, 0, 
            data, 
            byteCount);
        //Serial::TWritelnf("> Writing %d/%d bytes (%d/%d samples) -> %d", byteCount, audioDestination->buffer->byteCount, samplesReady,audioDestination->buffer->totalSampleCount, tCount);

        if (tCount != byteCount)
        {
            abortf("AC97Driver::HandleIRQ: tCount != byteCount");
        }

        // TODO:
        // audioDestination->buffer->ClearBuffer();
        // audioDestination->buffer->sampleCount = audioDestination->buffer->totalSampleCount;

        
        //lastDone = tDone;
        //Panic("bruh: {}", to_string(c), true);
        //Serial::Write("NICE");

        //Serial::Writeln("<WROTE LE MUSIC>");
        driver->QuickCheck = false;
        samplesReady = 0;
        return true;
    }
    else
    {
        // TODO: SEND MSG TO REQ AUDIO
        //AudioDeviceStuff::reqMoreData(audioDestination);
    }
    
    

    // if (!dataReady)
    // {
    //     int c = audioDestination->RequestBuffers();
    //     if (c > 0)
    //     {
    //         dataReady = true;
    //     }
    // }


    driver->QuickCheck = false;
    return false;
}

bool AC97_CheckMusic(AC97Driver* driver)
{
    // TODO
    // if (driver->samplesReady > 0)
    // {
    //     return false;
    //     bool ret =!handle_irq(); 
    //     //Serial::Writeln("</AC97 CheckMusic: {}>", to_string(ret));
    //     return ret;
    // }
    // //return true;


    // TODO
    // audioDestination->buffer->sampleCount = audioDestination->buffer->totalSampleCount;
    // samplesReady = audioDestination->RequestBuffers();
    // AudioDeviceStuff::reqMoreData(audioDestination);
    // if (samplesReady > 0)
    // {
    //     //Serial::Writeln("</AC97 CheckMusic: {}>", to_string(false));
    //     return false;
    // }
    // else
    // {
    //     // TODO: SEND MSG TO REQ AUDIO
    // }
    
    return false;

    
    //Serial::Writeln("</AC97 CheckMusic: {}>", to_string(true));
    return true;
}

bool AC97_handle_irq(AC97Driver* driver)
{
    debugf(" > AC97 Interrupt")
    driver->lastCheckTime = global_timer_driver->time_ms(global_timer_driver);
    //Read the status
    uint16_t status_byte = read_word(0, driver->m_bus_type, driver->m_output_channel + STATUS);//inw(m_output_channel + ChannelRegisters::STATUS);
    BufferStatus status = {.value = status_byte};

    if(status.fifo_error)
        debugf("> AC97 GOT A FIFO ERROR!");//Panic("AC97 GOT FIFO ERROR!");//KLog::err("AC97", "Encountered FIFO error!");



    //osData.debugTerminalWindow->newPosition.x--;

    status.value = 0;
    status.completion_interrupt_status = true;
    status.last_valid_interrupt = true;
    status.fifo_error = true;
    write_word(0, driver->m_bus_type, driver->m_output_channel + STATUS, status.value);;//outw(m_output_channel + ChannelRegisters::STATUS, status.value);

    // DoQuickCheck();
    // return true;



    uint8_t current_index = read_byte(0, driver->m_bus_type, driver->m_output_channel + CURRENT_INDEX);//inb(m_output_channel + ChannelRegisters::CURRENT_INDEX);
    uint8_t last_valid_index = read_byte(0, driver->m_bus_type, driver->m_output_channel + LAST_VALID_INDEX);//inb(m_output_channel + ChannelRegisters::LAST_VALID_INDEX);
    
    int offset = 4;
    int beginIndex = (last_valid_index + AC97_NUM_BUFFER_DESCRIPTORS - offset) % AC97_NUM_BUFFER_DESCRIPTORS;
    int endIndex = (last_valid_index + 2) % AC97_NUM_BUFFER_DESCRIPTORS;

    if (endIndex >= beginIndex)
    {
        if (current_index >= beginIndex && current_index <= endIndex)
        {
            return AC97_DoQuickCheck(driver);
        }
    }
    else
    {
        if (current_index >= beginIndex || current_index <= endIndex)
        {
            return AC97_DoQuickCheck(driver);
        }
    }

    //If we're not done, don't do anything
    if(!status.completion_interrupt_status)
        return false;
    
    // if(current_index >= ((last_valid_index + AC97_NUM_BUFFER_DESCRIPTORS - 4) % AC97_NUM_BUFFER_DESCRIPTORS)) {
    //     //reset_output();
    //     DoQuickCheck();
    // }
    driver->m_blocker = true;

    return false;
}

void HandleIRQ(AC97Driver* driver)
{
    debugf(" > AC97 Interrupt 2");
    driver->lastCheckTime = global_timer_driver->time_ms(global_timer_driver);
    AC97_handle_irq(driver);
    driver->doCheck = true;
}



void AC97_TimerCheckCallback(AC97Driver* driver)
{
    if (driver == NULL)
        return;

    int64_t currTime = (int64_t)global_timer_driver->time_ms(global_timer_driver); 
    //debugf("AC97 Start - %d", currTime);
    int64_t ac97Time = ((int64_t)(driver->lastCheckTime + 500));

    if (currTime > ac97Time)
    {
        debugf("AC97 Start 1 - %d", currTime)
        AC97_DoQuickCheck(driver);
        driver->lastCheckTime = currTime;


        HandleIRQ(driver);
        // driver->needManualRestart = true;  

        // debugf("AC97 End %d", global_timer_driver->time_ms(global_timer_driver))          
    }
    else if (driver->doCheck)
    {
        debugf("AC97 Start 2 - %d", currTime)
        driver->doCheck = false;
        // driver->needManualRestart = AC97_CheckMusic(driver);

        // // debugf("AC97 End %d", global_timer_driver->time_ms(global_timer_driver))
    }
    else if (driver->needManualRestart)
    {
        debugf("AC97 Start 3 - %d", currTime)
        
        driver->needManualRestart = AC97_CheckMusic(driver);

        // // debugf("AC97 End %d", global_timer_driver->time_ms(global_timer_driver))
    }

    //debugf("AC97 End %d", global_timer_driver->time_ms(global_timer_driver));
}