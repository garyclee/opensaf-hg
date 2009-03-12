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

  This file contains the main() function for the HISv entity-lookup
  API tests.
  hpl_entity_path_lookup().
..............................................................................

  FUNCTIONS INCLUDED in this module:
    int main(int argc, char** argv) 
    print_usage(char* name)
    uns32 check_user_input(uns32 type, uns32 chassis, uns32 blade);

******************************************************************************
*/

/* Definitiions */
typedef enum entity_path_type {
	STRING = 0,		/* Full string format for entity path */
	NUMERIC,		/* Numeric string format for entity path */
	ARRAY,			/* Array-based format for entity path */
    SHORT_STRING    /* Short string-format path */
} ENTITY_PATH_TYPE;

/* Headers */
#include "hisv_test.h"
#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <openhpi/SaHpi.h>

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
   m_NCS_CONS_PRINTF("%s:  Print the entity path returned by hpl_entity_path_lookup()\n", name);
   m_NCS_CONS_PRINTF("USAGE: %s -blade=<bay_num> -chassis=<chassisID> -type=<output_type>\n", name);
   m_NCS_CONS_PRINTF("\tOptional: [-f] [-path=\"<{expected_entity_path}>\"]\n");
   m_NCS_CONS_PRINTF("\t-blade\tRequired - Specify the bay number to the blade.\n");
   m_NCS_CONS_PRINTF("\t-chassis\tRequired - Specify the chassis ID of the system.\n");
   m_NCS_CONS_PRINTF("\t-type\tRequired - Specify the output type for the entity path.\n");
   m_NCS_CONS_PRINTF("\t-length\tOptional - Specify the size of the buffer for the entity path (in bytes).\n");
   m_NCS_CONS_PRINTF("\t-path\tOptional - Specify the path to the entity for comparison,\n"
                     "\te.g., \"{{SYSTEM_BLADE,12},{SYSTEM_CHASSIS,2}}\"\n");
   m_NCS_CONS_PRINTF("\t-force\tForce %s to use the exact parameters input (skip input checking)\n", name);
   m_NCS_CONS_PRINTF("\n");
   fflush(stdout);
}

/****************************************************************************
 * Name          : check_user_input
 *
 * Description   : Verifies that the user gave required information for
 *                 the test.
 *
 * Arguments     : uns32 type	           -- Type of 
 *                 HISV_PWR_ARGS pwr_state      -- Final power state of the entity
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : Checks user's input for completeness.  The checking is pretty
 *                 weak:
 *                 -  entity_path is a non-null string.
 *                 -  pwr_state   is a recognized power state
 *****************************************************************************/
uns32 check_user_input(uns32 type, uns32 chassis, uns32 blade)
{
    uns32 ret = NCSCC_RC_SUCCESS;

    if ((type < 0) || (type > EntityPathTypeMax)) {
		m_NCS_CONS_PRINTF("\nERROR: output type must be in [0 - %d]\n", EntityPathTypeMax);
        fflush(stdout);
		ret = NCSCC_RC_FAILURE;
    }
    if (chassis < 0) {
		m_NCS_CONS_PRINTF("\nERROR: Chassis ID must be a non-negative integer\n");
        fflush(stdout);
		ret = NCSCC_RC_FAILURE;
    }
    if ((blade < 1) || (blade > 16)) {
		m_NCS_CONS_PRINTF("\nERROR: Blade bay number must be in 1, 2, ... 16.\n");
        fflush(stdout);
		ret = NCSCC_RC_FAILURE;
    }
    return(ret);
}
       
int main(int argc, char** argv)
{
   // For starting up libraries
   uns32 ret = 0;

   // For user's input reference-entity path:
   SaUint8T refEntityPath[EPATH_STRING_SIZE];
   memset(refEntityPath, 0, sizeof(refEntityPath));

   SaUint8T help = 0;

   unsigned short Force = 0;
   uns32 chassis = -1;
   uns32 blade = -1;
   uns32 type = -1;
   uns32 length = EPATH_STRING_SIZE;
   short nullString = 0;  // For API error handling of null path string
   ENTITY_PATH_TYPE reqType;

   // Read command line arguments
   int getopt_long_only();
   static struct option long_options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "?", no_argument, 0, 'h' },
		{ "blade", required_argument, 0, 'b' },
		{ "bay", required_argument, 0, 'b' },
		{ "chassis", required_argument, 0, 'c' },
		{ "entity", required_argument, 0, 'p' },
		{ "force", no_argument, 0, 'f' },
		{ "length", required_argument, 0, 'l' },
		{ "path", required_argument, 0, 'p' },
		{ "type", required_argument, 0, 't' },
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
					if (optarg[0] == '-') {
					  // Let the user input a negative blade ID
				      // for API error handling test.
                        optarg++;
						blade = atoi(optarg);
						blade *= -1;
					}
					else {
						blade = atoi(optarg);
					}
					break;
				}
			case 'c':
				{
					if (optarg[0] == '-') {
					  // Let the user input a negative chassis ID
				      // for API error handling test.
                        optarg++;
						chassis = atoi(optarg);
						chassis *= -1;
					}
					else {
						chassis = atoi(optarg);
					}
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

			case 'l':
				{
					if (! strncasecmp(optarg, "NULL", 4)) {
                  		nullString = 1;
						length = 0;
						break;
					}
					// Size of the buffer for the entity path.
					length = atoi(optarg);
					break;
				}

			case 'p':
				{
				  strncpy(refEntityPath, optarg, sizeof(refEntityPath));
				  break;
				}

			case 't':
				{
				  if (isdigit(optarg[0])) {
				      type = atoi(optarg);
					  break;
				  }
				  if (optarg[0] == '-') {
				    // Let the user input a negative number
				    // for the requested type for API error handling
					// test.
					  optarg++;
					  type = atoi(optarg);
					  type *= -1;
				  }
				  if (!strncasecmp("string", optarg, 6)) {
                      reqType = STRING;
					  type = (uns32) reqType;
					  break;
				  }
				  if (!strncasecmp("numeric", optarg, 7)) {
                      reqType = NUMERIC;
					  type = (uns32) reqType;
					  break;
				  }
				  if (!strncasecmp("array", optarg, 5)) {
					  reqType = ARRAY;
					  type = (uns32) reqType;
					  break;
				  }
				  if (!strncasecmp("short", optarg, 5)) {
                      reqType = SHORT_STRING;
					  type = (uns32) reqType;
					  break;
				  }
				}

		}  // End switch
	}      // End while loop through command-line options

    
    // Check user input if user hasn't specfied the option to Force input
    if (!Force) {
       if (check_user_input(type, chassis, blade) != NCSCC_RC_SUCCESS)
           err_exit(1, "User input is invalid");
    }

    // Set up a cleanup routine
    atexit(hisv_cleanup);

    // Initialize the libraries to access the API:
    ret = init_ncs_hisv();
    if (ret != NCSCC_RC_SUCCESS) {
       err_exit(1, "Failed to initialize HPL\n");
    }

    // Now we're ready to use the API
    SaUint8T entityPath[length];

    memset(entityPath, 0, length);
    m_NCS_CONS_PRINTF("\nUsing buffer of %d B for entity path\n", length);
   	m_NCS_CONS_PRINTF("Before function call, entityPath memory = %p\n", entityPath);

    m_NCS_CONS_PRINTF("\nLooking up path type ");
    if ( (type >= 0 ) && (type <= EntityPathTypeMax) )  {
        m_NCS_CONS_PRINTF("%s using\n", EntityPathTypeDesc[type]);
    }
    else {
        m_NCS_CONS_PRINTF("%d using\n", type);
    }
    m_NCS_CONS_PRINTF("\thpl_entity_path_lookup(%d, %d, %d, %p, %d)\n", 
		type, chassis, blade, entityPath, (int)length);
    if ((hpl_entity_path_lookup(type, chassis, blade, entityPath, (size_t)length) != NCSCC_RC_SUCCESS) ||
		(!strlen(entityPath)) ) {
          err_exit(1, "Failed to get an entity path of type %d for blade %d at chassis %d",
                    type, (int)blade, (int)chassis);
    }
   	m_NCS_CONS_PRINTF("After function call, entityPath memory = %p\n", entityPath);
    if (type == 2) { 
        SaHpiEntityPathT* arrayEP = (SaHpiEntityPathT*) entityPath;
        m_NCS_CONS_PRINTF("\nReturned path:\n");
		int i;
		for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++) {
			if (arrayEP->Entry[i].EntityType == SAHPI_ENT_UNSPECIFIED) break;

			m_NCS_CONS_PRINTF("    Depth = %d\n", i);
			m_NCS_CONS_PRINTF("\tEntity Type = %d\n", arrayEP->Entry[i].EntityType);
			m_NCS_CONS_PRINTF("\tEntity Location = %d\n", arrayEP->Entry[i].EntityLocation);
		}
    }
	else {
		if (strlen(entityPath) > length) {
			
       		m_NCS_CONS_PRINTF("entityPath memory = %p\n", entityPath);
            err_exit(1, "Application trashed memory -- path is longer than allocated memory (%d > %d)\n",
 			         strlen(entityPath), length);
        }
        m_NCS_CONS_PRINTF("\nReturned path:\n%s\n", entityPath);
    }

    if (strlen(refEntityPath)) {
       m_NCS_CONS_PRINTF("\tExpected: %s\n", refEntityPath);
       if (strncmp(refEntityPath, entityPath, strlen(refEntityPath))) {
          m_NCS_CONS_PRINTF("\nERROR: Entity path returned by API does not match the expected entity path.\n");
          exit(0);
       }
    }
    return(0);    
}

// vim: tabstop=4
