set(DEPENDENT_MP_BIN2HEXFramework4PIC32_2026_default_H3ChhtIE "c:/Program Files/Microchip/xc32/v6.00/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_H3ChhtIE ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Framework4PIC32_2026/default.elf)
set(DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_H3ChhtIE ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Framework4PIC32_2026)
set(DEPENDENT_BYPRODUCTSFramework4PIC32_2026_default_H3ChhtIE ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_H3ChhtIE}/${sourceFileNameFramework4PIC32_2026_default_H3ChhtIE}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_H3ChhtIE}/${sourceFileNameFramework4PIC32_2026_default_H3ChhtIE}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXFramework4PIC32_2026_default_H3ChhtIE} --image ${DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_H3ChhtIE} --image-generated-c ${sourceFileNameFramework4PIC32_2026_default_H3ChhtIE}.c --image-generated-h ${sourceFileNameFramework4PIC32_2026_default_H3ChhtIE}.h --image-copy-mode ${modeFramework4PIC32_2026_default_H3ChhtIE} --image-offset ${addressFramework4PIC32_2026_default_H3ChhtIE} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_H3ChhtIE}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_H3ChhtIE})
add_custom_target(
    dependent_produced_source_artifactFramework4PIC32_2026_default_H3ChhtIE 
    DEPENDS ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_H3ChhtIE}/${sourceFileNameFramework4PIC32_2026_default_H3ChhtIE}.c
    )
