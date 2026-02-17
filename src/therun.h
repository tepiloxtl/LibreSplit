#ifndef THERUN_H
#define THERUN_H

#include "timer.h"
char* build_therun_live_payload(ls_timer* timer);
void therun_trigger_update(ls_timer* timer);

#endif // THERUN_H