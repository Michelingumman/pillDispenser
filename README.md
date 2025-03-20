# ✨ Smart Pill Dispenser

<div align="center">
  <p><i>An intelligent, power-efficient medication reminder and dispensing system</i></p>
  <img src="https://github.com/user-attachments/assets/f206e19e-825b-485d-aeec-251874fbe2c2" width="400" />
</div>

## 🌟 Features

<table>
  <tr>
    <td width="60%">
      <ul>
        <li>🔹 <b>Touch-activated dispensing</b> - Simple touch sensor activation</li>
        <li>🔹 <b>Servo-controlled mechanism</b> - Precise pill delivery</li>
        <li>🔹 <b>Animated OLED display</b> - Engaging visual feedback</li>
        <li>🔹 <b>Power-efficient design</b> - Extended battery life with ESP32 light sleep</li>
        <li>🔹 <b>Customizable messages</b> - Personalized medication reminders</li>
      </ul>
    </td>
    <td width="40%">
      <img src="https://github.com/user-attachments/assets/428e2f2a-9c63-459b-a809-05c0c11899d4" width="100%" />
    </td>
  </tr>
</table>

## 🛠️ Hardware & Components

<table>
  <tr>
    <td width="30%">
      <img src="https://github.com/user-attachments/assets/51ffb2ea-89b0-4041-9e24-de6e88566bf6" width="100%" />
    </td>
    <td width="30%">
      <img src="https://github.com/user-attachments/assets/c6e655cd-30ec-4cac-9598-82cacd1931f4" width="100%" />
    </td>
    <td width="30%">
      <ul>
        <li>▫️ ESP32 microcontroller</li>
        <li>▫️ 128x32 OLED display (SSD1306)</li>
        <li>▫️ Micro servo motor</li>
        <li>▫️ Touch sensor</li>
        <li>▫️ Power supply</li>
      </ul>
    </td>
  </tr>
</table>

## 📱 Display Animations

The system features three main animations:
- **Lady & Gentleman** - Default idle animation showing a couple with animated heart
- **Dancing Couple** - Celebratory animation after medication dispensed
- **Scrolling Text** - Personalized messages that scroll across the screen

## ⚡ Power Management

This project implements advanced power-saving techniques:
- **Light sleep mode** between animation frames
- **GPIO pin state holding** to prevent LED flickering
- **Efficient interrupt handling** for touch detection

## 🔄 Operation Flow

<table>
  <tr>
    <td width="60%">
      <ol>
        <li>User touches the sensor to activate</li>
        <li>Servo performs a sequence of movements to dispense pills</li>
        <li>Display shows "Giving u some meds"</li>
        <li>A motivational message scrolls across the screen</li>
        <li>Dancing couple animation plays to confirm completion</li>
        <li>System returns to power-saving idle mode</li>
      </ol>
    </td>

  </tr>
</table>

## 🧩 Software Architecture

The codebase is organized into several key components:
- `main.cpp` - Core program logic and power management
- `animations.h` - Animation system with function pointers for different displays
- `images.h` - Bitmap images for animations
- `secrets.h` - Customizable message storage

## 🔧 Setup & Customization

### Connections
- OLED display: SDA (pin 6), SCL (pin 7)
- Servo: pin 3
- Touch sensor: pin 10
- LED indicator: pin 8

### Customization Options
- **Messages**: Edit the messages array in `secrets.h`
- **Animations**: Add new animations in `animations.h`
- **Timing**: Adjust servo speed and animation durations

## 📝 License

[MIT License](LICENSE)

---

<div align="center">
  <small>Built with ESP32 & Arduino • Powered by light sleep technology</small>
</div>
