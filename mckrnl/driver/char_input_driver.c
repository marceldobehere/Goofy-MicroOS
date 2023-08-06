#include <driver/char_input_driver.h>

#include <fs/vfs.h>
#include <stdio.h>
#include <utils/string.h>
#include <memory/vmm.h>
#include <assert.h>

bool default_keyboard_is_device_present(driver_t* driver) {
	return true;
}

char* default_keyboard_get_device_name(driver_t* driver) {
	return "default_keyboard";
}

void default_keyboard_init(driver_t* driver) {}



char default_keyboard_async_getc(char_input_driver_t* driver) {
	return 0;
}

char default_keyboard_async_getarrw(char_input_driver_t* driver) {
	return 0;
}

char_input_driver_t default_char_input = {
    .driver = {
        .is_device_present = default_keyboard_is_device_present,
        .get_device_name = default_keyboard_get_device_name,
        .init = default_keyboard_init
    },
    .async_getc = default_keyboard_async_getc,
    .async_getarrw = default_keyboard_async_getarrw
};

char_input_driver_t* global_char_input_driver = &default_char_input;

keymap_file_header_t* loaded_keymap = NULL;
int current_keymap_idx = 0;

void init_keymap(char* path) {
    debugf("Loading keymap from %s", path);

    file_t* file = vfs_open(path, FILE_OPEN_MODE_READ);
    loaded_keymap = vmm_alloc(TO_PAGES(file->size));
    vfs_read(file, loaded_keymap, file->size, 0);
    vfs_close(file);

    assert(loaded_keymap->magic == 0xab1589fd);
    assert(loaded_keymap->num_keymaps > 0);

    keymap_t* default_keymap = (keymap_t*) &loaded_keymap[1];
    debugf("Default keymap is %s", default_keymap->name);
}

char keymap(uint8_t key, special_keys_down_t* special_keys_down) {
    if (loaded_keymap == NULL) {
        return 0;
    }

	bool shift = special_keys_down->left_shift || special_keys_down->right_shift;
	bool alt = special_keys_down->left_alt || special_keys_down->right_alt;

    keymap_t* current = &((keymap_t*) &loaded_keymap[1])[current_keymap_idx];

	if (special_keys_down->caps_lock) {
		if (shift) {
			return current->layout_normal[key];
		} else {
			return current->layout_shift[key];
		}
	} else if (shift) {
		return current->layout_shift[key];
	} else if (alt) {
		return current->layout_alt[key];
	} else {
		return current->layout_normal[key];
	}
}