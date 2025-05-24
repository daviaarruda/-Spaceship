
#ifndef TIMER_H
#define TIMER_H
#include <stdio.h>

void timerInit(int valueMilliSec);  
int timerTimeOver(void);            
void timerDestroy(void);           

void timerUpdateTimer(int valueMilliSec);  
void timerPrint(void); 

int timerGetTicks();
void timerDelay(int ms);

#endif
