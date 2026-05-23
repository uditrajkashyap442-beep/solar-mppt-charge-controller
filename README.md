# Solar MPPT Charge Controller 

An embedded systems university group project demonstrating **Maximum Power Point Tracking (MPPT)** using an Arduino and a DC-DC boost converter for highly efficient solar energy harvesting. 

### 🎓 University Group Project (VIT University)
**Team Members:**
- Udit Raj Kashyap
- [Group Member 2 Name]
- [Group Member 3 Name]

---

##  Overview
Solar panels have non-linear I-V characteristics. Standard charge controllers connect panels directly to batteries, wasting 20-40% of the potential energy. This project uses the **Perturb and Observe (P&O)** algorithm to continuously track the Maximum Power Point (MPP), dynamically adjusting the operating voltage to extract the absolute maximum power regardless of environmental fluctuations.

##  Hardware Components
- **Microcontroller:** Arduino Uno/Nano
- **Power Stage:** Custom DC-DC Boost Converter (MOSFET, Inductor, Diode, Capacitors)
- **Sensors:** Voltage Dividers and Current Sensors (ADC)
- **Display:** 16x2 I2C LCD Display
- **Output:** Multi-stage battery charging & 5V USB output port

##  Software Implementation
- **Algorithm:** The `run_charger()` state machine implements the P&O MPPT algorithm, seamlessly transitioning between Off, Bulk, and Float charging states.
- **High-Frequency PWM:** Utilizes the `TimerOne.h` library to generate a stable, 10-bit PWM signal on Pin 9, providing vastly superior switching control compared to standard `analogWrite()`.
- **Signal Processing:** Implements an ADC averaging function (`read_adc()`) to filter out electrical noise and stabilize sensor readings.

##  How P&O MPPT Works
1. **Perturb:** The Arduino slightly increases or decreases the PWM duty cycle.
2. **Observe:** It measures the new solar output power ($P = V \times I$).
3. **Decide:** If the power increased, it continues perturbing in the same direction. If the power decreased, it reverses direction, effectively "locking on" to the peak efficiency point.

## 🚀 Future Enhancements
- Implement the "Incremental Conductance" algorithm to reduce oscillation.
- Add an SD Card Data Logger for long-term power analytics.
- Integrate temperature compensation to extend battery lifespan.
