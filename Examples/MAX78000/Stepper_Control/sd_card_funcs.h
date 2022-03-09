// This file defines the functions for interacting with the SD card

#ifndef SD_CARD_FUNCS
#define SD_CARD_FUNCS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mxc_device.h>
#include <gpio.h>
#include <uart.h>
#include <mxc_delay.h>

#include "ff.h"


/*
    Description: This function generates a random message
    Parameters: desired length of message (max 256)
    Return: none
*/
void generateMessage(unsigned length);


/*
    Description: This function mounts the SD card
    Parameters: None
    Return: error code
*/
int mount();


/*
    Description: This function unmounts the SD card
    Parameters: None
    Return: error code
*/
int umount();


/*
    Description: This function formats the SD card
    Parameters: None
    Return: error code
*/
int formatSDHC();


/*
    Description: This function prints the size and available space
    Parameters: None
    Return: error code
*/
int getSize();

/*
    Description: This function lists the items in the current directory
    Parameters: None
    Return: error code
*/
int ls();


/*
    Description: This function creates a directory in the cwd from the passed in string
    Parameters: string
    Return: error code
*/
int make_directory(TCHAR* directory);


/*
    Description: This function creates a file in the cwd from the passed in string
    Parameters: string
    Return: error code
*/
int create_file(TCHAR* file_name);


/*
    Description: This function initializes the SD card with a directory structure
    Parameters: None
    Return: error code
*/
int init_card();


/*
    Description: This function captures and writes an image to the SD card
    Parameters: None
    Return: error code
*/
int write_image(TCHAR* file_name);


/*
    Description: This function creates a file in the current directory and writes a random message to it
    Parameters: None
    Return: error code
*/
int createFile();


/*
    Description: This function writes a 16 bit int to a file. This is used to save the
                 index of the number of images captured for file naming.
    Parameters: The address of the 16 bit int to write to the file
    Return: error code
*/
int num_to_file(TCHAR* file_name, uint16_t *data);


/*
    Description: This function reads a 16 bit from the file (reads two bytes).
    Parameters: The address of the 16 bit that will get filled with the data
    Return: error code
*/
int get_num_from_file(TCHAR* file_name, uint16_t *data);


/*
    Description: This function is used to test SD card functionality
    Parameters: None
    Return: error code
*/
void test_drive();


/*
    Description: This function appends a random message to the specified file
    Parameters: None
    Return: error code
*/
int appendFile();


/*
    Description: This function creates a directory
    Parameters: None
    Return: error code
*/
int mkdir();


/*
    Description: This function changes the current directory
    Parameters: None
    Return: error code
*/
int cd(TCHAR* directory);

/*
    Description: This function deletes (recursively) a file or directory
    Parameters: None
    Return: error code
*/
int delete_file();


/*
    Description: This function runs an example to test that the SD card is working properly
    Parameters: None
    Return: error code
*/
int example();

#endif