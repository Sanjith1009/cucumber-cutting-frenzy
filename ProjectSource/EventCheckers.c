/****************************************************************************
 Module
   EventCheckers.c

 Revision
   1.0.1

 Description
   This is the sample for writing event checkers along with the event
   checkers used in the basic framework test harness.

 Notes
   Note the use of static variables in sample event checker to detect
   ONLY transitions.

 History
 When           Who     What/Why
 -------------- ---     --------
 08/06/13 13:36 jec     initial version
****************************************************************************/
#include <stdlib.h>
// this will pull in the symbolic definitions for events, which we will want
// to post in response to detecting events
#include "ES_Configure.h"
// This gets us the prototype for ES_PostAll
#include "ES_Framework.h"
// this will get us the structure definition for events, which we will need
// in order to post events in response to detecting events
#include "ES_Events.h"
// if you want to use distribution lists then you need those function
// definitions too.
#include "ES_PostList.h"
// This include will pull in all of the headers from the service modules
// providing the prototypes for all of the post functions
#include "ES_ServiceHeaders.h"
// this test harness for the framework references the serial routines that
// are defined in ES_Port.c
#include "ES_Port.h"
// include our own prototypes to insure consistency between header &
// actual functionsdefinition
#include "EventCheckers.h"
#include "PIC32_AD_Lib.h"
#include "Potentiometer.h"
#include "LEDService.h"
#include "SerialUART.h"
// This is the event checking function sample. It is not intended to be
// included in the module. It is only here as a sample to guide you in writing
// your own event checkers
#if 0
/****************************************************************************
 Function
   Check4Lock
 Parameters
   None
 Returns
   bool: true if a new event was detected
 Description
   Sample event checker grabbed from the simple lock state machine example
 Notes
   will not compile, sample only
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
bool Check4Lock(void)
{
  static uint8_t  LastPinState = 0;
  uint8_t         CurrentPinState;
  bool            ReturnVal = false;

  CurrentPinState = LOCK_PIN;
  // check for pin high AND different from last time
  // do the check for difference first so that you don't bother with a test
  // of a port/variable that is not going to matter, since it hasn't changed
  if ((CurrentPinState != LastPinState) &&
      (CurrentPinState == LOCK_PIN_HI)) // event detected, so post detected event
  {
    ES_Event ThisEvent;
    ThisEvent.EventType   = ES_LOCK;
    ThisEvent.EventParam  = 1;
    // this could be any of the service post functions, ES_PostListx or
    // ES_PostAll functions
    ES_PostAll(ThisEvent);
    ReturnVal = true;
  }
  LastPinState = CurrentPinState; // update the state for next time

  return ReturnVal;
}

#endif
static uint32_t LastButtonState = 1;

// Potentiometer difficulty selection (mirrors GameFSM's UpdateDifficulty)
#define POT_CHECK_INTERVAL_MS 50u   // match GameFSM's 50 ms update rate
#define NUM_DIFFICULTY_LEVELS 5u

static uint8_t  LastDifficulty   = 0u;  // 0 = "no level yet", forces first post
static uint16_t LastPotCheckTime = 0u;

// Keyboard simulation of the handle limit switch: 'h' = touch, 'r' = release
#define HANDLE_TOUCH_KEY   'h'
#define HANDLE_RELEASE_KEY 'r'

static bool KeyHandleHeld = false;   // simulated switch state

// Handle limit switch (not wired yet). Change these to move it to another pin.
// RB13 is currently unused in this project.
#define LIMIT_SWITCH_PORT      PORTBbits.RB13
#define LIMIT_SWITCH_TRIS      TRISBbits.TRISB13
#define LIMIT_SWITCH_ANSEL     ANSELBbits.ANSB13
#define LIMIT_SWITCH_PULLUP    CNPUBbits.CNPUB13
#define LIMIT_SWITCH_SAMPLE_MS 10u   // debounce sample period
/****************************************************************************
 Function
   Check4Keystroke
 Parameters
   None
 Returns
   bool: true if a new key was detected & posted
 Description
   checks to see if a new key from the keyboard is detected and, if so,
   retrieves the key and posts an ES_NewKey event to TestHarnessService0
 Notes
   The functions that actually check the serial hardware for characters
   and retrieve them are assumed to be in ES_Port.c
   Since we always retrieve the keystroke when we detect it, thus clearing the
   hardware flag that indicates that a new key is ready this event checker
   will only generate events on the arrival of new characters, even though we
   do not internally keep track of the last keystroke that we retrieved.
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
bool Check4Keystroke(void)
{
  // Read keys from UART2 (SerialUART: RX on RB8, 9600 baud), the same
  // nonblocking read the GameFSM test loop used.
  char key;
  if (SerialUART_ReadChar(&key))   // new key waiting?
  {
    ES_Event_t ThisEvent;
    ThisEvent.EventParam = 0;

    if (key == HANDLE_TOUCH_KEY)
    {
      if (KeyHandleHeld) { return false; }   // already touched, ignore repeats
      KeyHandleHeld = true;
      ThisEvent.EventType = ES_HANDLE_TOUCH;
      SerialUART_WriteString("Handle TOUCH (key)\r\n");
    }
    else if (key == HANDLE_RELEASE_KEY)
    {
      if (!KeyHandleHeld) { return false; }  // already released, ignore
      KeyHandleHeld = false;
      ThisEvent.EventType = ES_HANDLE_RELEASE;
      SerialUART_WriteString("Handle RELEASE (key)\r\n");
    }
    else
    {
      ThisEvent.EventType  = ES_NEW_KEY;     // all other keys unchanged
      ThisEvent.EventParam = key;
    }

    ES_PostAll(ThisEvent);
    return true;
  }
  return false;
}

/****************************************************************************
 Function
   Check4LimitSwitch
 Description
   Hardware version of the 'h'/'r' handle keys. Posts ES_HANDLE_TOUCH when
   the limit switch closes and ES_HANDLE_RELEASE when it opens.
 Notes
   Wiring: switch between LIMIT_SWITCH pin and GND (active low). The internal
   pull-up holds the pin high while the switch is open or not wired yet, so
   this checker posts nothing until a switch is connected and pressed.
   Sampled every LIMIT_SWITCH_SAMPLE_MS to debounce mechanical bounce.
****************************************************************************/
bool Check4LimitSwitch(void)
{
  static bool     Initialized     = false;
  static uint8_t  LastSwitchState = 1u;   // 1 = open (released)
  static uint16_t LastSampleTime  = 0u;

  // One-time pin setup on first call: digital input with pull-up
  if (!Initialized)
  {
    LIMIT_SWITCH_ANSEL = 0;
    LIMIT_SWITCH_TRIS  = 1;
    LIMIT_SWITCH_PULLUP = 1;
    LastSwitchState = LIMIT_SWITCH_PORT;  // start from current state, no false edge
    LastSampleTime  = ES_Timer_GetTime();
    Initialized = true;
    return false;
  }

  // Debounce: only sample every LIMIT_SWITCH_SAMPLE_MS
  uint16_t now = ES_Timer_GetTime();
  if ((uint16_t)(now - LastSampleTime) < LIMIT_SWITCH_SAMPLE_MS)
  {
    return false;
  }
  LastSampleTime = now;

  uint8_t CurrentSwitchState = LIMIT_SWITCH_PORT;
  if (CurrentSwitchState == LastSwitchState)
  {
    return false;
  }
  LastSwitchState = CurrentSwitchState;

  ES_Event_t ThisEvent;
  ThisEvent.EventParam = 0;
  if (CurrentSwitchState == 0u)            // went LOW: switch closed
  {
    ThisEvent.EventType = ES_HANDLE_TOUCH;
    printf("Handle TOUCH (switch)\r\n");
  }
  else                                     // went HIGH: switch opened
  {
    ThisEvent.EventType = ES_HANDLE_RELEASE;
    printf("Handle RELEASE (switch)\r\n");
  }
  ES_PostAll(ThisEvent);
  return true;
}

bool Check4ChopButton(void)
{
  bool returnVal = false;
    
    // 1. Read the physical hardware pin (Replace with your actual PIC32 port read)
    uint8_t CurrentButtonState = PORTBbits.RB9; 

    // 2. Check if the state has changed since the last loop
    if (CurrentButtonState != LastButtonState)
    {
        returnVal = true; // Tell the framework we found an event
        ES_Event_t NewEvent;
        
        // 3. Determine if it was a press or release
        if (CurrentButtonState == 0) // Button went LOW (pressed)
        {
            NewEvent.EventType = ES_CHOP; // need to update when full funcitonality finished
            NewEvent.EventParam = 0; // You could pass port data here if needed
        
            // 4. Post the event. You can post to a specific service, or broadcast it to all
            ES_PostAll(NewEvent);
        }
        else // Button went HIGH (released)
        {
           
        }
        
        
    }
    
    // 5. Update the static variable for the next time the framework calls this function
    LastButtonState = CurrentButtonState;
    
    return returnVal;
}

bool PotentiometerRead(void)
{
  bool returnVal = false;
  uint16_t now = ES_Timer_GetTime();

  // 1. Only sample every 50 ms (unsigned subtraction handles timer rollover)
  if ((uint16_t)(now - LastPotCheckTime) < POT_CHECK_INTERVAL_MS)
  {
    return false;
  }
  LastPotCheckTime = now;

  // 2. Read pot (0-1023) and convert to difficulty level 1-5 (same as GameFSM)
  uint16_t potValue = ReadPotentiometer();
  uint8_t  newDifficulty =
      1u + (uint8_t)(((uint32_t)potValue * NUM_DIFFICULTY_LEVELS) / 1024u);

  // 3. Post only when the difficulty level changes
  if (newDifficulty != LastDifficulty)
  {
    LastDifficulty = newDifficulty;

    ES_Event_t newEvent;
    newEvent.EventType  = ES_POTENTIOMETER;
    newEvent.EventParam = newDifficulty;   // difficulty level 1-5
    printf("Pot ADC: %u | Difficulty: %u / %u\r\n",
           potValue, newDifficulty, NUM_DIFFICULTY_LEVELS);
    PostSetupService(newEvent);
    returnVal = true;
  }

  return returnVal;
}

bool CheckLEDAnimation(void)
{
  LEDService_Update();
  return false; // This function doesn't generate events, it just updates the LED service
}