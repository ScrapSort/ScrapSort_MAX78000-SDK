#include "capture_button.h"
#include "mxc_device.h"
#include <stdlib.h>
#include <stdio.h>
#include "camera_tft_funcs.h"
#include "sd_card_funcs.h"
#include "tft_fthr.h"
#include "camera.h"

#define TFT_BUFF_SIZE   50    // TFT buffer size

// ========================================================================================= //
// =================================== GLOBAL VARIABLES ==================================== //
// ========================================================================================= //

// gpios for capturing images and changing directories
mxc_gpio_cfg_t capture_gpio;
mxc_gpio_cfg_t cd_gpio;
volatile int capture_clicked = 0;
volatile int class_clicked = 0;

// dummy variable
char a = 'a';

// class category names (directory names)
char* classes[] = {"Cup", "Hex", "Trap", "Can","Bottle","None"};

// number of images in each directory
uint16_t img_amnts[] = {0,0,0,0,0,0};

// the current class directory you are in
int class_idx = 0;

// buffer for displaying text to LCD
char buff[TFT_BUFF_SIZE];
int font_1 = (int)&Arial12x12[0];
area_t cover_text = {0, 280, 240, 30};

// file name for images captured
char file_prefix[8] = "img0000";

// file paths
char class1[] = "sorting_imgs/Cup/num_imgs";
char class2[] = "sorting_imgs/Hex/num_imgs";
char class3[] = "sorting_imgs/Trap/num_imgs";
char class4[] = "sorting_imgs/Can/num_imgs";
char class5[] = "sorting_imgs/Bottle/num_imgs";
char class6[] = "sorting_imgs/None/num_imgs";

// ========================================================================================= //
// ================================ FUNCTION DEFINITIONS =================================== //
// ========================================================================================= //

void switch_class()
{
    // store the x_coord of the currently selected item
    cover_text.x = 0;//class_idx*48;
    for(int i = 0; i < TFT_BUFF_SIZE; i++)
    { 
        buff[i] = '\0';
    }

    // increment the button idx  
    class_idx++;

    // clear the last class text
    MXC_TFT_FillRect(&cover_text,BLACK);

    // go to quit button if get to idx 6
    if(class_idx == 6)
    {
        printf("Quit?\n");
        TFT_Print(buff,96,240,font_1,sprintf(buff,"QUIT?"));
    }
    else // otherwise go to the next class button
    {
        // if reseting
        if(class_idx == 7)
        {
            // clear the quit button
            class_idx = 0;
            cover_text.x = 96;
            cover_text.y = 240;
            MXC_TFT_FillRect(&cover_text,BLACK);
            cover_text.x = 0;
            cover_text.y = 280;
        }
        printf("class: %i\n",class_idx);
        TFT_Print(buff,class_idx*48,280,font_1,sprintf(buff,classes[class_idx]));

        // reset the file prefix to img0000
        file_prefix[3]='0';
        file_prefix[4]='0';
        file_prefix[5]='0';
        file_prefix[6]='0';
    }
}

void capture()
{
    // quit
    if(class_idx == 6)
    {
        printf("quit");
        // mount the SD card
        mount();

        // save image idxs
        num_to_file(class1,&img_amnts[0]);
        num_to_file(class2,&img_amnts[1]);
        num_to_file(class3,&img_amnts[2]);
        num_to_file(class4,&img_amnts[3]);
        num_to_file(class5,&img_amnts[4]);
        num_to_file(class6,&img_amnts[5]);

        // confirm they are saved
        get_num_from_file(class1,&img_amnts[0]);
        get_num_from_file(class2,&img_amnts[1]);
        get_num_from_file(class3,&img_amnts[2]);
        get_num_from_file(class4,&img_amnts[3]);
        get_num_from_file(class5,&img_amnts[4]);
        get_num_from_file(class6,&img_amnts[5]);

        // quit
        umount();
        reset();
        return;
    }
    
    // increment the class img amount
    img_amnts[class_idx] += 1;
    int n = img_amnts[class_idx];
    printf("capture: %i\n",n);
    int digit = 0;
    int end = 6;

    // save the img amount to the file prefix string
    do 
    {
        digit = n % 10;
        n /= 10;
        file_prefix[end] = 48+digit;
        end--;
    } while (n != 0);
    printf("file: %s\n",file_prefix);

    // go to the corresponding directory, save the image
    // mount the SD card
    mount();
    cd("sorting_imgs");
    cd(classes[class_idx]);
    write_image(file_prefix);
    reset();
    TFT_Print(buff,class_idx*48,280,font_1,sprintf(buff,classes[class_idx]));
}

int clicked()
{
    if(capture_clicked == 1)
    {
        capture_clicked = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

int switched()
{
    if(class_clicked == 1)
    {
        class_clicked = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

// This is the callback function (interrupt handler)
// that gets called when the capture button press is triggered.
void button_isr(void* action)
{
    capture_clicked = 1;
}


// This is the callback function (interrupt handler)
// that gets called when the class button press is triggered.
void button_isr2(void* action)
{
    class_clicked = 1;
}

// GPIO port 0 pin 2 is button 1 on the board
void init_capture_button()
{
    capture_gpio.port = MXC_GPIO2;
    capture_gpio.mask = MXC_GPIO_PIN_3;
    capture_gpio.pad = MXC_GPIO_PAD_PULL_UP; // HI by default
    capture_gpio.func = MXC_GPIO_FUNC_IN;
    capture_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;

    MXC_GPIO_Config(&capture_gpio);
    MXC_GPIO_RegisterCallback(&capture_gpio, button_isr, (void*)a);
    MXC_GPIO_IntConfig(&capture_gpio, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(MXC_GPIO2, MXC_GPIO_PIN_3);
    NVIC_EnableIRQ(GPIO2_IRQn);
}

// GPIO port 1 pin 7 is button 2 on the board
void init_class_button()
{
    cd_gpio.port = MXC_GPIO2;
    cd_gpio.mask = MXC_GPIO_PIN_4;
    cd_gpio.pad = MXC_GPIO_PAD_PULL_UP; // HI by default
    cd_gpio.func = MXC_GPIO_FUNC_IN;
    cd_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;

    MXC_GPIO_Config(&cd_gpio);
    MXC_GPIO_RegisterCallback(&cd_gpio, button_isr2, (void*)a);
    MXC_GPIO_IntConfig(&cd_gpio, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(MXC_GPIO2, MXC_GPIO_PIN_4);
    NVIC_EnableIRQ(GPIO2_IRQn);

    cd("sorting_imgs");
    ls();
    cd(classes[class_idx]);
    ls();

    get_num_from_file("../Cup/num_imgs",&img_amnts[0]);
    get_num_from_file("../Hex/num_imgs",&img_amnts[1]);
    get_num_from_file("../Trap/num_imgs",&img_amnts[2]);
    get_num_from_file("../Can/num_imgs",&img_amnts[3]);
    get_num_from_file("../Bottle/num_imgs",&img_amnts[4]);
    get_num_from_file("../None/num_imgs",&img_amnts[5]);

    reset();
    TFT_Print(buff,class_idx*48,280,font_1,sprintf(buff,classes[class_idx]));
}