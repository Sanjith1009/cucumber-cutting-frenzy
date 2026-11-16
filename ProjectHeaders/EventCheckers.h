#ifndef EVENT_CHECKERS_H
#define EVENT_CHECKERS_H

#include <stdbool.h>

// All checker names match Sanjith's ES_Configure.h EVENT_CHECK_LIST.
bool Check4Keystroke(void);
bool Check4LimitSwitch(void);
bool Check4ChopButton(void);
bool PotentiometerRead(void);
bool CheckLEDAnimation(void);
bool Check4BeamUnbroken(void);
bool Check4CapacitiveHandle(void);

// Direct sensor read used by PlayService when ES_PLAY begins.
bool IsBeamUnbroken(void);

#endif // EVENT_CHECKERS_H
