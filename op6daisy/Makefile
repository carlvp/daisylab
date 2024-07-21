# This Makefile (almost) follows the template in DaisyExamples
# also set LIBDAISY_DIR to point to where libDaisy is installed
# (../DaisyExamples/libDaisy in my case).
# See https://github.com/electro-smith/DaisyExamples
#
# I believe the included Makefile also expects:
# GCC_PATH
# OCD_DIR
# and they should be in PATH as well
#
# For further info on setting up the development environment for
# Daisy Seed, see:
# https://forum.electro-smith.com/t/welcome-to-daisy-get-started-here

# Project Name
TARGET = op6daisy

# Sources
CPP_SOURCES = op6daisy.cpp \
  AudioPath.cpp \
  Channel.cpp \
  Program.cpp \
  EnvelopeState.cpp \
  FmAlgorithm.cpp \
  FmOperator.cpp \
  Instrument.cpp \
  LfoState.cpp \
  MidiDispatcher.cpp \
  sine_lut.cpp \
  Voice.cpp \
  VoiceEditBuffer.cpp

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
