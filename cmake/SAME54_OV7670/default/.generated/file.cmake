# The following variables contains the files used by the different stages of the build process.
set(SAME54_OV7670_default_default_XC32_FILE_TYPE_assemble)
set_source_files_properties(${SAME54_OV7670_default_default_XC32_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${SAME54_OV7670_default_default_XC32_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(SAME54_OV7670_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
set_source_files_properties(${SAME54_OV7670_default_default_XC32_FILE_TYPE_assembleWithPreprocess} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${SAME54_OV7670_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(SAME54_OV7670_default_default_XC32_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/app.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/camera_capture.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/camera_hal_same54.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/driver/usb/usbfsv1/src/drv_usbfsv1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/driver/usb/usbfsv1/src/drv_usbfsv1_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/exceptions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/initialization.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/interrupts.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/libc_syscalls.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/clock/plib_clock.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/cmcc/plib_cmcc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/dmac/plib_dmac.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/eic/plib_eic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/evsys/plib_evsys.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/nvic/plib_nvic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/nvmctrl/plib_nvmctrl.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/port/plib_port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/sercom/i2c_master/plib_sercom3_i2c_master.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/sercom/usart/plib_sercom2_usart.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/peripheral/tc/plib_tc0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/startup_xc32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/stdio/xc32_monitor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/system/cache/sys_cache.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/system/int/src/sys_int.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/tasks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/usb/src/usb_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/usb/src/usb_device_cdc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/usb/src/usb_device_cdc_acm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/usb/src/usb_device_endpoint_functions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/usb_device_init_data.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/img_preprocess.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/ov7670.c")
set_source_files_properties(${SAME54_OV7670_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(SAME54_OV7670_default_default_XC32_FILE_TYPE_compile_cpp "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/app_ml.cpp")
set_source_files_properties(${SAME54_OV7670_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(SAME54_OV7670_default_default_XC32_FILE_TYPE_link)

# The linker script used for the build.
set(SAME54_OV7670_default_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/config/default/ATSAME54P20A.ld")
set(SAME54_OV7670_default_image_name "default.elf")
set(SAME54_OV7670_default_image_base_name "default")

# The output directory of the final image.
set(SAME54_OV7670_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/SAME54_OV7670")

# The full path to the final image.
set(SAME54_OV7670_default_full_path_to_image ${SAME54_OV7670_default_output_dir}/${SAME54_OV7670_default_image_name})
