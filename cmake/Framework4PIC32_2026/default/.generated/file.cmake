# The following variables contains the files used by the different stages of the build process.
set(Framework4PIC32_2026_default_default_XC32_FILE_TYPE_assemble)
set_source_files_properties(${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(Framework4PIC32_2026_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
set_source_files_properties(${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_assembleWithPreprocess} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(Framework4PIC32_2026_default_default_XC32_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/ES_CheckEvents.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/ES_DeferRecall.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/ES_Framework.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/ES_LookupTables.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/ES_Port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/ES_PostList.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/ES_Queue.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/ES_Timers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/circular_buffer_no_modulo_threadsafe.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/dbprintf.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../FrameworkSource/terminal.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/DM_DisplayStarter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/DeadmanHandling.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/EventCheckers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/FontStuff.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/LEDService.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/MessageService.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/PIC32_SPI_HAL_Starter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/PWM_PIC32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/PlayService.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/Service1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/SetupService.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/TestHarnessService0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../ProjectSource/main.c")
set_source_files_properties(${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(Framework4PIC32_2026_default_default_XC32_FILE_TYPE_compile_cpp)
set_source_files_properties(${Framework4PIC32_2026_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(Framework4PIC32_2026_default_default_XC32_FILE_TYPE_link)
set(Framework4PIC32_2026_default_image_name "default.elf")
set(Framework4PIC32_2026_default_image_base_name "default")

# The output directory of the final image.
set(Framework4PIC32_2026_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/Framework4PIC32_2026")

# The full path to the final image.
set(Framework4PIC32_2026_default_full_path_to_image ${Framework4PIC32_2026_default_output_dir}/${Framework4PIC32_2026_default_image_name})
