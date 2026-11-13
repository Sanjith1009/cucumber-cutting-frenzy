set(DEPENDENT_MP_BIN2HEXFramework4PIC32_2026_default_xymPBhYZ "c:/Program Files/Microchip/xc32/v6.00/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_xymPBhYZ ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Framework4PIC32_2026/production/default-production.elf)
set(DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_xymPBhYZ ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Framework4PIC32_2026/production)
set(DEPENDENT_BYPRODUCTSFramework4PIC32_2026_default_xymPBhYZ ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_xymPBhYZ}/${sourceFileNameFramework4PIC32_2026_default_xymPBhYZ}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_xymPBhYZ}/${sourceFileNameFramework4PIC32_2026_default_xymPBhYZ}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXFramework4PIC32_2026_default_xymPBhYZ} --image ${DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_xymPBhYZ} --image-generated-c ${sourceFileNameFramework4PIC32_2026_default_xymPBhYZ}.c --image-generated-h ${sourceFileNameFramework4PIC32_2026_default_xymPBhYZ}.h --image-copy-mode ${modeFramework4PIC32_2026_default_xymPBhYZ} --image-offset ${addressFramework4PIC32_2026_default_xymPBhYZ} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_xymPBhYZ}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_xymPBhYZ})
add_custom_target(
    dependent_produced_source_artifactFramework4PIC32_2026_default_xymPBhYZ 
    DEPENDS ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_xymPBhYZ}/${sourceFileNameFramework4PIC32_2026_default_xymPBhYZ}.c
    )
