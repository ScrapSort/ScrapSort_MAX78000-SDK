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
  TicStepMode__Microstep8  = (uint8_t)3,
  TicStepMode__Microstep16 = (uint8_t)4,
  TicStepMode__Microstep32 = (uint8_t)5,
  TicStepMode__Microstep2__100p = (uint8_t)6,
  TicStepMode__Microstep64 = (uint8_t)7,
  TicStepMode__Microstep128 = (uint8_t)8,
  TicStepMode__Microstep256 = (uint8_t)9,
}TicStepMode;