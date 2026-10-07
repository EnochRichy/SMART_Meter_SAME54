include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(SAME54_OV7670_default_library_list )

# Handle files with suffix s, for group default-XC32
if(SAME54_OV7670_default_default_XC32_FILE_TYPE_assemble)
add_library(SAME54_OV7670_default_default_XC32_assemble OBJECT ${SAME54_OV7670_default_default_XC32_FILE_TYPE_assemble})
    SAME54_OV7670_default_default_XC32_assemble_rule(SAME54_OV7670_default_default_XC32_assemble)
    list(APPEND SAME54_OV7670_default_library_list "$<TARGET_OBJECTS:SAME54_OV7670_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(SAME54_OV7670_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(SAME54_OV7670_default_default_XC32_assembleWithPreprocess OBJECT ${SAME54_OV7670_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    SAME54_OV7670_default_default_XC32_assembleWithPreprocess_rule(SAME54_OV7670_default_default_XC32_assembleWithPreprocess)
    list(APPEND SAME54_OV7670_default_library_list "$<TARGET_OBJECTS:SAME54_OV7670_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(SAME54_OV7670_default_default_XC32_FILE_TYPE_compile)
add_library(SAME54_OV7670_default_default_XC32_compile OBJECT ${SAME54_OV7670_default_default_XC32_FILE_TYPE_compile})
    SAME54_OV7670_default_default_XC32_compile_rule(SAME54_OV7670_default_default_XC32_compile)
    list(APPEND SAME54_OV7670_default_library_list "$<TARGET_OBJECTS:SAME54_OV7670_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(SAME54_OV7670_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(SAME54_OV7670_default_default_XC32_compile_cpp OBJECT ${SAME54_OV7670_default_default_XC32_FILE_TYPE_compile_cpp})
    SAME54_OV7670_default_default_XC32_compile_cpp_rule(SAME54_OV7670_default_default_XC32_compile_cpp)
    list(APPEND SAME54_OV7670_default_library_list "$<TARGET_OBJECTS:SAME54_OV7670_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(SAME54_OV7670_default_default_XC32_FILE_TYPE_dependentObject)
add_library(SAME54_OV7670_default_default_XC32_dependentObject OBJECT ${SAME54_OV7670_default_default_XC32_FILE_TYPE_dependentObject})
    SAME54_OV7670_default_default_XC32_dependentObject_rule(SAME54_OV7670_default_default_XC32_dependentObject)
    list(APPEND SAME54_OV7670_default_library_list "$<TARGET_OBJECTS:SAME54_OV7670_default_default_XC32_dependentObject>")

endif()


# Main target for this project
add_executable(SAME54_OV7670_default_image_wav9Qc_7 ${SAME54_OV7670_default_library_list})

set_target_properties(SAME54_OV7670_default_image_wav9Qc_7 PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${SAME54_OV7670_default_output_dir}")
target_link_libraries(SAME54_OV7670_default_image_wav9Qc_7 PRIVATE ${SAME54_OV7670_default_default_XC32_FILE_TYPE_link})
# Add the link options from the rule file.
SAME54_OV7670_default_link_rule( SAME54_OV7670_default_image_wav9Qc_7)

# Add bin2hex target for converting built file to a .hex file.
string(REGEX REPLACE [.]elf$ .hex SAME54_OV7670_default_image_name_hex ${SAME54_OV7670_default_image_name})
add_custom_target(SAME54_OV7670_default_Bin2Hex ALL
    COMMAND ${MP_BIN2HEX} \"${SAME54_OV7670_default_output_dir}/${SAME54_OV7670_default_image_name}\"
    BYPRODUCTS ${SAME54_OV7670_default_output_dir}/${SAME54_OV7670_default_image_name_hex}
    COMMENT "Convert built file to .hex")
add_dependencies(SAME54_OV7670_default_Bin2Hex SAME54_OV7670_default_image_wav9Qc_7)




