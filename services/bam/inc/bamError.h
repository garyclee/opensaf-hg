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

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <iostream.h>

XERCES_CPP_NAMESPACE_USE

/* ---------------------------------------------------------------------------
**  Simple error handler deriviative to install on parser
** ---------------------------------------------------------------------------
*/
class DOMBamErrorHandler : public DOMErrorHandler
{
public:
    /* -----------------------------------------------------------------------
    **  Constructors and Destructor
    ** -----------------------------------------------------------------------
    */
    DOMBamErrorHandler();
    ~DOMBamErrorHandler();


    /* -----------------------------------------------------------------------
    **  Getter methods
    ** -----------------------------------------------------------------------
    */
    bool getSawErrors() const;


    /* -----------------------------------------------------------------------
    **  Implementation of the DOM ErrorHandler interface
    ** -----------------------------------------------------------------------
    */
    bool handleError(const DOMError& domError);
    void resetErrors();


private :
    /* -----------------------------------------------------------------------
    **  Unimplemented constructors and operators
    ** -----------------------------------------------------------------------
    */
    DOMBamErrorHandler(const DOMBamErrorHandler&);
    void operator=(const DOMBamErrorHandler&);


    /* -----------------------------------------------------------------------
    **  Private data members
    **
    **  fSawErrors
    **      This is set if we get any errors, and is queryable via a getter
    **      method. Its used by the main code to suppress output if there are
    **      errors.
    ** -----------------------------------------------------------------------
    */
    bool    fSawErrors;
};
