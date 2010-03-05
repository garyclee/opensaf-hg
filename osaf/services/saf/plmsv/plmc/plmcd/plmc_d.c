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
 * PLMc daemon program
 *
 * plmc_d is a small daemon program that responds to PLMc commands
 * sent by a PLM server (or by the plmc_tcp_talker test program.)
 *
 * Usage: plmc_d [-c <plmc_conf file>] [<start | stop | restart>]
 *
 * Unless there is a networking issue, plmc_d will always respond to
 * a given command with one of the following responses:
 *
 *   "PLMC_ACK_MSG_RECD"
 *   "PLMC_ACK_SUCCESS"
 *   "PLMC_ACK_FAILURE"
 *   "PLMC_ACK_TIMEOUT"
 *
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <arpa/inet.h>

#include <plmc.h>
#include <plmc_cmds.h>

/* Command buffer size for starting/stopping OpenSAF services. */
#define LINE_BUFFER_SIZE	1024

/* We need these utility functions. */
extern int plmc_read_config(char *plmc_config_file, PLMC_config_data *config);
extern void plmc_print_config(PLMC_config_data *config);
extern int plmc_cmd_string_to_enum(char *cmd);

/* A variable to hold the PLMc config file data - used by parent */
/* and child threads.                                            */
static PLMC_config_data config;

/* A variable to hold the PLMc tcp socket used to communicate  */
/* with a PLMc client manager.                                 */
static int sockfd_tcp;

/* A simple structure to provide mutex locking between the parent  */
/* and child threads.  It also provides a flag telling the parent  */
/* thread when the child thread is done, and provides the child    */
/* thread return value.                                            */
typedef struct plmc_thread_data {
  pthread_mutex_t   td_lock;
  pthread_t         td_id;
  int		    plmc_cmd_i;
  int               child_done;
  char              *child_retval;
} plmc_tdata;

/*
 * plmc_termination_handler()
 *
 * A function to properly terminate the PLMc daemon.
 *
 * Inputs:
 *   signum : The signal that was received.
 *
 */
void plmc_termination_handler(int signum)
{
  /* If we get a SIGTERM signal, then close socket, close log, */
  /* unlink the pid file, and exit the process.                */
  close(sockfd_tcp);
  closelog();
  unlink(PLMC_D_PID_FILE_LOC);
  exit(PLMC_EXIT_SUCCESS);
}

/*
 * plmc_read_pid()
 *
 * A function to read the PLMc pid file.
 *
 * Inputs:
 *   line : A character array for reading the pid file into.
 *
 * Returns: 0 for success.
 * Returns: 1 for failure.
 *
 */
int plmc_read_pid(char *line) {
  FILE *plmc_d_pid_file;
  int n;

  plmc_d_pid_file = fopen(PLMC_D_PID_FILE_LOC,"r");
  if (plmc_d_pid_file != NULL) {
    /* Pid file exists - read it. */
    fgets(line,(PLMC_MAX_PID_LEN - 1),plmc_d_pid_file);

    /* Remove the new-line char at the end of the line. */
    n = strlen(line);
    line[n-1] = 0;
    fclose(plmc_d_pid_file);
    return(0);
  }
  else
    return(1);
}

/*
 * plmc_send_udp_msg()
 *
 * A function to send out a UDP datagram message.
 *
 * Inputs:
 *   config : Pointer to location where file data is stored.
 *   mesg   : Character array containing message to send.
 *
 */
void plmc_send_udp_msg(PLMC_config_data *config, char *mesg) {
  int sockfd_udp;
  struct sockaddr_in servaddr;
  char send_mesg[PLMC_MAX_TCP_DATA_LEN];

  /* Set up the UDP datagram - use controller_1_ip. */
  sockfd_udp=socket(AF_INET,SOCK_DGRAM,0);
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr=inet_addr(config->controller_1_ip);
  servaddr.sin_port=htons(atoi(config->udp_broadcast_port));

  /* Send UDP datagram that the EE is instantiated. */
  sprintf(send_mesg, "ee_id %s : %s : %s", config->ee_id, mesg, config->os_type);
  sendto(sockfd_udp, send_mesg, strlen(send_mesg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

  /* Now send same UDP datagram to controller_2_ip. */
  servaddr.sin_addr.s_addr=inet_addr(config->controller_2_ip);
  sendto(sockfd_udp, send_mesg, strlen(send_mesg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
  close(sockfd_udp);

  /* Post plmc_d activity to syslog. */
  syslog(LOG_INFO, "%s", send_mesg);
}

/*
 * plmc_connect_with_timeout()
 *
 * A function to provide a TCP connect with a settable timeout value.
 *
 * Inputs:
 *   sfd     : Socket file descriptor.
 *   addr    : Server address.
 *   addrlen : Sizeof server address.
 *   timeout : Pointer to timeout value structure.
 *
 * Returns: 0 on successful connection.
 * Returns: -1 on error (no connection established in the time allowed, or some other problem.)
 *
 */
int plmc_connect_with_timeout(int sfd, struct sockaddr *addr,
                              int addrlen, struct timeval *timeout)
{
  struct timeval rv_orig;
  struct timeval sv_orig;
  socklen_t rvlen = sizeof rv_orig;
  socklen_t svlen = sizeof sv_orig;
  int retval;

  if (!timeout)
    return(connect(sfd, addr, addrlen));
  
  /* Use socket options to set a connect timeout value. */
  if (getsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &rv_orig, &rvlen) < 0)
  {
#ifdef PLMC_DEBUG
    printf("error: plmc_d could not get the SO_RCVTIME0 option of the socket\n");
#endif
    return(-1);
  }
  if (getsockopt(sfd, SOL_SOCKET, SO_SNDTIMEO, &sv_orig, &svlen) < 0)
  {
#ifdef PLMC_DEBUG
    printf("error: plmc_d could not get the SO_SNDTIME0 option of the socket\n");
#endif
    return(-1);
  }

  if (setsockopt (sfd, SOL_SOCKET, SO_RCVTIMEO, (char *)timeout, sizeof *timeout) < 0)
  {
#ifdef PLMC_DEBUG
    printf("error: plmc_d could not set the SO_RCVTIME0 option of the socket\n");
#endif
    return(-1);
  }
  if (setsockopt (sfd, SOL_SOCKET, SO_SNDTIMEO, (char *)timeout, sizeof *timeout) < 0)
  {
#ifdef PLMC_DEBUG
    printf("error: plmc_d could not set the SO_SNDTIME0 option of the socket\n");
#endif
    return(-1);
  }

  retval = connect(sfd, addr, addrlen);

  /* Set socket options back to what they were before. */
  setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&rv_orig, sizeof rv_orig);
  setsockopt(sfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&sv_orig, sizeof sv_orig);
  return(retval);
}

/*
 * plmc_child_thread()
 *
 * A child thread that will perform the long-running PLMc commands, and
 * copy the result to the mutex locking structure.
 *
 * Inputs:
 *   arg : A pointer to the thread data structure.
 *
 */
void *plmc_child_thread(void *arg)
{
  plmc_tdata *td = (plmc_tdata *) arg;
  int i, retval = 0, tmp_retval = 0;
  char line[LINE_BUFFER_SIZE];

  if (td == NULL) {
#ifdef PLMC_DEBUG
    printf("error: plmc_d child thread did not get a valid thread_data structure.\n");
#endif
    exit(PLMC_EXIT_FAILURE);
  }

  /* Switch statement here to process the PLMc command. */
  switch (td->plmc_cmd_i) {
    case PLMC_SA_PLM_ADMIN_UNLOCK_CMD:
    syslog(LOG_INFO, "plmc_d executing unlock command: PLMC_SA_PLM_ADMIN_UNLOCK\n");

    /* Unlock the OS - start OpenSAF-related services, then start OpenSAF. */
    for(i=0; i<config.num_services_to_start; i++) {
#ifdef PLMC_DEBUG
      sprintf(line, "%s", config.services_to_start[i]);
#else
      sprintf(line, "%s >/dev/null 2>&1", config.services_to_start[i]);
#endif
      tmp_retval = system(line);
      if (tmp_retval != 0) retval = tmp_retval;
    }
#ifdef PLMC_DEBUG
    sprintf(line, "%s", config.osaf_to_start);
#else
    sprintf(line, "%s >/dev/null 2>&1", config.osaf_to_start);
#endif
    tmp_retval = system(line);
    if (tmp_retval != 0) retval = tmp_retval;
    break;

    case PLMC_SA_PLM_ADMIN_LOCK_CMD:
    syslog(LOG_INFO, "plmc_d executing lock command: PLMC_SA_PLM_ADMIN_LOCK\n");

    /* Lock the OS - stop OpenSAF, then stop OpenSAF-related services. */
#ifdef PLMC_DEBUG
    sprintf(line, "%s", config.osaf_to_stop);
#else
    sprintf(line, "%s >/dev/null 2>&1", config.osaf_to_stop);
#endif
    tmp_retval = system(line);
    if (tmp_retval != 0) retval = tmp_retval;
    for(i=0; i<config.num_services_to_stop; i++) {
#ifdef PLMC_DEBUG
      sprintf(line, "%s", config.services_to_stop[i]);
#else
      sprintf(line, "%s >/dev/null 2>&1", config.services_to_stop[i]);
#endif
      tmp_retval = system(line);
      if (tmp_retval != 0) retval = tmp_retval;
    }
    break;

    case PLMC_OSAF_START_CMD:
    syslog(LOG_INFO, "plmc_d executing OpenSAF start command: PLMC_OSAF_START\n");

    /* Start OpenSAF. */
#ifdef PLMC_DEBUG
    sprintf(line, "%s", config.osaf_to_start);
#else
    sprintf(line, "%s >/dev/null 2>&1", config.osaf_to_start);
#endif
    retval = system(line);
    break;

    case PLMC_OSAF_STOP_CMD:
    syslog(LOG_INFO, "plmc_d executing OpenSAF stop command: PLMC_OSAF_STOP\n");

    /* Stop OpenSAF. */
#ifdef PLMC_DEBUG
    sprintf(line, "%s", config.osaf_to_stop);
#else
    sprintf(line, "%s >/dev/null 2>&1", config.osaf_to_stop);
#endif
    retval = system(line);
    break;

    case PLMC_OSAF_SERVICES_START_CMD:
    syslog(LOG_INFO, "plmc_d executing OpenSAF-related services start command: PLMC_OSAF_SERVICES_START\n");

    /* Start OpenSAF-related services. */
    for(i=0; i<config.num_services_to_start; i++) {
#ifdef PLMC_DEBUG
      sprintf(line, "%s", config.services_to_start[i]);
#else
      sprintf(line, "%s >/dev/null 2>&1", config.services_to_start[i]);
#endif
      tmp_retval = system(line);
      if (tmp_retval != 0) retval = tmp_retval;
    }
    break;

    case PLMC_OSAF_SERVICES_STOP_CMD:
    syslog(LOG_INFO, "plmc_d executing OpenSAF-related services stop command: PLMC_OSAF_SERVICES_STOP\n");

    /* Stop OpenSAF-related services. */
    for(i=0; i<config.num_services_to_stop; i++) {
#ifdef PLMC_DEBUG
      sprintf(line, "%s", config.services_to_stop[i]);
#else
      sprintf(line, "%s >/dev/null 2>&1", config.services_to_stop[i]);
#endif
      tmp_retval = system(line);
      if (tmp_retval != 0) retval = tmp_retval;
    }
    break;
  }

  /* Begin critical section. */
  if (pthread_mutex_lock(&(td->td_lock)) != 0) {
#ifdef PLMC_DEBUG
    printf("error: plmc_d child thread could not lock thread_data structure.\n");
#endif
    exit(PLMC_EXIT_FAILURE);
  }

  /* Copy result of command into the thread_data structure.              */
  if (retval == 0) 
    td->child_retval = PLMC_ACK_SUCCESS;
  else
    td->child_retval = PLMC_ACK_FAILURE;
  td->child_done = 1;

  if (pthread_mutex_unlock(&(td->td_lock)) != 0) {
#ifdef PLMC_DEBUG
    printf("error: plmc_d child thread could not unlock thread_data structure.\n");
#endif
    exit(PLMC_EXIT_FAILURE);
  }
  /* End critical section. */

  pthread_exit((void *)NULL);
}

/*
 * plmc_start_child_thread()
 *
 * A function to start the PLMc child thread on a long-running PLMc command,
 * and to monitor the child thread so that it either completes, fails, or
 * times out.
 *
 * Inputs:
 *   plmc_cmd_i : The numeric value of the PLMc command to execute.
 *
 * Returns: "PLMC_ACK_SUCCESS" for a command that successfully completes.
 * Returns: "PLMC_ACK_FAILURE" for a command that fails.
 * Returns: "PLMC_ACK_TIMEOUT" for a command that times out.
 *
 */
char* plmc_start_child_thread(int plmc_cmd_i) {
  plmc_tdata th_data;
  int child_not_done = 1;
  void *join_res;
  time_t start_time, current_time;

  th_data.plmc_cmd_i = plmc_cmd_i;
  th_data.child_done = 0;
  th_data.child_retval = NULL; 

  /* Get the starting time. */
  time(&start_time);

  /* Initialize the mutex. */
  pthread_mutex_init(&(th_data.td_lock), NULL);

  /* Create the child thread. */
  pthread_create(&(th_data.td_id), NULL, plmc_child_thread, &th_data);

  /* Main watchdog loop - checks periodically to see if child thread is done.  */
  /* If timeout is reached, then we cancel child thread and return value of    */
  /* PLMC_ACK_TIMEOUT.                                                         */
  while(child_not_done) {
    sleep(PLMC_CHILD_THREAD_WATCHDOG_PERIOD_SECS);

    /* See if we've run out of time. */
    time(&current_time);

    /* We've run out of time, cancel the child thread. */
    if ((current_time - start_time) > config.plmc_cmd_timeout_secs) {
      pthread_cancel(th_data.td_id);
      break;
    }

    /* Begin critical section. */
    if (pthread_mutex_trylock(&(th_data.td_lock)) == 0) {
      if (th_data.child_done)
        child_not_done = 0;
      if (pthread_mutex_unlock(&(th_data.td_lock)) != 0) {
#ifdef PLMC_DEBUG
        printf("error: plmc_d could not unlock thread_data structure.\n");
#endif
        exit(PLMC_EXIT_FAILURE);
      }
    }
    /* End critical section. */
  }

  pthread_join(th_data.td_id, &join_res);
  if (join_res == PTHREAD_CANCELED) {
#ifdef PLMC_DEBUG
    printf("error: plmd_d child thread timed out.\n");
#endif
    th_data.child_retval = PLMC_ACK_TIMEOUT;
  }

  /* Destroy the mutex. */
  pthread_mutex_destroy(&(th_data.td_lock));

  return(th_data.child_retval);
}

/* Print out usage message. */
void usage() {
  printf("usage: plmc_d [-c <plmc_conf file>] [<start | stop | restart>]\n");
  exit(PLMC_EXIT_FAILURE);
}

/*
 * main()
 *
 * Implements plmc_d.
 *
 * Inputs:
 *   argc : Argument count.
 *   argv : Argument values.
 *
 * Returns: PLMC_EXIT_SUCCESS on success.
 * Returns: PLMC_EXIT_FAILURE on failure.
 *
 */
int main(int argc, char**argv) {
  char *plmc_config_file = NULL;
  int cmd_start_loc = 1;
  pid_t pid, sid;
  int retval = 0;
  char line[LINE_BUFFER_SIZE];
  int action;
  FILE *plmc_d_pid_file;
  struct sigaction sigact;
  int active_cont = 1;
  int plmc_cmd_i = 0;
  char use_ip[PLMC_MAX_TAG_LEN];

  int bytes_received;
  char send_mesg[PLMC_MAX_TCP_DATA_LEN];
  char recv_mesg[PLMC_MAX_TCP_DATA_LEN];
  char system_cmd[PLMC_MAX_TCP_DATA_LEN];
  struct sockaddr_in servaddr;
  struct timeval connect_timeout;
  char *child_thread_retval;

  /* Determine the arguments, and what order they are in. */
  if (argc < 2)
  {
    /* Equivalent commmand to: plmc_d start - except that a "ps -ef" command will */
    /* show plmc_d running but without the start argument in the command line.    */
    action = PLMC_START;
  }
  else {
    if (strcmp(argv[1], "-c") == 0) {
      if (argc < 3)
        usage();
      else {
        plmc_config_file = argv[2];  
        cmd_start_loc = 3;
      }
    }
    if ((cmd_start_loc == 3) && (argc < 4)) {
      /* Equivalent commmand to: plmc_d -c <config-file> start - except that a "ps -ef" command will */
      /* show plmc_d -c <config-file> running but without the start argument in the command line.    */
      action = PLMC_START;
    }
    else {
      if (strcmp(argv[cmd_start_loc], "start") == 0)
        action = PLMC_START;
      else {
        if (strcmp(argv[cmd_start_loc], "stop") == 0)
          action = PLMC_STOP;
        else {
          if (strcmp(argv[cmd_start_loc], "restart") == 0)
            action = PLMC_RESTART;
          else {
            usage();
          }
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
  }

  /* Print out some debug info. */
#ifdef PLMC_DEBUG
  if (action == PLMC_START)
    printf("plmc_d action = PLMC_START\n");
  else
    if (action == PLMC_STOP)
      printf("plmc_d action = PLMC_STOP\n");
    else
      if (action == PLMC_RESTART)
        printf("plmc_d action = PLMC_RESTART\n");
      else
        printf("plmc_d action = *** UNKNOWN ***\n");
#endif

  /* Open syslog. */
  openlog("plmc_d", LOG_PID|LOG_CONS, LOG_USER);

  /* Check that the plmc_config_file location is set. */
  if (plmc_config_file == NULL)
    plmc_config_file = PLMC_CONFIG_FILE_LOC;

  /* Read the plmc.conf config file. */
  retval = plmc_read_config(plmc_config_file, &config);
  if (retval != 0) {
    printf("error: plmc_d encountered error reading %s.  errno = %d\n", plmc_config_file, retval);
    syslog(LOG_ERR, "plmc_d encountered error reading %s.  errno = %d\n", plmc_config_file, retval);
    exit(PLMC_EXIT_FAILURE);
  }

  /* For now, print out the config data we just read. */
#ifdef PLMC_DEBUG
  plmc_print_config(&config);
#endif

  switch (action) {
    case PLMC_START:
      /* See if the daemon is already running - if it is, then error out. */
      retval = plmc_read_pid(line);
      if (retval == 0) {
        printf("error: plmc_d appears to be running (pid = %s), check existence of: %s\n", line, PLMC_D_PID_FILE_LOC);
        syslog(LOG_ERR, "error: plmc_d appears to be running (pid = %s), check existence of: %s", line, PLMC_D_PID_FILE_LOC);
        closelog();
        exit(PLMC_EXIT_FAILURE);
      }

      /* Now just fall thru, and the plmc_d daemon will be started. */
      break;
    case PLMC_STOP:
      /* See if the daemon is currently running - if it is, then stop it. */
      retval = plmc_read_pid(line);
      if (retval == 0) {
        /* Stop the daemon. */
        syslog(LOG_INFO, "stop");
        kill(atoi(line), SIGTERM);

        /* Send UDP datagram that the EE is terminating. */
        plmc_send_udp_msg(&config, PLMC_D_STOP_MSG);

        closelog();
        exit(PLMC_EXIT_SUCCESS);
      }
      printf("error: plmc_d is not running, %s does not exist\n", PLMC_D_PID_FILE_LOC);
      syslog(LOG_ERR, "error: plmc_d is not running, %s does not exist", PLMC_D_PID_FILE_LOC);
      closelog();
      exit(PLMC_EXIT_FAILURE);
      break;
    case PLMC_RESTART:
      /* See if the daemon is already running - if it is, then stop it, and start it up again. */
      /* If the daemon is not already running, then fall thru to just start it up normally.    */
      retval = plmc_read_pid(line);
      if (retval == 0) {
        /* Stop the daemon. */
        syslog(LOG_INFO, "stop");
        kill(atoi(line), SIGTERM);

        /* Don't need to send UDP datagram in the restart case. */
        /* plmc_send_udp_msg(&config, PLMC_D_STOP_MSG); */

        /* Give a second to let the signal handler clean up the old pid file,  */
        /* then fall thru to start up another plmc_d daemon.                   */
        sleep(1); /* Let the signal handler clean up the pid, then fall thru to start up another daemon. */
      }
      break;
  }

  /* Fork off the parent process. */
  closelog(); /* Close syslog before forking */
  pid = fork();
  if (pid < 0) {
    exit(PLMC_EXIT_FAILURE);
  }

  /* Daemon-specific initialization goes here. */

  /* If we got a good PID, then we can exit the parent process. */
  if (pid > 0) {
    exit(PLMC_EXIT_SUCCESS);
  }

  /* Open syslog in the child process. */        
  openlog("plmc_d", LOG_PID|LOG_CONS, LOG_USER);
  syslog(LOG_INFO, "start");
  closelog();

  /* If we are starting daemon (not restarting) then send */
  /* UDP datagram that the EE has instantiated.           */
  if (action == PLMC_START)
    plmc_send_udp_msg(&config, PLMC_D_START_MSG);

  /* Change the file mode mask. */
  umask(0);
                
  /* Create a new session id (sid) for the child process. */
  sid = setsid();

#ifdef PLMC_DEBUG
  printf("sid = %d\n", sid);
#endif

  if (sid < 0) {
    /* Log the failure */
    exit(PLMC_EXIT_FAILURE);
  }

  /* Save the sid (session-id) for later use. */
  plmc_d_pid_file = fopen(PLMC_D_PID_FILE_LOC,"w");
  if (plmc_d_pid_file == NULL) {
    /* Problem with creating pid file - return errno value. */
    return(errno);
  }
  sprintf(line, "%d\n", sid); 
  fputs(line, plmc_d_pid_file);
  fclose(plmc_d_pid_file);

  /* Set up termination signal handler here. */
  memset( &sigact, 0, sizeof(sigact));
  sigemptyset (&sigact.sa_mask);            /* All signals are unblocked. */
  sigact.sa_handler = plmc_termination_handler;
  sigact.sa_flags = 0;
  sigaction(SIGTERM, &sigact, NULL);

  /* Close out the standard file descriptors.                       */
  /* Note: don't close these file descriptors if we need to debug.  */
#ifndef PLMC_DEBUG
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
#endif
        
  /* Set connect timeout - this is used only when establishing a connection.       */
  /* After connection is established, the default socket timeouts are reinstated.  */
  connect_timeout.tv_sec = PLMC_TCP_TIMEOUT_SECS;
  connect_timeout.tv_usec = 0;

  /*
   *  The outer plmc_d daemon loop.
   *
   *  The purpose of this while loop is to connect and register with 
   *  a PLM server process over a TCP socket, and then listen for
   *  incoming commands from the PLM server process, and carry
   *  out these commands on the local node.  If the connection is
   *  lost, this outer loop will attempt to reestablish the connection
   *  with one of the two PLM servers.
   *
   */

  /* This is the outer while loop that attempts to establish a connection to a PLM server. */
  while (1) {
    if ((sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#ifndef PLMC_DEBUG
      printf("error: plmc_d could not open a socket\n");
#endif
      openlog("plmc_d", LOG_PID|LOG_CONS, LOG_USER);
      syslog(LOG_ERR, "plmc_d cannot open a socket\n");
      closelog();
      exit(PLMC_EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port=htons(atoi(config.tcp_plms_listening_port));

    /* Select which IP to use. */
    if (active_cont) {
      strcpy(use_ip, config.controller_1_ip);
      active_cont = 0;
    }
    else {
      strcpy(use_ip, config.controller_2_ip);
      active_cont = 1;
    }
    servaddr.sin_addr.s_addr=inet_addr(use_ip);
    bzero(&(servaddr.sin_zero),8);

#ifdef PLMC_DEBUG
    printf("plmc_d attempting to connect to: %s\n", use_ip);
#endif

    openlog("plmc_d", LOG_PID|LOG_CONS, LOG_USER);
    /* Attempt socket connection with a strict timeout in place. */
    if (plmc_connect_with_timeout(sockfd_tcp, (struct sockaddr *) &servaddr, sizeof(struct sockaddr), &connect_timeout) == -1) { 
#ifdef PLMC_DEBUG
      printf("error: plmc_d could not connect to plms at: %s\n", use_ip);
      syslog(LOG_ERR, "plmc_d cannot connect to plms at: %s\n", use_ip);
#endif
    }
    else {
      /* We connected to one of the PLM servers.  */
#ifdef PLMC_DEBUG
      printf("plmc_d connected to plms at: %s\n", use_ip);
#endif
      syslog(LOG_INFO, "plmc_d connected to plms at: %s\n", use_ip);

      /* This is the inner while loop that waits on a socket connection */
      /* for a plmc command to come from the PLM server.  If socket     */
      /* connection is lost, we fall thru to the outer while loop, and  */
      /* and attempt to reconnect to one of the designated PLM servers. */
      while (1) {
        bytes_received=recv(sockfd_tcp, recv_mesg, PLMC_MAX_TCP_DATA_LEN, 0);

        /* If we get an empty message, or and error, assume bad socket connection, */
        /* so break inner loop, and reconnect to plms.                             */
        if (bytes_received < 1) {
          break;
        }

        recv_mesg[bytes_received] = '\0';

        plmc_cmd_i = plmc_cmd_string_to_enum(recv_mesg);
#ifdef PLMC_DEBUG
        printf("plmc_d received the following command message: %s\n", recv_mesg);
#endif
        syslog(LOG_INFO, "received the following command message: %s\n", recv_mesg);

        /* If we cannot decode the command name into a command enum - then it is */
        /* and error, so break inner loop, and reconnect to plms.                */
        if (plmc_cmd_i == -1) {
          syslog(LOG_ERR, "plmc_d could not decode the following command: %s\n", recv_mesg);
          break;
        }
        switch(plmc_cmd_i) {
     
          case PLMC_NOOP_CMD:
          /* Just send an acknowledgement. */
#ifdef PLMC_DEBUG
          printf("plmc_d sending MSG_RECD in acknowledgement: %s\n", PLMC_ACK_MSG_RECD);
#endif
          /* Send PLMC_ACK_MSG_RECD in the acknowledgement. */
          sprintf(send_mesg, "plmc_d %s ack %s : %s", PLMC_cmd_name[plmc_cmd_i], config.ee_id, PLMC_ACK_MSG_RECD);
          send(sockfd_tcp, send_mesg, strlen(send_mesg), 0);
          break;

          case PLMC_GET_ID_CMD:
#ifdef PLMC_DEBUG
          printf("plmc_d sending ee_id in acknowledgement: %s\n", config.ee_id);
#endif
          /* Send the ee_id in the acknowledgement. */
          sprintf(send_mesg, "plmc_d %s ack %s : %s", PLMC_cmd_name[plmc_cmd_i], config.ee_id, config.ee_id);
          send(sockfd_tcp, send_mesg, strlen(send_mesg), 0);
          break;

          case PLMC_GET_PROTOCOL_VER_CMD:
#ifdef PLMC_DEBUG
          printf("plmc_d sending msg_protocol_version in acknowledgement: %s\n", config.msg_protocol_version);
#endif
          /* Send the msg_protocol_version in the acknowledgement. */
          sprintf(send_mesg, "plmc_d %s ack %s : %s", PLMC_cmd_name[plmc_cmd_i], config.ee_id, config.msg_protocol_version);
          send(sockfd_tcp, send_mesg, strlen(send_mesg), 0);
          break;

          case PLMC_GET_OSINFO_CMD:
#ifdef PLMC_DEBUG
          printf("plmc_d sending os_type in acknowledgement: %s\n", config.os_type);
#endif
          /* Send the os_type in the acknowledgement. */
          sprintf(send_mesg, "plmc_d %s ack %s : %s", PLMC_cmd_name[plmc_cmd_i], config.ee_id, config.os_type);
          send(sockfd_tcp, send_mesg, strlen(send_mesg), 0);
          break;

          case PLMC_SA_PLM_ADMIN_LOCK_INSTANTIATION_CMD:
#ifdef PLMC_DEBUG
          printf("plmc_d sending MSG_RECD in acknowledgement: %s\n", PLMC_ACK_MSG_RECD);
#endif
          syslog(LOG_INFO, "plmc_d executing lock instantiation command: PLMC_SA_PLM_ADMIN_LOCK_INSTANTIATION\n");
          /* Send PLMC_ACK_MSG_RECD in the acknowledgement - before executing command. */
          sprintf(send_mesg, "plmc_d %s ack %s : %s", PLMC_cmd_name[plmc_cmd_i], config.ee_id, PLMC_ACK_MSG_RECD);
          send(sockfd_tcp, send_mesg, strlen(send_mesg), 0);

          /* Shutdown the OS. */
          close(sockfd_tcp);
#ifdef PLMC_DEBUG
          sprintf(line, "%s", config.os_shutdown_cmd);
#else
          sprintf(line, "%s >/dev/null 2>&1", config.os_shutdown_cmd);
#endif
          system(line);
          break;

          case PLMC_SA_PLM_ADMIN_RESTART_CMD:
#ifdef PLMC_DEBUG
          printf("plmc_d sending MSG_RECD in acknowledgement: %s\n", PLMC_ACK_MSG_RECD);
#endif
          syslog(LOG_INFO, "plmc_d executing restart command: PLMC_SA_PLM_ADMIN_RESTART\n");
          /* Send PLMC_ACK_MSG_RECD in the acknowledgement - before executing command. */
          sprintf(send_mesg, "plmc_d %s ack %s : %s", PLMC_cmd_name[plmc_cmd_i], config.ee_id, PLMC_ACK_MSG_RECD);
          send(sockfd_tcp, send_mesg, strlen(send_mesg), 0);

          /* Reboot the OS */
          close(sockfd_tcp);
#ifdef PLMC_DEBUG
          sprintf(line, "%s", config.os_reboot_cmd);
#else
          sprintf(line, "%s >/dev/null 2>&1", config.os_reboot_cmd);
#endif
          system(line);
          break;

          case PLMC_PLMCD_RESTART_CMD:
#ifdef PLMC_DEBUG
          printf("plmc_d sending MSG_RECD in acknowledgement: %s\n", PLMC_ACK_MSG_RECD);
#endif
          /* Send PLMC_ACK_MSG_RECD in the acknowledgement - before executing command. */
          sprintf(send_mesg, "plmc_d %s ack %s : %s", PLMC_cmd_name[plmc_cmd_i], config.ee_id, PLMC_ACK_MSG_RECD);
          send(sockfd_tcp, send_mesg, strlen(send_mesg), 0);
          close(sockfd_tcp);

          /* Restart plmc_d. */
          plmc_config_file = getenv("PLMC_CONF_PATH");
          /* See if the plmc.conf file location is specified. */
          if (plmc_config_file == NULL) {
#ifdef PLMC_DEBUG
            sprintf(system_cmd, "%s restart", argv[0]);
#else
            sprintf(system_cmd, "%s restart >/dev/null 2>&1", argv[0]);
#endif
          }
          else {
#ifdef PLMC_DEBUG
            sprintf(system_cmd, "%s -c %s restart", argv[0], plmc_config_file);
#else
            sprintf(system_cmd, "%s -c %s restart >/dev/null 2>&1", argv[0], plmc_config_file);
#endif
          }

          system(system_cmd);
          break;

          case PLMC_SA_PLM_ADMIN_UNLOCK_CMD:
          case PLMC_SA_PLM_ADMIN_LOCK_CMD:
          case PLMC_OSAF_START_CMD:
          case PLMC_OSAF_STOP_CMD:
          case PLMC_OSAF_SERVICES_START_CMD:
          case PLMC_OSAF_SERVICES_STOP_CMD:
          /* These are long-running commands - so create a child thread to handle this case. */
          child_thread_retval = plmc_start_child_thread(plmc_cmd_i);
#ifdef PLMC_DEBUG
          printf("plmc_d sending acknowledgement: %s\n", child_thread_retval);
#endif
          /* Send the acknowledgement. */
          sprintf(send_mesg, "plmc_d %s ack %s : %s", PLMC_cmd_name[plmc_cmd_i], config.ee_id, child_thread_retval);
          send(sockfd_tcp, send_mesg, strlen(send_mesg), 0);
          break;
        }
      }
    }

    /* We close the socket because we want to establish a clean connection */
    /* the next time around - and we do not want any dangling file         */
    /* descriptors.                                                        */
    closelog();
    close(sockfd_tcp);

    sleep(PLMC_TCP_TIMEOUT_SECS); /* Wait 5 seconds, then try to connect to the alternate IP. */
  }
  return(PLMC_EXIT_SUCCESS);
}

