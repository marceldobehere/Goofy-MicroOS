use core::ffi::{c_char};

use crate::utils::ptr::CPtr;

pub mod clock_driver;
pub mod char_output_driver;
pub mod nic_driver;
pub mod pci;

extern {
	pub fn register_driver(driver: *mut Driver);
}

type DriverIsDevicePresend = extern fn(driver: *mut Driver) -> bool;
type DriverGetDeviceName = extern fn(driver: *mut Driver) -> *const c_char;
type DriverInit = extern fn(driver: *mut Driver);

#[repr(C)]
pub struct Driver {
	pub is_device_present: DriverIsDevicePresend,
	pub get_device_name: DriverGetDeviceName,
	pub init: DriverInit,
	pub driver_specific_data: CPtr
}