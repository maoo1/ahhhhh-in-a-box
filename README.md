# Ahhhhh In a Box

This project utilized a ESP32 TTGO T-Display to simulate feelings of frustration, visualized through glitch noise, screen
shake, and procedurally generated screams. Each boot creates a new visual world using a randomized seed. Some pictures of the 
different screams:

![ahh-caps](https://github.com/user-attachments/assets/6caf2ce7-ce3f-498e-8cc4-ba54659a8cda)
A scream in caps.

![ahh-lower-case](https://github.com/user-attachments/assets/aa0693ed-5132-48fb-bcf9-fd2507ac284d)
A scream in lower case.

![ahh-exclaimation](https://github.com/user-attachments/assets/6d269645-84e9-4405-81ce-21bf5eb5a177)
A scream with exclaimation points. 

# Hardware Requirements

- ESP32 TTGO T-Display
- USB Data Cable
- Battery or any power source

# Software Setup

For the software, I installed the PlatformIO extension on VS Code and created a new project. After, I installed the TFT-eSPI
library by running the following lines in my terminal in VS Code. 

```bash
pio pkg search "tft espi"
pio pkg install -l bodmer/TFT_eSPI
```

The playformio.ini file should look like below. The lib_deps gets added automatically, and the build_flags I did by hand.

```bash
[env:ttgo-t1]
platform = espressif32
board = ttgo-t1
framework = arduino
lib_deps = bodmer/TFT_eSPI@^2.5.43
build_flags =
 ;###############################################################
 ; TFT_eSPI library setting here (no need to edit library files):
 ;###############################################################
 -D USER_SETUP_LOADED=1                        ; Set this settings as valid
 -include $PROJECT_LIBDEPS_DIR/$PIOENV/TFT_eSPI/User_Setups/Setup25_TTGO_T_Display.h
```

After, the main.cpp can be edited with whatever code. Once the code is ready, you must run it so it compiles, as well as upload the code onto your display.
The TTGO will solely display code that exists on its own device, so that's why it's imperative that if you modify your code, you must run it and upload it again. Additionally, 
I struggled with port issues, but if that occurs, try plugging the cable into another port.

# Code Overview

## 1. Procedural Randomness

I created a custom xorshift32 that would generate a reproducible randomness from a seed. Each boot would create a new seed, and, thus, 
a new series of screams. 

The seeding was done just the esp_random() that functions as hardware RNG on ESP32. Below is my xorshift:

```bash
uint32_t rngState = 1;

uint32_t xrnd() {
  uint32_t x = rngState;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rngState = x;
  return rngState;
}
```

Additionally, I had an integer helper that would return a random integer from [a, b].

```bash
int irnd(int a, int b) {
  return a + (int)(xrnd() % (uint32_t)(b - a + 1));
}
```

## 2. Rage System

I created a floating-point variable known as "rage" that changed with each frame. As the code ran longer, 
the "rage" would slowly increase, with occasional spikes and natural decays. Additionally, the system would
flash white when exceeding a certain threshold. All these states were controlled by a random number generator. 

To modify the behavior of the rage system, I had a variable for growth of the "rage," as well as a variable 
that determined how frequent the spikes in intensity could occur. Additionally, I also had a variable that 
controlled the decaying of rage, that way it wouldn't just be intense the entire time. 

## 3. Visual Output

As the rage increased, white specks would multiply, and after crossing a certain threshhold, the text would glitch. 
Additionally, letters would stretch and repeat, sometimes in lower case or upper case. Exclamation marks would
also appear. I set the frame rate for ~10 FPS, but this can also be adjusted. 
