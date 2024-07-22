#ifndef _LED_H
#define _LED_H

#include "mb.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_led();
void trigger_led();
void update_led();
void error(eMBErrorCode code);

#ifdef __cplusplus
}
#endif

#endif
