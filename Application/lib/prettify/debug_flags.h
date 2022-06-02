/**
 * Debug flags to toggle on/off print statements 
 **/

// #define DEBUG_ALL

// #define DEBUG_MOTORS
// #define DEBUG_CNN
// #define DEBUG_LCD
// #define DEBUG_SORTER
#define DEBUG_HEARTBEAT
// #define DEBUG_...

#ifdef DEBUG_ALL
    #define DEBUG_MOTORS
    #define DEBUG_CNN
    #define DEBUG_HEARTBEAT
    // #define DEBUG_...
#endif



