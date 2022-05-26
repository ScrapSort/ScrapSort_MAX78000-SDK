#ifndef I2C_FUNCS_H
#define I2C_FUNCS_H


#define I2C_MASTER      MXC_I2C1

//Original freq
// #define I2C_FREQ        100000

// #define I2C_FREQ        50000 //New freq after vector board
#define I2C_FREQ        100000 //New freq after vector board
#define I2C_BYTES       16

#define NUM_SLAVES 3
#define START_SLAVE_ADDR 0

extern volatile uint8_t txdata[I2C_BYTES]; //was static 
extern volatile uint8_t rxdata[I2C_BYTES]; //was static 

void I2C_Callback(mxc_i2c_req_t* req, int error);
void fill_tx_32b(uint32_t num);
void printTransaction(int slave_addr, int tx_len, int rx_len);
void printData(void);
int I2C_Init();
int I2C_Broadcast_Message(int tx_len, int rx_len, int restart);
int I2C_Send_Message(int slave_addr, int tx_len, int rx_len, int restart);

#endif