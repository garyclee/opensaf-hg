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

#include "ncs_posix_tmr.h"

#if (NCS_POSIX_TMR == 1)

#define SIG_NCS_TMR SIGUSR1

#define FAILURE -1
#define OK       0

struct ncs_ptmr_t {
    timer_t timerid;
    ncs_ptmr_notify_t cb;
};

static struct ncs_ptmr_t ncs_ptmrs[NCS_MAX_TMRS];
static sem_t ncs_ptmr_sem;
static int ncs_ptmr_init_done = 0;

static void ncs_ptmr_notify_handler();

static int ncs_ptmr_init()
{

    ncs_ptmr_init_done = 1;

    if (sem_init(&ncs_ptmr_sem, 0, 1) == -1)
    {
        perror("ncs_ptmr_init");
        return(FAILURE);
    }

    memset(ncs_ptmrs, 0, sizeof(ncs_ptmrs));
}

timer_t ncs_ptmr_create(ncs_ptmr_notify_t cb)
{
    int i, status;
    struct sigevent evp;

    if (!ncs_ptmr_init_done)
        ncs_ptmr_init();

    evp.sigev_value.sival_ptr = (void *)cb;
    evp.sigev_signo = SIG_NCS_TMR;
    evp.sigev_notify = SIGEV_SIGNAL;

    if (!sem_wait(&ncs_ptmr_sem)) {
        for(i=0;ncs_ptmrs[i].timerid != 0; i++);
        status = timer_create(CLOCK_REALTIME, &evp, &ncs_ptmrs[i].timerid);
        if (status == FAILURE) {
            perror("timer_create");
            sem_post(&ncs_ptmr_sem);
            return(FAILURE);
        }
        sem_post(&ncs_ptmr_sem);
    }
    return(ncs_ptmrs[i].timerid);
}

int ncs_ptmr_settime(timer_t timerid, const struct itimerspec *timeout)
{
    sigset_t newmask, oldmask;
    int status;
    struct sigaction action;

    /* Mask out SIG_NCS_TMR untill we are done setting up things */
    sigemptyset(&newmask);
    sigaddset(&newmask, SIG_NCS_TMR);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    status = timer_settime(timerid, 0, timeout, NULL);
    if (status == FAILURE) {
        perror("timer_settime failed: ");
        return(FAILURE);
    }

    action.sa_flags = SA_SIGINFO; 
    action.sa_sigaction = ncs_ptmr_notify_handler;

    if (sigaction(SIG_NCS_TMR, &action, NULL) == -1) {
        perror("sigusr: sigaction");
        return(FAILURE);
    }

    sigprocmask(SIG_SETMASK, &oldmask, NULL);
   
    return(OK);
}

int ncs_ptmr_stop(timer_t timerid)
{
    struct itimerspec timeout;

    timeout.it_value.tv_sec = 0;
    timeout.it_value.tv_nsec = 0;
    timeout.it_interval.tv_sec = 0;
    timeout.it_interval.tv_nsec = 0; 

    return ncs_ptmr_settime(timerid, &timeout);
}

int ncs_ptmr_delete(timer_t timerid)
{
    int i;

    if (!sem_wait(&ncs_ptmr_sem)) {
        for(i=0;ncs_ptmrs[i].timerid == timerid; i++);

        if (i==NCS_MAX_TMRS) {
            sem_post(&ncs_ptmr_sem);
            return(FAILURE);
        }

        memset(&ncs_ptmrs[i], 0, sizeof(struct ncs_ptmr_t));
        sem_post(&ncs_ptmr_sem);
    }

    return(i);
}

static void ncs_ptmr_notify_handler(int signo, siginfo_t *info, void *extra)
{
       ncs_ptmr_notify_t ptr_val = info->si_value.sival_ptr;
       int int_val = info->si_value.sival_int;
       ptr_val();
       return;
}
#endif /*NCS_POSIX_TMR*/
