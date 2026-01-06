# VoiceRecognitionESP32modiifca

## Description

This repository contains an **unofficial academic adaptation** of the Elechouse **VoiceRecognitionV3** library for use on **ESP32** microcontrollers.

The original library (available at https://github.com/elechouse/VoiceRecognitionV3) relies on `SoftwareSerial`, which is not suitable for ESP32. This adapted version replaces `SoftwareSerial` with `HardwareSerial` to support ESP32 UART hardware and improve communication stability.

This version is intended for **educational and research purposes only** and is **not an official release by Elechouse**. Use at your own risk.

---

## Background

The original Elechouse VoiceRecognitionV3 library provides an Arduino interface to the Voice Recognition V3 module. It uses `SoftwareSerial` for communication, which is incompatible or unstable on ESP32 due to hardware limitations. This repository modifies the library to use the ESP32 hardware UART interface instead.

Original library: https://github.com/elechouse/VoiceRecognitionV3

---

## Features

- Replaces `SoftwareSerial` with `HardwareSerial` for ESP32
- Maintains original command structure and core functionality of the library
- Includes example sketches adapted for ESP32 UART usage
- Designed for research, learning, and experimentation

---

## Compatibility

- Works with **ESP32** microcontrollers
- Not compatible with AVR-based boards (Arduino Uno, Nano, Mega, etc.)
- Not an official Elechouse-supported library

---

## Usage

1. Connect your ESP32 UART pins to the Voice Recognition V3 module
2. Import this library into your Arduino/PlatformIO project
3. Use the provided example sketches to test command training and recognition

Make sure to configure appropriate UART pins for your ESP32 setup.

---

## Limitations

- Not guaranteed to work in all ESP32 configurations
- May contain untested or experimental behavior
- Users must verify and test in their own applications

---

## Responsibility

This adaptation is provided for academic and research use. The authors and contributors make **no warranties** of any kind, either expressed or implied.

You are responsible for:

- Correct use of this library
- Any damages to hardware or software
- Ensuring this library meets your needs

By using this library, you agree that the authors are not liable for any direct or indirect issues resulting from its use.

---

## License

This project is distributed under a **MIT-style license**.

The software is provided "as is", without warranty of any kind, express or implied, including but not limited to warranties of merchantability, fitness for a particular purpose, or noninfringement.

In no event shall the authors be liable for any claim, damages, or other liabilities, whether in an action of contract, tort, or otherwise, arising from the software or its use.

---

## Credits

- Original Elechouse VoiceRecognitionV3 library
- ESP32 hardware and UART documentation
- Academic and embedded systems community resources
