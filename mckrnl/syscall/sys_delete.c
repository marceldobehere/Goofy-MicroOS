#include <syscall/syscalls.h>

#include <fs/vfs.h>
#include <fs/fd.h>

cpu_registers_t* sys_delete(cpu_registers_t* regs) {
	int fd = regs->ebx;

	file_t* file = fd_to_file(fd);
	vfs_delete(file);

	return regs;
}