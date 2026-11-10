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

#include "../ProjectHeaders/PIC32_SPI_HAL.h"
#include "PWM_PIC32.h"
#include "AudioService.h"

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
  SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA0);
  SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA1);
  SPISetup_SetClockIdleState(SPI_SPI1, 1);
  SPISetup_SetActiveEdge(SPI_SPI1, 0);
  SPISetup_SetXferWidth(SPI_SPI1, SPI_16BIT);
  SPISetEnhancedBuffer(SPI_SPI1, true);
  SPISetup_EnableSPI(SPI_SPI1);
  
  // PWM setup in audioservice.c

  
  /*************************************************************** */

  // now initialize the Events and Services Framework and start it running
ErrorType = ES_Initialize(ES_Timer_RATE_1mS);

if (ErrorType == Success)
{
    ES_Event_t audioEvent = {ES_AUDIO, 0};

    if (!PostAudioService(audioEvent))
    {
        puts("Failed to post audio event");
    }

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