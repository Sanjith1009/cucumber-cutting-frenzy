include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(Me218aLab3Part3_default_library_list )

set(OUTPUT_SUFFIX elf)
# Handle files with suffix s, for group default-XC32
if(Me218aLab3Part3_default_default_XC32_FILE_TYPE_assemble)
add_library(Me218aLab3Part3_default_default_XC32_assemble OBJECT ${Me218aLab3Part3_default_default_XC32_FILE_TYPE_assemble})
    Me218aLab3Part3_default_default_XC32_assemble_rule(Me218aLab3Part3_default_default_XC32_assemble)
    list(APPEND Me218aLab3Part3_default_library_list "$<TARGET_OBJECTS:Me218aLab3Part3_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(Me218aLab3Part3_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(Me218aLab3Part3_default_default_XC32_assembleWithPreprocess OBJECT ${Me218aLab3Part3_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    Me218aLab3Part3_default_default_XC32_assembleWithPreprocess_rule(Me218aLab3Part3_default_default_XC32_assembleWithPreprocess)
    list(APPEND Me218aLab3Part3_default_library_list "$<TARGET_OBJECTS:Me218aLab3Part3_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(Me218aLab3Part3_default_default_XC32_FILE_TYPE_compile)
add_library(Me218aLab3Part3_default_default_XC32_compile OBJECT ${Me218aLab3Part3_default_default_XC32_FILE_TYPE_compile})
    Me218aLab3Part3_default_default_XC32_compile_rule(Me218aLab3Part3_default_default_XC32_compile)
    list(APPEND Me218aLab3Part3_default_library_list "$<TARGET_OBJECTS:Me218aLab3Part3_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(Me218aLab3Part3_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(Me218aLab3Part3_default_default_XC32_compile_cpp OBJECT ${Me218aLab3Part3_default_default_XC32_FILE_TYPE_compile_cpp})
    Me218aLab3Part3_default_default_XC32_compile_cpp_rule(Me218aLab3Part3_default_default_XC32_compile_cpp)
    list(APPEND Me218aLab3Part3_default_library_list "$<TARGET_OBJECTS:Me218aLab3Part3_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(Me218aLab3Part3_default_default_XC32_FILE_TYPE_dependentObject)
add_library(Me218aLab3Part3_default_default_XC32_dependentObject OBJECT ${Me218aLab3Part3_default_default_XC32_FILE_TYPE_dependentObject})
    Me218aLab3Part3_default_default_XC32_dependentObject_rule(Me218aLab3Part3_default_default_XC32_dependentObject)
    list(APPEND Me218aLab3Part3_default_library_list "$<TARGET_OBJECTS:Me218aLab3Part3_default_default_XC32_dependentObject>")

endif()


# Main target for this project
add_executable(Me218aLab3Part3_default_image_1Dj29I66 ${Me218aLab3Part3_default_library_list})

set_target_properties(Me218aLab3Part3_default_image_1Dj29I66 PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${Me218aLab3Part3_default_output_dir}")
target_link_libraries(Me218aLab3Part3_default_image_1Dj29I66 PRIVATE ${Me218aLab3Part3_default_default_XC32_FILE_TYPE_link})
# Add the link options from the rule file.
Me218aLab3Part3_default_link_rule( Me218aLab3Part3_default_image_1Dj29I66)

# Add bin2hex target for converting built file to a .hex file.
string(REGEX REPLACE [.]elf$ .hex Me218aLab3Part3_default_image_name_hex ${Me218aLab3Part3_default_image_name})
add_custom_target(Me218aLab3Part3_default_Bin2Hex ALL
    COMMAND ${MP_BIN2HEX} \"${Me218aLab3Part3_default_output_dir}/${Me218aLab3Part3_default_image_name}\"
    DEPENDS "${Me218aLab3Part3_default_output_dir}/${Me218aLab3Part3_default_image_name}"
    BYPRODUCTS ${Me218aLab3Part3_default_output_dir}/${Me218aLab3Part3_default_image_name_hex}
    COMMENT "Convert built file to .hex")
add_dependencies(Me218aLab3Part3_default_Bin2Hex Me218aLab3Part3_default_image_1Dj29I66)




