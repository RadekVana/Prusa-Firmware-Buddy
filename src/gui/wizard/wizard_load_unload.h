// wizard_load_unload.h
#ifndef _WIZARD_LOAD_UNLOAD_H
#define _WIZARD_LOAD_UNLOAD_H

#include <inttypes.h>
#include "gui.h"
#include "wizard_types.h"

typedef enum {
    LD_UNLD_INIT,
    LD_UNLD_MSG_DECIDE_CONTINUE_LOAD_UNLOAD,
    LD_UNLD_DIALOG_PREHEAT,
    LD_UNLD_DIALOG_LOAD,
    LD_UNLD_DIALOG_UNLOAD,
    LD_UNLD_DONE
} LD_UNLD_STATE_t;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern LD_UNLD_STATE_t wizard_load_unload(LD_UNLD_STATE_t state);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_WIZARD_LOAD_UNLOAD_H
