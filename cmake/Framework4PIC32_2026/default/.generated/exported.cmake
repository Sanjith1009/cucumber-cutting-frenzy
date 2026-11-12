set(DEPENDENT_MP_BIN2HEXFramework4PIC32_2026_default_A_B1raXS "c:/Program Files/Microchip/xc32/v6.00/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_A_B1raXS ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Framework4PIC32_2026/default.elf)
set(DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_A_B1raXS ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Framework4PIC32_2026)
set(DEPENDENT_BYPRODUCTSFramework4PIC32_2026_default_A_B1raXS ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_A_B1raXS}/${sourceFileNameFramework4PIC32_2026_default_A_B1raXS}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_A_B1raXS}/${sourceFileNameFramework4PIC32_2026_default_A_B1raXS}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXFramework4PIC32_2026_default_A_B1raXS} --image ${DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_A_B1raXS} --image-generated-c ${sourceFileNameFramework4PIC32_2026_default_A_B1raXS}.c --image-generated-h ${sourceFileNameFramework4PIC32_2026_default_A_B1raXS}.h --image-copy-mode ${modeFramework4PIC32_2026_default_A_B1raXS} --image-offset ${addressFramework4PIC32_2026_default_A_B1raXS} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_A_B1raXS}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_A_B1raXS})
add_custom_target(
    dependent_produced_source_artifactFramework4PIC32_2026_default_A_B1raXS 
    DEPENDS ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_A_B1raXS}/${sourceFileNameFramework4PIC32_2026_default_A_B1raXS}.c
    )
