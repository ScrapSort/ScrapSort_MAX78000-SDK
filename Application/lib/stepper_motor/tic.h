#ifndef TIC_H
#define TIC_H

#include <stdint.h>

typedef enum TicCommand
{
  TicCommand__SetTargetPosition                 = 0xE0,
  TicCommand__SetTargetVelocity                 = 0xE3,
  TicCommand__HaltAndSetPosition                = 0xEC,
  TicCommand__HaltAndHold                       = 0x89,
  TicCommand__GoHome                            = 0x97,
  TicCommand__ResetCommandTimeout               = 0x8C,
  TicCommand__Deenergize                        = 0x86,
  TicCommand__Energize                          = 0x85,
  TicCommand__ExitSafeStart                     = 0x83,
  TicCommand__EnterSafeStart                    = 0x8F,
  TicCommand__Reset                             = 0xB0,
  TicCommand__ClearDriverError                  = 0x8A,
  TicCommand__SetSpeedMax                       = 0xE6,
  TicCommand__SetStartingSpeed                  = 0xE5,
  TicCommand__SetAccelMax                       = 0xEA,
  TicCommand__SetDecelMax                       = 0xE9,
  TicCommand__SetStepMode                       = 0x94,
  TicCommand__SetCurrentLimit                   = 0x91,
  TicCommand__SetDecayMode                      = 0x92,
  TicCommand__SetAgcOption                      = 0x98,
  TicCommand__GetVariable                       = 0xA1,
  TicCommand__GetVariableAndClearErrorsOccurred = 0xA2,
  TicCommand__GetSetting                        = 0xA8,
}TicCommand;



typedef enum TicStepMode
{
  TicStepMode__Full    = (uint8_t)0,
  TicStepMode__Half    = (uint8_t)1,

  TicStepMode__Microstep1  = (uint8_t)0,
  TicStepMode__Microstep2  = (uint8_t)1,
  TicStepMode__Microstep4  = (uint8_t)2,
  TicStepMode__Microstep8  = (uint8_t)3
//   TicStepMode__Microstep16 = (uint8_t)4,
//   TicStepMode__Microstep32 = (uint8_t)5,
//   TicStepMode__Microstep2__100p = (uint8_t)6,
//   TicStepMode__Microstep64 = (uint8_t)7,
//   TicStepMode__Microstep128 = (uint8_t)8,
//   TicStepMode__Microstep256 = (uint8_t)9,
}TicStepMode;


typedef enum TicVarOffset
{
    TicVarOffset__OperationState        = 0x00, // uint8_t
    TicVarOffset__MiscFlags1            = 0x01, // uint8_t
    TicVarOffset__ErrorStatus           = 0x02, // uint16_t
    TicVarOffset__ErrorsOccurred        = 0x04, // uint32_t
    TicVarOffset__PlanningMode          = 0x09, // uint8_t
    TicVarOffset__TargetPosition        = 0x0A, // int32_t
    TicVarOffset__TargetVelocity        = 0x0E, // int32_t
    TicVarOffset__StartingSpeed         = 0x12, // uint32_t
    TicVarOffset__SpeedMax              = 0x16, // uint32_t
    TicVarOffset__DecelMax              = 0x1A, // uint32_t
    TicVarOffset__AccelMax              = 0x1E, // uint32_t
    TicVarOffset__CurrentPosition       = 0x22, // int32_t
    TicVarOffset__CurrentVelocity       = 0x26, // int32_t
    TicVarOffset__ActingTargetPosition  = 0x2A, // int32_t
    TicVarOffset__TimeSinceLastStep     = 0x2E, // uint32_t
    TicVarOffset__DeviceReset           = 0x32, // uint8_t
    TicVarOffset__VinVoltage            = 0x33, // uint16_t
    TicVarOffset__UpTime                = 0x35, // uint32_t
    TicVarOffset__EncoderPosition       = 0x39, // int32_t
    TicVarOffset__RCPulseWidth          = 0x3D, // uint16_t
    TicVarOffset__AnalogReadingSCL      = 0x3F, // uint16_t
    TicVarOffset__AnalogReadingSDA      = 0x41, // uint16_t
    TicVarOffset__AnalogReadingTX       = 0x43, // uint16_t
    TicVarOffset__AnalogReadingRX       = 0x45, // uint16_t
    TicVarOffset__DigitalReadings       = 0x47, // uint8_t
    TicVarOffset__PinStates             = 0x48, // uint8_t
    TicVarOffset__StepMode              = 0x49, // uint8_t
    TicVarOffset__CurrentLimit          = 0x4A, // uint8_t
    TicVarOffset__DecayMode             = 0x4B, // uint8_t
    TicVarOffset__InputState            = 0x4C, // uint8_t
    TicVarOffset__InputAfterAveraging   = 0x4D, // uint16_t
    TicVarOffset__InputAfterHysteresis  = 0x4F, // uint16_t
    TicVarOffset__InputAfterScaling     = 0x51, // uint16_t
    TicVarOffset__LastMotorDriverError  = 0x55, // uint8_t
    TicVarOffset__AgcMode               = 0x56, // uint8_t
    TicVarOffset__AgcBottomCurrentLimit = 0x57, // uint8_t
    TicVarOffset__AgcCurrentBoostSteps  = 0x58, // uint8_t
    TicVarOffset__AgcFrequencyLimit     = 0x59, // uint8_t
    TicVarOffset__LastHpDriverErrors    = 0xFF, // uint8_t
    TicVarOffset__HomingSpeedTowards    = 0x61, //uint32_t
    TicVarOffset__HomingSpeedAway       = 0x65, //uint32_t
}TicVarOffset;


#endif