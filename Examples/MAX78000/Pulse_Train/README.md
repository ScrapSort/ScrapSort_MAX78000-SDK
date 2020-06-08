## Description

Two pulse trains are configured in different modes.  

The first, PT0, is set to generate a repeating bit pattern of 0x10110 (lsb first) at a rate of 2 bits per second.  If you make the connections described below, you can observe the pattern on LED1.

The second, PT1, is set to generate a 10Hz square wave.  If you make the connections described below, you can observe the square wave on LED2.

## Required Connections

-   Connect a USB cable between the PC and the CN1 (USB/PWR) connector.
-   Connect pins 1 and 2 (P0_1) of the JH1 (UART 0 EN) header.
-   Open an terminal application on the PC and connect to the EV kit's console UART at 115200, 8-N-1.
-   Connect pin 12 of JH4 (Camera) to pin 2 of JP1 (LED1).
-   Connect pin 9 of the DSP2 connector to pin 2 of JP2 (LED2).
-   Connect pins 1 and 2 of R52.

## Expected Output

The Console UART of the device will output these messages:

```
*************** Pulse Train Demo ****************
PT0 (P0.18) = Outputs continuous pattern of 10110b at 2bps
PT1 (P0.19) = Outputs 10Hz continuous square wave
```

