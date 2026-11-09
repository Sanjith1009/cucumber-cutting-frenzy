set(DEPENDENT_MP_BIN2HEXFramework4PIC32_2026_default_wE7bqttB "c:/Program Files/Microchip/xc32/v6.00/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_wE7bqttB ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Framework4PIC32_2026/production/default-production.elf)
set(DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_wE7bqttB ${CMAKE_CURRENT_LIST_DIR}/../../../../out/Framework4PIC32_2026/production)
set(DEPENDENT_BYPRODUCTSFramework4PIC32_2026_default_wE7bqttB ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_wE7bqttB}/${sourceFileNameFramework4PIC32_2026_default_wE7bqttB}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_wE7bqttB}/${sourceFileNameFramework4PIC32_2026_default_wE7bqttB}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXFramework4PIC32_2026_default_wE7bqttB} --image ${DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_wE7bqttB} --image-generated-c ${sourceFileNameFramework4PIC32_2026_default_wE7bqttB}.c --image-generated-h ${sourceFileNameFramework4PIC32_2026_default_wE7bqttB}.h --image-copy-mode ${modeFramework4PIC32_2026_default_wE7bqttB} --image-offset ${addressFramework4PIC32_2026_default_wE7bqttB} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_wE7bqttB}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFFramework4PIC32_2026_default_wE7bqttB})
add_custom_target(
    dependent_produced_source_artifactFramework4PIC32_2026_default_wE7bqttB 
    DEPENDS ${DEPENDENT_TARGET_DIRFramework4PIC32_2026_default_wE7bqttB}/${sourceFileNameFramework4PIC32_2026_default_wE7bqttB}.c
    )
