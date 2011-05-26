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
*                                                                            *
*  MODULE NAME:  eds_util.c                                                  *
*                                                                            *
*                                                                            *
*  DESCRIPTION:                                                              *
*  This module contains utility routines used in the                         *
*  NCS Event Distribution Service Server (EDS).                              *
*                                                                            *
*****************************************************************************/
#include "eds.h"

/***************************************************************************
 *
 * eds_pattern_match() - Compare a patternArray with a filterArray
 * 
 * Returns TRUE   If all pattern/filter compares succeed.
 *         FALSE  On the first miss-match.
 *
 ***************************************************************************/
NCS_BOOL eds_pattern_match(SaEvtEventPatternArrayT *patternArray, SaEvtEventFilterArrayT *filterArray)
{
	uns32 x;
	uint8_t *p = NULL;
	SaEvtEventFilterT *filter;
	SaEvtEventPatternT *pattern;
	SaEvtEventPatternT emptyPattern = { 0, 0, NULL };

	if ((patternArray == NULL) || (filterArray == NULL))
		return (FALSE);

	pattern = patternArray->patterns;
	filter = filterArray->filters;

	if (!pattern)
		pattern = &emptyPattern;

	for (x = 1; x <= filterArray->filtersNumber; x++) {
		switch (filter->filterType) {
		case SA_EVT_PREFIX_FILTER:
			/* if either filter or pattern alone is empty, then no match */
			if ((pattern->patternSize == 0) && (filter->filter.patternSize != 0))
				return (FALSE);
			if (memcmp(filter->filter.pattern, pattern->pattern, (size_t)filter->filter.patternSize) != 0)
				return (FALSE);	/* No match */
			break;

		case SA_EVT_SUFFIX_FILTER:
			/* if either filter or pattern alone is empty, then no match */
			if ((pattern->patternSize == 0) && (filter->filter.patternSize != 0))
				return (FALSE);

			/* Pattern must be at least as long as filter for a match */
			if (pattern->patternSize < filter->filter.patternSize)
				return (FALSE);

			if ((pattern->patternSize == 0) && (filter->filter.patternSize != 0))
				return (FALSE);

			/* Set p to offset into pattern */
			p = pattern->pattern + ((int)pattern->patternSize - (int)filter->filter.patternSize);
			if (memcmp(filter->filter.pattern, p, (size_t)filter->filter.patternSize) != 0)
				return (FALSE);
			break;

		case SA_EVT_EXACT_FILTER:
			if ((pattern == NULL) && (filter != NULL))
				return (FALSE);	/* Fix. More filters than patterns case */

			if (filter->filter.patternSize == pattern->patternSize) {
				if (memcmp(filter->filter.pattern, pattern->pattern,
					   (size_t)filter->filter.patternSize) != 0)
					return (FALSE);
			} else
				return FALSE;
			break;

		case SA_EVT_PASS_ALL_FILTER:
			break;

		default:
			return (FALSE);
		}

		/*
		 * Increment to next filter and pattern.
		 * If more filters than patterns, set pattern to the empty pattern.
		 * The remaining filters MUST match the empty pattern.
		 *
		 * If more patterns than filters, simply exit the loop assuming a match
		 * for the remaining patterns (assume filter = SA_EVT_PASS_ALL_FILTER).
		 */
		filter++;

		if (x < patternArray->patternsNumber)
			pattern++;
		else
			pattern = &emptyPattern;

	}			/* End for */

	return (TRUE);
}

/***************************************************************************
 *
 * eds_calc_filter_size() - Calculate the size in bytes of a filterArray.
 * 
 * Calculates the size in bytes of a filterArray by adding up the sizes
 * of the structures, then adding in the size of each individual pattern.
 *
 ***************************************************************************/
uns32 eds_calc_filter_size(SaEvtEventFilterArrayT *filterArray)
{
	uns32 size = 0;
	uns32 x;
	SaEvtEventFilterT *filterp;

	if (filterArray == NULL)
		return (0);

	/* First compute how much space is needed to hold the structures */
	size = sizeof(SaEvtEventFilterArrayT) + ((uns32)filterArray->filtersNumber * sizeof(SaEvtEventFilterT));

	/*
	 * Now add in the individual pattern data sizes.
	 */

	/* Point to first filter in the array */
	filterp = filterArray->filters;
	for (x = 1; x <= filterArray->filtersNumber; x++) {
		size += (uns32)filterp->filter.patternSize;
		filterp++;
	}

	return (size);
}
