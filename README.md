# Embedded Programming Portfolio

## Overview

This repository contains a collection of embedded programming projects developed for the **Programmeerproject Embedded** course (Academic Year 2023-2024) at Karel de Grote Hogeschool.

## Project Structure

The repository is organized into multiple workspace files, each representing a standalone project:

### Individual Project Workspaces

1. **🎵 DJController** - Advanced music control system
2. **🎯 Nim** - Strategic number game implementation
3. **📡 Morse Code** - Morse code encoder/decoder
4. **🎮 Simon Says** - Memory sequence game

### Portfolio Workspace

- **📁 Portfolio** - Complete workspace containing all projects in one consolidated view

## Project Details

### DJController 🎵

**Type**: Hybrid Application (Embedded C + Desktop Java)

The DJController is a sophisticated music control system consisting of two integrated components:

#### **C/Arduino Component**
- **Platform**: Arduino/AVR microcontroller
- **Features**: 
  - Hardware control (LEDs, buttons, 7-segment display, buzzer)
  - Real-time audio processing signals
  - Serial communication protocol
  - Beat detection and rhythm synchronization
- **Libraries**: Custom modular libraries for all hardware interfaces

#### **JavaFX Component**
- **Platform**: Desktop Java application
- **Features**:
  - Graphical user interface for playlist management
  - Audio playback engine using JavaFX Media
  - Real-time synchronization with Arduino
  - Track visualization and control
- **Integration**: Seamless communication with Arduino via serial protocol

**Key Technologies**: C/AVR, JavaFX, Serial Communication, Audio Processing

### Nim 🎯
Strategic number-based game implementation with AI opponent capabilities.

### Morse Code 📡
Comprehensive Morse code communication system with encoding/decoding functionality.

### Simon Says 🎮
Interactive memory game with visual and audio feedback systems.

## Getting Started

### Prerequisites
- **Hardware**: Arduino Uno Rev3 + Expansion Shield
- **Software**: 
  - Arduino IDE or compatible C compiler
  - Java Development Kit (JDK 11+) for JavaFX projects
  - Git for version control

### Workspace Setup

1. **Individual Projects**: Open the specific `.code-workspace` file for the project you want to work on
2. **Complete Portfolio**: Open `portfolio.code-workspace` to access all projects simultaneously

### Running the DJController

1. **djcontrollerarduino** - Hardware-oriented counterpart written in C, meant to be uploaded to an Arduino Uno Rev3 model.
2. **djcontrollerv4** - JavaFX application counterpart meant for usage as a Media Player that can be controlled by an Arduino Uno Rev3 model running main.c file with library dependencies.

## Technical Specifications

All projects implement the following embedded programming concepts:
- ✅ LED control and dimming
- ✅ Button input detection with debouncing
- ✅ Buzzer control with multiple frequencies
- ✅ 7-segment display management
- ✅ ADC (Analog-to-Digital Conversion)
- ✅ Custom hardware abstraction libraries
- ✅ Pointer manipulation and memory management
- ✅ Function parameters by value and reference
- ✅ Interrupt service routines
- ✅ Timer/counter implementation
- ✅ Serial communication (UART)
- ✅ Array and struct usage
- ✅ Dynamic memory allocation

## Documentation

Each project directory contains:
- **Source code** with comprehensive comments
- **Header files** with function documentation
- **Hardware schematics** (where applicable)

## Author

**Academic Project** - Karel de Grote Hogeschool  
**Course** - Programmeerproject Embedded (2024-2025)
**Student** - Romeo Weyns

## License

This project is developed for educational purposes as part of academic coursework.

---

*For specific project details, navigate to the individual project directories or open the corresponding workspace files.*