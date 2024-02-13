#include <driver/sound/ac97/ac97.h>
#include <stddef.h>
#include <stdio.h>
#include <memory/vmm.h>
#include <driver/pci/pci_io.h>
#include <utils/io.h>
#include <interrupts/interrupts.h>
#include <driver/timer_driver.h>

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
    debugf("AC97 IRQ");
}

void ac97_init(driver_t* driver)
{
    // DO INIT
    AC97Driver* ac97Driver = (AC97Driver*)driver;

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



    io_wait(500);


    ac97Driver->irqId = io_read_byte(address, PCI_INTERRUPT_LINE);
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
    ac97Driver->m_mixer_type = pci_get_bar(hdr.BAR0, 0, ac97Driver->GOOFY_bus, ac97Driver->GOOFY_device, ac97Driver->GOOFY_function);
    // //PrintMsgCol("> Mixer Address: {}", ConvertHexToString(m_mixer_address), Colors.yellow);
    // //PrintMsgCol("> Mixer Address (2): {}", ConvertHexToString(((PCI::PCIHeader0*)address)->BAR0), Colors.yellow);
    if (ac97Driver->m_mixer_type.type == MMIO64)
    {
        //osData.debugTerminalWindow->Log("> BUS TYPE USING BAR2", Colors.orange);
        ac97Driver->m_bus_type =  pci_get_bar(hdr.BAR0, 2, ac97Driver->GOOFY_bus, ac97Driver->GOOFY_device, ac97Driver->GOOFY_function);;//pci_get_bar((PCI::PCIHeader0*)address, 2);
    }
    else
    {
        //osData.debugTerminalWindow->Log("> BUS TYPE USING BAR1", Colors.orange);
        ac97Driver->m_bus_type = pci_get_bar(hdr.BAR0, 1, ac97Driver->GOOFY_bus, ac97Driver->GOOFY_device, ac97Driver->GOOFY_function);// pci_get_bar((PCI::PCIHeader0*)address, 1);
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
    int control = read_dword(address, ac97Driver->m_bus_type, GLOBAL_CONTROL);//inl(m_bus_address + BusRegisters::GLOBAL_CONTROL);
    control |= COLD_RESET | INTERRUPT_ENABLE;
    control &= ~(0b11 << 22);
    control |=   (0b00 << 22);
    write_dword(address, ac97Driver->m_bus_type, GLOBAL_CONTROL, control);//outl(m_bus_address + BusRegisters::GLOBAL_CONTROL, control);
    write_mixer(ac97Driver, RESET, 1);
    //PrintMsg("> Initialized Card");
    //Println();


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
    //PrintMsg("> Card Sample Rate: {}", to_string((int)m_sample_rate));

    if (ac97Driver->m_sample_rate != wantedSampleRate)
        debugf("> SAMPLE RATE IS BORKED");// PrintMsg("> Sample rate is borked!");//Panic("AC97: Failed to set sample rate! GOT: {}", to_string((int)m_sample_rate), true);

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
    // audioDestination->buffer->sampleCount = audioDestination->buffer->totalSampleCount;
    // //lastDone = true;
    // needManualRestart = true;
    // doCheck = true;

    // if (osData.ac97Driver == NULL)
    // {
    //     osData.ac97Driver = this;
    //     Audio::AudioOutputDevice* dev = new Audio::AudioOutputDevice("AC97 Device", audioDestination);
    //     osData.audioOutputDevices.Add(dev);
    //     osData.defaultAudioOutputDevice = dev;
    //     PrintMsg("> Set as main AC97 Driver");
    // }

    // PrintMsgEndLayer("AC97Driver");
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



bool AC97_DoQuickCheck(AC97Driver* driver)
{

}

void AC97_reset_output(AC97Driver* driver)
{

}

void AC97_set_sample_rate(AC97Driver* driver, uint32_t sample_rate)
{

}

bool AC97_handle_irq(AC97Driver* driver)
{

}

uint64_t AC97_writeBuffer(AC97Driver* driver, uint64_t offset, uint8_t* buffer, uint64_t count)
{

}

bool AC97_CheckMusic(AC97Driver* driver)
{

}

void HandleIRQ(AC97Driver* driver)
{

}