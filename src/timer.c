#include "timer.h"
#include <time.h>
#include <stdio.h>
#include <time.h>

static clock_t start_time;
static int delay_ms = 0;

void timerInit(int valueMilliSec) {
    delay_ms = valueMilliSec;
    start_time = clock();
}

void timerDestroy() {
    delay_ms = 0;
}

int timerTimeOver() {
    clock_t current = clock();
    int elapsed = (int)((current - start_time) * 1000 / CLOCKS_PER_SEC);
    
    if (elapsed >= delay_ms) {
        start_time = current;
        return 1;
    }
    return 0;
}

void timerUpdateTimer(int valueMilliSec) {
    delay_ms = valueMilliSec;
    start_time = clock();
}

int getTimeDiff() {
    clock_t current = clock();
    return (int)((current - start_time) * 1000 / CLOCKS_PER_SEC);
}

void timerPrint() {
    printf("Timer: %d ms\n", getTimeDiff());
}
void timerDelay(int ms) {
    clock_t start = clock();
    while ((clock() - start) * 1000 / CLOCKS_PER_SEC < ms);
}

int timerGetTicks() {
    return getTimeDiff();
}
