#include "capture_button.h"
#include "mxc_device.h"
#include <stdlib.h>
#include <stdio.h>
#include "camera_tft_funcs.h"
#include "sd_card_funcs.h"
#include "tft_fthr.h"
#include "camera.h"
#include "tmr_funcs.h"

#define TFT_BUFF_SIZE   50    // TFT buffer size
#define QUIT_IDX 5

#define CLOSE_THRESH 18 /// cm
#define FAR_THRESH 22 // cm
#define DEBOUNCE 5000 // 1 gc = 100us --> 1000 gc = 1sec

#define PAUSED 1
#define ON 0
// ========================================================================================= //
// =================================== GLOBAL VARIABLES ==================================== //
// ========================================================================================= //

// gpios for capturing images and changing directories
mxc_gpio_cfg_t capture_gpio;
mxc_gpio_cfg_t cd_gpio;

// isr callback data
const uint8_t cam_sensor_id = 0;
const uint8_t flipper_sensor_id = 1;
uint8_t volatile active = cam_sensor_id;

// state variables for ultrasonic sensors
uint32_t volatile current_pulse[] = {0,0}; // rising edge time
uint32_t volatile intervals[] = {100,100}; // pulse width in ticks, init to 100 to prevent false alarm on init
uint16_t volatile statuses[] = {0,0}; // state variable to track if object in front of sensor
uint32_t volatile timestamps[] = {0,0}; // debouncing
uint8_t volatile trigger_states[] = {0,0}; // state variable to track if a sensor needs to fire
uint8_t volatile capture_state = 0;
uint8_t volatile switch_state = 0;
uint8_t sensor_state = ON;

// class category names (directory names)
char* classes[] = {"Paper", "Metal", "Plastic", "Other","None"};

// number of images in each directory
uint16_t img_amnts[] = {0,0,0,0,0};

// the current class directory you are in
int class_idx = 0;

// buffer for displaying text to LCD
char buffer[TFT_BUFF_SIZE];
int font = (int)&Arial12x12[0];
area_t cover_text = {0, 280, 240, 30};

// file name for images captured
char file_prefix[8] = "img0000";

// file paths
char class1[] = "recycling_imgs/Paper/num_imgs";
char class2[] = "recycling_imgs/Metal/num_imgs";
char class3[] = "recycling_imgs/Plastic/num_imgs";
char class4[] = "recycling_imgs/Other/num_imgs";
char class5[] = "recycling_imgs/None/num_imgs";

// ========================================================================================= //
// ================================ FUNCTION DEFINITIONS =================================== //
// ========================================================================================= //

void pause_sensor()
{
    // wait until in a stable state (echo received) by checking that something needs to be triggered
    int sensor_idx = -1;
    while(sensor_idx == -1)
    {
        for(int i = 0; i < 2; i++)
        {
            if(trigger_states[i] == 1)
            {
                sensor_idx = i;
                break;
            }
        }
    }
    sensor_state = PAUSED;
}

void resume_sensor()
{
    sensor_state = ON;
}

int get_capture_state()
{
    if(capture_state == 1)
    {
        capture_state = 0;
        return 1;
    }
    return 0;
}

int get_switch_state()
{
    if(switch_state == 1)
    {
        switch_state = 0;
        return 1;
    }
    return 0;
}

void trigger_cam()
{
    MXC_GPIO_OutSet(MXC_GPIO2, MXC_GPIO_PIN_4);
    MXC_Delay(10);
    MXC_GPIO_OutClr(MXC_GPIO2, MXC_GPIO_PIN_4);
}

void trigger_flipper()
{
    MXC_GPIO_OutSet(MXC_GPIO3, MXC_GPIO_PIN_1);
    MXC_Delay(10);
    MXC_GPIO_OutClr(MXC_GPIO3, MXC_GPIO_PIN_1);
}

void switch_class()
{
    // store the x_coord of the currently selected item
    cover_text.x = 0;//class_idx*48;
    for(int i = 0; i < TFT_BUFF_SIZE; i++)
    { 
        buffer[i] = '\0';
    }

    // increment the button idx  
    class_idx++;

    // clear the last class text
    pause_sensor();
    MXC_TFT_FillRect(&cover_text,BLACK); // this is the bug, interrupts disabled
    resume_sensor();

    // go to quit button if get to idx 7
    if(class_idx == QUIT_IDX)
    {
        printf("Quit?\n");
        TFT_Print(buffer,96,240,font,sprintf(buffer,"QUIT?"));
    }
    else // otherwise go to the next class button
    {
        // if reseting
        if(class_idx == (QUIT_IDX+1))
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
        TFT_Print(buffer,class_idx*48,280,font,sprintf(buffer,classes[class_idx]));

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
    if(class_idx == QUIT_IDX)
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

        // confirm they are saved
        get_num_from_file(class1,&img_amnts[0]);
        get_num_from_file(class2,&img_amnts[1]);
        get_num_from_file(class3,&img_amnts[2]);
        get_num_from_file(class4,&img_amnts[3]);
        get_num_from_file(class5,&img_amnts[4]);

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
    cd("recycling_imgs");
    cd(classes[class_idx]);
    write_image(file_prefix);
    reset();
    TFT_Print(buffer,class_idx*48,280,font,sprintf(buffer,classes[class_idx]));
}

void echo_isr(void* sensor_id)
{
    // get the sensor idx from the callback data
    uint8_t sensor_idx = *(uint8_t*)(sensor_id);

    // don't allow nonactive sensors to triger interrupts
    // for example if there is interference between sensors
    if(sensor_idx != active)
    {
        return;
    }

    // first interrupt (rising edge)
    if(current_pulse[sensor_idx] == 0)
    {
        // store the start time
        current_pulse[sensor_idx] = global_counter;
        //printf("sensor %d ^\n",sensor_idx);
    }
    // second interrupt (falling edge)
    else
    {
        //printf("sensor %d _\n",sensor_idx);
        // store the end time, convert to cm, reset the start time
        intervals[sensor_idx] = (global_counter - current_pulse[sensor_idx])*100/58;
        current_pulse[sensor_idx] = 0;

        // no object in front of the sensor yet and it is within the threshold, trigger the arm to close
        if(!statuses[sensor_idx] && intervals[sensor_idx] < CLOSE_THRESH)
        {
            //printf("sensor %d present\n",sensor_idx);
            statuses[sensor_idx] = 1; 
            timestamps[sensor_idx] = global_counter;
            if(sensor_idx == cam_sensor_id)
            {
                capture_state = 1;
            }
            else if(sensor_idx == flipper_sensor_id)
            {
                switch_state = 1;
            }
        }
        else if(statuses[sensor_idx] && intervals[sensor_idx] < CLOSE_THRESH)
        {
            timestamps[sensor_idx] = global_counter;
        }
        // object in front of the sensor and beyond the threshold, update the state
        else if(statuses[sensor_idx] && intervals[sensor_idx] >= FAR_THRESH && ((global_counter-timestamps[sensor_idx]) > DEBOUNCE))
        {
            //printf("sensor %d left\n",sensor_idx);
            // reset the state
            statuses[sensor_idx] = 0;
        }

        // after receiving a response, tell the next sensor to trigger
        active += 1;
        if(active == 2)
        {
            active = 0;
        }
        trigger_states[active] = 1;
    }
}

void trigger()
{
    // check if any sensor needs to be triggered
    int sensor_idx = -1;
    for(int i = 0; i < 2; i++)
    {
        //printf("trig %d: %d\n", i, trigger_states[i]);
        if(trigger_states[i] == 1)
        {
            sensor_idx = i;
            break;
        }
    }
    // if none need to be triggered, return
    if(sensor_idx == -1)
    {
        return;
    }
    
    // trigger the corresponding sensor
    switch (sensor_idx)
    {
        case cam_sensor_id:
        {
            if(sensor_state == ON)
            {
                trigger_cam();
                trigger_states[cam_sensor_id] = 0;
            }
            break;
        }

        case flipper_sensor_id:
        {
            if(sensor_state == ON)
            {
                trigger_flipper();
                trigger_states[flipper_sensor_id] = 0;
            }
            break;
        }

        default:
        {
            break;
        }
    }
}


// First Ultrasonic
void init_capture_button()
{
    capture_gpio.port = MXC_GPIO1;
    capture_gpio.mask = MXC_GPIO_PIN_6;
    capture_gpio.func = MXC_GPIO_FUNC_IN;
    capture_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;

    MXC_GPIO_Config(&capture_gpio);
    MXC_GPIO_RegisterCallback(&capture_gpio, echo_isr, (void*)(&cam_sensor_id));
    MXC_GPIO_IntConfig(&capture_gpio, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(MXC_GPIO1, MXC_GPIO_PIN_6);
    NVIC_EnableIRQ(GPIO1_IRQn);
    NVIC_SetPriority(GPIO1_IRQn,0);
}

// Second Ultrasonic
void init_class_button()
{
    cd_gpio.port = MXC_GPIO1;
    cd_gpio.mask = MXC_GPIO_PIN_1;
    cd_gpio.func = MXC_GPIO_FUNC_IN;
    cd_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;

    MXC_GPIO_Config(&cd_gpio);
    MXC_GPIO_RegisterCallback(&cd_gpio, echo_isr, (void*)(&flipper_sensor_id));
    MXC_GPIO_IntConfig(&cd_gpio, MXC_GPIO_INT_BOTH);
    MXC_GPIO_EnableInt(MXC_GPIO1, MXC_GPIO_PIN_1);
    NVIC_EnableIRQ(GPIO1_IRQn);

    cd("recycling_imgs");
    ls();
    cd(classes[class_idx]);
    ls();

    get_num_from_file("../Paper/num_imgs",&img_amnts[0]);
    get_num_from_file("../Metal/num_imgs",&img_amnts[1]);
    get_num_from_file("../Plastic/num_imgs",&img_amnts[2]);
    get_num_from_file("../Other/num_imgs",&img_amnts[3]);
    get_num_from_file("../None/num_imgs",&img_amnts[4]);

    reset();
    TFT_Print(buffer,class_idx*48,280,font,sprintf(buffer,classes[class_idx]));
}

void init_triggers()
{
    mxc_gpio_cfg_t triggercam_gpio;
    mxc_gpio_cfg_t trigger0_gpio;

    // cam
    triggercam_gpio.port = MXC_GPIO2;
    triggercam_gpio.mask = MXC_GPIO_PIN_4;
    triggercam_gpio.func = MXC_GPIO_FUNC_OUT;
    triggercam_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&triggercam_gpio);

    // flipper 2
    trigger0_gpio.port = MXC_GPIO3;
    trigger0_gpio.mask = MXC_GPIO_PIN_1;
    trigger0_gpio.func = MXC_GPIO_FUNC_OUT;
    trigger0_gpio.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&trigger0_gpio);
}