# arcade-07 🕹️
> a hardware-based gaming suite running on arduino. no fluff, just pure pixel logic.

### 🚀 Project overview
ARCADE 07 is a custom-built snake game and multi-game console running on an Arduino Uno. The system features a visual output powered by a 32x8 MAX7219 Dot Matrix and control inputs via a KY-023 joystick and a 10kΩ potentiometer for speed control.

### 🛠️ Hardware requirements
To build this project, you need the following components
* Arduino Uno + Power Cable
* 8*32 LED Matrix (MAX7219 Dot Matrix Module, 4 in one display with 5p line)
* KY-023 XY-axis Joystick Module
* 10kΩ Potentiometer
* 2x LEDs (Blue and Green)
* 2x 220Ω Resistors
* Breadboard and jumper wires
* 9V Battery

## 💻 Software & Libraries
To compile this code, you will need the following libraries installed in your Arduino IDE:
* **[MD_MAX72xx by majicDesigns]**

## ⚠️ Technical Note
* **Pin 12 (Blue LED):** On some Arduino Uno boards, Pin 12 is reserved for SPI communication (MISO). If your Blue LED doesn't light up or flickers, move it to **Digital Pin 9** and update the `LED_PIN` in the code.

### 🔌 Circuit connections
Wire the components as follows :
* **Power System:** Connect the 9V Battery to the Arduino VIN and GND pins.
* **MAX7219 Matrix (Visual Output):** Connect VCC to Arduino +5V and GND to GND. Connect DIN to Digital Pin 11, CS to Digital Pin 10, and CLK to Digital Pin 13.
* **KY-023 Joystick (Control Input):** Connect VCC to +5V and GND to GND. Connect VRx to Analog Pin A0, VRy to Analog Pin A1, and the SW (switch) to Analog Pin A2.
* **Speed Control:** Connect the outer pins of the 10kΩ potentiometer to +5V and GND, and the wiper pin to Analog Pin A3.
* **Indicators:** Connect the Blue LED to Digital Pin 12 (SuperMode) via a 220Ω resistor. Connect the Green LED to Analog Pin A4 (DoublePoints) via a 220Ω resistor.

### 🎮 How to play guide
refer to "ARCADE_07 MANUAL.pdf"

---
*built for the love of physics and pixels.*
