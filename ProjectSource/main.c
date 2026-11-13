/****************************************************************************
 Module
     main.c
 Description
     starter main() function for Events and Services Framework applications
 Notes

*****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"

#include "../ProjectHeaders/DM_Display.h"
#include "../ProjectHeaders/PIC32_SPI_HAL.h"
#include "PWM_PIC32.h"
#include "AudioService.h"
#include "PIC32_AD_Lib.h"

static void blocking_tone(int pitch, int duration, int volume);

void main(void)
{
  ES_Return_t ErrorType = Success;

  _HW_PIC32Init(); // basic PIC hardware init
  // Your hardware initialization function calls go here
  // Initialize SPI peripheral
  SPISetup_BasicConfig(SPI_SPI1);
  SPISetup_SetLeader(SPI_SPI1, 0);
  SPISetup_SetBitTime(SPI_SPI1, 100); // 100ns = 10MHz clock
  SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA1);
  SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA2);
  SPISetup_SetClockIdleState(SPI_SPI1, 1);
  SPISetup_SetActiveEdge(SPI_SPI1, 0);
  SPISetup_SetXferWidth(SPI_SPI1, SPI_16BIT);
  SPISetEnhancedBuffer(SPI_SPI1, true);
  SPISetup_EnableSPI(SPI_SPI1);

  // 1. Configure RA0 physical pin as an analog input
  TRISAbits.TRISA0 = 1;   // Set RA0 as input
  ANSELAbits.ANSA0 = 1;   // Enable analog mode on RA0 (disable digital buffer)
    
  // 2. Configure the ADC using the HAL
  // The function expects a bitmask where bit 0 selects AN0
  ADC_ConfigAutoScan(1 << 0);

  // Force MAX7219 to clear its internal memory and shut off random LEDs
  DM_ClearDisplayBuffer();
  while (DM_TakeInitDisplayStep() == false) {
      // Blocks here for a fraction of a millisecond until all 4 modules are blanked
  }

  // now initialize the Events and Services Framework and start it running
  ErrorType = ES_Initialize(ES_Timer_RATE_1mS);
  if (ErrorType == Success)
  {
    // ES_Event_t audioEvent = {ES_AUDIO_CHOP, 0};

    // if (!PostAudioService(audioEvent))
    // {
    //     puts("Failed to post audio event");
    // }
    ErrorType = ES_Run();
  }
  //if we got to here, there was an error
  switch (ErrorType)
  {
    case FailedPost:
    {
      ErrorType = 0; // a hack to get a readable assert message
      assert( FailedPost == ErrorType );
      //DB_printf("Failed on attempt to Post\n");
    }
    break;
    case FailedPointer:
    {
      ErrorType = 0; // a hack to get a readable assert message
      assert( FailedPointer  == ErrorType );
//      DB_printf("Failed on NULL pointer\n");
    }
    break;
    case FailedInit:
    {
      ErrorType = 0; // a hack to get a readable assert message
      assert( FailedInit  == ErrorType );
//      DB_printf("Failed Initialization\n");
    }
    break;
    default:
    {
      ErrorType = 0; // a hack to get a readable assert message
      assert( FailedOther  == ErrorType );
//      DB_printf("Other Failure\n");
    }
    break;
  }
  for ( ; ;)
  {
    ;
  }
}

//define function to play a tone of duration
static void blocking_tone(int pitch, int duration, int volume)
{
  // Start with tone frequency
  PWMSetup_SetFreqOnTimer(pitch, _Timer2_);
  // set volume
  PWMOperate_SetDutyOnChannel(volume, 1);
  for (int i = 0; i < duration; i++) {
    // Wait for a while
  }
}