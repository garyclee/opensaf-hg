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

#include "lgs.h"
#include "lgs_fmt.h"
#include "lgs_util.h"

/**
 * 
 * @param inputString
 * @param numOfDigits
 * 
 * @return SaInt32T
 */
static SaInt32T checkFieldSize(SaStringT inputString,
                               SaUint16T *numOfDigits)
{
    SaInt32T result = 0;
    SaInt8T tempString[SA_MAX_NAME_LENGTH];
    char **endptr = NULL;

    (void)strcpy(tempString, ""); /* Init tempString */
    *numOfDigits = 0;

    /* Build string of characters */
    while ((isdigit(*inputString) != 0) &&
           (*numOfDigits < SA_MAX_NAME_LENGTH))
    {
        *numOfDigits = *numOfDigits + 1;
        (void)strncat(tempString, inputString++, sizeof(SaInt8T)); 
    }

    result = strtol(tempString, endptr, 0); 
    return result;
}

/**
 * 
 * @param fmtExpPtr
 * @param fmtExpPtrOffset
 * @param tokenFlags
 * @param twelveHourModeFlag
 * @param logStreamId
 * 
 * @return SaBoolT
 */
static SaBoolT validateComToken(SaStringT fmtExpPtr, 
                                SaUint16T *fmtExpPtrOffset,
                                SaUint32T *tokenFlags,
                                SaBoolT *twelveHourModeFlag,
                                logStreamTypeT logStreamType)
{
    SaBoolT tokenOk = SA_TRUE;
    SaInt32T fieldSize = 0;
    SaUint16T fieldSizeOffset = 0;
    SaUint16T shiftOffset;
    *fmtExpPtrOffset = DEFAULT_FMT_EXP_PTR_OFFSET; 

    switch (*fmtExpPtr++)
    {
        case C_LR_ID_LETTER: 
            shiftOffset = (int)C_LR_ID_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_LR_TIME_STAMP_LETTER: 
            shiftOffset = (int)C_LR_TIME_STAMP_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_TIME_STAMP_HOUR_LETTER:
            shiftOffset = (int)C_TIME_STAMP_HOUR_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_TIME_STAMP_MINUTE_LETTER:
            shiftOffset = (int)C_TIME_STAMP_MINUTE_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_TIME_STAMP_SECOND_LETTER:
            shiftOffset = (int)C_TIME_STAMP_SECOND_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_TIME_STAMP_12_24_MODE_LETTER:
            shiftOffset = (int)C_TIME_STAMP_12_24_MODE_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            *twelveHourModeFlag = SA_TRUE;
            break;

        case C_TIME_STAMP_MONTH_LETTER:
            shiftOffset = (int)C_TIME_STAMP_MONTH_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_TIME_STAMP_MON_LETTER:
            shiftOffset = (int)C_TIME_STAMP_MON_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_TIME_STAMP_DAY_LETTER: 
            shiftOffset = (int)C_TIME_STAMP_DAY_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_TIME_STAMP_YEAR_LETTER:
            shiftOffset = (int)C_TIME_STAMP_YEAR_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_TIME_STAMP_FULL_YEAR_LETTER:
            shiftOffset = (int)C_TIME_STAMP_FULL_YEAR_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_NOTIFICATION_CLASS_ID_LETTER:
            shiftOffset = (int)C_NOTIFICATION_CLASS_ID_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_LR_TRUNCATION_INFO_LETTER:
            shiftOffset = (int)C_LR_TRUNCATION_INFO_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case C_LR_STRING_BODY_LETTER:
            shiftOffset = (int)C_LR_STRING_BODY_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset);
            if (fieldSize > MAX_FIELD_SIZE)
            {
                tokenOk = SA_FALSE;
            }
            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;
            break;

        case C_LR_HEX_CHAR_BODY_LETTER:
            shiftOffset = (int)C_LR_HEX_CHAR_BODY_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset);
            if (fieldSize > MAX_FIELD_SIZE)
            {
                tokenOk = SA_FALSE;
            }
            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;

            /* This token is valid for application log streams only */
            if (logStreamType <= STREAM_TYPE_SYSTEM)
            {
                tokenOk = SA_FALSE;
            }
            break;

        default: /* Non valid token letter */
            *fmtExpPtrOffset = 1;
            tokenOk = SA_FALSE;
            break;
    }

    return tokenOk;
}

/**
 * 
 * @param fmtExpPtr
 * @param fmtExpPtrOffset
 * @param tokenFlags
 * @param twelveHourModeFlag
 * 
 * @return SaBoolT
 */
static SaBoolT validateNtfToken(SaStringT fmtExpPtr,
                                SaUint16T *fmtExpPtrOffset,
                                SaUint32T *tokenFlags,
                                SaBoolT *twelveHourModeFlag)
{
    SaBoolT tokenOk = SA_TRUE;
    SaInt32T fieldSize = 0;
    SaUint16T fieldSizeOffset;
    SaUint16T shiftOffset = 0;
    *fmtExpPtrOffset = DEFAULT_FMT_EXP_PTR_OFFSET;

    switch (*fmtExpPtr++)
    {
        case N_NOTIFICATION_ID_LETTER: 
            shiftOffset = (int)N_NOTIFICATION_ID_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case N_EVENT_TIME_LETTER:
            shiftOffset = (int)N_EVENT_TIME_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case N_EVENT_TIME_HOUR_LETTER:
            shiftOffset = (int)N_EVENT_TIME_HOUR_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case N_EVENT_TIME_MINUTE_LETTER:
            shiftOffset = (int)N_EVENT_TIME_MINUTE_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case N_EVENT_TIME_12_24_MODE_LETTER:
            shiftOffset = (int)N_EVENT_TIME_12_24_MODE_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            *twelveHourModeFlag = SA_TRUE;
            break;

        case N_EVENT_TIME_MONTH_LETTER:
            shiftOffset = (int)N_EVENT_TIME_MONTH_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case N_EVENT_TIME_MON_LETTER:
            shiftOffset = (int)N_EVENT_TIME_MON_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case N_EVENT_TIME_DAY_LETTER:
            shiftOffset = (int)N_EVENT_TIME_DAY_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case N_EVENT_TIME_YEAR_LETTER:
            shiftOffset = (int)N_EVENT_TIME_YEAR_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case N_EVENT_TIME_FULL_YEAR_LETTER:
            shiftOffset = (int)N_EVENT_TIME_FULL_YEAR_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case N_EVENT_TYPE_LETTER:
            shiftOffset = (int)N_EVENT_TYPE_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset);
            if (fieldSize > MAX_FIELD_SIZE)
            {
                tokenOk = SA_FALSE;
            }
            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;
            break;

        case N_NOTIFICATION_OBJECT_LETTER:
            shiftOffset = (int)N_NOTIFICATION_OBJECT_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset);
            if (fieldSize > MAX_FIELD_SIZE)
            {
                tokenOk = SA_FALSE;
            }
            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;
            break;

        case N_NOTIFYING_OBJECT_LETTER:
            shiftOffset = (int)N_NOTIFYING_OBJECT_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset);
            if (fieldSize > MAX_FIELD_SIZE)
            {
                tokenOk = SA_FALSE;
            }
            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;
            break;

        default: /* Non valid token letter */
            *fmtExpPtrOffset = 1;
            tokenOk = SA_FALSE;
            break;
    }

    return tokenOk;
}

/**
 * 
 * @param fmtExpPtr
 * @param fmtExpPtrOffset
 * @param tokenFlags
 * 
 * @return SaBoolT
 */
static SaBoolT validateSysToken(SaStringT fmtExpPtr,
                                SaUint16T *fmtExpPtrOffset,
                                SaUint32T *tokenFlags)
{
    SaBoolT tokenOk = SA_TRUE;
    SaInt32T fieldSize = 0;
    SaUint16T fieldSizeOffset = 0;
    SaUint16T shiftOffset;
    *fmtExpPtrOffset = DEFAULT_FMT_EXP_PTR_OFFSET;

    switch (*fmtExpPtr++)
    {
        case S_SEVERITY_ID_LETTER:
            shiftOffset = (int)S_SEVERITY_ID_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            break;

        case S_LOGGER_NAME_LETTER:
            shiftOffset = (int)S_LOGGER_NAME_SHIFT_OFFSET;
            if ((SaBoolT)((*tokenFlags >> shiftOffset) & 1) == SA_TRUE)
            {
                tokenOk = SA_FALSE; /* Same token used two times */
            }
            else
            {
                *tokenFlags = (*tokenFlags | (1 << shiftOffset));
            }
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset);
            if (fieldSize > MAX_FIELD_SIZE)
            {
                tokenOk = SA_FALSE;
            }
            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;
            break;

        default: /* Non valid token letter */
            *fmtExpPtrOffset = 1;
            tokenOk = SA_FALSE;
            break;
    }

    return tokenOk;
}

/**
 * 
 * @param fmtExpPtr
 * @param fmtExpPtrOffset
 * @param truncationLetterPos
 * @param inputPos
 * @param twelveHourModeFlag
 * @param timeStampData
 * @param commonData
 * @param genHeader
 * 
 * @return SaStringT
 */
static SaStringT extractCommonField(SaStringT fmtExpPtr, 
                                    SaUint16T *fmtExpPtrOffset,
                                    SaInt32T *truncationLetterPos,
                                    SaInt32T inputPos,
                                    SaUint32T logRecordIdCounter,
                                    const SaBoolT *twelveHourModeFlag,
                                    const struct tm *timeStampData,
                                    const SaLogRecordT *logRecord)
{
    SaInt32T fieldSize;
    SaInt32T stringSize;
    SaUint16T fieldSizeOffset = 0;
    SaInt32T characters = 0;
    static SaInt8T tempString[SA_MAX_NAME_LENGTH];

    tempString[SA_MAX_NAME_LENGTH - 1] = TERMINATION_CHARACTER; 

    *fmtExpPtrOffset = DEFAULT_FMT_EXP_PTR_OFFSET;

    switch (*fmtExpPtr++)
    { /* input points to a possible field size */
        case C_LR_ID_LETTER:
            stringSize = 11 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "% 10d", 
                                  (int)logRecordIdCounter);
            break; 

        case C_LR_TIME_STAMP_LETTER: 
            stringSize = 19 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%#016llx", 
                                  logRecord->logTimeStamp);  
            break;

        case C_TIME_STAMP_HOUR_LETTER:
            stringSize = 3 * sizeof (char);
            if (*twelveHourModeFlag == SA_TRUE)
            {
                characters = snprintf(tempString, 
                                      (size_t)stringSize, 
                                      "%02d", 
                                      (timeStampData->tm_hour % 12));
            }
            else
            {
                characters = snprintf(tempString, 
                                      (size_t)stringSize, 
                                      "%02d", 
                                      timeStampData->tm_hour);
            } 
            break;

        case C_TIME_STAMP_MINUTE_LETTER:
            stringSize = 3 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize,
                                  "%02d", 
                                  timeStampData->tm_min);
            break;

        case C_TIME_STAMP_SECOND_LETTER:
            stringSize = 3 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%02d", 
                                  timeStampData->tm_sec);
            break;

        case C_TIME_STAMP_12_24_MODE_LETTER:
            stringSize = 3 * sizeof(char);
            if (timeStampData->tm_hour >= 00 && timeStampData->tm_hour < 12)
            {
                characters = snprintf(tempString, (size_t)stringSize, "am");
            }
            else
            {
                characters = snprintf(tempString, (size_t)stringSize, "pm");
            }
            break;

        case C_TIME_STAMP_MONTH_LETTER:
            stringSize = 3 * sizeof(char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%02d", 
                                  (timeStampData->tm_mon + 1));
            break;

        case C_TIME_STAMP_MON_LETTER:
            stringSize = 4 * sizeof(char);
            switch (timeStampData->tm_mon)
            {
                case MONTH_JANUARY:
                    characters = snprintf(tempString, (size_t)stringSize, "Jan"); 
                    break;

                case MONTH_FEBRUARY:
                    characters = snprintf(tempString, (size_t)stringSize, "Feb"); 
                    break;

                case MONTH_MARCH:
                    characters = snprintf(tempString, (size_t)stringSize, "Mar"); 
                    break;

                case MONTH_APRIL:
                    characters = snprintf(tempString, (size_t)stringSize, "Apr"); 
                    break;

                case MONTH_MAY:
                    characters = snprintf(tempString, (size_t)stringSize, "May"); 
                    break;

                case MONTH_JUNE:
                    characters = snprintf(tempString, (size_t)stringSize, "Jun"); 
                    break;

                case MONTH_JULY:
                    characters = snprintf(tempString, (size_t)stringSize, "Jul"); 
                    break;

                case MONTH_AUGUST:
                    characters = snprintf(tempString, (size_t)stringSize, "Aug"); 
                    break;

                case MONTH_SEPTEMBER:
                    characters = snprintf(tempString, (size_t)stringSize, "Sep"); 
                    break;

                case MONTH_OCTOBER:
                    characters = snprintf(tempString, (size_t)stringSize, "Oct"); 
                    break;

                case MONTH_NOVEMBER:
                    characters = snprintf(tempString, (size_t)stringSize, "Nov"); 
                    break;

                case MONTH_DECEMBER:
                    characters = snprintf(tempString, (size_t)stringSize, "Dec"); 
                    break;

                default:
                    (void)strcpy(tempString, "");
                    break;

            }

            break;

        case C_TIME_STAMP_DAY_LETTER:
            stringSize = 3 * sizeof(char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%02d", 
                                  timeStampData->tm_mday);
            break;

        case C_TIME_STAMP_YEAR_LETTER:
            stringSize = 3 * sizeof(char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%02d", 
                                  timeStampData->tm_year);
            break;

        case C_TIME_STAMP_FULL_YEAR_LETTER:
            stringSize = 5 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%d", 
                                  (timeStampData->tm_year + START_YEAR));
            break;

        case C_NOTIFICATION_CLASS_ID_LETTER:
            stringSize = 30 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "NCI[%#08x,%#04x,%#04x]",
                                  (unsigned int)logRecord->logHeader.genericHdr.notificationClassId->vendorId,
                                  logRecord->logHeader.genericHdr.notificationClassId->majorId,
                                  logRecord->logHeader.genericHdr.notificationClassId->minorId);
            break;

        case C_LR_TRUNCATION_INFO_LETTER:
            stringSize = 2 * sizeof (char);
            /* A space inserted at the truncationCharacter:s position */
            characters = snprintf(tempString, (size_t)stringSize, " ");                                   
            *truncationLetterPos = inputPos; /* The position of the truncation 
                                                character in the log record */
            break;

        case C_LR_STRING_BODY_LETTER:
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset); 
            if (fieldSize == 0)
            {
                /* Copy whole body */
                stringSize = (SaInt32T)logRecord->logBuffer->logBufSize + 1;
                characters = snprintf(tempString,
                                      (size_t)stringSize,
                                      "%s",
                                      (SaStringT)logRecord->logBuffer->logBuf);
            }
            else
            {
                /* Truncate or pad the body with blanks until fieldSize */ 
                stringSize = (size_t)fieldSize + 1;
                characters = snprintf(tempString,
                                      stringSize,
                                      "%*.*s", 
                                      (int)-fieldSize, 
                                      (int)fieldSize, 
                                      (SaStringT)logRecord->logBuffer->logBuf);
            }
            if (stringSize < (SA_MAX_NAME_LENGTH - 1))
            {
                tempString[stringSize + 1] = TERMINATION_CHARACTER; /* '\0' terminate */
            }

            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;
            break;

        case C_LR_HEX_CHAR_BODY_LETTER:  
            stringSize = (SaInt32T)logRecord->logBuffer->logBufSize;
            characters = snprintf(tempString, 
                                  (size_t)stringSize,
                                  "%lx",
                                  (long unsigned int)logRecord->logBuffer->logBuf);
            break;

        default:
            characters = 0; 
            break;

    }

    /* Error */
    if (characters == -1)
    {
        (void)strcpy(tempString, ""); /* Only '\0' */
    }
    else
    {
        tempString[characters] = STRING_END_CHARACTER; /* NULL terminate */
    }

    return tempString;
}

/**
 * 
 * @param fmtExpPtr
 * @param fmtExpPtrOffset
 * @param twelveHourModeFlag
 * @param commonData
 * @param ntfHeader
 * 
 * @return SaStringT
 */
static SaStringT extractNotificationField(SaStringT fmtExpPtr, 
                                          SaUint16T *fmtExpPtrOffset,
                                          const SaBoolT *twelveHourModeFlag,
                                          const SaLogRecordT *logRecord)
{
    struct tm *eventTimeData;
    SaTimeT totalTime;
    static SaInt8T tempString[SA_MAX_NAME_LENGTH];
    SaInt32T fieldSize;
    SaInt32T stringSize;
    SaInt32T characters = 0;
    SaUint16T fieldSizeOffset = 0;

    *fmtExpPtrOffset = DEFAULT_FMT_EXP_PTR_OFFSET;

    /* Convert event time into */
    totalTime = (logRecord->logHeader.ntfHdr.eventTime / (SaTimeT)SA_TIME_ONE_SECOND);

    /* Split timestamp in timeStampData */
    eventTimeData = localtime((const time_t *)&totalTime);

    switch (*fmtExpPtr++)
    {
        case N_NOTIFICATION_ID_LETTER:
            stringSize = 19 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%#016llx", 
                                  logRecord->logHeader.ntfHdr.notificationId); 
            break;

        case N_EVENT_TIME_LETTER:
            stringSize = 19 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%#016llx", 
                                  logRecord->logHeader.ntfHdr.eventTime);
            break;

        case N_EVENT_TIME_HOUR_LETTER:
            stringSize = 3 * sizeof (char);
            if (*twelveHourModeFlag == SA_TRUE)
            {
                characters = snprintf(tempString, 
                                      (size_t)stringSize, 
                                      "%02d", 
                                      (eventTimeData->tm_hour % 12));
            }
            else
            {
                characters = snprintf(tempString, 
                                      (size_t)stringSize, 
                                      "%02d", 
                                      eventTimeData->tm_hour);
            }
            break;

        case N_EVENT_TIME_MINUTE_LETTER:
            stringSize = 3 * sizeof (char);
            characters = snprintf(tempString, (size_t)stringSize, "%02d", eventTimeData->tm_min);
            break;

        case N_EVENT_TIME_SECOND_LETTER:
            stringSize = 3 * sizeof (char);
            characters = snprintf(tempString, (size_t)stringSize, "%02d", eventTimeData->tm_sec);
            break;

        case N_EVENT_TIME_12_24_MODE_LETTER:
            stringSize = 3 * sizeof (char);
            if (eventTimeData->tm_hour >= 00 && eventTimeData->tm_hour < 12)
            {
                characters = snprintf(tempString, (size_t)stringSize, "am");
            }
            else
            {
                characters = snprintf(tempString, (size_t)stringSize, "pm");
            }
            break;

        case N_EVENT_TIME_MONTH_LETTER:
            stringSize = 3 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%02d", 
                                  (eventTimeData->tm_mon + 1));
            break;

        case N_EVENT_TIME_MON_LETTER:
            stringSize = 4 * sizeof (char);
            switch (eventTimeData->tm_mon)
            {
                case MONTH_JANUARY:
                    characters = snprintf(tempString, (size_t)stringSize, "Jan"); 
                    break;

                case MONTH_FEBRUARY:
                    characters = snprintf(tempString, (size_t)stringSize, "Feb"); 
                    break;

                case MONTH_MARCH:
                    characters = snprintf(tempString, (size_t)stringSize, "Mar"); 
                    break;

                case MONTH_APRIL:
                    characters = snprintf(tempString, (size_t)stringSize, "Apr"); 
                    break;

                case MONTH_MAY:
                    characters = snprintf(tempString, (size_t)stringSize, "May"); 
                    break;

                case MONTH_JUNE:
                    characters = snprintf(tempString, (size_t)stringSize, "Jun"); 
                    break;

                case MONTH_JULY:
                    characters = snprintf(tempString, (size_t)stringSize, "Jul"); 
                    break;

                case MONTH_AUGUST:
                    characters = snprintf(tempString, (size_t)stringSize, "Aug"); 
                    break;

                case MONTH_SEPTEMBER:
                    characters = snprintf(tempString, (size_t)stringSize, "Sep"); 
                    break;

                case MONTH_OCTOBER:
                    characters = snprintf(tempString, (size_t)stringSize, "Oct"); 
                    break;

                case MONTH_NOVEMBER:
                    characters = snprintf(tempString, (size_t)stringSize, "Nov"); 
                    break;

                case MONTH_DECEMBER:
                    characters = snprintf(tempString, (size_t)stringSize, "Dec"); 
                    break;

                default:
                    (void)strcpy(tempString, "");
                    break;

            }
            break;

        case N_EVENT_TIME_DAY_LETTER:
            stringSize = 3 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%02d", 
                                  eventTimeData->tm_mday);
            break;

        case N_EVENT_TIME_YEAR_LETTER:
            stringSize = 3 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%02d", 
                                  eventTimeData->tm_year);
            break;

        case N_EVENT_TIME_FULL_YEAR_LETTER:
            stringSize = 5 * sizeof (char);
            characters = snprintf(tempString, 
                                  (size_t)stringSize, 
                                  "%d", 
                                  (eventTimeData->tm_year + START_YEAR));
            break;

        case N_EVENT_TYPE_LETTER:
            /* Check field size */
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset);
            /* TODO!!! Fit hex output to size => two steps for non strings */
            /* 0x included in total field size */
            characters = snprintf(tempString, 
                                  ((size_t)fieldSize + 1), /* Incl NULL */
                                  "%#.*x", 
                                  (int)(fieldSize - 2), 
                                  logRecord->logHeader.ntfHdr.eventType);
            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;
            break;

        case N_NOTIFICATION_OBJECT_LETTER:
            /* Check field size and trunkate alternative pad with blanks */
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset);
            characters = snprintf(tempString, 
                                  ((size_t)fieldSize + 1), /* Incl NULL */
                                  "%*.*s", 
                                  (int)-fieldSize, 
                                  (int)fieldSize, 
                                  logRecord->logHeader.ntfHdr.notificationObject->value);
            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;
            break;

        case N_NOTIFYING_OBJECT_LETTER:
            /* Check field size and trunkate alternative pad with blanks */
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset);
            characters = snprintf(tempString, 
                                  ((size_t)fieldSize + 1), /* Incl NULL */
                                  "%*.*s", 
                                  (int)-fieldSize, 
                                  (int)fieldSize, 
                                  logRecord->logHeader.ntfHdr.notifyingObject->value);
            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;
            break;

        default:
            (void)strcpy(tempString,"");
            break;
    }

    /* Error */
    if (characters == -1)
    {
        (void)strcpy(tempString, "");
    }
    else
    {
        tempString[characters] = STRING_END_CHARACTER; /* NULL terminate */
    }

    return tempString;
}

/**
 * 
 * @param fmtExpPtr
 * @param fmtExpPtrOffset
 * @param commonData
 * @param genHeader
 * 
 * @return SaStringT
 */
static SaStringT extractSystemField(SaStringT fmtExpPtr, 
                                    SaUint16T *fmtExpPtrOffset,
                                    const SaLogRecordT *logRecord)
{
    SaInt32T fieldSize;
    SaInt32T stringSize;
    SaInt32T characters = 0;
    SaUint16T fieldSizeOffset = 0;
    static SaInt8T tempString[SA_MAX_NAME_LENGTH];
    *fmtExpPtrOffset = DEFAULT_FMT_EXP_PTR_OFFSET;

    switch (*fmtExpPtr++)
    { /* input points to a possible field size */
        case S_LOGGER_NAME_LETTER:
            fieldSize = checkFieldSize(fmtExpPtr, &fieldSizeOffset);
            if (fieldSize != 0)
            {
                characters = snprintf(
                                     tempString,
                                     ((size_t)fieldSize + 1), /* Incl NULL */
                                     "%*.*s", 
                                     (int)-fieldSize, 
                                     (int)fieldSize, 
                                     logRecord->logHeader.genericHdr.logSvcUsrName->value);
            }
            else
            {
                characters = snprintf(
                                     tempString, 
                                     sizeof (tempString), 
                                     "%s", 
                                     logRecord->logHeader.genericHdr.logSvcUsrName->value);
            }
            *fmtExpPtrOffset = *fmtExpPtrOffset + fieldSizeOffset;
            break;

        case S_SEVERITY_ID_LETTER:
            stringSize = 3 * sizeof (char);
            switch (logRecord->logHeader.genericHdr.logSeverity)
            {
                case SA_LOG_SEV_EMERGENCY:
                    characters = snprintf(tempString, (size_t)stringSize, "EM");
                    break;

                case SA_LOG_SEV_ALERT:
                    characters = snprintf(tempString, (size_t)stringSize, "AL");
                    break;

                case SA_LOG_SEV_CRITICAL:
                    characters = snprintf(tempString, (size_t)stringSize, "CR");
                    break;

                case SA_LOG_SEV_ERROR:
                    characters = snprintf(tempString, (size_t)stringSize, "ER");
                    break;

                case SA_LOG_SEV_WARNING:
                    characters = snprintf(tempString, (size_t)stringSize, "WA");
                    break;

                case SA_LOG_SEV_NOTICE:
                    characters = snprintf(tempString, (size_t)stringSize, "NO");
                    break;

                case SA_LOG_SEV_INFO:
                    characters = snprintf(tempString, (size_t)stringSize, "IN");
                    break;

                default:
                    (void)strcpy(tempString, "");
                    break;

            }
            break;

        default:
            (void)strcpy(tempString, "");
            break;

    }
    if (characters == -1)
    {
        (void)strcpy(tempString, "");
    }
    else
    {
        tempString[characters] = STRING_END_CHARACTER; /* NULL terminate */
    }

    return tempString;
}

/**
 * Validate a format expression
 * @param formatExpression
 * @param logStreamType
 * @param twelveHourModeFlag
 * 
 * @return SaBoolT
 */
SaBoolT lgs_is_valid_format_expression(const SaStringT formatExpression,
                                       logStreamTypeT logStreamType,
                                       SaBoolT *twelveHourModeFlag)
{
    SaBoolT   formatExpressionOk = SA_FALSE;
    SaBoolT   tokenOk = SA_FALSE;
    SaUint32T comTokenFlags = 0x00000000;
    SaUint32T ntfTokenFlags = 0x00000000;
    SaUint32T sysTokenFlags = 0x00000000;
    SaUint16T fmtExpTokenOffset = 1;
    SaStringT fmtExpPtr = &formatExpression[0];
    SaStringT fmtExpPtrSnabel = &formatExpression[1];

    if (formatExpression == NULL)
    {
        goto error_exit;
    }

    /* Main checking loop */
    for (; ;)
    { /* Scan the Format Expression */
        if ((*fmtExpPtr == TOKEN_START_SYMBOL) && 
            (*fmtExpPtrSnabel != STRING_END_CHARACTER))
        {
            switch (*fmtExpPtrSnabel++)
            { /* New token to be validated */
                case COMMON_LOG_RECORD_FIELD_TYPE:    
                    tokenOk = validateComToken(fmtExpPtrSnabel, 
                                               &fmtExpTokenOffset,
                                               &comTokenFlags,
                                               twelveHourModeFlag,
                                               logStreamType);
                    break;

                case NOTIFICATION_LOG_RECORD_FIELD_TYPE:
                    tokenOk = validateNtfToken(fmtExpPtrSnabel, 
                                               &fmtExpTokenOffset,
                                               &ntfTokenFlags,
                                               twelveHourModeFlag);
                    if (logStreamType > STREAM_TYPE_NOTIFICATION)
                    {
                        tokenOk = SA_FALSE; /* These tokens valid for alarm and
                                               ntf log streams only */
                    }
                    break;

                case SYSTEM_LOG_RECORD_FIELD_TYPE:
                    tokenOk = validateSysToken(fmtExpPtrSnabel, 
                                               &fmtExpTokenOffset,
                                               &sysTokenFlags);
                    if (logStreamType < STREAM_TYPE_SYSTEM)
                    {
                        tokenOk = SA_FALSE; /* These tokens valid for app and
                                               sys log streams only */
                    }
                    break;

                default: /* Non valid log record field type */
                    tokenOk = SA_FALSE;
                    break;
            }  
        }
        else if (*fmtExpPtrSnabel != STRING_END_CHARACTER)
        {
            /* All chars between tokens */
            fmtExpTokenOffset = LITTERAL_CHAR_OFFSET; 
        }
        else
        { /* End of formatExpression */
            if (*fmtExpPtr == TOKEN_START_SYMBOL)
            {
                tokenOk = SA_FALSE; /* Illegal litteral character at the end */
            }
            break;
        }

        /* Step forward according to token offset */
        fmtExpPtr += fmtExpTokenOffset;
        if ((tokenOk == SA_FALSE) || (*fmtExpPtr == STRING_END_CHARACTER))
        {
            break; /* Illegal token or end of formatExpression */
        }
        fmtExpPtrSnabel = (fmtExpPtr + 1); 
    }
    formatExpressionOk = tokenOk;

    error_exit:
    return formatExpressionOk;
}  

/**
 * Format and write a log record
 * @param logRecord
 * @param formatExpression
 * @param fixedLogRecordSize
 * @param outputLogRecord
 * @param logRecordIdCounter
 * 
 * @return SaAisErrorT
 */
SaAisErrorT lgs_format_log_record(SaLogRecordT *logRecord,
                                  const SaStringT formatExpression,
                                  const SaUint16T fixedLogRecordSize,
                                  SaStringT outputLogRecord,
                                  SaUint32T logRecordIdCounter)
{
    SaAisErrorT error = SA_AIS_OK;
    SaStringT fmtExpPtr = &formatExpression[0];
    SaStringT fmtExpPtrSnabel = &formatExpression[1];
    SaTimeT totalTime;
    SaInt8T tempString[SA_MAX_NAME_LENGTH];
    SaInt8T truncationCharacter = (SaInt8T)COMPLETED_LOG_RECORD;
    SaUint16T fmtExpTokenOffset = 1;
    SaInt32T truncationLetterPos = -1;
    SaUint32T logRecordCharCounter = 0; 
    SaInt32T characters = 0;
    struct tm *timeStampData;
    SaBoolT _twelveHourModeFlag;
    const SaBoolT *twelveHourModeFlag = &_twelveHourModeFlag;

    TRACE_ENTER();

    if (formatExpression == NULL)
    {
        goto error_exit;
    }

    /* Init output vector with a '\0' */
    (void)strcpy(outputLogRecord, "");

    totalTime = (logRecord->logTimeStamp / (SaTimeT)SA_TIME_ONE_SECOND);

    /* Split timestamp in timeStampData */
    timeStampData = localtime((const time_t *)&totalTime);

    /* Main formatting loop */
    for (; ;)
    {
        /* Scan the Format Expression */
        if ((*fmtExpPtr == TOKEN_START_SYMBOL) && 
            (*fmtExpPtrSnabel != STRING_END_CHARACTER))
        {
            switch (*fmtExpPtrSnabel++)
            {
                /* Check log record field types if a new token is present */
                case COMMON_LOG_RECORD_FIELD_TYPE:  
                    (void)strncpy(tempString, 
                                  extractCommonField(fmtExpPtrSnabel, 
                                                     &fmtExpTokenOffset,
                                                     &truncationLetterPos,
                                                     (SaInt32T)strlen(outputLogRecord),
                                                     logRecordIdCounter,
                                                     twelveHourModeFlag,
                                                     timeStampData,
                                                     logRecord), 
                                  sizeof (tempString));
                    break;

                case NOTIFICATION_LOG_RECORD_FIELD_TYPE:
                    (void)strncpy(tempString, 
                                  extractNotificationField(fmtExpPtrSnabel, 
                                                           &fmtExpTokenOffset,
                                                           twelveHourModeFlag,  
                                                           logRecord),
                                  sizeof (tempString));
                    break;

                case SYSTEM_LOG_RECORD_FIELD_TYPE: 
                    (void)strncpy(tempString, 
                                  extractSystemField(fmtExpPtrSnabel, 
                                                     &fmtExpTokenOffset,  
                                                     logRecord),
                                  sizeof (tempString));
                    break;

                default:
                    goto error_exit;

            }
            logRecordCharCounter += strlen(tempString);

            /* Check truncation */
            if (logRecordCharCounter > fixedLogRecordSize)
            {
                truncationCharacter = (SaInt8T)TRUNCATED_LOG_RECORD;
                (void)strncat(
                             outputLogRecord, 
                             tempString, 
                             strlen(tempString) - \
                             ((logRecordCharCounter - fixedLogRecordSize) + 1));
            }
            else
            {
                (void)strncat(outputLogRecord, tempString, strlen(tempString));
            }

        }
        else
        { /* All chars between tokens */        
            fmtExpTokenOffset = 1;
            logRecordCharCounter += 1;
            /* Insert litteral chars i.e. [:, ,/ and "] */ 
            if (logRecordCharCounter < fixedLogRecordSize)
            {
                (void)strncat(outputLogRecord, 
                              (const char *)&fmtExpPtr[0], 
                              sizeof (char));
            }
            else
            { /* Truncation exists */
                truncationCharacter = (SaInt8T)TRUNCATED_LOG_RECORD;
                break;
            }
            if (*fmtExpPtrSnabel == STRING_END_CHARACTER)
            {
                break; /* End of formatExpression */
            }
        }

        /* Step forward */
        fmtExpPtr += fmtExpTokenOffset;
        if (*fmtExpPtr == STRING_END_CHARACTER)
        {
            break;  /* End of formatExpression */
        }
        fmtExpPtrSnabel = (fmtExpPtr + 1); 
    } /* for ( ; ; ) */

    /* Pad log record to fixed log record fieldSize */ /* TODO!! FIX OVERFLOW ISSUE!!!!*/
    characters = sprintf(outputLogRecord,   
                         "%*.*s", 
                         -(fixedLogRecordSize - 1), 
                         (fixedLogRecordSize - 1), 
                         outputLogRecord);

    if (truncationLetterPos != -1)
    { /* Insert truncation info letter */
        outputLogRecord[truncationLetterPos] = truncationCharacter;
    }
    outputLogRecord[characters] = STRING_END_CHARACTER; /* NULL terminate */
    /* Insert termination character before '\0' */
    outputLogRecord[characters - 1] = TERMINATION_CHARACTER;

    error_exit:
    TRACE_LEAVE();
    return error;
}

