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

#include "clmtest.h"
#include <sys/types.h>
#include <sys/wait.h>

SaClmClusterNotificationBufferT notificationBuffer_1;
SaClmClusterNotificationBufferT_4 notificationBuffer_4;
SaUint8T trackFlags;
SaClmNodeIdT nodeId;
SaInvocationT invocation;
SaInvocationT lock_inv;

/*
void saClmResponse_01(void)
{
        int ret;

        trackFlags = SA_TRACK_CHANGES|SA_TRACK_START_STEP;

        safassert(saClmInitialize_4(&clmHandle, &clmCallback4, &clmVersion_4), SA_AIS_OK);
        safassert(saClmSelectionObjectGet(clmHandle, &selectionObject),SA_AIS_OK);
        rc = saClmClusterTrack_4(clmHandle, trackFlags, NULL);

        fds[0].fd = (int) selectionObject;
        fds[0].events = POLLIN;
        while (1){
                ret = poll(fds, 1, 1000);
                if (fds[0].revents & POLLIN){
                        break;
                }
        }
        safassert(saClmDispatch(clmHandle,SA_DISPATCH_ALL), SA_AIS_OK);
        safassert(saClmResponse_4(clmHandle,lock_inv,SA_CLM_CALLBACK_RESPONSE_OK),SA_AIS_OK);

        fds[0].fd = (int) selectionObject;
        fds[0].events = POLLIN;
        while (1){
                ret = poll(fds, 1, 1000);
                if (fds[0].revents & POLLIN){
                        break;
                }
        }

        safassert(saClmDispatch(clmHandle,SA_DISPATCH_ALL), SA_AIS_OK);
        
        safassert(saClmClusterTrackStop(clmHandle), SA_AIS_OK);
        safassert(saClmFinalize(clmHandle), SA_AIS_OK);
        test_validate(rc, SA_AIS_OK);

}

void saClmResponse_02(void)
{
        int ret;

        trackFlags = SA_TRACK_CHANGES|SA_TRACK_START_STEP;

        safassert(saClmInitialize_4(&clmHandle, &clmCallback4, &clmVersion_4), SA_AIS_OK);
        safassert(saClmSelectionObjectGet(clmHandle, &selectionObject),SA_AIS_OK);
        rc = saClmClusterTrack_4(clmHandle, trackFlags, NULL);

        fds[0].fd = (int) selectionObject;
        fds[0].events = POLLIN;
        while (1){
                ret = poll(fds, 1, 1000);
                if (fds[0].revents & POLLIN){
                        break;
                }
        }
        safassert(saClmDispatch(clmHandle,SA_DISPATCH_ALL), SA_AIS_OK);
        safassert(saClmResponse_4(clmHandle,lock_inv,SA_CLM_CALLBACK_RESPONSE_ERROR),SA_AIS_OK);

        fds[0].fd = (int) selectionObject;
        fds[0].events = POLLIN;
        while (1){
                ret = poll(fds, 1, 1000);
                if (fds[0].revents & POLLIN){
                        break;
                }
        }

        safassert(saClmDispatch(clmHandle,SA_DISPATCH_ALL), SA_AIS_OK);

        safassert(saClmClusterTrackStop(clmHandle), SA_AIS_OK);
        safassert(saClmFinalize(clmHandle), SA_AIS_OK);
        test_validate(rc, SA_AIS_OK);

}

void saClmResponse_03(void)
{
        int ret;

        trackFlags = SA_TRACK_CHANGES|SA_TRACK_START_STEP;

        safassert(saClmInitialize_4(&clmHandle, &clmCallback4, &clmVersion_4), SA_AIS_OK);
        safassert(saClmSelectionObjectGet(clmHandle, &selectionObject),SA_AIS_OK);
        rc = saClmClusterTrack_4(clmHandle, trackFlags, NULL);

        fds[0].fd = (int) selectionObject;
        fds[0].events = POLLIN;
        while (1){
                ret = poll(fds, 1, 1000);
                if (fds[0].revents & POLLIN){
                        break;
                }
        }
        safassert(saClmDispatch(clmHandle,SA_DISPATCH_ALL), SA_AIS_OK);
        safassert(saClmResponse_4(clmHandle,lock_inv,SA_CLM_CALLBACK_RESPONSE_REJECTED),SA_AIS_OK);

        fds[0].fd = (int) selectionObject;
        fds[0].events = POLLIN;
        while (1){
                ret = poll(fds, 1, 1000);
                if (fds[0].revents & POLLIN){
                        break;
                }
        }

        safassert(saClmDispatch(clmHandle,SA_DISPATCH_ALL), SA_AIS_OK);

        safassert(saClmClusterTrackStop(clmHandle), SA_AIS_OK);
        safassert(saClmFinalize(clmHandle), SA_AIS_OK);
        test_validate(rc, SA_AIS_OK);

}
*/

void saClmResponse_04(void)
{
	rc = saClmResponse_4(NULL,200,SA_CLM_CALLBACK_RESPONSE_REJECTED);
	test_validate(rc, SA_AIS_ERR_INVALID_PARAM);
        rc = saClmResponse_4(1234,NULL,SA_CLM_CALLBACK_RESPONSE_REJECTED);
        test_validate(rc, SA_AIS_ERR_INVALID_PARAM);
        rc = saClmResponse_4(1234,200,10);
        test_validate(rc, SA_AIS_ERR_INVALID_PARAM);
        rc = saClmResponse_4(-1,200,SA_CLM_CALLBACK_RESPONSE_REJECTED);
        test_validate(rc, SA_AIS_ERR_BAD_HANDLE);
}

void saClmResponse_05(void)
{
	safassert(saClmInitialize_4(&clmHandle, NULL, &clmVersion_4), SA_AIS_OK);
	rc = saClmResponse_4(clmHandle,100,SA_CLM_CALLBACK_RESPONSE_OK);
	test_validate(rc, SA_AIS_OK);
}

__attribute__ ((constructor)) static void saClmResponse_constructor(void)
{
        test_suite_add(8, "Test case for saClmResponse");
/*
	test_case_add(8, saClmResponse_01, "saClmResponse with response as OK");
	test_case_add(8, saClmResponse_02, "saClmResponse with response as error");
	test_case_add(8, saClmResponse_03, "saClmResponse with response as rejected");
*/
	test_case_add(8, saClmResponse_04, "saClmResponse with invalid param");
	test_case_add(8, saClmResponse_05, "saClmResponse with invalid param");
}
