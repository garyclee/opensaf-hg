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
 * Author(s): Ericsson AB
 *
 */

#ifndef LGS_FMT_H
#define LGS_FMT_H

#define TOKEN_START_SYMBOL '@'
#define STRING_END_CHARACTER '\0'
#define TERMINATION_CHARACTER '\n'
#define START_YEAR 1900
#define DEFAULT_FMT_EXP_PTR_OFFSET 3
#define LITTERAL_CHAR_OFFSET 1
#define MAX_FIELD_SIZE 255

typedef enum {
    C_LR_ID_SHIFT_OFFSET,              
    C_LR_TIME_STAMP_SHIFT_OFFSET,      
    C_TIME_STAMP_HOUR_SHIFT_OFFSET,
    C_TIME_STAMP_MINUTE_SHIFT_OFFSET,
    C_TIME_STAMP_SECOND_SHIFT_OFFSET,
    C_TIME_STAMP_12_24_MODE_SHIFT_OFFSET,
    C_TIME_STAMP_MONTH_SHIFT_OFFSET,
    C_TIME_STAMP_MON_SHIFT_OFFSET,
    C_TIME_STAMP_DAY_SHIFT_OFFSET,
    C_TIME_STAMP_YEAR_SHIFT_OFFSET,
    C_TIME_STAMP_FULL_YEAR_SHIFT_OFFSET,
    C_NOTIFICATION_CLASS_ID_SHIFT_OFFSET,
    C_LR_TRUNCATION_INFO_SHIFT_OFFSET,
    C_LR_STRING_BODY_SHIFT_OFFSET,     
    C_LR_HEX_CHAR_BODY_SHIFT_OFFSET
} commonTokenShiftOffsetT;

typedef enum {
    N_NOTIFICATION_ID_SHIFT_OFFSET,    
    N_EVENT_TIME_SHIFT_OFFSET,
    N_EVENT_TIME_HOUR_SHIFT_OFFSET,
    N_EVENT_TIME_MINUTE_SHIFT_OFFSET,
    N_EVENT_TIME_SECOND_SHIFT_OFFSET,
    N_EVENT_TIME_12_24_MODE_SHIFT_OFFSET,
    N_EVENT_TIME_MONTH_SHIFT_OFFSET,
    N_EVENT_TIME_MON_SHIFT_OFFSET,
    N_EVENT_TIME_DAY_SHIFT_OFFSET,
    N_EVENT_TIME_YEAR_SHIFT_OFFSET,
    N_EVENT_TIME_FULL_YEAR_SHIFT_OFFSET,
    N_EVENT_TYPE_SHIFT_OFFSET,
    N_NOTIFICATION_OBJECT_SHIFT_OFFSET,
    N_NOTIFYING_OBJECT_SHIFT_OFFSET
} notificationTokenShiftOffsetT;

typedef enum {
    S_LOGGER_NAME_SHIFT_OFFSET,
    S_SEVERITY_ID_SHIFT_OFFSET
} systemTokenShiftOffsetT;

typedef enum {
    C_LR_ID_LETTER = 'r',              
    C_LR_TIME_STAMP_LETTER = 't',      
    C_TIME_STAMP_HOUR_LETTER = 'h',
    C_TIME_STAMP_MINUTE_LETTER = 'n',
    C_TIME_STAMP_SECOND_LETTER = 's',
    C_TIME_STAMP_12_24_MODE_LETTER = 'a',
    C_TIME_STAMP_MONTH_LETTER = 'm',
    C_TIME_STAMP_MON_LETTER = 'M',
    C_TIME_STAMP_DAY_LETTER = 'd',
    C_TIME_STAMP_YEAR_LETTER = 'y',
    C_TIME_STAMP_FULL_YEAR_LETTER = 'Y',
    C_NOTIFICATION_CLASS_ID_LETTER = 'c',
    C_LR_TRUNCATION_INFO_LETTER = 'x',
    C_LR_STRING_BODY_LETTER = 'b',     
    C_LR_HEX_CHAR_BODY_LETTER = 'i'
} commonTokenLetterT;

typedef enum {
    N_NOTIFICATION_ID_LETTER = 'i',
    N_EVENT_TIME_LETTER = 't',
    N_EVENT_TIME_HOUR_LETTER = 'h',
    N_EVENT_TIME_MINUTE_LETTER = 'n',
    N_EVENT_TIME_SECOND_LETTER = 's',
    N_EVENT_TIME_12_24_MODE_LETTER = 'a',
    N_EVENT_TIME_MONTH_LETTER = 'm',
    N_EVENT_TIME_MON_LETTER = 'M',
    N_EVENT_TIME_DAY_LETTER = 'd',
    N_EVENT_TIME_YEAR_LETTER = 'y',
    N_EVENT_TIME_FULL_YEAR_LETTER = 'Y',
    N_EVENT_TYPE_LETTER = 'e',
    N_NOTIFICATION_OBJECT_LETTER = 'o',
    N_NOTIFYING_OBJECT_LETTER = 'g'
} notificationTokenLetterT;

typedef enum {
    S_LOGGER_NAME_LETTER = 'l',
    S_SEVERITY_ID_LETTER = 'v'
} systemTokenLetterT;

typedef enum {
    COMMON_LOG_RECORD_FIELD_TYPE = 'C',
    NOTIFICATION_LOG_RECORD_FIELD_TYPE = 'N',
    SYSTEM_LOG_RECORD_FIELD_TYPE = 'S'
} logRecordFieldTypeT;

typedef enum {
    COMPLETED_LOG_RECORD = 'C',
    TRUNCATED_LOG_RECORD = 'T'
} logRecordTruncationCharacterT;

typedef enum {
    MONTH_JANUARY = 0,
    MONTH_FEBRUARY,
    MONTH_MARCH,
    MONTH_APRIL,
    MONTH_MAY,
    MONTH_JUNE,
    MONTH_JULY,
    MONTH_AUGUST,
    MONTH_SEPTEMBER,
    MONTH_OCTOBER,
    MONTH_NOVEMBER,
    MONTH_DECEMBER
} logRecordMonthTypeT;

typedef enum {
    STREAM_TYPE_ALARM = 0,
    STREAM_TYPE_NOTIFICATION,
    STREAM_TYPE_SYSTEM,
    STREAM_TYPE_APPLICATION
} logStreamTypeT;

extern SaBoolT lgs_is_valid_format_expression(const SaStringT,
                                            logStreamTypeT,
                                            SaBoolT *);
extern SaAisErrorT lgs_format_log_record(SaLogRecordT *,
                                       const SaStringT,
                                       const SaUint16T,
                                       SaStringT,
                                       SaUint32T);

#endif
