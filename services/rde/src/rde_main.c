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
 *
 */

#include <configmake.h>

/*****************************************************************************
..............................................................................

  $$

  MODULE NAME: rde_main.c

..............................................................................

  DESCRIPTION:

  This file contains the main() routine for RDE.


******************************************************************************/

#include "rde.h"
#include "rde_cb.h"

/***************************************************************\
 *                                                               *
 *         Prototypes for static functions                       *
 *                                                               *
\***************************************************************/

static uns32        rde_create_pidfile(void);
static uns32        rde_main_loop  (void);
static uns32        rde_shutdown  (void);
extern uns32               rde_initialize(void);

/*****************************************************************************

  PROCEDURE NAME:       main

  DESCRIPTION:          Main routine for RDE
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

*****************************************************************************/

int
main (int    argc, char * argv[])
{
   uns32          rc    =  NCSCC_RC_FAILURE;
   RDE_CONTROL_BLOCK * rde_cb = rde_get_control_block();
   char line[256];
   FILE *fp;
   NCS_PHY_SLOT_ID slot_id;

   fp = fopen(OSAF_SYSCONFDIR "slot_id", "r");
   if (fp == NULL)
   {
      m_NCS_CONS_PRINTF (OSAF_SYSCONFDIR "slot_id couldn't be opened\n");
      return RDE_RDE_RC_FAILURE;
   }

   /***************************************************************\
    *                                                               *
    *         Parse command line options                            *
    *                                                               *
   \***************************************************************/

   if (rde_get_options(rde_cb, argc, argv) != NCSCC_RC_SUCCESS)
   {
      exit (-1);
   }

   if(fgets (line, sizeof (line), fp) != NULL)
   {
     slot_id = atoi(line);
     rde_cb->options.slot_number = slot_id;
     fclose(fp);
   }
   else
   {
      m_NCS_CONS_PRINTF ("Slot_id couldn't be read\n");
      fclose(fp);
      return RDE_RDE_RC_FAILURE;
   }
   /*
   ** Print options
   */
   m_NCS_CONS_PRINTF ("PID  file          : %s\n", rde_cb->options.pid_file);
   m_NCS_CONS_PRINTF ("Shelf number       : %d\n", rde_cb->options.shelf_number);
   m_NCS_CONS_PRINTF ("Slot number        : %d\n", rde_cb->options.slot_number);
   m_NCS_CONS_PRINTF ("Site number        : %d\n", rde_cb->options.site_number);
   m_NCS_CONS_PRINTF ("Log level          : %u\n", rde_cb->options.log_level);
   m_NCS_CONS_PRINTF ("Interactive mode   : %s\n", (rde_cb->options.is_daemon ? "FALSE" : "TRUE")) ;

   /***************************************************************\
    *                                                               *
    *         Create PID file                                       *
    *                                                               *
   \***************************************************************/
   if (rde_create_pidfile () != NCSCC_RC_SUCCESS)
   {
      exit (-1);
   }

   /***************************************************************\
    *                                                               *
    *         Start LEAP Environment                                *
    *         Initialize memory tracking                            *
    *                                                               *
   \***************************************************************/

   if (rde_agents_startup () != NCSCC_RC_SUCCESS)
   {
      remove(rde_cb-> options. pid_file);
      m_NCS_CONS_PRINTF ("RDE NCS_AGENTS START FAILED\n");
      exit (-1);
   }

    /***************************************************************\
    *                                                               *
    *         Initialize processing                                 *
    *                                                               *
   \***************************************************************/

   /***************************************************************\
    *                                                               *
    *         Initialize processing                                 *
    *                                                               *
   \***************************************************************/

   if ((rc = rde_initialize ()) != NCSCC_RC_SUCCESS)
   {
      syslog(LOG_ERR, "RDE: rde_initialize failed, check rde.conf");
      rde_agents_shutdown ();    
      remove(rde_cb-> options. pid_file);
      m_NCS_CONS_PRINTF ("RDE INITIALIZATION FAILED: %l\n", rc);
      exit (-1);
   }


   /* RDE is successfully initialized, notify the same to NID later after 
      getting the correct role. */
   {
      m_NCS_CONS_PRINTF ("RDE INITIALIZATION SUCCESS\n");
   }


   /***************************************************************\
    *                                                               *
    *         Main processing loop for this task                    *
    *                                                               *
   \***************************************************************/

   rde_main_loop ();

   /***************************************************************\
    *                                                               *
    *         Shutdown and exit                                     *
    *                                                               *
   \***************************************************************/

   rde_shutdown ();

   rde_agents_shutdown ();
   exit(0);
}

/*****************************************************************************

  PROCEDURE NAME:       rde_get_control_block

  DESCRIPTION:          Singleton implementation for RDE Context
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

*****************************************************************************/

RDE_CONTROL_BLOCK *rde_get_control_block (void)
{
   static NCS_BOOL   initialized = FALSE;
   static RDE_CONTROL_BLOCK rde_cb;

   if (!initialized)
   {
      initialized = TRUE;
      m_NCS_OS_MEMSET( &rde_cb, 0, sizeof(rde_cb));
   }

   return & rde_cb;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_get_options

  DESCRIPTION:          Extract options specified on command line
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

*****************************************************************************/

uns32 rde_get_options (
   RDE_CONTROL_BLOCK * rde_cb,
   int          argc,
   char       * argv[]
   )
{
   RDE_OPTIONS  * options  = & rde_cb-> options ;
   uns32         rc       = NCSCC_RC_SUCCESS    ;
   const char  * progName = argv[0]             ;

   int           opt                            ;
   extern char * optarg                         ;
   extern int    optind, opterr, optopt         ;


   m_RDE_ENTRY ("rde_get_options");
 
   /***************************************************************\
    *                                                               *
    *         Initialize default options                            *
    *                                                               *
   \***************************************************************/

   options-> is_daemon       = TRUE                          ;
   options-> pid_file        = RDE_DEFAULT_PID_FILE          ;
   options-> log_level       = RDE_DEFAULT_LOG_LEVEL         ;
   options->shelf_number     = RDE_DEFAULT_SHELF_NUMBER      ;
   options->slot_number      = RDE_DEFAULT_SLOT_NUMBER       ;
   options->site_number      = RDE_DEFAULT_SITE_NUMBER       ;

   /***************************************************************\
    *                                                               *
    *         Parse command line options                            *
    *                                                               *
   \***************************************************************/

   while ((opt = getopt(argc, argv, "p:f:s:t:l:vhi")) != -1)
   {
      switch (opt)   
      {

         /*******************************************************\
         *                                                       *
         *         Pid file                                      *
         *                                                       *
         \*******************************************************/

      case 'p':

         if (optarg != NULL)
         {
            options-> pid_file = optarg;
         }
         else
         {
            m_RDE_LOG_COND (RDE_SEV_ERROR, RDE_LOG_COND_NO_PID_FILE);
            m_NCS_CONS_PRINTF ("PID file not specified, for help use -h option\n");
            rc = NCSCC_RC_FAILURE;
         }

         break;

         /*******************************************************\
         *                                                       *
         *         Shelf number                                  *
         *                                                       *
         \*******************************************************/

      case 'f':

         if (optarg == NULL)
         {
            m_RDE_LOG_COND (RDE_SEV_ERROR, RDE_LOG_COND_NO_SHELF_NUMBER);
            m_NCS_CONS_PRINTF ("Shelf number not specified, for help use -h option\n");
            rc = NCSCC_RC_FAILURE;
         }
         else
         {
            options->shelf_number = atoi(optarg);
         }

         break;

         /*******************************************************\
         *                                                       *
         *         Slot number                                   *
         *                                                       *
         \*******************************************************/

      case 's':

         if (optarg == NULL)
         {
            m_RDE_LOG_COND (RDE_SEV_ERROR, RDE_LOG_COND_NO_SLOT_NUMBER);
            m_NCS_CONS_PRINTF ("Slot number not specified, for help use -h option\n");
            rc = NCSCC_RC_FAILURE;
         }
         else
         {
            options->slot_number = atoi(optarg);
         }

         break;

         /*******************************************************\
         *                                                       *
         *         Site number                                   *
         *                                                       *
         \*******************************************************/

      case 't':

         if (optarg == NULL)
         {
            m_RDE_LOG_COND (RDE_SEV_ERROR, RDE_LOG_COND_NO_SITE_NUMBER);
            m_NCS_CONS_PRINTF ("Site number not specified, for help use -h option\n");
            rc = NCSCC_RC_FAILURE;
         }
         else
         {
            options->site_number = atoi(optarg);
         }

         break;


         /*******************************************************\
         *                                                       *
         *         Log level                                     *
         *                                                       *
         \*******************************************************/

      case 'l':
                   {
            uns32 log_level = atoi (optarg) ;
      
            if (log_level < RDE_LOG_EMERGENCY ||
                log_level > RDE_LOG_DEBUG)
            {
               m_NCS_CONS_PRINTF ("Invalid Loglevel. 0(EMERG) to 7(DEBUG)\n");
               rc = NCSCC_RC_FAILURE;
            }

            options-> log_level = log_level;
         }

         break;

         /*******************************************************\
         *                                                       *
         *         Interactive/daemonize                         *
         *                                                       *
         \*******************************************************/

      case 'i':

         options-> is_daemon = FALSE;

         break;


        /*******************************************************\
         *                                                       *
         *         Version                                       *
         *                                                       *
         \*******************************************************/

      case 'v':
         
         m_NCS_CONS_PRINTF(" rde_rde version:  Development Version\n");
         break;

         /*******************************************************\
         *                                                       *
         *         Help / usage                                  *
         *                                                       *
         \*******************************************************/

      case 'u':
      case 'h':
      default :
         m_NCS_CONS_PRINTF("Usage:\n");
         m_NCS_CONS_PRINTF("  %s [options]\n", progName);
         m_NCS_CONS_PRINTF("     Available options:\n");
         m_NCS_CONS_PRINTF("        -h             :     Display this help message.\n");
         m_NCS_CONS_PRINTF("        -i             :     Run interactively\n");
         m_NCS_CONS_PRINTF("        -l <0-7>       :     Set log level <0-7>\n");
         m_NCS_CONS_PRINTF("        -v             :     Display version information\n");
         m_NCS_CONS_PRINTF("        -p <file path> :     Use file as PID file \n");
         m_NCS_CONS_PRINTF("        -f <shelfnum>  :     Use the specified shelf number\n");
         m_NCS_CONS_PRINTF("        -s <slotnum>   :     Use the specified slot number\n");
         m_NCS_CONS_PRINTF("        -t <sitenum>   :     Use the specified site number\n");
         m_NCS_CONS_PRINTF("\n");
         m_NCS_CONS_PRINTF ("    Default Values:\n");
         m_NCS_CONS_PRINTF ("       PID  file          : %s\n", RDE_DEFAULT_PID_FILE);
         m_NCS_CONS_PRINTF ("       Shelf number       : %d\n", RDE_DEFAULT_SHELF_NUMBER);
         m_NCS_CONS_PRINTF ("       Slot number        : %d\n", RDE_DEFAULT_SLOT_NUMBER);
         m_NCS_CONS_PRINTF ("       Site number        : %d\n", RDE_DEFAULT_SITE_NUMBER);
         m_NCS_CONS_PRINTF ("       Log level          : %u\n", RDE_DEFAULT_LOG_LEVEL);
         m_NCS_CONS_PRINTF ("       Interactive mode   : %s\n", (options-> is_daemon ? "FALSE" : "TRUE")) ;

         return NCSCC_RC_FAILURE;  /* Cause program to exit */
         break;
      }
   }

   if (optind!=argc)
   {
      m_NCS_CONS_PRINTF("Bad Arguments for rde_rde... Use -h option for help.\n");
      return NCSCC_RC_FAILURE;
   }
   return rc;

}

/*****************************************************************************

  PROCEDURE NAME:       rde_create_pidfile

  DESCRIPTION:          Daemonizes the current process
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

*****************************************************************************/

static
uns32 rde_create_pidfile (void)
{
   FILE       * pidfd;
   RDE_CONTROL_BLOCK * rde_cb;

   m_RDE_ENTRY("rde_create_pidfile");

   rde_cb = rde_get_control_block();

   if ((pidfd = fopen(rde_cb-> options. pid_file, "w")) != NULL) 
   {
      fprintf(pidfd, "%d\n", (int) getpid());
      fclose(pidfd);
   }
   else 
   {
      m_NCS_CONS_PRINTF("%s: pidfile %s open failed\n", 
                        rde_cb->prog_name, 
                        rde_cb-> options. pid_file);

      return NCSCC_RC_FAILURE;
   }
   
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_main_loop

  DESCRIPTION:          Main loop 
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

*****************************************************************************/

uns32 rde_main_loop (void)
{
   RDE_CONTROL_BLOCK * rde_cb = rde_get_control_block();

   /* TBD: Wait forever for now */
   if (m_NCS_OS_SEM (&rde_cb-> semaphore, NCS_OS_SEM_TAKE) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF ("RDE MAIN LOOP, SEM TAKE FAILED\n");
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_shutdown

  DESCRIPTION:          Main loop 
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

*****************************************************************************/

uns32
rde_shutdown (void)
{
   return NCSCC_RC_SUCCESS;
}
/* #endif */ 
