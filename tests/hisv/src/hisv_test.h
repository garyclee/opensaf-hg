/* HISv power-test code definitions */
#ifndef __HISV_POWER_CTRL
#define __HISV_POWER_CTRL
#include <hpl_api.h>
#include <saEvt.h>
#include <ncssysf_tsk.h>
#include <ncssysf_def.h>
#include <os_defs.h>
  // Parameters
#define MAX_MSG_LEN		1024
#define API_ARG_LEN 128

 // Define some translation info that will be useful
 //   for printing and checking power and reset state
static short PowerStateMaxIndex = 2;
static char* PowerStateDesc[] = {
	"HISV_RES_POWER_OFF",
	"HISV_RES_POWER_ON",
	"HISV_RES_POWER_CYCLE",
};

static short ResetTypeMaxIndex = 4;
static char* ResetStateDesc[] = {
	"HISV_RES_COLD_RESET",
	"HISV_RES_WARM_RESET",
	"HISV_RES_RESET_ASSERT",
	"HISV_RES_RESET_DEASSERT",
	"HISV_RES_GRACEFUL_REBOOT",
};

 // This identifies entity path types
static short EntityPathTypeMax = 3;
static char* EntityPathTypeDesc[] = {
    "HISV_ENTITY_PATH_STRING",
    "HISV_ENTITY_PATH_NUMERIC",
    "HISV_ENTITY_PATH_ARRAY",
    "HISV_ENTITY_PATH_SHORT_STRING"
};

  // Function prototypes
void err_exit(int exitVal, char* fmt, ...);
uns32 init_ncs_hisv(void);
void hisv_cleanup(void);

#endif
// vim: tabstop=4
