cmake_minimum_required(VERSION 3.22)
project(Tamura VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE is added as a subdirectory. Clone it alongside this project or set JUCE_DIR.
# git clone https://github.com/juce-framework/JUCE.git
if(DEFINED ENV{JUCE_DIR})
    set(JUCE_DIR $ENV{JUCE_DIR})
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
else()
    message(FATAL_ERROR
        "JUCE not found. Either:\n"
        "  1. Clone JUCE next to this project: git clone https://github.com/juce-framework/JUCE.git ../JUCE\n"
        "  2. Set JUCE_DIR environment variable: export JUCE_DIR=/path/to/JUCE\n"
        "  3. Clone JUCE inside this project: git clone https://github.com/juce-framework/JUCE.git JUCE")
endif()

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(Tamura
    PRODUCT_NAME "Tamura"
    COMPANY_NAME "TamuraAudio"
    BUNDLE_ID "com.tamuraaudio.tamura"
    PLUGIN_MANUFACTURER_CODE "Tamu"
    PLUGIN_CODE "Tama"
    FORMATS VST3 AU Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(Tamura
    PRIVATE
        NewProject/Source/PluginProcessor.cpp
        NewProject/Source/PluginEditor.cpp
        NewProject/Source/SpinEngine.cpp
        NewProject/Source/SamplePlayer.cpp
        NewProject/Source/TokenManager.cpp
        NewProject/Source/SoundLibrary.cpp
        NewProject/Source/UI/TamuraLookAndFeel.cpp
        NewProject/Source/UI/SlotReelComponent.cpp
        NewProject/Source/UI/SpinButton.cpp
        NewProject/Source/UI/ControlPanel.cpp
        NewProject/Source/UI/WaveformDisplay.cpp
        NewProject/Source/UI/NeonLabel.cpp
)

target_include_directories(Tamura
    PRIVATE
        NewProject/Source
)

target_compile_definitions(Tamura
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Tamura
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
