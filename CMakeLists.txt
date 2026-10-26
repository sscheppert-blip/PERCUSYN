cmake_minimum_required(VERSION 3.22)

project(Percusyn VERSION 0.1.0 LANGUAGES C CXX)

option(PERCUSYN_BUILD_TESTS "Build the offline tests" ON)
option(PERCUSYN_COPY_PLUGIN "Copy the built plugins into the user's plugin folders" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")

# Dependencies are fetched into the build tree, never committed (pins: docs/11_DECISIONS.md).
include(FetchContent)

FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.3
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL)

FetchContent_Declare(clap_juce_extensions
    GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
    GIT_TAG        7adee3a1bd4684d4caa5601100e364abccff4b4d
    EXCLUDE_FROM_ALL)

FetchContent_MakeAvailable(JUCE clap_juce_extensions)

# ---------------------------------------------------------------------------
# Engine (no JUCE): voices, metal stages, FX, mixer and sequencer clock
# ---------------------------------------------------------------------------
add_library(PercusynEngine STATIC
    Source/dsp/Voice.cpp
    Source/dsp/Fx.cpp
    Source/dsp/Engine.cpp
    Source/dsp/Sequencer.cpp)
target_include_directories(PercusynEngine PUBLIC Source)
target_compile_features(PercusynEngine PUBLIC cxx_std_20)
set_target_properties(PercusynEngine PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(PercusynEngine PRIVATE /W4)
else()
    target_compile_options(PercusynEngine PRIVATE -Wall -Wextra)
endif()

# Factory presets are the handoff examples, embedded unchanged.
juce_add_binary_data(PercusynPresetData
    NAMESPACE PresetData
    SOURCES
        examples/presets/01-acid-hats.json
        examples/presets/02-warehouse-909.json
        examples/presets/03-dust-and-metal.json
        examples/presets/04-broken-circuit.json
        examples/presets/05-minimal-pulse.json
        examples/presets/06-metal-controls-demo.json)
set_target_properties(PercusynPresetData PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------------------------------------
# Plugin
# ---------------------------------------------------------------------------
juce_add_plugin(Percusyn
    PRODUCT_NAME             "PERCUSYN"
    COMPANY_NAME             "sscheppert-blip"
    COMPANY_WEBSITE          "https://github.com/sscheppert-blip"
    BUNDLE_ID                "io.github.sscheppert-blip.percusyn"
    PLUGIN_MANUFACTURER_CODE Sbli
    PLUGIN_CODE              Pcsy
    FORMATS                  AU VST3 Standalone
    IS_SYNTH                 TRUE
    NEEDS_MIDI_INPUT         TRUE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD  ${PERCUSYN_COPY_PLUGIN}
    VST3_CATEGORIES          Instrument Drum Synth
    AU_MAIN_TYPE             kAudioUnitType_MusicDevice
    DESCRIPTION              "Nine-voice percussion synthesizer")

set(PERCUSYN_SOURCES
    Source/Params.cpp
    Source/Pattern.cpp
    Source/Presets.cpp
    Source/OneShot.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/ui/Look.cpp
    Source/ui/Widgets.cpp
    Source/ui/Pages.cpp)

target_sources(Percusyn PRIVATE ${PERCUSYN_SOURCES})
target_include_directories(Percusyn PRIVATE Source)

target_compile_definitions(Percusyn
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Percusyn
    PRIVATE
        PercusynEngine
        PercusynPresetData
        clap_juce_extensions
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

clap_juce_extensions_plugin(TARGET Percusyn
    CLAP_ID       "io.github.sscheppert-blip.percusyn"
    CLAP_FEATURES instrument drum synthesizer stereo)

# No signing or notarization (docs/11_DECISIONS.md). Apple Silicon hosts still need an
# ad-hoc signature to load a bundle, which identifies nobody.
if(APPLE)
    foreach(format VST3 CLAP AU Standalone)
        add_custom_command(TARGET Percusyn_${format} POST_BUILD
            COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:Percusyn_${format}>"
            VERBATIM)
    endforeach()
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(PERCUSYN_BUILD_TESTS)
    juce_add_console_app(PercusynTests PRODUCT_NAME "PercusynTests")

    target_sources(PercusynTests PRIVATE ${PERCUSYN_SOURCES} tests/Tests.cpp)
    target_include_directories(PercusynTests PRIVATE Source)
    target_compile_definitions(PercusynTests
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_MODAL_LOOPS_PERMITTED=1
            JUCE_PLUGINHOST_VST3=1
            PERCUSYN_TESTS=1
            JucePlugin_Name="PERCUSYN"
            JucePlugin_VersionString="${PROJECT_VERSION}")

    target_link_libraries(PercusynTests
        PRIVATE
            PercusynEngine
            PercusynPresetData
            juce::juce_audio_utils
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    enable_testing()
    add_test(NAME PercusynTests COMMAND PercusynTests)
endif()
