# Darbuka Plugin

This repository is a JUCE Plugin implementing a physical model of a Darbuka-style hand drum.
The finite-element method and modal synthesis techniques are used.

CMakeLists.txt is configured for Apple Silicon MacOS, and assumes that OpenMP is installed on your system.

Apple specific build flags and linking with OpenMP can be removed to build on other platforms.

The JUCE library is assumed to be placed within the project directory.

The only other dependency is Eigen, which is provided in the repo.

## Playing

The plugin is designed to interface with the Roland handsonic HPD-20 controller. 

To play with a standard MIDI controller, the notes C4 - E4 map to distinct regions on the drumhead.

The bending gesture is controlled by aftertouch.

