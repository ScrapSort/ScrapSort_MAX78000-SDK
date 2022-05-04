#include "sd_card_funcs.h"
#include "camera.h"
#include "camera_tft_funcs.h"

/***** Definitions *****/

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define MAXLEN 256


// ========================================================================================= //
// =================================== GLOBAL VARIABLES ==================================== //
// ========================================================================================= //
FATFS* fs;      //FFat Filesystem Object
FATFS fs_obj;
FIL file;       //FFat File Object
FRESULT err;    //FFat Result (Struct)
FILINFO fno;    //FFat File Information Object
DIR dir;        //FFat Directory Object
TCHAR message[MAXLEN], directory[MAXLEN], cwd[MAXLEN], filename[MAXLEN], volume_label[24], volume = '0';
TCHAR* FF_ERRORS[20];
DWORD clusters_free = 0, sectors_free = 0, sectors_total = 0, volume_sn = 0;
UINT bytes_written = 0, bytes_read = 0, mounted = 0;
BYTE work[4096];
static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
mxc_gpio_cfg_t SDPowerEnablePin = {MXC_GPIO1, MXC_GPIO_PIN_12, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO};



// ========================================================================================= //
// ================================ FUNCTION DEFINITIONS =================================== //
// ========================================================================================= //

void generateMessage(unsigned length)
{
    for (int i = 0 ; i < length; i++) {
        /*Generate some random data to put in file*/
        message[i] =  charset[rand() % (sizeof(charset) - 1)];
    }
}

int mount()
{
    fs = &fs_obj;
    
    if ((err = f_mount(fs, "", 1)) != FR_OK) {          //Mount the default drive to fs now
        printf("Error opening SD card: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
    }
    else {
        printf("SD card mounted.\n");
        mounted = 1;
    }
    
    f_getcwd(cwd, sizeof(cwd));                         //Set the Current working directory
    
    return err;
}

int umount()
{
    if ((err = f_mount(NULL, "", 0)) != FR_OK) {        //Unmount the default drive from its mount point
        printf("Error unmounting volume: %s\n", FF_ERRORS[err]);
    }
    else {
        printf("SD card unmounted.\n");
        mounted = 0;
    }
    
    return err;
}

int formatSDHC()
{
    printf("\n\n*****THE DRIVE WILL BE FORMATTED IN 5 SECONDS*****\n");
    printf("**************PRESS ANY KEY TO ABORT**************\n\n");
    MXC_UART_ClearRXFIFO(MXC_UART0);
    MXC_Delay(MSEC(5000));
    
    if (MXC_UART_GetRXFIFOAvailable(MXC_UART0) > 0) {
        return E_ABORT;
    }
    
    printf("FORMATTING DRIVE\n");

    if ((err = f_mkfs("", FM_ANY, 0, work, sizeof(work))) != FR_OK) {    //Format the default drive to FAT32
        printf("Error formatting SD card: %s\n", FF_ERRORS[err]);
    }
    else {
        printf("Drive formatted.\n");
    }
    
    mount();
    
    if ((err = f_setlabel("MAXIM")) != FR_OK) {
        printf("Error setting drive label: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
    }
    
    umount();
    
    return err;
}

int getSize()
{
    if (!mounted) {
        mount();
    }
    
    if ((err = f_getfree(&volume, &clusters_free, &fs)) != FR_OK) {
        printf("Error finding free size of card: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
    }
    
    sectors_total = (fs->n_fatent - 2) * fs->csize;
    sectors_free = clusters_free * fs->csize;
    
    printf("Disk Size: %u bytes\n", sectors_total / 2);
    printf("Available: %u bytes\n", sectors_free / 2);
    
    return err;
}

int ls()
{
    if (!mounted) {
        mount();
    }
    
    printf("Listing Contents of %s - \n", cwd);
    
    if ((err = f_opendir(&dir, cwd)) == FR_OK) {
        while (1) {
            err = f_readdir(&dir, &fno);
            
            if (err != FR_OK || fno.fname[0] == 0) {
                break;
            }
            
            printf("%s/%s", cwd, fno.fname);
            
            if (fno.fattrib & AM_DIR) {
                printf("/");
            }
            
            printf("\n");
        }
        
        f_closedir(&dir);
    }
    else {
        printf("Error opening directory!\n");
        return err;
    }
    
    printf("\nFinished listing contents\n");
    
    return err;
}

int make_directory(TCHAR* dir)
{
    err = f_stat((const TCHAR*) dir, &fno);
    
    if (err == FR_NO_FILE) {
        printf("Creating directory...\n");
        
        if ((err = f_mkdir((const TCHAR*) dir)) != FR_OK) {
            printf("Error creating directory: %s\n", FF_ERRORS[err]);
            f_mount(NULL, "", 0);
            return err;
        }
        else {
            printf("Directory %s created.\n", dir);
        }
        
    }
    else {
        printf("Directory already exists.\n");
        return -1;
    }
}


int create_file(TCHAR* file_name)
{
    if ((err = f_open(&file, (const TCHAR*) file_name, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
        printf("Error opening file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }

    if ((err = f_close(&file)) != FR_OK) {
        printf("Error closing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
}


int init_card()
{
    int status;
    if (!mounted) {
        mount();
    }

    printf("Initializing SD Card \n");

    sprintf(directory,"recycling_imgs");
    status = make_directory(directory);
    
    if(status < 0)
    {
        printf("SD card already initialized\n");
        return status;
    }
    else
    {
        // make directories for object categories
        sprintf(directory,"recycling_imgs/Paper");
        sprintf(filename,"recycling_imgs/Paper/num_imgs");
        make_directory(directory);
        create_file(filename);

        sprintf(directory,"recycling_imgs/Metal");
        sprintf(filename,"recycling_imgs/Metal/num_imgs");
        make_directory(directory);
        create_file(filename);

        sprintf(directory,"recycling_imgs/Plastic");
        sprintf(filename,"recycling_imgs/Plastic/num_imgs");
        make_directory(directory);
        create_file(filename);

        sprintf(directory,"recycling_imgs/Other");
        sprintf(filename,"recycling_imgs/Other/num_imgs");
        make_directory(directory);
        create_file(filename);

        sprintf(directory,"recycling_imgs/None");
        sprintf(filename,"recycling_imgs/None/num_imgs");
        make_directory(directory);
        create_file(filename);
    }
    printf("Finished Initializing SD Card \n");
    return status;
}

int write_image(TCHAR* file_name)
{
    if (!mounted) {
        mount();
    }

    uint8_t   *raw;
	uint32_t  imgLen;
	uint32_t  w, h;
    unsigned int length = 128;
    camera_start_capture_image();
    camera_get_image(&raw, &imgLen, &w, &h);
    
    
    if ((err = f_open(&file, (const TCHAR*) file_name, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
        printf("Error opening file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("File opened!\n");
    printf("file size: %d\n",f_size(&file));
    printf("img len:%i\n",imgLen);

    // write 4096 bytes at a time until hit imgLen
    for(int i = 0; i < imgLen; i+=4096,raw+=4096)
    {
        if ((err = f_write(&file, raw, 4096, &bytes_written)) != FR_OK) {
            printf("Error writing file: %s\n", FF_ERRORS[err]);
            f_mount(NULL, "", 0);
            return err;
        }
        //printf("%d bytes written to file!\n", bytes_written);
    }

    //printf("file size: %d\n",f_size(&file));
    
    if ((err = f_close(&file)) != FR_OK) {
        printf("Error closing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    //printf("file size: %d\n",f_size(&file));
    printf("File Closed!\n");

    return err;

}

int num_to_file(TCHAR* file_name, uint16_t *data)
{
    if (!mounted) {
        mount();
    }
    if ((err = f_open(&file, (const TCHAR*) file_name, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
        printf("Error opening file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("File opened!\n");

    if ((err = f_write(&file, data, 2, &bytes_written)) != FR_OK) {
        printf("Error writing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("%d bytes written to file!\n", bytes_written);
    
    if ((err = f_close(&file)) != FR_OK) {
        printf("Error closing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("File Closed!\n");
    return err;
}

int get_num_from_file(TCHAR* file_name, uint16_t *data)
{
    printf("Attempting to read back file...\n");
    
    if ((err = f_open(&file, file_name, FA_READ)) != FR_OK) {
        printf("Error opening file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    if ((err = f_read(&file, data, 2, &bytes_read)) != FR_OK) {
        printf("Error reading file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("Read Back %d bytes\n", bytes_read);
    printf("Message: ");
    printf("%u\n", *data);
    printf("\n");

    if ((err = f_close(&file)) != FR_OK) {
        printf("Error closing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("File Closed!\n----------------------\n");
    return err;
}

void test_drive()
{
    uint16_t num = 0;
    mount();

    // if ((err = f_getlabel(&volume, volume_label, &volume_sn)) != FR_OK) {
    //     printf("Error reading drive label: %s\n", FF_ERRORS[err]);
    //     f_mount(NULL, "", 0);
    //     return err;
    // }
    
    // num_to_file("test.txt", &num);
    // get_num_from_file("test.txt",&num);
    // umount();

    // mount();
    // num = 0;
    get_num_from_file("test.txt",&num);
}

int createFile()
{

    unsigned int length = 128;
    
    if (!mounted) {
        mount();
    }
    
    printf("Enter the name of the text file: \n");
    scanf("%255s", filename);
    printf("Enter the length of the file: (256 max)\n");
    scanf("%d", &length);
    printf("Creating file %s with length %d\n", filename, length);
    
    if ((err = f_open(&file, (const TCHAR*) filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
        printf("Error opening file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("File opened!\n");
    
    generateMessage(length);
    
    if ((err = f_write(&file, &message, length, &bytes_written)) != FR_OK) {
        printf("Error writing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("%d bytes written to file!\n", bytes_written);
    
    if ((err = f_close(&file)) != FR_OK) {
        printf("Error closing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("File Closed!\n");
    return err;
}

int appendFile()
{
    unsigned int length = 0;
    
    if (!mounted) {
        mount();
    }
    
    printf("Type name of file to append: \n");
    scanf("%255s", filename);
    printf("Type length of random data to append: \n");
    scanf("%d", &length);
    
    if ((err = f_stat((const TCHAR*) filename, &fno)) == FR_NO_FILE) {
        printf("File %s doesn't exist!\n", (const TCHAR*) filename);
        return err;
    }
    
    if ((err = f_open(&file, (const TCHAR*) filename, FA_OPEN_APPEND | FA_WRITE)) != FR_OK) {
        printf("Error opening file %s\n", FF_ERRORS[err]);
        return err;
    }
    
    printf("File opened!\n");
    
    generateMessage(length);
    
    if ((err = f_write(&file, &message, length, &bytes_written)) != FR_OK) {
        printf("Error writing file: %s\n", FF_ERRORS[err]);
        return err;
    }
    
    printf("%d bytes written to file\n", bytes_written);
    
    if ((err = f_close(&file)) != FR_OK) {
        printf("Error closing file: %s\n", FF_ERRORS[err]);
        return err;
    }
    
    printf("File closed.\n");
    return err;
}

int mkdir()
{
    if (!mounted) {
        mount();
    }
    
    printf("Enter directory name: \n");
    scanf("%255s", directory);
    
    err = f_stat((const TCHAR*) directory, &fno);
    
    if (err == FR_NO_FILE) {
        printf("Creating directory...\n");
        
        if ((err = f_mkdir((const TCHAR*) directory)) != FR_OK) {
            printf("Error creating directory: %s\n", FF_ERRORS[err]);
            f_mount(NULL, "", 0);
            return err;
        }
        else {
            printf("Directory %s created.\n", directory);
        }
        
    }
    else {
        printf("Directory already exists.\n");
    }
    
    return err;
}

int cd(TCHAR* directory)
{
    if (!mounted) {
        mount();
    }
    
    printf("Directory to change into: %s\n", directory);
    //scanf("%255s", directory);
    
    if ((err = f_stat((const TCHAR*) directory, &fno)) == FR_NO_FILE) {
        printf("Directory doesn't exist (Did you mean mkdir?)\n");
        return err;
    }
    
    if ((err = f_chdir((const TCHAR*) directory)) != FR_OK) {
        printf("Error in chdir: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("Changed to %s\n", directory);
    f_getcwd(cwd, sizeof(cwd));
    
    return err;
}

int delete_file()
{
    if (!mounted) {
        mount();
    }
    
    printf("File or directory to delete (always recursive!)\n");
    scanf("%255s", filename);
    
    if ((err = f_stat((const TCHAR*) filename, &fno)) == FR_NO_FILE) {
        printf("File or directory doesn't exist\n");
        return err;
    }
    
    if ((err = f_unlink(filename)) != FR_OK) {
        printf("Error deleting file\n");
        return err;
    }
    
    printf("Deleted file %s\n", filename);
    return err;
    
}

int example()
{
    unsigned int length = 256;
    
    if ((err = formatSDHC()) != FR_OK) {
        printf("Error Formatting SD Card: %s\n", FF_ERRORS[err]);
        return err;
    }
    
    //open SD Card
    if ((err = mount()) != FR_OK) {
        printf("Error opening SD Card: %s\n", FF_ERRORS[err]);
        return err;
    }
    
    printf("SD Card Opened!\n");
    
    if ((err = f_setlabel("MAXIM")) != FR_OK) {
        printf("Error setting drive label: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    if ((err = f_getfree(&volume, &clusters_free, &fs)) != FR_OK) {
        printf("Error finding free size of card: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    if ((err = f_getlabel(&volume, volume_label, &volume_sn)) != FR_OK) {
        printf("Error reading drive label: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    if ((err = f_open(&file, "0:HelloWorld.txt", FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
        printf("Error opening file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("File opened!\n");
    
    generateMessage(length);
    
    if ((err = f_write(&file, &message, length, &bytes_written)) != FR_OK) {
        printf("Error writing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("%d bytes written to file!\n", bytes_written);
    
    if ((err = f_close(&file)) != FR_OK) {
        printf("Error closing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("File Closed!\n");
    
    if ((err = f_chmod("HelloWorld.txt", 0, AM_RDO | AM_ARC | AM_SYS | AM_HID)) != FR_OK) {
        printf("Error in chmod: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    err = f_stat("MaximSDHC", &fno);
    
    if (err == FR_NO_FILE) {
        printf("Creating Directory...\n");
        
        if ((err = f_mkdir("MaximSDHC")) != FR_OK) {
            printf("Error creating directory: %s\n", FF_ERRORS[err]);
            f_mount(NULL, "", 0);
            return err;
        }
    }
    
    printf("Renaming File...\n");
    
    if ((err = f_rename("0:HelloWorld.txt", "0:MaximSDHC/HelloMaxim.txt")) != FR_OK) {   //cr: clearify 0:file notation
        printf("Error moving file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    if ((err = f_chdir("/MaximSDHC")) != FR_OK) {
        printf("Error in chdir: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("Attempting to read back file...\n");
    
    if ((err = f_open(&file, "HelloMaxim.txt", FA_READ)) != FR_OK) {
        printf("Error opening file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    if ((err = f_read(&file, &message, bytes_written, &bytes_read)) != FR_OK) {
        printf("Error reading file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("Read Back %d bytes\n", bytes_read);
    printf("Message: ");
    printf("%s", message);
    printf("\n");
    
    if ((err = f_close(&file)) != FR_OK) {
        printf("Error closing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }
    
    printf("File Closed!\n");
    
    //unmount SD Card
    //f_mount(fs, "", 0);
    if ((err = f_mount(NULL, "", 0)) != FR_OK) {
        printf("Error unmounting volume: %s\n", FF_ERRORS[err]);
        return err;
    }
    
    return 0;
}