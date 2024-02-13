#include <driver/pci/pci.h>

#include <utils/io.h>

uint32_t pci_readd(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	outl(0xcf8, id);
	uint32_t result = inl(0xcfc);
	return result >> (8 * (registeroffset % 4));
}

void pci_writed(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	outl(0xcf8, id);
	outl(0xcfc, value);
}

void pci_writeb(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint8_t value) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	outl(0xcf8, id);
	outb(0xcfc, value);
}

uint8_t pci_readb(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	outl(0xcf8, id);
	uint8_t result = inb(0xcfc);
	return result >> (8 * (registeroffset % 4));
}

void pci_writew(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint16_t value) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	outl(0xcf8, id);
	outw(0xcfc, value);
}

uint16_t pci_readw(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset) {
	uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
	outl(0xcf8, id);
	uint16_t result = inw(0xcfc);
	return result >> (8 * (registeroffset % 4));
}

int device_has_functions(uint16_t bus, uint16_t device) {
	return pci_readd(bus, device, 0, 0xe) & (1 << 7);
}

void enable_mmio(uint16_t bus, uint16_t device, uint16_t function) {
	pci_writed(bus, device, function, 0x4, pci_readd(bus, device, function, 0x4) | (1 << 1));
}

void become_bus_master(uint16_t bus, uint16_t device, uint16_t function) {
	pci_writed(bus, device, function, 0x4, pci_readd(bus, device, function, 0x4) | (1 << 2));
}

pci_device_header_t get_device_header(uint16_t bus, uint16_t device, uint16_t function) {
	pci_device_header_t result;
	result.vendor_id = pci_readd(bus, device, function, 0);
	result.device_id = pci_readd(bus, device, function, 2);
	result.revision_id = pci_readd(bus, device, function, 8);
	result.prog_if = pci_readd(bus, device, function, 9);
	result.subclass = pci_readd(bus, device, function, 0xa);
	result.class_ = pci_readd(bus, device, function, 0xb);
	result.BAR0 = pci_readd(bus, device, function, 0x10);
	result.BAR1 = pci_readd(bus, device, function, 0x14);
	result.BAR2 = pci_readd(bus, device, function, 0x18);
	result.BAR3 = pci_readd(bus, device, function, 0x1c);
	result.BAR4 = pci_readd(bus, device, function, 0x20);
	result.BAR5 = pci_readd(bus, device, function, 0x24);
	result.interrupt_line = pci_readd(bus, device, function, 0x3c);
	result.interrupt_pin = pci_readd(bus, device, function, 0x3d);
	return result;
}

#include <driver/pci/pci_io.h>

IOAddress get_address(uint64_t addr, uint8_t field)
{
    IOAddress address;
    address.attrs.field = field;
    address.attrs.function = (addr >> 12) & 0b111; // 3 bit
    address.attrs.slot = (addr >> 15) & 0b11111; // 5 bit
    address.attrs.bus = (addr >> 20) & 0b11111111; // 8 bit
    address.attrs.enable = true;
    //0x80000000 
    return address;
}

uint8_t io_read_byte(uint64_t address, uint8_t field) {
    outl(PCI_ADDRESS_PORT, get_address(address, field).value);
    return inb(PCI_DATA_PORT + (field & 3));
}

uint16_t io_read_word(uint64_t address, uint8_t field){
    outl(PCI_ADDRESS_PORT, get_address(address, field).value);
    return inw(PCI_DATA_PORT + (field & 2));
}

uint32_t io_read_dword(uint64_t address, uint8_t field) {
    outl(PCI_ADDRESS_PORT, get_address(address, field).value);
    return inl(PCI_DATA_PORT);
}

void io_write_byte(uint64_t address, uint8_t field, uint8_t value) {
    outl(PCI_ADDRESS_PORT, get_address(address, field).value);
    outb(PCI_DATA_PORT + (field & 3), value);
}

void io_write_word(uint64_t address, uint8_t field, uint16_t value) {
    outl(PCI_ADDRESS_PORT, get_address(address, field).value);
    outw(PCI_DATA_PORT + (field & 2), value);
}

void io_write_dword(uint64_t address, uint8_t field, uint32_t value) {
    outl(PCI_ADDRESS_PORT, get_address(address, field).value);
    outl(PCI_DATA_PORT, value);
}

void enable_io_space(uint64_t address)
{
	Command comm = {.value = io_read_word(address, PCI_COMMAND)};
	comm.attrs.io_space = true;
	io_write_word(address, PCI_COMMAND, comm.value);
}
void disable_io_space(uint64_t address)
{
	Command comm = {.value = io_read_word(address, PCI_COMMAND)};
	comm.attrs.io_space = false;
	io_write_word(address, PCI_COMMAND, comm.value);
}
void enable_mem_space(uint64_t address)
{
	Command comm = {.value = io_read_word(address, PCI_COMMAND)};
	comm.attrs.mem_space = true;
	io_write_word(address, PCI_COMMAND, comm.value);
}
void disable_mem_space(uint64_t address)
{
	Command comm = {.value = io_read_word(address, PCI_COMMAND)};
	comm.attrs.mem_space = false;
	io_write_word(address, PCI_COMMAND, comm.value);
}

void enable_interrupt(uint64_t address) {
	Command comm = {.value = io_read_word(address, PCI_COMMAND)};
	comm.attrs.interrupt_disable = false;
	io_write_word(address, PCI_COMMAND, comm.value);
}

void disable_interrupt(uint64_t address) {
	Command comm = {.value = io_read_word(address, PCI_COMMAND)};
	comm.attrs.interrupt_disable = true;
	io_write_word(address, PCI_COMMAND, comm.value);
}

void enable_bus_mastering(uint64_t address) {
	Command comm = {.value = io_read_word(address, PCI_COMMAND)};
	comm.attrs.bus_master = true;
	io_write_word(address, PCI_COMMAND, comm.value);
}

void disable_bus_mastering(uint64_t address) {
	Command comm = {.value = io_read_word(address, PCI_COMMAND)};
	comm.attrs.bus_master = false;
	io_write_word(address, PCI_COMMAND, comm.value);
}