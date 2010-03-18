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
 * plmc_read_config() utility function.
 *
 * plmc_read_config() is an internal utility function used by
 * plmc_boot, plmc_d, and the plmc_lib to read a plmcd.conf
 * configuration file.  Users of PLMc should not attempt to use
 * this function as the library APIs provide all of the necessary
 * means to access the capabilities of PLMc.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <plmc.h>

/*
 * plmc_read_config()
 *
 * A function to read the plmcd.conf confguration file.
 *
 * Inputs:
 *   plmc_config_file : A file pathname.
 *   config           : Pointer to location where file data is stored.
 *
 * Returns: 0 on success.
 * Returns: errno on error.
 *
 */
int plmc_read_config(char *plmc_config_file, PLMC_config_data *config) {
  FILE *plmc_conf_file;
  char line[PLMC_MAX_TAG_LEN];
  int i, n, comment_line, tag = 0;
  int num_start_services = 0, num_stop_services = 0;

  /* Open plmcd.conf config file. */
  plmc_conf_file = fopen(plmc_config_file, "r");
  if (plmc_conf_file == NULL) {
    /* Problem with conf file - return errno value */
    return(errno);
  }
    
  /* Read file. */
  while(!feof(plmc_conf_file)){
    fgets(line,(PLMC_MAX_TAG_LEN - 1),plmc_conf_file);

    /* If blank line, skip it - and set tag back to 0. */
    if (strcmp(line, "\n") == 0) {
      tag = 0;
      continue;
    }

    /* If a comment line, skip it. */
    n = strlen(line);
    comment_line = 0;
    for (i=0; i<n; i++) {
      if ((line[i] == ' ') || (line[i] == '\t'))
        continue;
      else
        if (line[i] == '#') {
          comment_line = 1;
          break;
        }
        else
          break;
    }
    if (comment_line)
      continue;

    /* Remove the new-line char at the end of the line. */
    line[n-1] = 0;

    /* Set tag if we are not in a tag. */
    if ((tag == 0) || (line[0] == '[')) {
      if (strcmp(line, "[ee_id]") == 0)
        tag = PLMC_EE_ID;
      if (strcmp(line, "[plmc_msg_protocol_version]") == 0)
        tag = PLMC_MSG_PROTOCOL_VERSION;
      if (strcmp(line, "[controller_1_ip]") == 0)
        tag = PLMC_CONTROLLER_1_IP;
      if (strcmp(line, "[controller_2_ip]") == 0)
        tag = PLMC_CONTROLLER_2_IP;
      if (strcmp(line, "[services_to_start]") == 0)
        tag = PLMC_SERVICES_TO_START;
      if (strcmp(line, "[services_to_stop]") == 0)
        tag = PLMC_SERVICES_TO_STOP;
      if (strcmp(line, "[osaf_to_start]") == 0)
        tag = PLMC_OSAF_TO_START;
      if (strcmp(line, "[osaf_to_stop]") == 0)
        tag = PLMC_OSAF_TO_STOP;
      if (strcmp(line, "[tcp_plms_listening_port]") == 0)
        tag = PLMC_TCP_PLMS_LISTENING_PORT;
      if (strcmp(line, "[udp_broadcast_port]") == 0)
        tag = PLMC_UDP_BROADCAST_PORT;
      if (strcmp(line, "[os_type]") == 0)
        tag = PLMC_OS_TYPE;
      if (strcmp(line, "[plmc_cmd_timeout_secs]") == 0)
        tag = PLMC_CMD_TIMEOUT_SECS;
      if (strcmp(line, "[os_reboot_cmd]") == 0)
        tag = PLMC_OS_REBOOT_CMD;
      if (strcmp(line, "[os_shutdown_cmd]") == 0)
        tag = PLMC_OS_SHUTDOWN_CMD;
    }
    else {
      /* Set the value of the tag in config data structure. */
      switch (tag) {
        case PLMC_EE_ID:
          strcpy(config->ee_id, line);
          tag = 0;
          break;
        case PLMC_MSG_PROTOCOL_VERSION:
          strcpy(config->msg_protocol_version, line);
          tag = 0;
          break;
        case PLMC_CONTROLLER_1_IP:
          strcpy(config->controller_1_ip, line);
          tag = 0;
          break;
        case PLMC_CONTROLLER_2_IP:
          strcpy(config->controller_2_ip, line);
          tag = 0;
          break;
        case PLMC_SERVICES_TO_START:
          strcpy(config->services_to_start[num_start_services], line);
          num_start_services++;
          if (num_start_services == PLMC_MAX_SERVICES)
            /* Don't want to overrun our array of services. */
            tag = 0;
          break;
        case PLMC_SERVICES_TO_STOP:
          strcpy(config->services_to_stop[num_stop_services], line);
          num_stop_services++;
          if (num_stop_services == PLMC_MAX_SERVICES)
            /* Don't want to overrun our array of services. */
            tag = 0;
          break;
        case PLMC_OSAF_TO_START:
          strcpy(config->osaf_to_start, line);
          tag = 0;
          break;
        case PLMC_OSAF_TO_STOP:
          strcpy(config->osaf_to_stop, line);
          tag = 0;
          break;
        case PLMC_TCP_PLMS_LISTENING_PORT:
          strcpy(config->tcp_plms_listening_port, line);
          tag = 0;
          break;
        case PLMC_UDP_BROADCAST_PORT:
          strcpy(config->udp_broadcast_port, line);
          tag = 0;
          break;
        case PLMC_OS_TYPE:
          strcpy(config->os_type, line);
          tag = 0;
          break;
        case PLMC_CMD_TIMEOUT_SECS:
          config->plmc_cmd_timeout_secs = atoi(line);
          tag = 0;
          break;
        case PLMC_OS_REBOOT_CMD:
          strcpy(config->os_reboot_cmd, line);
          tag = 0;
          break;
        case PLMC_OS_SHUTDOWN_CMD:
          strcpy(config->os_shutdown_cmd, line);
          tag = 0;
          break;
        default:
          break;
      }
    }
  }
  config->num_services_to_start = num_start_services;
  config->num_services_to_stop = num_stop_services;

  /* Close file. */
  fclose(plmc_conf_file);

  /* All done - return 0. */
  return(0);
}

/*
 * plmc_print_config()
 *
 * A function to print the plmcd.conf confguration file.
 *
 * Inputs:
 *   config : Pointer to location where file data is stored.
 *
 */
void plmc_print_config(PLMC_config_data *config)
{
  int i;

  printf("plmc_boot: config file contains:\n");

  printf("  [ee_id]\n");
  printf("  %s\n", config->ee_id);

  printf("  [plmc_msg_protocol_version]\n");
  printf("  %s\n", config->msg_protocol_version);

  printf("  [controller_1_ip]\n");
  printf("  %s\n", config->controller_1_ip);

  printf("  [controller_2_ip]\n");
  printf("  %s\n", config->controller_2_ip);

  printf("  [services_to_start]\n");
  for (i=0; i<config->num_services_to_start; i++) {
    printf("  %s\n", config->services_to_start[i]);
  }

  printf("  [services_to_stop]\n");
  for (i=0; i<config->num_services_to_stop; i++) {
    printf("  %s\n", config->services_to_stop[i]);
  }

  printf("  [osaf_to_start]\n");
  printf("  %s\n", config->osaf_to_start);

  printf("  [osaf_to_stop]\n");
  printf("  %s\n", config->osaf_to_stop);

  printf("  [tcp_plms_listening_port]\n");
  printf("  %s\n", config->tcp_plms_listening_port);

  printf("  [udp_broadcast_port]\n");
  printf("  %s\n", config->udp_broadcast_port);

  printf("  [os_type]\n");
  printf("  %s\n", config->os_type);

  printf("  [plmc_cmd_timeout_secs]\n");
  printf("  %d\n", config->plmc_cmd_timeout_secs);

  printf("  [os_reboot_cmd]\n");
  printf("  %s\n", config->os_reboot_cmd);

  printf("  [os_shutdown_cmd]\n");
  printf("  %s\n", config->os_shutdown_cmd);

  printf("\n");
}

