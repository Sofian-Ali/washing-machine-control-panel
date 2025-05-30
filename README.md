# Washing Machine Control Panel
# Overview
This project simulates a washing machine control panel system implemented using C++ via Keil Studio on a NUCLEO-L476RG microcontroller board. It has been designed with a wide variety of different sensors and components, including an LDR and FSR to measure light intensity and force, to provide the user with a smooth and user-friendly experience.
# Functions
POWER ON – The power on function is controlled by a SDPT push button. To turn the washing machine on, you must press the button, in which case a green LED will light up to indicate the washing machine has turned on.  You can then interact with all the other controls, but to run a washing cycle, refer to START CYCLE.

START CYCLE – Starting a wash cycle manually is controlled by the FSR. When force is applied to the FSR, the washing machine will run a wash cycle.

MORNING & EVENING CYCLES – After powering the washing machine on, you have the option to set-up daily morning and evening washes. For morning washes, press the morning cycle SDPT switch to run automatic daily washes at 9am to 10am in the morning when daylight is detected. Alternatively, for evening washes, press the evening cycle SDPT switch to run automatic daily washes at 9pm to 10pm in the evening when low levels of light are detected. Your daily cycle selection will be confirmed with a system message. The control panel can determine the appropriate time for a morning or evening wash by using an LDR to measure light levels throughout the day.

ADJUSTING WASHING TIMER – The washing timer can be adjusted by turning the ‘Cycle Time’ potentiometer to determine how long each cycle should run. Rotating the potentiometer anticlockwise will increase the cycle duration, whilst rotating it clockwise will decrease the duration.

ADJUSTING WATER TEMPERATURE – Water temperature can be adjusted by turning the ‘Temperature’ potentiometer. There are three options: hot, cold and room temperature. Rotating the potentiometer will allow you to toggle between different temperatures. The RGB LED will indicate the temperature of the water: hot indicated by red, room temperature indicated by green and cold indicated by blue.
