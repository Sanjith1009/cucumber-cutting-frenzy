set(DEPENDENT_MP_BIN2HEXMe218aLab3Part3_default_UzTEwlhr "c:/Program Files/Microchip/xc32/v6.00/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFMe218aLab3Part3_default_UzTEwlhr ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Me218aLab3Part3/default.elf)
set(DEPENDENT_TARGET_DIRMe218aLab3Part3_default_UzTEwlhr ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Me218aLab3Part3)
set(DEPENDENT_BYPRODUCTSMe218aLab3Part3_default_UzTEwlhr ${DEPENDENT_TARGET_DIRMe218aLab3Part3_default_UzTEwlhr}/${sourceFileNameMe218aLab3Part3_default_UzTEwlhr}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRMe218aLab3Part3_default_UzTEwlhr}/${sourceFileNameMe218aLab3Part3_default_UzTEwlhr}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXMe218aLab3Part3_default_UzTEwlhr} --image ${DEPENDENT_DEPENDENT_TARGET_ELFMe218aLab3Part3_default_UzTEwlhr} --image-generated-c ${sourceFileNameMe218aLab3Part3_default_UzTEwlhr}.c --image-generated-h ${sourceFileNameMe218aLab3Part3_default_UzTEwlhr}.h --image-copy-mode ${modeMe218aLab3Part3_default_UzTEwlhr} --image-offset ${addressMe218aLab3Part3_default_UzTEwlhr} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRMe218aLab3Part3_default_UzTEwlhr}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFMe218aLab3Part3_default_UzTEwlhr})
add_custom_target(
    dependent_produced_source_artifactMe218aLab3Part3_default_UzTEwlhr 
    DEPENDS ${DEPENDENT_TARGET_DIRMe218aLab3Part3_default_UzTEwlhr}/${sourceFileNameMe218aLab3Part3_default_UzTEwlhr}.c
    )
