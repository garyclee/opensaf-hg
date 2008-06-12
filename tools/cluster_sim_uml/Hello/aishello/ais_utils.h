/*      -*- OpenSAF  -*-
 *
 *  Copyright (c) 2007, Ericsson AB
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under High-Availability Operating 
 * Environment Software License Version 1.4.
 * Complete License can be accesseble from below location.
 * http://www.opensaf.org/license 
 * See the Copying file included with the OpenSAF distribution for
 * full licensing terms.
 *
 * Author(s):
 *   
 */

#ifndef ais_utils_h
#define ais_utils_h

#include <saAmf.h>

/**
 * Convert various AIS error to strings for printout.
 */
char const* strSaAisErrorT(SaAisErrorT rc);
char const* strSaNameT(SaNameT const* name);
char const* strSaAmfHAStateT(SaAmfHAStateT s);

/**
 * Wraps AIS calls, takes care of retries and aborts on failures.  
 *
 * All calls to the OpenAIS API follows the same pattern; call, retry,
 * abort on failure. This is about 10 LOC for each call and quicly
 * feels awfully repetitive. This function does these tasks in one
 * call. The drawback is that the parameter typechecking is more or
 * less removed. Never the less, this is a convenient function :-)
 */
SaAisErrorT aisCallq(SaAisErrorT (*fn)(), ...);

#endif
