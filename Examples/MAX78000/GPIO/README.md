## Description

Basic GPIO input, output, and interrupts are demonstrated in this example.

P2.6 (PB1) is continuously scanned and whatever value is read on that pin is then output to P0.2 (LED1).  An interrupt is set up on P2.7 (PB2). P0.3 (LED2) toggles when that interrupt occurs.

## Required Connections

-   Connect a USB cable between the PC and the CN1 (USB/PWR) connector.
-   Connect pins 1 and 2 (P0_1) of the JH1 (UART 0 EN) header.
-   Open an terminal application on the PC and connect to the EV kit's console UART at 115200, 8-N-1.
-   Close jumper JP1 (LED1 EN).
-   Close jumper JP2 (LED2 EN).

## Expected Output

The Console UART of the device will output these messages:

```
************************* GPIO Example ***********************

1. This example reads P2.6 and outputs the same state onto P0.2.
2. An interrupt is set up on P2.7 . P0.3 toggles when that
   interrupt occurs.
```

You will also observe the LED behavior given in the Description section above.