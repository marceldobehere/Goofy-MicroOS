#include <driver/sound/ac97/ac97.h>
#include <stddef.h>
#include <stdio.h>
#include <memory/vmm.h>

sound_driver_t* get_ac97_driver(pci_device_header_t header, uint16_t bus, uint16_t device, uint16_t function)
{
    return (sound_driver_t*)NEW_AC97Driver(header, bus, device, function);
}

void ac97_pci_found(pci_device_header_t header, uint16_t bus, uint16_t device, uint16_t function) 
{
	register_driver((driver_t*) get_ac97_driver(header, bus, device, function));
}

void ac97_init(driver_t* driver)
{
    // DO INIT
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

    

    return driver;
}



bool AC97_DoQuickCheck()
{

}

void AC97_reset_output()
{

}

void AC97_set_sample_rate(uint32_t sample_rate)
{

}

bool AC97_handle_irq()
{

}

uint64_t AC97_writeBuffer(uint64_t offset, uint8_t* buffer, uint64_t count)
{

}

bool AC97_CheckMusic()
{

}

void HandleIRQ()
{

}