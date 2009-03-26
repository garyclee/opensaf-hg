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
  hpl_resource_reset().
..............................................................................

  FUNCTIONS INCLUDED in this module:
    int main(int argc, char** argv) 
    print_usage(char* name)
    uns32 check_user_input(char* entity_path, HISV_RESET_ARGS resetType, int chassis, int blade)
******************************************************************************
*/


/* Headers */
#include "hisv_test.h"
#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Global Variables */
short SpecifiedChassis= 0;  // Flag if the user specified a chassis ID
short SpecifiedBlade= 0;    // Flag if the user specified a blade ID

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
   printf("%s:  reset an entity\n", name);
   printf("USAGE: %s -path=\"<{entity_path}>\" -reset=<cold|warm|assert|deassert|reboot> [-force]\n", name);
   printf("   OR: %s -c=<chassis_id> -b=<blade_id> -reset=<cold|warm|assert|deassert|reboot> [-force]\n", name);
   printf("\t-path\tRequired if chassis_id and blade_id are not specified.\n\t\t\tSpecify the path to the entity, e.g., \"{{SYSTEM_BLADE,12},{SYSTEM_CHASSIS,2}}\"\n");
   printf("\t\tNOTE: Be sure to quote the path on the command line (braces are interpreted by the shell.\n");
   printf("\t-c\tRequired if entity_path is not specified.\n\t\t\tSpecify the chassis of entity.\n");
   printf("\t-b\tRequired if entity_path is not specified.\n\t\t\tSpecify the blade-bay of the entity.\n");
   printf("\t-reset\tRequired - Specify the type of reset to apply e.g., warm, cold, assert, deasert, reboot \n");
   printf("\t-force\tSkip parameter checking to force %s to use the exact parameters input.\n", name);
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
 *                 HISV_RESET_ARGS resetType    -- Final power state of the entity
 *                 int chassis                  -- Chassis ID
 *                 int blade                    -- Blade ID
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Checks user's input for completeness.  The checking is pretty
 *                                 weak:
 *                 -  entity_path is a non-null string.
 *                    OR
 *                    chassis and blade are positive numbers
 *                 -  resetType   is a recognized power state
 *****************************************************************************/
uns32 check_user_input(char* entity_path, HISV_RESET_ARGS resetType, int chassis, int blade)
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

	if ( (resetType > ResetTypeMaxIndex) || (resetType < 0)) {
		printf("\nERROR: User requested an unrecognized reset state.\n");
		printf("\tAllowed reset states are:\n\t\tcold (0)\n\t\twarm (1)\n\t\tassert (2)\n\t\tdeassert (3)\n\t\treboot (4)\n");
        fflush(stdout);
		ret = NCSCC_RC_FAILURE;
    }
    if (!strlen(entity_path)) {
        if ( (!SpecifiedChassis) || (!SpecifiedBlade) ) {
			printf("\nERROR: Specify an entity path using -path on the command line\n");
			printf("\tOR: Specify the chassis ID and blade ID using -chassis and -blade\n");
        	fflush(stdout);
			ret = NCSCC_RC_FAILURE;
		}
    }

	if ( (resetType > ResetTypeMaxIndex) || (resetType < 0)) {
		printf("\nERROR:  User requested an unrecognized reset, %d.\n", (int)resetType);
		printf("\tAllowed resets are cold (0), warm (1), assert (2), deassert (3), reboot (4).\n");
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
   // Reset type requested
   HISV_RESET_ARGS resetType;
   unsigned short Force = 0;
   int chassis = -1;
   int blade = -1;
   
   // Read command line arguments
   int getopt_long_only();
   extern char* optarg;
   static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "?", no_argument, 0, 'h' },
		{ "blade", required_argument, 0, 'b' },
		{ "chassis", required_argument, 0, 'c' },
		{ "entity", required_argument, 0, 'p' },
		{ "force", no_argument, 0, 'f' },
		{ "path", required_argument, 0, 'p' },
		{ "reset", required_argument, 0, 'r' },
		{0, 0, 0, 0}
   };
   
   int c;
   int optIndex = 0;
   extern char *optarg;

   while (1) {
		c = getopt_long_only(argc, argv, "hp:r:", long_options, &optIndex);
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

			case 'h':
			case '?':
				{
				  print_usage(argv[0]);
				  exit(0);
				  break;
				}

			case 'f':
                {
                    Force = 1;
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

			case 'r':
				{
				  if (isdigit(optarg[0])) {
				      resetType = atoi(optarg);
					  break;
				  }
                   // Only checkt the first character,
                   // since that's enough to distiguish the reset types.
                  switch(optarg[0]) {
						case 'a':
					  		resetType = HISV_RES_RESET_ASSERT;
					  		break;

						case 'c':
					  		resetType = HISV_RES_COLD_RESET;
					  		break;

						case 'd':
					  		resetType = HISV_RES_RESET_DEASSERT;
					  		break;

						case 'g':
						case 'r':
					  		resetType = HISV_RES_GRACEFUL_REBOOT;
					  		break;

						case 'w':
					  		resetType = HISV_RES_WARM_RESET;
					  		break;

						case '-':
                            optarg++;
					  		resetType = (atoi(optarg));
                            resetType *= -1;
					  		break;

						default:
							err_exit(1, "Unknown reset type requested, %s, can't map to HISV_RESET_ARGS type\n", optarg);
					  		break;
                  }
				}

		}  // End switch
	}      // End while loop through command-line options

    // Check user input
    if (!Force) {
       if (check_user_input(hisv_entity_path, resetType, chassis, blade) != NCSCC_RC_SUCCESS)
           err_exit(1, "%d reset of %s\n", resetType,  hisv_entity_path);
    }

    // Set a default value for the chassis ID if the user gave an entity path.
    if (strlen(hisv_entity_path) && (!SpecifiedChassis)) {
       chassis = 2;
       printf("\tUser specified an entity path, but not chassis ID.  Using %d\n", chassis);
    }
    // Guarantee cleanup at exit
    atexit(hisv_cleanup);

    // Start up services and initialize the HPI libraries:
	ret = init_ncs_hisv();
    if (ret != NCSCC_RC_SUCCESS)
       err_exit(1, "Failed to initialize the hisv subsystem, return is %d\n", ret);

    // Check to see if the user input a path or the chassis and
    //  blade IDs to generate one.
    if (!strlen(hisv_entity_path)) {
       if (SpecifiedBlade && SpecifiedChassis) {
		  printf("Looking up entity path for chassis %d / blade %d\n", chassis, blade);
		  printf("\thpl_entity_path_lookup(3, chassis, blade, hisv_entity_path, sizeof(hisv_entity_path))\n");
		  if ( (hpl_entity_path_lookup(3, (uns32) chassis, (uns32) blade, hisv_entity_path, sizeof(hisv_entity_path)) != NCSCC_RC_SUCCESS) 
			|| (!strlen(hisv_entity_path)) )
          {
			 err_exit(1, "Failed to get an entity path for blade %d at chassis %d",
					 (int)blade, (int)chassis);
		  }
	   }
       else {
			err_exit(1, "You must enter either an entity path or the system chassis _and_ blade IDs\n");
	   }

	}

    if ( (resetType > 0) && (resetType <= ResetTypeMaxIndex) ) {
        printf("\n--> Applying %s reset ", ResetStateDesc[resetType]);
    }
    else {
        printf("\n--> Applying %d reset ", (int)resetType);
    }
    printf("to %s <--\n", hisv_entity_path);
    printf("\n--> hpl_resource_reset(%d, %s, %d)\n", chassis, hisv_entity_path, (int)resetType);
    fflush(stdout);
    ret = hpl_resource_reset(chassis, hisv_entity_path, resetType);
    if (ret != NCSCC_RC_SUCCESS) {
	   if (resetType <= ResetTypeMaxIndex) {
       	  printf("\nERROR: Failed to apply %s reset to %s!\n", ResetStateDesc[resetType], hisv_entity_path);
	   }
	   else {
      	  printf("\nERROR: Failed to apply type %d reset to %s!\n", (int)resetType, hisv_entity_path);
	   }
       err_exit(1, "hpl_resource_reset(chassis, hisv_entity_path, reset_type) returned an error\n");
    }
	else {
    	printf("Waiting a moment for the request to trickle down to ham\n");
    	m_NCS_TASK_SLEEP(3000);
		ret = 0;
	}

    return((int)ret);    
}

// vim: tabstop=4
