include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(Framework4PIC32_2026_default_library_list )

set(OUTPUT_SUFFIX elf)
# Handle files with suffix s, for group default-XC32
if(Framework4PIC32_2026_default_default_XC32_FILE_TYPE_assemble)
add_library(Framework4PIC32_2026_default_default_XC32_assemble OBJECT ${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_assemble})
    Framework4PIC32_2026_default_default_XC32_assemble_rule(Framework4PIC32_2026_default_default_XC32_assemble)
    list(APPEND Framework4PIC32_2026_default_library_list "$<TARGET_OBJECTS:Framework4PIC32_2026_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(Framework4PIC32_2026_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(Framework4PIC32_2026_default_default_XC32_assembleWithPreprocess OBJECT ${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    Framework4PIC32_2026_default_default_XC32_assembleWithPreprocess_rule(Framework4PIC32_2026_default_default_XC32_assembleWithPreprocess)
    list(APPEND Framework4PIC32_2026_default_library_list "$<TARGET_OBJECTS:Framework4PIC32_2026_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(Framework4PIC32_2026_default_default_XC32_FILE_TYPE_compile)
add_library(Framework4PIC32_2026_default_default_XC32_compile OBJECT ${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_compile})
    Framework4PIC32_2026_default_default_XC32_compile_rule(Framework4PIC32_2026_default_default_XC32_compile)
    list(APPEND Framework4PIC32_2026_default_library_list "$<TARGET_OBJECTS:Framework4PIC32_2026_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(Framework4PIC32_2026_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(Framework4PIC32_2026_default_default_XC32_compile_cpp OBJECT ${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_compile_cpp})
    Framework4PIC32_2026_default_default_XC32_compile_cpp_rule(Framework4PIC32_2026_default_default_XC32_compile_cpp)
    list(APPEND Framework4PIC32_2026_default_library_list "$<TARGET_OBJECTS:Framework4PIC32_2026_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(Framework4PIC32_2026_default_default_XC32_FILE_TYPE_dependentObject)
add_library(Framework4PIC32_2026_default_default_XC32_dependentObject OBJECT ${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_dependentObject})
    Framework4PIC32_2026_default_default_XC32_dependentObject_rule(Framework4PIC32_2026_default_default_XC32_dependentObject)
    list(APPEND Framework4PIC32_2026_default_library_list "$<TARGET_OBJECTS:Framework4PIC32_2026_default_default_XC32_dependentObject>")

endif()


# Main target for this project
add_executable(Framework4PIC32_2026_default_image_A_B1raXS ${Framework4PIC32_2026_default_library_list})

set_target_properties(Framework4PIC32_2026_default_image_A_B1raXS PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${Framework4PIC32_2026_default_output_dir}")
target_link_libraries(Framework4PIC32_2026_default_image_A_B1raXS PRIVATE ${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_link})
# Add the link options from the rule file.
Framework4PIC32_2026_default_link_rule( Framework4PIC32_2026_default_image_A_B1raXS)

# Add bin2hex target for converting built file to a .hex file.
string(REGEX REPLACE [.]elf$ .hex Framework4PIC32_2026_default_image_name_hex ${Framework4PIC32_2026_default_image_name})
add_custom_target(Framework4PIC32_2026_default_Bin2Hex ALL
    COMMAND ${MP_BIN2HEX} \"${Framework4PIC32_2026_default_output_dir}/${Framework4PIC32_2026_default_image_name}\"
    DEPENDS "${Framework4PIC32_2026_default_output_dir}/${Framework4PIC32_2026_default_image_name}"
    BYPRODUCTS ${Framework4PIC32_2026_default_output_dir}/${Framework4PIC32_2026_default_image_name_hex}
    COMMENT "Convert built file to .hex")
add_dependencies(Framework4PIC32_2026_default_Bin2Hex Framework4PIC32_2026_default_image_A_B1raXS)




