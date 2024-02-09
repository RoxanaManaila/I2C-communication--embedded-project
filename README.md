# I2C-communication--embedded-project
This project is developed in TinkerCAD using C++, and it focuses on the I2C (TWI) communication
between 3 microcontrollers. The hardware is composed of one master device and two slave devices and
the main idea of the project is sending and receving data between the master and the other two devices.
What is special about this project is that the code itself is written using only Atmega328P registers. 
The first slave device records the temperature through a temperature sensor, while the second slave device is responsible with monitoring the voltage in a battery.
The LEDs on each slave device are used for the purpose of visualizing the current state of each device, whether they are in transmit mode or receiving mode.
The LCD is used by the master device to display the data received from the slave devices.

![image](https://github.com/RoxanaManaila/I2C-communication--embedded-project/assets/156087923/84de5471-8717-4fd4-832a-2c0a1d5749bd)

