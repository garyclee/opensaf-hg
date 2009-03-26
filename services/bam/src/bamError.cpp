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

/*****************************************************************************
..............................................................................



..............................................................................

  DESCRIPTION:

  
******************************************************************************
*/

#include "bamError.h"
#include "bam.h"
#include "bam_log.h"

XERCES_CPP_NAMESPACE_USE

DOMBamErrorHandler::DOMBamErrorHandler()
{   
    fSawErrors = false;
}

DOMBamErrorHandler::~DOMBamErrorHandler()
{
}


// ---------------------------------------------------------------------------
//  DOMBamHandlers: Overrides of the DOM ErrorHandler interface
// ---------------------------------------------------------------------------
bool DOMBamErrorHandler::handleError(const DOMError& domError)
{
    fSawErrors = true;
    char location[128];

    char *str1 = XMLString::transcode(domError.getLocation()->getURI());
    char *str2 = XMLString::transcode(domError.getMessage());
    sprintf(location, "%s%ld, %s%ld", "line: ", 
            domError.getLocation()->getLineNumber(),
            "char: ", domError.getLocation()->getColumnNumber()); 

    syslog(LOG_ERR, "NCS_AvSv: XML Parse error! : %s", str1);
    syslog(LOG_ERR, "NCS_AvSv: XML Parse error! : %s", location);
    if(str2)
        syslog(LOG_ERR, "NCS_AvSv: XML Parse error! : %s", str2);

    if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING)
        m_LOG_BAM_MSG_TICL(BAM_BOMFILE_ERROR, NCSFL_SEV_ERROR, 
                        "Warning at file. ", str1);
    else if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR)
        m_LOG_BAM_MSG_TICL(BAM_BOMFILE_ERROR, NCSFL_SEV_ERROR, 
                        "ERROR at file. ", str1);
    else
        m_LOG_BAM_MSG_TICL(BAM_BOMFILE_ERROR, NCSFL_SEV_ERROR, 
                        "FATAL ERROR at file. ", str1);

    XMLString::release(&str1);
    XMLString::release(&str2);

    return true;
}

void DOMBamErrorHandler::resetErrors()
{
    fSawErrors = false;
}

bool DOMBamErrorHandler::getSawErrors() const
{
    return fSawErrors;
}
