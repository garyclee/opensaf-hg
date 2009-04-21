/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Emerson Network Power
 *            Hewlett-Packard Company
 */

/*****************************************************************************
  DESCRIPTION:

  This file contains the main() function of HISv power-control application.
  It initializes the basic infrastructure and calls the SAF-HPI api,
  hpl_resource_power_set().
..............................................................................

  FUNCTIONS INCLUDED in this module:
    int main(int argc, char** argv) 
    print_usage(char* name)
    uns32 check_user_input(char* entity_path, HISV_PWR_ARGS pwr_state, int chassis, int blade)
******************************************************************************
*/


/* Headers */
#include "hisv_test.h"
#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Globals */
  // Flag to specify user-input type
short SpecifiedChassis = 0;
short SpecifiedBlade = 0;

/*****************************************************************************
 *  Name			: print_usage
 *  
 *  Description		: print a usage message to the console
 *  
 *  Arguments		: none
 *  
 *  Return Values	: void
 *  
 *****************************************************************************/
void print_usage(const char* name) {
   printf("%s:  Set the power state of an entity\n", name);
   printf("USAGE: %s -path=\"<{entity_path}>\" -state=<on|off|cycle> [-f]\n", name);
   printf("   OR: %s -chassis=<chassis_id> -blade=<blade_id> -state=<on|off|cycle> [-f]\n", name);
   printf("\t-path\tRequired if chassis and blade ID not specified.\n\t\t\tSpecify the path to the entity, e.g., \"{{SYSTEM_BLADE,12},{SYSTEM_CHASSIS,2}}\"\n");
   printf("\t\tNOTE: Be sure to quote the path on the command line (braces are interpreted by the shell.\n");
   printf("\t-chassis\tRequired if entity_path is not specified.\n\t\t\tSpecify the chassis of the entity\n");
   printf("\t-blade\tRequired if entity_path is not specified.\n\t\t\tSpecify the blade-bay of the entity\n");
   printf("\t-state\tRequired - Specify the power state, e.g., off, on or (power) cycle\n");
   printf("\t-force\tForce %s to use the exact parameters input (no input checking)\n", name);
   printf("\n");
   fflush(stdout);
}

/****************************************************************************
 * Name          : check_user_input
 *
 * Description   : Verifies that the user gave required information for
 *                 the test.
 *
 * Arguments     : char * entity_path           -- The entity path
 *                 HISV_PWR_ARGS pwr_state      -- Final power state of the entity
 *                 int chassis                  -- Chassis ID
 *                 int blade                    -- Blade ID (slot number)
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Checks user's input for completeness.  The checking is pretty
 *                 weak:
 *                 -  entity_path is a non-null string.
 *                      OR
 *                    chassis and blade are positive numbers 
 *                 -  pwr_state   is a recognized power state
 *****************************************************************************/
uns32 check_user_input(char* entity_path, HISV_PWR_ARGS pwr_state, int chassis, int blade)
{
    int ret = NCSCC_RC_SUCCESS;

    printf("\ncheck_user_input:\n");
    printf("\tSpecifiedChassis : %d\n", SpecifiedChassis);
    printf("\tSpecifiedBlade : %d\n", SpecifiedBlade);
    if (strlen(entity_path)) {printf("\tentity_path : %s\n", entity_path);}
    printf("\tchassis : %d\n", chassis);
    printf("\tblade : %d\n", blade);

    if (!strlen(entity_path)) {
        if ((!SpecifiedChassis) || (!SpecifiedBlade)) {
		   printf("\nERROR: Specify an entity path using -path on the command line\n");
		   printf("   OR: Specify the chassis ID and blade ID using -chassis and -blade\n");
           fflush(stdout);
		   ret = NCSCC_RC_FAILURE;
        }
    }

	if ( (pwr_state > PowerStateMaxIndex) || (pwr_state < 0)) {
		printf("\nERROR: User requested an unrecognized power state.\n");
		printf("\tAllowed power states are ON (0), OFF (1), CYCLE (2).\n");
        fflush(stdout);
		ret = NCSCC_RC_FAILURE;
    }
    return(ret);
}
       
int main(int argc, char** argv)
{
   // For starting up libraries
   uns32 ret = 0;

   // For user's input entity path:
   SaUint8T hisv_entity_path[EPATH_STRING_SIZE];
   memset(hisv_entity_path, 0, sizeof(hisv_entity_path));

   SaUint8T help = 0;
   HISV_PWR_ARGS pwr_state;

   unsigned short Force = 0;
   int chassis = -1;
   int blade = -1;

   // Read command line arguments
   int getopt_long_only();
   static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "?", no_argument, 0, 'h' },
		{ "blade", required_argument, 0, 'b' },
		{ "chassis", required_argument, 0, 'c' },
		{ "entity", required_argument, 0, 'p' },
		{ "force", no_argument, 0, 'f' },
		{ "path", required_argument, 0, 'p' },
		{ "state", required_argument, 0, 's' },
		{ "off", no_argument, 0, 'o' },
		{ "on", no_argument, 0, 'n' },
		{ "cycle", no_argument, 0, 'y' },
		{0, 0, 0, 0}
   };
   
   int c;
   int optIndex = 0;
   extern char *optarg;

   while (1) {
		c = getopt_long_only(argc, argv, "fhp:s:", long_options, &optIndex);
		if (c == -1)
			break;
		switch (c) {
            case 'b':
				{
					blade = atoi(optarg);
                    SpecifiedBlade = 1;
					break;
				}
			case 'c':
				{
					chassis = atoi(optarg);
                    SpecifiedChassis = 1;
					break;
				}
			case 'f':
				{
				  Force = 1;
				  break;
				}

			case 'h':
			case '?':
				{
				  print_usage(argv[0]);
				  exit(0);
				  break;
				}

			case 'o':
				{
				  pwr_state = HISV_RES_POWER_OFF;
				  break;
				}

			case 'n':
				{
				  pwr_state = HISV_RES_POWER_ON;
				  break;
				}

			case 'p':
				{
                   /* For testing error handling */
                  if (!strncmp(optarg, "NULL", 5)) {
                      break;
                  }
				  strncpy(hisv_entity_path, optarg, sizeof(hisv_entity_path));
				  break;
				}

			case 's':
				{
				  if (isdigit(optarg[0])) {
				      pwr_state = atoi(optarg);
					  break;
				  }
				  if (!strncasecmp("OFF", optarg, 3)) {
					  pwr_state = HISV_RES_POWER_OFF;
					  break;
				  }
				  if (!strncasecmp("ON", optarg, 2)) {
					  pwr_state = HISV_RES_POWER_ON;
					  break;
				  }
				  if (!strncasecmp("CYCLE", optarg, 5)) {
					  pwr_state = HISV_RES_POWER_CYCLE;
					  break;
				  }
                  if (optarg[0] == '-') {
					// For testing error cases - negative index for 
					// power state.
                      optarg++;
                      pwr_state = atoi(optarg);
                      pwr_state *= -1;
                      break;
				  }
					// If we get here, this is an unknown, unmappable power type, so exit
				  err_exit(1, "Unknown power state '%s' requested, no mapping to HISV_PWR_ARGS type", optarg);
				}

			case 'y':
				{
				  pwr_state = HISV_RES_POWER_CYCLE;
				  break;
				}


		}  // End switch
	}      // End while loop through command-line options

    // Check user input if user hasn't specfied the option to Force input
    if (!Force) {
       if (check_user_input(hisv_entity_path, pwr_state, chassis, blade) != NCSCC_RC_SUCCESS)
           err_exit(1, "Setting power state for %s to %d\n",
                      hisv_entity_path, pwr_state);
    }

    // Set a default value for the chassis ID if the user gave an entity path.
	if (strlen(hisv_entity_path) && (!SpecifiedChassis)) {
	   chassis = 2;
	   printf("\tUser specified an entity path, but not chassis ID.  Using %d\n", chassis);
	} 

    // Now register a cleanup routine when we exit.  From this
    // point on, we will run the message service and initialized
    // the libraries.
    atexit(hisv_cleanup);

	ret = init_ncs_hisv();
    if (ret != NCSCC_RC_SUCCESS)
       err_exit(1,  "Failed to initialize the event subsystem, return is %d\n", ret);

    // Check to see if the user input a path or the chassis and
    //  blade IDs to generate one.
    if (!strlen(hisv_entity_path) && (SpecifiedBlade)) {
       printf("Looking up path for chassis %d / blade %d\n", chassis, blade);
       printf("\thpl_entity_path_lookup(3, chassis, blade, hisv_entity_path, sizeof(hisv_entity_path))\n");
       if ((hpl_entity_path_lookup(3, (uns32) chassis, (uns32) blade, hisv_entity_path, sizeof(hisv_entity_path)) != NCSCC_RC_SUCCESS)
			|| (!strlen(hisv_entity_path)) )
	   {
           err_exit(1, "Failed to get an entity path for blade %d at chassis %d",
                     (int)blade, (int)chassis);
       }
    }

    printf("\n--> Setting power state for %s to ", hisv_entity_path);
    if ( (pwr_state > 0) && (pwr_state <= PowerStateMaxIndex) ) {
       printf("%s <--\n", PowerStateDesc[pwr_state]);
    }
	else {
       printf("%d <--\n", (int)pwr_state);
    }
    printf("\n");
    printf("--> hpl_resource_power_set(%d, %s, %d)\n", chassis, hisv_entity_path, pwr_state);
    fflush(stdout);
    ret = hpl_resource_power_set(chassis, hisv_entity_path, pwr_state);
    if (ret != NCSCC_RC_SUCCESS) {
       printf("ERROR: Failed to set power to %d on %s\n", pwr_state, hisv_entity_path);
       printf("\thpl_resource_power_set(chassis, hisv_entity_path, power_state) returned an error\n");
       fflush(stdout);
    }
	if (ret == NCSCC_RC_SUCCESS) {
 	 	printf("Waiting a moment for the request to trickle down to ham\n");
   		m_NCS_TASK_SLEEP(3000);
		ret = 0;
	}
	else {
		ret = 1;
	}

    // ret = (ret == NCSCC_RC_SUCCESS) ? 0 : 1;
    return((int)ret);
}

// vim: tabstop=4
