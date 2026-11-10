# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "C:/Users/harmo/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(Framework4PIC32_2026_default_default_XC32_assemble_rule target)
    set(options
        "-g"
        "${ASSEMBLER_PRE}"
        "-mprocessor=32MX170F256B"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1"
        "-g,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,-Ic:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/FrameworkHeaders"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32MX_DFP/1.7.380")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG=1"
        PRIVATE "__MPLAB_DEBUGGER_SIMULATOR=1")
    target_include_directories(${target}
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/FrameworkSource"
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/ProjectHeaders"
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/ProjectSource")
endfunction()
function(Framework4PIC32_2026_default_default_XC32_assembleWithPreprocess_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "${MP_EXTRA_AS_PRE}"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32MX_DFP/1.7.380"
        "-mprocessor=32MX170F256B"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "__MPLAB_DEBUGGER_SIMULATOR=1"
        PRIVATE "XPRJ_default=default")
endfunction()
function(Framework4PIC32_2026_default_default_XC32_compile_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-x"
        "c"
        "-c"
        "-mprocessor=32MX170F256B"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32MX_DFP/1.7.380")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "__MPLAB_DEBUGGER_SIMULATOR=1"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/FrameworkHeaders"
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/FrameworkSource"
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/ProjectHeaders"
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/ProjectSource")
endfunction()
function(Framework4PIC32_2026_default_default_XC32_compile_cpp_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-mprocessor=32MX170F256B"
        "-frtti"
        "-fexceptions"
        "-fno-check-new"
        "-fenforce-eh-specs"
        "-fno-common"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32MX_DFP/1.7.380")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "__MPLAB_DEBUGGER_SIMULATOR=1"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/FrameworkHeaders"
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/FrameworkSource"
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/ProjectHeaders"
        PRIVATE "c:/Users/harmo/MPLABProjects/Framework4PIC32_2026_Lab3/ProjectSource")
endfunction()
function(Framework4PIC32_2026_default_dependentObject_rule target)
    set(options
        "-mprocessor=32MX170F256B"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32MX_DFP/1.7.380")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
endfunction()
function(Framework4PIC32_2026_default_link_rule target)
    set(options
        "-g"
        "${MP_EXTRA_LD_PRE}"
        "-mdebugger"
        "-mprocessor=32MX170F256B"
        "-Wl,--defsym=__MPLAB_BUILD=1${MP_EXTRA_LD_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map=mem.map,--report-mem,--memorysummary,memoryfile.xml"
        "-mdfp=${PACK_REPO_PATH}/Microchip/PIC32MX_DFP/1.7.380")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__MPLAB_DEBUGGER_SIMULATOR=1"
        PRIVATE "XPRJ_default=default")
endfunction()
