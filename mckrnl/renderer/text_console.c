#include <renderer/text_console.h>

#include <utils/io.h>
#include <stdio.h>

int text_console_x = 0;
int text_console_y = 0;

char* text_console_video = (char*) VIDEO_MEM;
uint32_t text_console_color = BACKGROUND_BLACK | FOREGROUND_WHITE;

void text_console_setcursor(uint16_t pos) {
	outb(0x3d4, 14);
	outb(0x3d5, pos >> 8);
	outb(0x3d4, 15);
	outb(0x3d5, pos);
}

void text_console_putc(char c){
	if ((c == '\n') || (text_console_x > SCREEN_WIDTH - 1)) {
		text_console_x = 0;
		text_console_y++;
	}
	
	//if(SERIAL_DEBUG) write_serial(c);

	// if (c == '\n') {
	//	if(SERIAL_DEBUG) write_serial('\r');
		// return;
	// }

	if (c == '\r') {
		text_console_x = 0;
		return;
	}

	if (c == '\b') {
		if (text_console_x > 0) {
			text_console_x--;
		}

		text_console_video[2 * (text_console_y * SCREEN_WIDTH + text_console_x)] = ' ';
		text_console_video[2 * (text_console_y * SCREEN_WIDTH + text_console_x) + 1] = text_console_color;
		text_console_setcursor(text_console_y * SCREEN_WIDTH + text_console_x);
		return;
	}

	if (text_console_y > SCREEN_HEIGHT - 1) {
		int i;
		for (i = 0; i < 2 * (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i++) {
			text_console_video[i] = text_console_video[i + SCREEN_WIDTH * 2];
		}

		for (; i < 2 * SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
			text_console_video[i] = 0;
		}
		text_console_y--;
	}

	if (c == '\n') {
		return;
	}

	text_console_video[2 * (text_console_y * SCREEN_WIDTH + text_console_x)] = c;
	text_console_video[2 * (text_console_y * SCREEN_WIDTH + text_console_x) + 1] = text_console_color;

	text_console_x++;

	text_console_setcursor(text_console_y * SCREEN_WIDTH + text_console_x);
}

void text_console_puts(const char *s){
	while(*s){
		text_console_putc(*s++);
	}
}

void text_console_clrscr(){
	int i;
	for (i = 0; i < 25 * 80; i++) {
		text_console_video[2*i] = ' ';
		text_console_video[2*i+1] = BACKGROUND_BLACK | FOREGROUND_WHITE;
	}

	text_console_x = text_console_y = 0;
}

bool text_console_driver_is_present(driver_t* driver) {
	return true;
}

char* text_console_driver_get_device_name(driver_t* driver) {
	return "text_console";
}

void text_console_driver_init(driver_t* driver) {
	text_console_clrscr();

	printf_driver = (char_output_driver_t*) driver;
}

void text_console_driver_putc(char_output_driver_t* driver, char c) {
	text_console_putc(c);
}

char_output_driver_t text_console_driver = {
	.driver = {
		.is_device_present = text_console_driver_is_present,
		.get_device_name = text_console_driver_get_device_name,
		.init = text_console_driver_init
	},
	.putc = text_console_driver_putc
};