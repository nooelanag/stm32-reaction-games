# STM32 Reaction Games

A bare-metal embedded systems project implementing two reaction-time games on an STM32L152RE NUCLEO board. Developed as the laboratory project for the *Microprocessor-based Digital Systems* course at Universidad Carlos III de Madrid (academic year 2021–2022).

## Project Description

The firmware implements two two-player reaction games controlled through external buttons, LEDs, a buzzer, and a potentiometer, all connected to the NUCLEO-L152RE development board. Players interact with the hardware peripherals in real time; the microcontroller measures reaction times, drives visual and audible feedback, and outputs results over USART to a serial console.

The project is structured around four incremental milestones that progressively integrate GPIO configuration, hardware timer interrupts (TIM2/TIM4), ADC-based potentiometer reading, PWM-driven buzzer melodies, and UART serial communication.

## Objectives

- Practice low-level peripheral programming on ARM Cortex-M3 (STM32L1xx) using direct register access alongside the HAL library.
- Design and implement interrupt-driven architectures using EXTI and timer capture/compare channels.
- Integrate multiple peripherals (GPIO, Timers, ADC, USART, buzzer via PWM) into a cohesive real-time application.
- Gain experience with hardware–software co-design, debugging, and iterative development on embedded platforms.

## Features

### Game 1 — Reaction Time

Two players watch an external LED (LED1). After a random delay, the LED lights up and both players race to press their respective button. The first player to press wins. The console displays the winner and their reaction time in milliseconds in the format `-Pxxxx` (e.g., `-10234` means Player 1 won with a 234 ms reaction time).

Key aspects:

- Random LED activation delay derived from the previous winner's reaction time.
- Reaction time measured via TIM4 input capture channels (CH1 and CH2) connected to BUTTON1 (PB7) and BUTTON2 (PB6).
- Continuous match loop — a new round begins automatically after each result.

### Game 2 — Countdown

A countdown from 10 is displayed on the serial console. At a random point the display stops, and players must mentally continue counting. When a player believes the countdown has reached zero, they press their button. The player closest to the actual zero moment wins.

Key aspects:

- Countdown step duration configurable via a 5 kΩ potentiometer read through ADC channel 4 (PA4), mapped to three speeds: 500 ms, 1 s, or 2 s.
- Winner's LED lights up; the console shows the timing offset with a `+` or `-` prefix indicating whether the press was after or before the countdown ended.
- Buzzer plays distinct melodies depending on whether the winning press was early or late (Melody 1 / Melody 2), generated via PWM on TIM2 CH1 (PA5).

### General

- The USER button (PC13) toggles between Game 1 and Game 2 at any time via an EXTI falling-edge interrupt.
- All timing is handled through hardware timers with interrupt service routines — no blocking delays in game logic.
- Results are printed to a serial terminal over USART2 at 115200 baud.

## System Architecture / Design Overview

```
┌──────────────────────────────────────────────────────────┐
│                    STM32L152RE (NUCLEO)                  │
│                                                          │
│  ┌──────────┐   EXTI13 (falling edge)                    │
│  │ USER BTN │──────────────────────► Game selection      │
│  │  (PC13)  │                        (option toggle)     │
│  └──────────┘                                            │
│                                                          │
│  ┌──────────┐   TIM4 CH1 (Input Capture)                 │
│  │ BUTTON1  │──────────────────────► Player 1 reaction   │
│  │  (PB7)   │                                            │
│  └──────────┘                                            │
│                                                          │
│  ┌──────────┐   TIM4 CH2 (Input Capture)                 │
│  │ BUTTON2  │──────────────────────► Player 2 reaction   │
│  │  (PB6)   │                                            │
│  └──────────┘                                            │
│                                                          │
│  ┌──────────┐   GPIO Output                              │
│  │  LED1    │◄──────────────────── Game 1 indicator      │
│  │  (PA12)  │                      / Game 2 P1 winner    │
│  └──────────┘                                            │
│                                                          │
│  ┌──────────┐   GPIO Output                              │
│  │  LED2    │◄──────────────────── Game 2 P2 winner      │
│  │  (PD2)   │                                            │
│  └──────────┘                                            │
│                                                          │
│  ┌──────────┐   ADC1 CH4                                 │
│  │  POT     │──────────────────────► Countdown speed     │
│  │  (PA4)   │                        (500ms/1s/2s)       │
│  └──────────┘                                            │
│                                                          │
│  ┌──────────┐   TIM2 CH1 (PWM / Alternate Function)      │
│  │ BUZZER   │◄──────────────────── Melody output         │
│  │  (PA5)   │                                            │
│  └──────────┘                                            │
│                                                          │
│  ┌──────────┐   USART2 TX/RX (PA2/PA3)                   │
│  │ SERIAL   │◄─────────────────── Console I/O            │
│  │ (USB)    │                                            │
│  └──────────┘                                            │
└──────────────────────────────────────────────────────────┘
```

### Interrupt Architecture

| IRQ Handler            | Source              | Purpose |
|------------------------|---------------------|---------|
| `EXTI15_10_IRQHandler` | USER button (PC13)  | Toggles the active game (`option` variable) |
| `TIM4_IRQHandler`      | TIM4 CH1–CH4        | Captures button press times (CH1/CH2), manages random LED delay (CH3), inter-round pause (CH4) |
| `TIM2_IRQHandler`      | TIM2 CH1–CH2        | Drives buzzer melody playback — CH1 controls tone frequency, CH2 controls note duration and melody progression |

### Timer Configuration

- **TIM4**: PSC = 31999 → 1 ms resolution at 32 MHz. Used for all game timing (reaction measurement, countdown steps, delays).
- **TIM2**: PSC = 31 → 1 µs resolution. Used for PWM generation on the buzzer at audio frequencies.

## Project Structure

```
Project_3/
├── .project                        # Eclipse/CubeIDE project descriptor
├── Project_3.ioc                   # STM32CubeMX configuration file
├── STM32L152RETX_FLASH.ld          # Linker script (flash)
├── STM32L152RETX_RAM.ld            # Linker script (RAM)
├── Core/
│   ├── Inc/
│   │   ├── main.h                  # Pin definitions and function prototypes
│   │   ├── stm32l1xx_hal_conf.h    # HAL configuration
│   │   └── stm32l1xx_it.h          # Interrupt handler prototypes
│   ├── Src/
│   │   ├── main.c                  # Application entry point and game logic
│   │   ├── stm32l1xx_it.c          # Default interrupt handlers
│   │   ├── stm32l1xx_hal_msp.c     # HAL MSP initialization (low-level peripheral setup)
│   │   ├── system_stm32l1xx.c      # System clock configuration
│   │   ├── syscalls.c              # Newlib system call stubs (printf redirection)
│   │   └── sysmem.c                # Newlib memory allocation stubs
│   └── Startup/
│       └── startup_stm32l152retx.s # Cortex-M3 startup assembly (vector table, Reset_Handler)
└── Drivers/
    ├── CMSIS/                      # ARM CMSIS core headers and STM32L1xx device headers
    └── STM32L1xx_HAL_Driver/       # ST HAL library sources and headers
```

The main application logic — GPIO setup, timer/ADC/EXTI configuration, ISR implementations, and the game state machine — resides entirely in `Core/Src/main.c`.

## Requirements / Dependencies

**Hardware:**

- NUCLEO-L152RE development board (STM32L152RET6, Cortex-M3, 32 MHz)
- 2 external push buttons (connected to PB7 and PB6)
- 2 LEDs with 330 Ω series resistors (connected to PA12 and PD2)
- 1 buzzer HS1212A (connected to PA5)
- 1 potentiometer, 5 kΩ single-turn (connected to PA4)
- Breadboard and wiring (DuPont cables)

**Software:**

- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) (Eclipse-based IDE with integrated compiler and debugger)
- STM32L1xx HAL Driver (included in the project under `Drivers/`)
- ARM CMSIS headers (included under `Drivers/CMSIS/`)
- `Utiles_SDM` utility library (provided by the university; contains `Bin2Ascii` helper function)

## Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/nooelanag/stm32-reaction-games.git
   ```

2. Open STM32CubeIDE and import the project:
   - **File → Import → General → Existing Projects into Workspace**
   - Select the `Project_3/` directory as root.

3. Ensure the `Utiles_SDM` library files are placed in `Core/Src/` and `Core/Inc/` (these files are provided separately by the course and are not included in this repository).

4. Connect the NUCLEO-L152RE board via USB.

5. Build the project (**Project → Build All** or `Ctrl+B`).

6. Flash the binary to the board (**Run → Debug As → STM32 C/C++ Application**).

## Configuration

### Pin Assignment (NUCLEO Board)

| Function     | Pin  | Mode                | Component             |
|-------------|------|---------------------|-----------------------|
| USER Button | PC13 | Digital Input       | On-board button       |
| BUTTON1     | PB7  | Alternate Function  | External push button  |
| BUTTON2     | PB6  | Alternate Function  | External push button  |
| LED1        | PA12 | Digital Output      | External LED          |
| LED2        | PD2  | Digital Output      | External LED          |
| ADC_IN4     | PA4  | Analog Input        | Potentiometer         |
| BUZZER      | PA5  | Alternate Function  | Buzzer (TIM2 CH1 PWM) |
| USART2_TX   | PA2  | Alternate Function  | Serial output         |
| USART2_RX   | PA3  | Alternate Function  | Serial input          |

### Serial Console

- Baud rate: **115200**
- Data bits: 8, Stop bits: 1, Parity: None
- Use any serial terminal (PuTTY, Tera Term, minicom, `screen`, etc.) connected to the virtual COM port exposed by the ST-LINK.

## Usage / Examples

1. Power the board via USB. The serial console displays `GAME 1:` indicating the active game.
2. **Game 1**: Wait for LED1 to turn on, then both players press their button as fast as possible. The console outputs the winner and reaction time, e.g.:

   ```
   GAME 1:
   -10234
   ```

   This means Player 1 won with a reaction time of 234 ms.

3. Press the **USER button** to switch to Game 2. The console displays `GAME 2`.
4. **Game 2**: Watch the countdown on the console (10, 9, 8, …). When the display stops, keep counting mentally and press your button when you think zero is reached. The winner's LED lights up and the console shows the timing error.
5. Adjust the **potentiometer** to change the countdown speed (500 ms / 1 s / 2 s per step).

## Experiments and Results

Based on the project report, the team successfully achieved Milestones 1 and 2 in full: both Game 1 (Reaction Time with random delays and timer-based measurement) and Game 2 (Countdown with dual-player timing comparison) are functional.

Milestone 3 was partially completed. The ADC-based potentiometer reading for configurable countdown speed was implemented, but the buzzer melody generation via TIM2 did not produce correct pitch variation. The buzzer triggers at the correct moments, but the frequency modulation for distinct melodies was not fully achieved. The code for melody playback is present but commented out to avoid interfering with normal program flow. The team identified a potential root cause: the TIM2 status register flag was not being cleared properly after the IRQ handler executed.

Milestone 4 (UART-based game selection replacing the USER button) was not implemented.

## Development Notes

- The project mixes HAL initialization (generated by CubeMX) with direct register-level access for timer, ADC, GPIO, and interrupt configuration. This is typical of the course's pedagogical approach — students learn both abstraction layers.
- The `TIM4_IRQHandler` in `main.c` overrides the weak default in `stm32l1xx_it.c`. The same applies to `EXTI15_10_IRQHandler` and `TIM2_IRQHandler`.
- Game 2 logic within `TIM4_IRQHandler` is commented out in the final submission due to integration issues with the buzzer; the main loop only prints `"GAME 2"` without entering the full game loop.
- The `Utiles_SDM` library (not included) provides the `Bin2Ascii()` function used to convert integer reaction times to printable strings.
- The system clock is configured at 32 MHz using the internal HSI oscillator with PLL (HSI × 6 / 3 = 32 MHz).

## License

This project uses STMicroelectronics HAL drivers and CMSIS headers, which are provided under their respective licenses (see `Drivers/CMSIS/LICENSE.txt` and `Drivers/STM32L1xx_HAL_Driver/License.md`). The application code was developed as academic coursework at UC3M.

## Authors

[Noel Andolz Aguado](https://github.com/nooelanag), Luis Elvira Sastre and [Daniel Lozano Uceda](https://github.com/dalouc)
