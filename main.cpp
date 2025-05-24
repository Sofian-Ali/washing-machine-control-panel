 #include "mbed.h"

// LED & Buzzer Pins
DigitalOut redLED(PC_0);
DigitalOut greenLED1(PC_1);
DigitalOut greenLED2(PB_0);
DigitalOut greenLED3(PA_4);
DigitalOut buzzer(PA_15);

// RGB LED (Custom)
DigitalOut rgbRed(PB_3);
DigitalOut rgbGreen(PB_4);
DigitalOut rgbBlue(PB_5);

// 7-Segment Display Pins
DigitalOut segA(PA_11);
DigitalOut segB(PA_12);
DigitalOut segC(PB_1);
DigitalOut segD(PB_15);
DigitalOut segE(PB_14);
DigitalOut segF(PB_12);
DigitalOut segG(PB_11);

// Buttons
InterruptIn startStopButton(PC_10);
InterruptIn morningWash(PC_11);
InterruptIn eveningWash(PD_2);

// Potentiometers
AnalogIn pot1(PA_5); // Duration
AnalogIn pot2(PA_6); // Temperature
AnalogIn pot3(PA_7); // Fabric type

// Sensors
AnalogIn LDR(PC_2); // Measures light intensity
AnalogIn FSR(PA_1); // Measures force

// States
bool machineOn = false;
bool washing = false;
int cycleTimeSeconds = 5;

char segPatterns[6][7] = {
    {1,1,1,1,1,1,0},  // 0
    {0,1,1,0,0,0,0},  // 1
    {1,1,0,1,1,0,1},  // 2
    {1,1,1,1,0,0,1},  // 3
    {0,1,1,0,0,1,1},  // 4
    {1,0,1,1,0,1,1}   // 5
};


void displayDigit(int number) {
    if (number > 5) number = 5;
    char *pat = segPatterns[number];
    segA = pat[0]; segB = pat[1]; segC = pat[2];
    segD = pat[3]; segE = pat[4]; segF = pat[5]; segG = pat[6];
}

void playMelody() {
    PwmOut tone(PA_15);

    // Define the melody: {frequency in Hz, duration in ms}
    const int melody[][2] = {
        {262, 300}, // C4
        {330, 300}, // E4
        {392, 300}, // G4
        {523, 600}  // C5
    };

    for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); ++i) {
        tone.period(1.0 / melody[i][0]);
        tone = 0.5;
        ThisThread::sleep_for(chrono::milliseconds(melody[i][1]));
        tone = 0;
        ThisThread::sleep_for(100ms); // Short pause between notes
    }
}



void beepCNote() {
    PwmOut tone(PA_15);
    tone.period(1.0 / 262); tone = 0.5; ThisThread::sleep_for(200ms);
    tone = 0; ThisThread::sleep_for(100ms);
    tone = 0.5; ThisThread::sleep_for(200ms);
    tone = 0;
}

void setRGB(bool r, bool g, bool b) {
    rgbRed = r;
    rgbGreen = g;
    rgbBlue = b;
}

void applyFabricMode(int fabric, int &duration, float &temperature) {
    switch (fabric) {
        case 0: // Regular
            duration = pot1.read() * 5.9f + 1;
            temperature = pot2.read();
            setRGB(1, 1, 1); // White
            break;

        case 1: // Cotton
            duration = 5;
            temperature = 1.0f; // Hot
            setRGB(1, 0.5f, 0); // Orange
            break;

        case 2: // Synthetics
            duration = 3;
            temperature = 0.5f; // Room temperature
            setRGB(0, 0.5f, 1); // Light blue
            break;

        case 3: // Delicates
            duration = 2;
            temperature = 0.3f;
            setRGB(0, 1, 1); // Blue
            break;

        case 4: // Bedding
            duration = 4;
            temperature = 0.5f;
            setRGB(1, 1, 0); // Yellow
            break;

        default:
            duration = 5;
            temperature = 0.5f;
            setRGB(0, 0, 0); // Off
    }
}



void updateTemperatureColor(float tempVal) {
    if (tempVal < 0.33f)
        setRGB(0, 0, 1); // Cold - Blue
    else if (tempVal < 0.66f)
        setRGB(0, 1, 0); // Room Temperature - Green
    else
        setRGB(1, 0, 0); // Hot - Red
}

bool obstruction() {
    return LDR.read() < 0.1f; // Obstruction is true if light intensity is less than 10%
}

void toggleMachinePower() {
    machineOn = !machineOn;
    if (!machineOn) { // Initalises LEDs
        washing = false;
        redLED = 1;
        greenLED1 = greenLED2 = greenLED3 = 0;
        setRGB(0, 0, 0);
    }
}

void runWashingCycle() {
    greenLED2 = greenLED3 = 0;

    for (int cycle = 1; cycle <= 3 && machineOn; cycle++) { // Runs washing cycle loop up to 3 times if machine is on
        printf("Starting washing cycle %d...\r\n", cycle);

        // Read current fabric mode
        static int lastFabricMode = -1;
        int fabricMode = pot3.read() * 4.0f;

    if (fabricMode != lastFabricMode) { // Checks if fabric mode has been changed
    switch (fabricMode) {
        case 0: printf("System: Regular wash selected.\n"); break;
        case 1: printf("System: Cotton wash selected.\n"); break;
        case 2: printf("System: Synthetics wash selected.\n"); break;
        case 3: printf("System: Delicates wash selected.\n"); break;
        case 4: printf("System: Bedding wash selected.\n"); break;
        default: printf("System: Unknown fabric mode. Defaulting.\n");
    }
    lastFabricMode = fabricMode;
}

        // Only print system message and update RGB if mode changed
        if (fabricMode != lastFabricMode) {
            float tempPreview;
            int timeDuration;
            applyFabricMode(fabricMode, timeDuration, tempPreview);  // Will print and set RGB
            lastFabricMode = fabricMode;
        }

        float userTemperature = 37.5;
        int userSelectedTime = (int)(pot1.read() * 5.9f + 1); // pot1 input
        cycleTimeSeconds = userSelectedTime;
        int tempCycleTime = userSelectedTime;

        applyFabricMode(fabricMode, tempCycleTime, userTemperature);
        const char* modeName;
       switch (fabricMode) {
       case 0: modeName = "Regular"; break;
       case 1: modeName = "Cotton"; break;
       case 2: modeName = "Synthetics"; break;
       case 3: modeName = "Delicates"; break;
       case 4: modeName = "Bedding"; break;
       default: modeName = "Unknown";
}

            printf("System: Starting wash with Mode: %s | Duration: %d sec | Temp: %.2f\n",
            modeName, tempCycleTime, userTemperature);


        for (int sec = tempCycleTime; sec >= 0 && machineOn; sec--) {
            displayDigit(sec > 5 ? 5 : sec);

            // Only allow temperature-based RGB color in REGULAR mode
            if (fabricMode == 0) {
                updateTemperatureColor(pot2.read());
            }

            ThisThread::sleep_for(1000ms);

            // Live update time only in REGULAR
            if (fabricMode == 0) {
                tempCycleTime = pot1.read() * 5.9f + 1;
            }
        }

        if (obstruction()) { // Checks for obstruction
            printf("Obstruction detected! Halting washing process.\r\n");
            washing = false;
            break;
        }

        // Green LEDs turn on in different cycles
        greenLED1 = (cycle == 3);
        greenLED2 = (cycle == 1);
        greenLED3 = (cycle == 2);

        printf("Cycle %d completed successfully.\r\n", cycle);

        if (cycle < 3) { // Buzzer beeps in cycle 1 & 2
            beepCNote();
        }
    }

    if (washing && machineOn) { // Melody played in cycle 3
        printf("All washing cycles completed. Playing completion melody...\r\n");
        playMelody();
    }

    washing = false; // LEDs reinitalised after washing ends
    greenLED2 = greenLED3 = 0;
    greenLED1 = 1;

    ThisThread::sleep_for(200ms);
}




void morningWashCycle() {
printf("Morning wash schedule activated. Your washing machine will begin its cycle automatically when morning light is detected. Have a fresh and bright start to your day!\n");
 if (LDR.read() > 0.6f) { // Checks if light intensity is greater than 60%
 washing = true;
 runWashingCycle();
 }
}

void eveningWashCycle() {
printf("Evening wash schedule set. Your washing machine will begin its cycle when evening light levels are appropriate. Rest easy knowing your laundry is taken care of.\n");
 if (LDR.read() < 0.3f) { //Checks if light intensity is less than 30%
 washing = true;
  runWashingCycle();
 }
}

int main() {
    // Initial state: Machine OFF
    redLED = 1;
    greenLED1 = greenLED2 = greenLED3 = 0;
    buzzer = 0;
    setRGB(0, 0, 0);

    startStopButton.mode(PullUp); // Internal pull-up in NUCLEO activated 
    startStopButton.fall(&toggleMachinePower); // If button pressed, machine powers on
   
    morningWash.mode(PullUp);
    morningWash.fall(&morningWashCycle); // Runs wash when LDR detects daylight
    
    eveningWash.mode(PullUp);
    eveningWash.fall(&eveningWashCycle); // Runs wash when LDR detects less light
    
    if (machineOn == true) { 
        printf("Machine powered on. Ready to start washing cycle."); }

    while (true) {
        if (!machineOn) {
            // Show machine is OFF
            redLED = 1;
            greenLED1 = greenLED2 = greenLED3 = 0;
            ThisThread::sleep_for(300ms);
            continue;
        }
        
        // Machine is ON but not washing yet
        redLED = 0;
        greenLED1 = 1;

        int potVal = pot1.read() * 5.9f; // Displays digit based on pot1 user input
        displayDigit(potVal);
        cycleTimeSeconds = potVal + 1;
        updateTemperatureColor(pot2.read()); // Temperature based on pot2 user input

        // Check FSR to start washing
        if (FSR.read() > 0.2f && !washing) {
            printf("Pressure detected on FSR\r\n");
            washing = true;
            runWashingCycle();
 
    }
}
}