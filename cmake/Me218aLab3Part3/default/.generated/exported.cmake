set(DEPENDENT_MP_BIN2HEXMe218aLab3Part3_default_1Dj29I66 "c:/Program Files/Microchip/xc32/v6.00/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFMe218aLab3Part3_default_1Dj29I66 ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Me218aLab3Part3/default.elf)
set(DEPENDENT_TARGET_DIRMe218aLab3Part3_default_1Dj29I66 ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Me218aLab3Part3)
set(DEPENDENT_BYPRODUCTSMe218aLab3Part3_default_1Dj29I66 ${DEPENDENT_TARGET_DIRMe218aLab3Part3_default_1Dj29I66}/${sourceFileNameMe218aLab3Part3_default_1Dj29I66}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRMe218aLab3Part3_default_1Dj29I66}/${sourceFileNameMe218aLab3Part3_default_1Dj29I66}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXMe218aLab3Part3_default_1Dj29I66} --image ${DEPENDENT_DEPENDENT_TARGET_ELFMe218aLab3Part3_default_1Dj29I66} --image-generated-c ${sourceFileNameMe218aLab3Part3_default_1Dj29I66}.c --image-generated-h ${sourceFileNameMe218aLab3Part3_default_1Dj29I66}.h --image-copy-mode ${modeMe218aLab3Part3_default_1Dj29I66} --image-offset ${addressMe218aLab3Part3_default_1Dj29I66} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRMe218aLab3Part3_default_1Dj29I66}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFMe218aLab3Part3_default_1Dj29I66})
add_custom_target(
    dependent_produced_source_artifactMe218aLab3Part3_default_1Dj29I66 
    DEPENDS ${DEPENDENT_TARGET_DIRMe218aLab3Part3_default_1Dj29I66}/${sourceFileNameMe218aLab3Part3_default_1Dj29I66}.c
    )
