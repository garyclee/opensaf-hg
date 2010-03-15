/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2009 The OpenSAF Foundation
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
 * Author(s): Hewlett-Packard Development Company, L.P.
 *
 */

/*
 * PLMc boot program
 *
 * plmc_boot is a small program that emits a UDP message and then
 * exits.  The message emitted is either EE_INSTANTIATING or
 * EE_TERMINATED, depending on whether the program is called with
 * a start or stop parameter.
 *
 * Usage: plmc_boot [-c <plmc_conf file>] <start | stop>
 *
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <arpa/inet.h>

#include <plmc.h>

/* We need these two utility functions. */
extern int plmc_read_config(char *plmc_config_file, PLMC_config_data *config);
extern void plmc_print_config(PLMC_config_data *config);

/* Print out usage message. */
void usage() {
  printf("usage: plmc_boot [-c <plmc_conf file>] <start | stop>\n");
  exit(PLMC_EXIT_FAILURE);
}

/*
 * main()
 *
 * Implements plmc_boot.
 *
 * Inputs:
 *   argc : Argument count.
 *   argv : Argument values.
 *
 * Returns: PLMC_EXIT_SUCCESS on success.
 * Returns: PLMC_EXIT_FAILURE on failure.
 *
 */
int main(int argc, char**argv)
{
  char *plmc_config_file = NULL;
  int cmd_start_loc = 1;
  int sockfd;
  struct sockaddr_in servaddr;
  char sendline[PLMC_MAX_UDP_DATA_LEN];
  PLMC_config_data config;
  int action, retval = 0;
   
  if (argc < 2)
  {
    usage();
  }

  /* Determine the arguments, and what order they are in. */
  if (strcmp(argv[1], "-c") == 0) {
    if (argc < 3)
      usage();
    else {
      plmc_config_file = argv[2];
      cmd_start_loc = 3;
    }
  }
  if ((cmd_start_loc == 3) && (argc < 4)) {
    usage();
  }
  else {
    if (strcmp(argv[cmd_start_loc], "start") == 0)
      action = PLMC_START; 
    else {
      if (strcmp(argv[cmd_start_loc], "stop") == 0)
        action = PLMC_STOP; 
      else {
        usage();
      }
    }
    if (argc > (cmd_start_loc + 1)) {
      if (strcmp(argv[2], "-c") == 0) {
        if (argc < 4)
          usage();
        else {
          plmc_config_file = argv[3];
        }
      }
      else {
        usage();
      }
    }
  }

  /* Print out some debug info. */
#ifdef PLMC_DEBUG
  if (action == PLMC_START)
    printf("plmc_boot action = PLMC_START\n");
  else
    if (action == PLMC_STOP)
      printf("plmc_boot action = PLMC_STOP\n");
    else
      printf("plmc_boot action = *** UNKNOWN ***\n");
#endif

  /* Start a connection to syslog. */
  openlog("plmc_boot", LOG_PID|LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "%s", argv[cmd_start_loc]);

  if (plmc_config_file == NULL)
    plmc_config_file = PLMC_CONFIG_FILE_LOC;
  retval = plmc_read_config(plmc_config_file, &config); 
  
  /* If there is no config file, then we are done. */
  if (retval != 0) {
    printf("error: plmc_boot encountered error reading %s.  errno = %d\n", plmc_config_file, retval);
    syslog(LOG_ERR, "error: plmc_boot encountered error reading %s.  errno = %d", plmc_config_file, retval);
    closelog();
    exit(PLMC_EXIT_FAILURE);
  }

  /* Print more debug info. */
#ifdef PLMC_DEBUG
  plmc_print_config(&config);
#endif

  /* Set up the UDP datagram - use controller_1_ip. */
  sockfd=socket(AF_INET,SOCK_DGRAM,0);
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr=inet_addr(config.controller_1_ip);
  servaddr.sin_port=htons(atoi(config.udp_broadcast_port));

  /* Get UDP message ready. */
  switch (action) {
    case PLMC_START:
      sprintf(sendline, "ee_id %s : %s : %s", config.ee_id, PLMC_BOOT_START_MSG, config.os_type);
      break;
    case PLMC_STOP:
      sprintf(sendline, "ee_id %s : %s : %s", config.ee_id, PLMC_BOOT_STOP_MSG, config.os_type);
      break;
  }

  /* Send UDP datagram to controller_1_ip. */
  sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

  /* Now send same UDP datagram to controller_2_ip. */
  servaddr.sin_addr.s_addr=inet_addr(config.controller_2_ip);
  sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

  /* Post plmc_boot activity to syslog. */
  syslog(LOG_INFO, "%s", sendline);
  close(sockfd);
  closelog();

  /* All done - we can exit. */
  return(PLMC_EXIT_SUCCESS);
}
