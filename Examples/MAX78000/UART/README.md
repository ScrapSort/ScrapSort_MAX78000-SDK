## Description

This application uses two serial ports to send and receive data.  One serial port transmits data while the other receives it.

## Required Connections

-   Connect a USB cable between the PC and the CN1 (USB/PWR) connector.
-   Connect pins 1 and 2 (P0.1) of the JH1 (UART 0 EN) header.
-   Open an terminal application on the PC and connect to the EV kit's console UART at 115200, 8-N-1.
-   Connect JH2.2 (P0.12) to JH4.2 (P1.1).
-   Connect JH2.4 (P0.13) to JH4.4 (P1.0).

## Expected Output

The Console UART of the device will output these messages:

```
**************** UART Example ******************
This example sends data from one UART to another


Connect UART1 to UART2 for this example.
P0.12 -> P1.1 and P0.13 -> P1.0


-->UART Baud    : 115200 Hz

-->Test Length  : 1024 bytes

-->UART 1 Baud rate: 115384
-->UART 2 Baud rate: 115384

-->UART Initialized

-->Data verified

-->EXAMPLE SUCCEEDED
```

