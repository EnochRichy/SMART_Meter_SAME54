set(DEPENDENT_MP_BIN2HEXSAME54_OV7670_default_wav9Qc_7 "c:/Program Files/Microchip/xc32/v5.10/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFSAME54_OV7670_default_wav9Qc_7 ${CMAKE_CURRENT_LIST_DIR}/../../../../out/SAME54_OV7670/default.elf)
set(DEPENDENT_TARGET_DIRSAME54_OV7670_default_wav9Qc_7 ${CMAKE_CURRENT_LIST_DIR}/../../../../out/SAME54_OV7670)
set(DEPENDENT_BYPRODUCTSSAME54_OV7670_default_wav9Qc_7 ${DEPENDENT_TARGET_DIRSAME54_OV7670_default_wav9Qc_7}/${sourceFileNameSAME54_OV7670_default_wav9Qc_7}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRSAME54_OV7670_default_wav9Qc_7}/${sourceFileNameSAME54_OV7670_default_wav9Qc_7}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXSAME54_OV7670_default_wav9Qc_7} --image ${DEPENDENT_DEPENDENT_TARGET_ELFSAME54_OV7670_default_wav9Qc_7} --image-generated-c ${sourceFileNameSAME54_OV7670_default_wav9Qc_7}.c --image-generated-h ${sourceFileNameSAME54_OV7670_default_wav9Qc_7}.h --image-copy-mode ${modeSAME54_OV7670_default_wav9Qc_7} --image-offset ${addressSAME54_OV7670_default_wav9Qc_7} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRSAME54_OV7670_default_wav9Qc_7}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFSAME54_OV7670_default_wav9Qc_7})
add_custom_target(
    dependent_produced_source_artifactSAME54_OV7670_default_wav9Qc_7 
    DEPENDS ${DEPENDENT_TARGET_DIRSAME54_OV7670_default_wav9Qc_7}/${sourceFileNameSAME54_OV7670_default_wav9Qc_7}.c
    )
