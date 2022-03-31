## Description

This example uses the I2C Master to read/write from/to the I2C Slave. 

'****' Note '****': This example is not compatible with the Featherboard EV Kit. This is due to a lack of exposed I2C pins needed for multiple I2C peripheral instances to communicate with one another, a necessary condition for this test. 

## Setup

##### Required Connections:

-   Connect a USB cable between the PC and the CN1 (USB/PWR) connector.
-   Connect pins 1 and 2 (P0_1) of the JH1 (UART 0 EN) header.
-   Open a terminal application on the PC and connect to the EV kit's console UART at 115200, 8-N-1.
- 	Connect pins P0.10 to P0.16 (SCL) and P0.11 to P0.17 (SDA)

## Expected Output

The Console UART of the device will output these messages:

```

```