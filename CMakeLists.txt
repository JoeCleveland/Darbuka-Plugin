# Example Audio Plugin CMakeLists.txt

cmake_minimum_required(VERSION 3.22)

project(DARBUKA_PLUGIN VERSION 0.0.1)

set(CMAKE_BUILD_TYPE Release)

find_package(OpenMP REQUIRED)

# SET(GCC_COVERAGE_COMPILE_FLAGS "-w -march=native -O3 -mfpu=neon -funroll-loops -DNDEBUG")
SET(GCC_COVERAGE_COMPILE_FLAGS "-w -march=native -O3 -mfpu=neon -funroll-loops -pthread")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
file(GLOB_RECURSE EIGEN_SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "Eigen/*")

add_subdirectory(JUCE)                    # If you've put JUCE in a subdirectory called JUCE

juce_add_plugin(DarbukaPlugin
    VERSION 0.1                                 # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME jcleveland                     # Specify the name of the plugin's author
    IS_SYNTH TRUE                               # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE                       # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    BUNDLE_ID "com.jcleveland.darbuka"
    PLUGIN_NAME "Darbuka Plugin"
    COPY_PLUGIN_AFTER_BUILD TRUE                # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE jClv               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Drbk                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS AU VST3 Standalone                            # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "Darbuka Plugin")        # The name of the final executable, which can differ from the target name

target_include_directories(DarbukaPlugin PUBLIC "Eigen")

target_sources(DarbukaPlugin
    PRIVATE
        "${EIGEN_SOURCES}"
        source/Params.cpp
        source/AllPassFilter.cpp
        source/DrumFilter.cpp
        source/CircularBeamMesh.cpp
        source/BeamElement.cpp
        source/BeamSystem3D.cpp
        source/ModalElement.cpp
        source/CircularMesh.cpp
        source/RenderView.cpp
        source/ModalPlate.cpp
        source/FEMembrane.cpp
        source/RolandMIDIMap.cpp
        source/PluginEditor.cpp
        source/PluginProcessor.cpp)

target_compile_definitions(DarbukaPlugin
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(DarbukaPlugin
    PRIVATE
        # DarbukaPluginData           # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_utils
        juce::juce_opengl
        juce::juce_dsp
        OpenMP::OpenMP_CXX
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
