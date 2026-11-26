cmake_minimum_required(VERSION 3.22)

# Runtime estático en Windows: el .vst3 no necesita instalar el "Visual C++ Redistributable"
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# macOS: binario universal (Intel + Apple Silicon)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

project(EXPANSOR VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.12
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(EXPANSOR
    COMPANY_NAME            "LARGARTIXA SOUNDLAB"
    PLUGIN_MANUFACTURER_CODE Lgsl
    PLUGIN_CODE             Lex1
    FORMATS                 VST3 Standalone
    PRODUCT_NAME            "LARGARTIXA SOUNDLAB EXPANSOR"
    VST3_CATEGORIES         Fx Spatial
    IS_SYNTH                FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

# Los archivos de código están sueltos en esta misma carpeta
target_sources(EXPANSOR PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp)

target_compile_definitions(EXPANSOR PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_compile_options(EXPANSOR PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

target_link_libraries(EXPANSOR
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
