/*
 * dump-netsnmp.c --
 *
 *      Operations to generate NET-SNMP mib module implementation code.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: dump-netsnmp.c,v 1.14 2002/10/30 09:17:37 schoenw Exp $
 */

/*
 * TODO:
 *     - assume that we build a dynamic loadable module
 *     - update to 4.X version of the UCD API
 *     - generate #defines for deprecated and obsolete objects
 *     - generate stub codes for the various functions
 *     - generate type and range checking code
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include "smi.h"
#include "data.h"
#include "smidump.h"

#if ((NCS_NETSNMP != 0) || (NCS_MIBLIB != 0))
#include <dirent.h>
#include <errno.h>
#endif


static int noMgrStubs = 0;
static int noAgtStubs = 0;

#if ((NCS_NETSNMP != 0) || (NCS_MIBLIB != 0))
/* This global variable - NCS netsnmp option is used specifically for 
 * NCS specific functionality. It avoids the redefinition of the code
 * which is required for both NCS & non-NCS applications. 
 */
static int   esmiNetSnmpOpt = ESMI_NETSNMP_NULL_OPT; 
static char  esmiTempMibFile[40] = ESMI_TEMP_MIB_FILE;   
static int checkColumnsMaxAccess (SmiNode *grpNode);

/* 
 * global parameter used in PSSv driver 
 * to cleanup the generated files, in case of 
 * an error. 
 */
static int removeGeneratedFiles = 0; 
#endif /* NCS_NETSNMP || NCS_MIBLIB  */

static char *getAccessString(SmiAccess access)
{
    if (access == SMI_ACCESS_READ_WRITE) {
   return "RWRITE";
    } else if (access == SMI_ACCESS_READ_ONLY) {
   return "RONLY";
    } else {
   return "NOACCESS";
    }
}


#if ((NCS_NETSNMP != 0) || (NCS_MIBLIB != 0))
static char *esmiGetAccessString(SmiNode *grpNode)
{
   SmiNode *smiNode;

   /* Traverse through all of the child objects of the table and check 
      whether it has the objects of read-write or read-create type */ 
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode)) 
   {
       /* Check whether the object is of read-write type */
       if ((smiNode->nodekind == SMI_NODEKIND_SCALAR
           || smiNode->nodekind == SMI_NODEKIND_COLUMN)
           && (smiNode->access == SMI_ACCESS_READ_WRITE))
       {
          return "HANDLER_CAN_RWRITE";
       }
   } /* End of for loop */
                                                                                
   return "HANDLER_CAN_RONLY";
}
#endif /* NCS_NETSNMP || NCS_MIBLIB  */


static char *getBaseTypeString(SmiBasetype basetype)
{
    switch(basetype) {
    case SMI_BASETYPE_UNKNOWN:
   return "ASN_NULL";
    case SMI_BASETYPE_INTEGER32:
    case SMI_BASETYPE_ENUM:
   return "ASN_INTEGER";
    case SMI_BASETYPE_OCTETSTRING:
    case SMI_BASETYPE_BITS:
   return "ASN_OCTET_STR";
    case SMI_BASETYPE_OBJECTIDENTIFIER:
   return "ASN_OBJECT_ID";
    case SMI_BASETYPE_UNSIGNED32:
   return "ASN_INTEGER";
    case SMI_BASETYPE_INTEGER64:
   return "ASN_INTEGER";
    case SMI_BASETYPE_UNSIGNED64:
   return "ASN_INTEGER";
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
   return "ASN_Real";
    }

    return NULL;
}


#if (NCS_NETSNMP != 0)
/**************************************************************************
Name          : esmiGetBaseTypeString

Description   : This function returns the corresponding ASN type based on
                the object base type.

Arguments     : basetype -  Object base type. 

Return values : ASN type string. 

Notes         :     
**************************************************************************/
static char *esmiGetBaseTypeString(SmiBasetype basetype)
{
    switch(basetype) {
    case SMI_BASETYPE_UNKNOWN:
   return "ASN_NULL";
    case SMI_BASETYPE_INTEGER32:
    case SMI_BASETYPE_ENUM:
   return "ASN_INTEGER";
    case SMI_BASETYPE_OCTETSTRING:
    case SMI_BASETYPE_BITS:
   return "ASN_OCTET_STR";
    case SMI_BASETYPE_OBJECTIDENTIFIER:
   return "ASN_OBJECT_ID";
    case SMI_BASETYPE_UNSIGNED32:
   return "ASN_UNSIGNED";
    case SMI_BASETYPE_INTEGER64:
   return "ASN_COUNTER64";
    case SMI_BASETYPE_UNSIGNED64:
   return "ASN_COUNTER64";
    case SMI_BASETYPE_FLOAT32:
    case SMI_BASETYPE_FLOAT64:
    case SMI_BASETYPE_FLOAT128:
   return "ASN_Real";
    }

    return NULL;
}


/**************************************************************************
Name          : esmiGetAsnTypeString

Description   : This function returns the corresponding ASN type based on
                the object base type.

Arguments     : basetype -  Object base type. 

Return values : ASN type string. 

Notes         :     
**************************************************************************/
static char *esmiGetAsnTypeString(EsmiAsntype asntype)
{
   switch(asntype) {
   case ESMI_ASN_NULL:      
       return "ASN_NULL";
   case ESMI_ASN_INTEGER:      
        return "ASN_INTEGER";
   case ESMI_ASN_OCTET_STR:    
       return "ASN_OCTET_STR";
   case ESMI_ASN_OBJECT_ID:    
       return "ASN_OBJECT_ID";
   case ESMI_ASN_IPADDRESS:    
       return "ASN_IPADDRESS";
   case ESMI_ASN_UNSIGNED:     
       return "ASN_UNSIGNED";
   case ESMI_ASN_COUNTER:      
       return "ASN_COUNTER";
   case ESMI_ASN_GAUGE:        
       return "ASN_GAUGE";
   case ESMI_ASN_TIMETICKS:    
       return "ASN_TIMETICKS";
   case ESMI_ASN_OPAQUE:       
       return "ASN_OPAQUE";
   case ESMI_ASN_COUNTER64:
       return "ASN_COUNTER64";
    }

    return NULL;
}
#endif

#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
static void 
cleanUpDriverGeneratedFiles(char *dir)
{ 
    DIR *ldir = NULL; 
    struct  dirent *file = NULL; 
    char    *fullPath = NULL; 
    char    *fullName = NULL; 
    
   fullPath = xmalloc(strlen(esmiDirPath) + strlen(pssvDirPath) + 2); 
   if (fullPath == NULL)
   {
       return; 
   }

   /* build the complete path */ 
   strcpy(fullPath, esmiDirPath);
   strcat(fullPath, dir);

    /* open the directory */ 
    ldir = opendir(fullPath); 
    if (ldir == NULL)
    { 
        /* nothing to delete */ 
        xfree (fullPath); 
        fprintf(stdout, "smidump: opendir() failed: %s\n", fullPath);
        return; 
    }
    
    fprintf(stdout, "smidump: Cleaning the directory %s\n", fullPath);

    /* traverse the list of files in this directory, and delete them */ 
    while((file = readdir(ldir)) != NULL)    
    {
        if ((strcmp(file->d_name, ".")==0)||(strcmp(file->d_name, "..")==0))
            continue; 

        fullName = xmalloc(strlen(fullPath)+strlen(file->d_name)+2); 
        if (fullName == NULL)
            return; 
        strcpy(fullName, fullPath); 
        strcat(fullName, file->d_name); 

        /* print the message on to the console */     
        fprintf(stdout, "smidump: deleting the file: %s\n", fullName);

        /* remove the file */ 
        ESMI_FILE_DELETE(fullName);
        xfree (fullName); 
    } /* end of while() */ 

    /* free the allocated fullpath */ 
    xfree (fullPath); 
    return; 
}
#endif

static char* translate(char *m)
{
    char *s;
    int i;

    s = xstrdup(m);
    for (i = 0; s[i]; i++) {
   if (s[i] == '-') s[i] = '_';
    }
  
    return s;
}



static char* translateUpper(char *m)
{
    char *s;
    int i;

    s = xstrdup(m);
    for (i = 0; s[i]; i++) {
   if (s[i] == '-') s[i] = '_';
   if (islower((int) s[i])) {
       s[i] = toupper(s[i]);
   }
    }
  
    return s;
}



static char* translateLower(char *m)
{
    char *s;
    int i;

    s = xstrdup(m);
    for (i = 0; s[i]; i++) {
   if (s[i] == '-') s[i] = '_';
   if (isupper((int) s[i])) {
       s[i] = tolower(s[i]);
   }
    }
  
    return s;
}



static char* translateFileName(char *m)
{
    char *s;
    int i;

    s = xstrdup(m);
    for (i = 0; s[i]; i++) {
   if (s[i] == '_') s[i] = '-';
   if (isupper((int) s[i])) {
       s[i] = tolower(s[i]);
   }
    }
  
    return s;
}



static FILE * createFile(char *name, char *suffix)
{
    char *fullname;
    FILE *f;

    fullname = xmalloc(strlen(name) + (suffix ? strlen(suffix) : 0) + 2);
    strcpy(fullname, name);
    if (suffix) {
        strcat(fullname, suffix);
    }
    if (!access(fullname, R_OK)) {
        fprintf(stderr, "smidump: %s already exists\n", fullname);
        xfree(fullname);
        return NULL;
    }
    f = fopen(fullname, "w");
    if (!f) {
        fprintf(stderr, "smidump: cannot open %s for writing: ", fullname);
        perror(NULL);
        xfree(fullname);
        exit(1);
    }
    xfree(fullname);
    return f;
}


static int isGroup(SmiNode *smiNode)
{
    SmiNode *childNode;

#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
    if (smiNode->status == SMI_STATUS_OBSOLETE)
       return 0;
#endif

    if (smiNode->nodekind == SMI_NODEKIND_ROW) {
   return 1;
    }
    
    for (childNode = smiGetFirstChildNode(smiNode);
    childNode;
    childNode = smiGetNextChildNode(childNode)) {
   if (childNode->nodekind == SMI_NODEKIND_SCALAR) {
       return 1;
   }
    }

    return 0;
}



static int isAccessible(SmiNode *grpNode)
{
    SmiNode *smiNode;
    int num = 0;
    
    for (smiNode = smiGetFirstChildNode(grpNode);
    smiNode;
    smiNode = smiGetNextChildNode(smiNode)) {
   if ((smiNode->nodekind == SMI_NODEKIND_SCALAR
        || smiNode->nodekind == SMI_NODEKIND_COLUMN)
       && (smiNode->access == SMI_ACCESS_READ_ONLY
      || smiNode->access == SMI_ACCESS_READ_WRITE)) {
       num++;
   }
    }

    return num;
}

#if ((NCS_NETSNMP != 0) || (NCS_MIBLIB != 0))
/**************************************************************************
Name          :  esmiPrintMotoCopyRight

Description   :  This function prints the Motorola's copy right information 
                 into the specified file.

Arguments     :  fd - File Descriptor of the file to which the copy right
                      information is dumped.
                 fileName - Name of the file.

Return values :  Nothing.

Notes         :   
**************************************************************************/
static void esmiPrintMotoCopyRight(FILE *fd, char *fileName, char *fileExt)
{   
   char *fileNameCaps = NULL;
   char *fileExtCaps = NULL;
   char s[128]={0};
   FILE *crFd = NULL; 

   fileNameCaps = translateUpper(fileName);

   if (fileExt)
      fileExtCaps = translateUpper(fileExt);

   /* copy the copyright information, from the source file to this file */ 
   if (esmiCopyRightFile != NULL)
   {
       crFd = fopen(esmiCopyRightFile, "r");
       if (crFd == NULL)
       {
           fprintf(stderr, "smidump:esmiPrintMotoCopyRight():%s:%s\n",esmiCopyRightFile, strerror(errno));
           return; 
       }

       while (fgets(s, 128, crFd)!= NULL)
           fputs(s, fd); 
       
       /* close the CopuRights file */ 
       fclose(crFd);     
   }

   fprintf(fd,
"/*****************************************************************************\n");
   if (fileExtCaps)
      fprintf(fd, "  MODULE NAME: %s%s \n", fileNameCaps, fileExtCaps);
   else
      fprintf(fd, "  MODULE NAME: %s \n", fileNameCaps);

   fprintf(fd,
"******************************************************************************/\n");

   xfree(fileNameCaps);

   if (fileExt)
      xfree(fileExtCaps);

    return;
}
static char* esmigetCompleteName(char *name, char *suffix, char *dirPath)
{
   char *fullName;
   FILE *fd;

   /* Alloc sufficient memory and frame the full name (name + siffix) */
   fullName = xmalloc(strlen(name) + 
                      (esmiDirPath ? strlen(esmiDirPath) : 0) +
                      (dirPath ? strlen(dirPath) : 0) +
                      (suffix  ? strlen(suffix)  : 0) + 2);
   if (dirPath)
   {
      /* Copy the given dir path (view_dir path from /home + relative path to
       * file) and then concatenate file name to it */
      if (esmiDirPath)
      {
         strcpy(fullName, esmiDirPath);
         strcat(fullName, dirPath);
      }
      else
      {
         strcpy(fullName, dirPath);
      }
      strcat(fullName, name);
   } /* Directory path not defined */
   else
      strcpy(fullName, name);

   /* Append the suffix if it is given */
   if (suffix) {
      strcat(fullName, suffix);
   }

   /* return the complete name of the file */
   return (fullName);
}

/**************************************************************************
Name          : esmiCreateFile

Description   : This function creates a file and returns the corresponding 
                file descriptor. The file name is framed by concatenating 
                the input 'suffix' to the input 'name'. 

Arguments     : name -  pointer to a string 
                suffix - ptr to a string need to append to the above name.

Return values : fd - file descriptor of the created file. 

Notes         :     
**************************************************************************/
static FILE *esmiCreateFile(char *name, char *suffix, char *dirPath)
{
   char *fullName;
   DIR  *dir = NULL;
   char command[1024];
   FILE *fd;

   /* Alloc sufficient memory and frame the full name (name + siffix) */
   fullName = xmalloc(strlen(name) + 
                      (esmiDirPath ? strlen(esmiDirPath) : 0) +
                      (dirPath ? strlen(dirPath) : 0) +
                      (suffix  ? strlen(suffix)  : 0) + 2);
   if (dirPath)
   {
      /* Copy the given dir path (view_dir path from /home + relative path to
       * file) and then concatenate file name to it */
      if (esmiDirPath)
      {
         strcpy(fullName, esmiDirPath);
         strcat(fullName, dirPath);
      }
      else
      {
         strcpy(fullName, dirPath);
      }

      /* create esmi specified directories if they doesn't exist */
      if ((dir = opendir(fullName)) == NULL)
      {
         memset(command, 0, sizeof(command));
         snprintf(command, sizeof(command) - 1,
                "\\mkdir -p %s", fullName);
         system(command);
      }

      strcat(fullName, name);
   } /* Directory path not defined */
   else
      strcpy(fullName, name);

   /* Append the suffix if it is given */
   if (suffix) {
      strcat(fullName, suffix);
   }

   /* Delete the file if it exists */
   ESMI_FILE_DELETE(fullName);

   /* Create a FILE */
   fd = fopen(fullName, "w");
   if (!fd) {
       fprintf(stderr, "ERROR: cannot open %s for writing: ", fullName);
       perror(NULL);
       xfree(fullName);
       exit(1);
   }

   /* Free the allotted memory for fullName */
   xfree(fullName);

   return fd;
}

/**************************************************************************
Name          :  esmiTranslateHungarian
                                                                                
Description   :  This function derives an output string from the input 
                 string. All the CAPITAL letters in an input string 
                 (say X) is replaced with '_x' in an output string.
                 And the character '-' to '_'.

Arguments     :  iStr - Input string
                                                                                
Return Values :  oStr - Ptr to a string.
                                                                                
Notes         :
**************************************************************************/
static char* esmiTranslateHungarian(char *iStr)
{
    char *oStr;
    int  i, j = 0, charBig = 0;

    /* Get the number of CAPITAL letters exists in a string */
    for (i = 0; iStr[i]; i++) {
       if (isupper((int) iStr[i]))  charBig++;
    }

    /* Allocate the memory for an ouput string */
    oStr = xmalloc(strlen(iStr) + charBig + 2);

    /* Process the complete input string */
    for (i = 0; iStr[i]; i++) {
       /* If the input string has a capital letter say 'X' in it, 
          then replace the capital letter '_x' in ouput string */
       if (isupper((int) iStr[i])) {
          oStr[j++] = '_';
          oStr[j++] = tolower(iStr[i]);
       }
       else
       {  /* if the input string has '-' in it then replace it 
             with '_' in output string */
          if (iStr[i] == '-') 
             oStr[j++] = '_';
          else
             oStr[j++] = iStr[i];
       }
    }

    /* Treminate the output string with NULL character */
    oStr[j] = '\0';
  
    return oStr;
}


/**************************************************************************
Name          :  esmiTranslateLower
                                                                                
Description   :  This function creates a string (length of the input
                 string + length of suffix) and copies the contents of
                 the input string and concatenate the suffix. It replaces
                 '-' to '_' if it exists and translates the upper case 
                 letters to smaller case letters.

Arguments     :  iStr - Input string
                 suffix - suffix string to be concatenated. 
                                                                                
Return Values :  oStr - Ptr to a string.
                                                                                
Notes         :
**************************************************************************/
static char* esmiTranslateLower(char *iStr, char *suffix)
{
   char *oStr;
   int  i;

   /* Alloc sufficient memory and frame the output string(name + suffix) */
   oStr = xmalloc(strlen(iStr) + (suffix ? strlen(suffix) : 0) + 2);
   strcpy(oStr, iStr);
   if (suffix) {
      strcat(oStr, suffix);
   }

   /* Translate the char '-' to '_' and any upper case letter to 
      small case letter */
   for (i = 0; oStr[i]; i++)
   {
      if (oStr[i] == '-') oStr[i] = '_';

      if (isupper((int) oStr[i]))
         oStr[i] = tolower(oStr[i]);
   }

   return oStr;
}


/**************************************************************************
Name          : isWritable

Description   : This function checks whether the table has the objects
                of read-write (or) read-create type.

Arguments     : grpNode - Ptr to the SmiNode structure of the table 

Return values : TRUE / FALSE

Notes         :     
**************************************************************************/
static int isWritable(SmiNode *grpNode)
{
   SmiNode *smiNode;

   /* Traverse through all of the child objects of the table and check 
      whether it has the objects of read-write or read-create type */ 
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode)) 
   {
       /* Check whether the object is of read-write type */
       if ((smiNode->nodekind == SMI_NODEKIND_SCALAR
           || smiNode->nodekind == SMI_NODEKIND_COLUMN)
           && (smiNode->access == SMI_ACCESS_READ_WRITE))
       {
           return TRUE;
       }
   } /* End of for loop */
                                                                                
   return FALSE;
}


/**************************************************************************
Name          : areScalars

Description   : This function checks whether the table has the objects
                of SCALAR type.

Arguments     : grpNode - Ptr to the SmiNode structure of the table 

Return values : TRUE / FALSE

Notes         :     
**************************************************************************/
static int areScalars(SmiNode *grpNode)
{
   SmiNode *smiNode;
                          
   /* Traverse through all of the child objects of the table and check 
      whether the table has an object of SCALAR type */
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode)) 
   {
      if (smiNode->nodekind == SMI_NODEKIND_SCALAR)
         return TRUE;
   }
                                                                                
   return FALSE;
}


/**************************************************************************
Name          : isTrap 

Description   : This function checks whether the table has all the objects
                of NOTIFICATION type.

Arguments     : smiNode - Ptr to the SmiNode structure of the table 

Return values : TRUE / FALSE

Notes         :     
**************************************************************************/
static int isTrap(SmiNode *smiNode)
{
   SmiNode *childNode;

   if (smiNode->status == SMI_STATUS_OBSOLETE)
       return FALSE;

   if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT)
   {
      /* If Table/Trap ID is not defined for the table, then don't 
         generate any stubs */
      if (smiNode->esmi.esmiDataKind != ESMI_TABLE_DATA)
         return FALSE;
   }

   /* Check if smiNode is a table of SCALAR type, has a Scalar object in
      it. If it is of SCALAR type then no need to generate specifically
      for TRAP */
   if (areScalars(smiNode))
       return FALSE;

   /* Traverse through all of the child objects of the table and check 
      whether they are of NOTIFICATION (TRAPs) type. */
   for (childNode = smiGetFirstChildNode(smiNode);
        childNode;
        childNode = smiGetNextChildNode(childNode)) 
   {
      if (childNode->nodekind == SMI_NODEKIND_NOTIFICATION)
      {
         return TRUE;
      }
   }

   return FALSE;
}


/**************************************************************************
Name          : esmiCheckTblObject
                                                                                
Description   : This function validates the table data.  
                                                                                
Arguments     : grpNode - pointer to the SmiNode data structs, holds
                          table object information.
                                                                                
Return values : ESMI_SNMP_SUCCESS / ESMI_SNMP_FAILURE
                                                                                
Notes         :
**************************************************************************/
static int esmiCheckTblObject(SmiNode *grpNode)
{
   char tempStr[64];
   int  lineNum = ((Object *)grpNode)->line;

   memset(tempStr, '\0', sizeof(tempStr));

   /* check the object data kind is of ESMI_TABLE_DATA,
      else just return error */
   if (grpNode->esmi.esmiDataKind != ESMI_TABLE_DATA)
   {
      fprintf(stderr, "\n%s:%d  WARNING: ESMI tag is not defined for the table:  %s\n", esmiTempMibFile, lineNum, grpNode->name);
      return ESMI_SNMP_FAILURE;
   }

   /* Check whether the table ID is defined for table object */
   if (! strcmp(grpNode->esmi.info.tableInfo.tableId, tempStr))
   {
      fprintf(stderr, "\n\n%s:%d  WARNING: Table ID is not defined for the table:  %s  ", esmiTempMibFile, lineNum, grpNode->name);
      return ESMI_SNMP_FAILURE;
   }
      
   /* Check whether the table ID is defined for table object */
   if ((! strcmp(grpNode->esmi.info.tableInfo.hdrFile, tempStr)) &&
       ((esmiNetSnmpOpt == ESMI_MIBLIB_OPT)))
   {
      fprintf(stderr, "\n\n%s:%d  WARNING: Header File is not defined for the table:  %s  ", esmiTempMibFile, lineNum, grpNode->name);
      return ESMI_SNMP_FAILURE;
   }

   if (esmiNetSnmpOpt == ESMI_PSSV_OPT) 
   {
      /* Check whether the table ID is defined for table object */
      if (pssvDirPath == NULL) 
      {
         fprintf(stderr, "\n\n%s:%d\n   WARNING: Directory path is not defined for the table:  %s  ", esmiTempMibFile, lineNum, grpNode->name);

         fprintf(stderr, "\n   Please define ESMI tag for directory path in your ESMI MIB file");
         return ESMI_SNMP_FAILURE;
      }
   }
   else
   {
      /* Check whether the table ID is defined for table object */
      if ((! strcmp(grpNode->esmi.info.tableInfo.dirPath, tempStr)) &&
          (esmiNetSnmpOpt == ESMI_MIBLIB_OPT))
      {
         fprintf(stderr, "\n\n%s:%d\n   WARNING: Directory path is not defined for the table:  %s  ", esmiTempMibFile, lineNum, grpNode->name);

         fprintf(stderr, "\n   Please define ESMI tag for directory path in your ESMI MIB file");
         return ESMI_SNMP_FAILURE;
      }
   }

   /* Check the correctness of the table, 
      scalar_table Vs index/indices defined */
   if (esmiCheckTblIndex(grpNode) != ESMI_SNMP_SUCCESS)
      return ESMI_SNMP_FAILURE;

   return ESMI_SNMP_SUCCESS;
}
#endif /* NCS_NETSNMP || NCS_MIBLIB  */


static unsigned int getMinSize(SmiType *smiType)
{
    SmiRange *smiRange;
    SmiType  *parentType;
    unsigned int min = 65535, size;
    
    switch (smiType->basetype) {
    case SMI_BASETYPE_BITS:
   return 0;
    case SMI_BASETYPE_OCTETSTRING:
    case SMI_BASETYPE_OBJECTIDENTIFIER:
   size = 0;
   break;
    default:
   return 0;
    }

    for (smiRange = smiGetFirstRange(smiType);
    smiRange ; smiRange = smiGetNextRange(smiRange)) {
   if (smiRange->minValue.value.unsigned32 < min) {
       min = smiRange->minValue.value.unsigned32;
   }
    }
    if (min < 65535 && min > size) {
   size = min;
    }

    parentType = smiGetParentType(smiType);
    if (parentType) {
   unsigned int psize = getMinSize(parentType);
   if (psize > size) {
       size = psize;
   }
    }

    return size;
}



static unsigned int getMaxSize(SmiType *smiType)
{
    SmiRange *smiRange;
    SmiType  *parentType;
    SmiNamedNumber *nn;
    unsigned int max = 0, size;
    
    switch (smiType->basetype) {
    case SMI_BASETYPE_BITS:
    case SMI_BASETYPE_OCTETSTRING:
   size = 65535;
   break;
    case SMI_BASETYPE_OBJECTIDENTIFIER:
   size = 128;
   break;
    default:
   return 0xffffffff;
    }

    if (smiType->basetype == SMI_BASETYPE_BITS) {
   for (nn = smiGetFirstNamedNumber(smiType);
        nn;
        nn = smiGetNextNamedNumber(nn)) {
       if (nn->value.value.unsigned32 > max) {
      max = nn->value.value.unsigned32;
       }
   }
   size = (max / 8) + 1;
   return size;
    }

    for (smiRange = smiGetFirstRange(smiType);
    smiRange ; smiRange = smiGetNextRange(smiRange)) {
   if (smiRange->maxValue.value.unsigned32 > max) {
       max = smiRange->maxValue.value.unsigned32;
   }
    }
    if (max > 0 && max < size) {
   size = max;
    }

    parentType = smiGetParentType(smiType);
    if (parentType) {
   unsigned int psize = getMaxSize(parentType);
   if (psize < size) {
       size = psize;
   }
    }

    return size;
}



static void printHeaderTypedef(FILE *f, SmiModule *smiModule,
                SmiNode *grpNode)
{
    SmiNode *smiNode;
    SmiType *smiType;
    char    *cModuleName, *cGroupName, *cName;
    unsigned minSize, maxSize;

    cModuleName = translateLower(smiModule->name);
    cGroupName = translate(grpNode->name);

    fprintf(f,
       "/*\n"
       " * C type definitions for %s::%s.\n"
       " */\n\n",
       smiModule->name, grpNode->name);
    
    fprintf(f, "typedef struct %s {\n", cGroupName);
       
    for (smiNode = smiGetFirstChildNode(grpNode);
    smiNode;
    smiNode = smiGetNextChildNode(smiNode)) {
   if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
#if 0
       && (smiNode->access == SMI_ACCESS_READ_ONLY
      || smiNode->access == SMI_ACCESS_READ_WRITE)
#endif
       ) {
       smiType = smiGetNodeType(smiNode);
       if (!smiType) {
      continue;
       }
       
       cName = translate(smiNode->name);
       switch (smiType->basetype) {
       case SMI_BASETYPE_OBJECTIDENTIFIER:
      maxSize = getMaxSize(smiType);
      minSize = getMinSize(smiType);
      fprintf(f,
         "    uint32_t  *%s;\n", cName);
      if (maxSize != minSize) {
          fprintf(f,
             "    size_t    _%sLength;\n", cName);
      }
      break;
       case SMI_BASETYPE_OCTETSTRING:
       case SMI_BASETYPE_BITS:
      maxSize = getMaxSize(smiType);
      minSize = getMinSize(smiType);
      fprintf(f,
         "    u_char    *%s;\n", cName);
      if (maxSize != minSize) {
          fprintf(f,
             "    size_t    _%sLength;\n", cName);
      }
      break;
       case SMI_BASETYPE_ENUM:
       case SMI_BASETYPE_INTEGER32:
      fprintf(f,
         "    int32_t   *%s;\n", cName);
      break;
       case SMI_BASETYPE_UNSIGNED32:
      fprintf(f,
         "    uint32_t  *%s;\n", cName);
      break;
       case SMI_BASETYPE_INTEGER64:
      fprintf(f,
         "    int64_t   *%s; \n", cName);
      break;
       case SMI_BASETYPE_UNSIGNED64:
      fprintf(f,
         "    uint64_t  *%s; \n", cName);
      break;
       default:
      fprintf(f,
         "    /* ?? */  __%s; \n", cName);
      break;
       }
       xfree(cName);
   }
    }
    
    fprintf(f,
       "    void      *_clientData;\t\t"
       "/* pointer to client data structure */\n");
    if (grpNode->nodekind == SMI_NODEKIND_ROW) {
   fprintf(f, "    struct %s *_nextPtr;\t"
      "/* pointer to next table entry */\n", cGroupName);
    }
    fprintf(f,
       "\n    /* private space to hold actual values */\n\n");

    for (smiNode = smiGetFirstChildNode(grpNode);
    smiNode;
    smiNode = smiGetNextChildNode(smiNode)) {
   if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
#if 0
       && (smiNode->access == SMI_ACCESS_READ_ONLY
      || smiNode->access == SMI_ACCESS_READ_WRITE)
#endif
       ) {
       smiType = smiGetNodeType(smiNode);
       if (!smiType) {
      continue;
       }
       
       cName = translate(smiNode->name);
       switch (smiType->basetype) {
       case SMI_BASETYPE_OBJECTIDENTIFIER:
      maxSize = getMaxSize(smiType);
      fprintf(f,
         "    uint32_t  __%s[%u];\n", cName, maxSize);
      break;
       case SMI_BASETYPE_OCTETSTRING:
       case SMI_BASETYPE_BITS:
      maxSize = getMaxSize(smiType);
      fprintf(f,
         "    u_char    __%s[%u];\n", cName, maxSize);
      break;
       case SMI_BASETYPE_ENUM:
       case SMI_BASETYPE_INTEGER32:
      fprintf(f,
         "    int32_t   __%s;\n", cName);
      break;
       case SMI_BASETYPE_UNSIGNED32:
      fprintf(f,
         "    uint32_t  __%s;\n", cName);
      break;
       case SMI_BASETYPE_INTEGER64:
      fprintf(f,
         "    int64_t   __%s; \n", cName);
      break;
       case SMI_BASETYPE_UNSIGNED64:
      fprintf(f,
         "    uint64_t  __%s; \n", cName);
      break;
       default:
      fprintf(f,
         "    /* ?? */  __%s; \n", cName);
      break;
       }
       xfree(cName);
   }
    }

    fprintf(f, "} %s_t;\n\n", cGroupName);

    fprintf(f,
       "/*\n"
       " * C manager interface stubs for %s::%s.\n"
       " */\n\n",
       smiModule->name, grpNode->name);
       
    fprintf(f, "extern int\n"
       "%s_mgr_get_%s(struct snmp_session *s, %s_t **%s);\n",
       cModuleName, cGroupName, cGroupName, cGroupName);
    fprintf(f, "\n");

    fprintf(f,
       "/*\n"
       " * C agent interface stubs for %s::%s.\n"
       " */\n\n",
       smiModule->name, grpNode->name);
    
    fprintf(f, "extern int\n"
       "%s_agt_read_%s(%s_t *%s);\n",
       cModuleName, cGroupName, cGroupName, cGroupName);
    fprintf(f, "extern int\n"
       "%s_agt_register_%s();\n\n",
       cModuleName, cGroupName);
    xfree(cGroupName);
    xfree(cModuleName);
}



static void printHeaderTypedefs(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;
    char      *cModuleName;
    char      *cSmiNodeName;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
    smiNode;
    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
   if (isGroup(smiNode) && isAccessible(smiNode)) {
       cnt++;
       printHeaderTypedef(f, smiModule, smiNode);
   }
    }
    
    if (cnt) {
   fprintf(f, "\n");
    }

    if (cnt) {
   /*
    * Should this go into the agent implementation module?
    */
   cModuleName = translateLower(smiModule->name);
   fprintf(f, "typedef struct %s {\n", cModuleName);
   for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
        smiNode;
        smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
       if (isGroup(smiNode) && isAccessible(smiNode)) {
      cSmiNodeName = translate(smiNode->name);
      if (smiNode->nodekind == SMI_NODEKIND_ROW) {
          fprintf(f, "    %s_t\t*%s;\n", cSmiNodeName, cSmiNodeName);
      } else {
          fprintf(f, "    %s_t\t%s;\n", cSmiNodeName, cSmiNodeName);
      }
      xfree(cSmiNodeName);
       }
   }
   fprintf(f, "} %s_t;\n\n", cModuleName);
   xfree(cModuleName);
    }
}



static void dumpHeader(SmiModule *smiModule, char *baseName)
{
    char   *pModuleName;
    char   *cModuleName;
    FILE   *f;

    pModuleName = translateUpper(smiModule->name);

    f = createFile(baseName, ".h");
    if (! f) {
   return;
    }
    
    fprintf(f,
       "/*\n"
       " * This C header file has been generated by smidump "
       SMI_VERSION_STRING ".\n"
       " * It is intended to be used with the NET-SNMP package.\n"
       " *\n"
       " * This header is derived from the %s module.\n"
       " *\n * $I" "d$\n"
       " */\n\n", smiModule->name);

    fprintf(f, "#ifndef _%s_H_\n", pModuleName);
    fprintf(f, "#define _%s_H_\n\n", pModuleName);

    fprintf(f, "#include <stdlib.h>\n\n");

    fprintf(f,
       "#ifdef HAVE_STDINT_H\n"
       "#include <stdint.h>\n"
       "#endif\n\n");

    printHeaderTypedefs(f, smiModule);

    fprintf(f,
       "/*\n"
       " * Initialization function:\n"
       " */\n\n");
    cModuleName = translateLower(smiModule->name);
    fprintf(f, "void %s_agt_init(void);\n\n", cModuleName);
    xfree(cModuleName);

    fprintf(f, "#endif /* _%s_H_ */\n", pModuleName);

    fclose(f);
    xfree(pModuleName);
}


static void printAgtReadMethodDecls(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
    smiNode;
    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
   if (isGroup(smiNode) && isAccessible(smiNode)) {
       cnt++;
       if (cnt == 1) {
            fprintf(f, "\n/***************************************************************************\n"
         "  Forward declaration of read methods for groups of scalars and tables\n"
         "**************************************************************************/\n\n");
       }
#if (NCS_NETSNMP != 0)
    if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT)
    {
       char *sName = esmiTranslateHungarian(smiNode->name);
       fprintf(f,
               "static unsigned char *\nread_%s_stub(struct variable *,"
               " oid *, size_t *, int, size_t *, WriteMethod **);\n\n",
               sName);
       xfree(sName);
    }
    else
#endif
       fprintf(f,
               "static unsigned char *\nread_%s_stub(struct variable *,"
               " oid *, size_t *, int, size_t *, WriteMethod **);\n",
               smiNode->name);
  
   }
    }
    
    if (cnt) {
   fprintf(f, "\n");
    }
}



static void printAgtWriteMethodDecls(FILE *f, SmiModule *smiModule)
{
    SmiNode     *smiNode;
    int         cnt = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
    smiNode;
    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
   if ((smiNode->access == SMI_ACCESS_READ_WRITE)
#if (NCS_NETSNMP != 0) 
 &&        /* not generating any kind of stubs for obsolete objects/tables 
              in NCS Net-Snmp */
            (smiNode->status != SMI_STATUS_OBSOLETE)
#endif
           ) {
       cnt++;
       if (cnt == 1) {
      fprintf(f,
         "\n/**************************************************************************\n"
         "            Write methods for read-write/read-create objects\n"
         "**************************************************************************/\n\n");
       }
#if (NCS_NETSNMP != 0)
            if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT)
            {
               char *sName = esmiTranslateHungarian(smiNode->name);
               fprintf(f, "static int\nwrite_%s_stub(int,"
                          " u_char *, u_char, size_t, u_char *, oid *, size_t);\n\n",
                sName);
               xfree(sName);
            }
            else
#endif
               fprintf(f, "static int\nwrite_%s_stub(int,"
                          " u_char *, u_char, int, u_char *, oid *, int);\n",
             smiNode->name);
   }
    }
    
    if (cnt) {
   fprintf(f, "\n");
    }
}


static void printAgtDefinesGroup(FILE *f, SmiNode *grpNode, int cnt)
{
    char         *cName, *cGroupName;
    SmiNode       *smiNode;
    SmiType       *smiType;
    int             num = 0, valid = 0;
    unsigned int i;


    if (cnt == 1) {
   fprintf(f,
   "\n/**************************************************************************\n"
   "  Definitions of tags that are used internally to read/write\n"
   "  the selected object type. These tags should be unique.\n"
   "***************************************************************************/\n\n");
    }

    for (smiNode = smiGetFirstChildNode(grpNode);
    smiNode;
    smiNode = smiGetNextChildNode(smiNode)) {
    if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
       && (smiNode->access == SMI_ACCESS_READ_ONLY
      || smiNode->access == SMI_ACCESS_READ_WRITE)
#if (NCS_NETSNMP != 0)
            && (smiNode->status != SMI_STATUS_OBSOLETE)
#endif
           ) {
       num++;
     }
    }

    /* No read-only/read-write/read-create obj's exists in this table */
    if (! num)
    {
       if (checkColumnsMaxAccess(grpNode) == ESMI_SNMP_FAILURE)
          return;
    }
    num = 0;

#if (NCS_NETSNMP != 0)
    fprintf(f, "\n/*************** '%s' table Definitions ***************/\n", grpNode->name);

    if ((esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT) ||
        (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT))
    {
       cGroupName = esmiTranslateHungarian(grpNode->name);
    }
    else
#endif
    cGroupName = translate(grpNode->name);

    for (smiNode = smiGetFirstChildNode(grpNode);
    smiNode;
    smiNode = smiGetNextChildNode(smiNode)) {
   if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
       && ((smiNode->access == SMI_ACCESS_READ_ONLY) ||
       (smiNode->access == SMI_ACCESS_READ_WRITE) ||
       (smiNode->access == SMI_ACCESS_NOTIFY) ||
       ((esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT) && 
        (smiNode->access == SMI_ACCESS_NOT_ACCESSIBLE))) 
#if (NCS_NETSNMP != 0)
            && (smiNode->status != SMI_STATUS_OBSOLETE)
#endif
           ) {
       num++;
       cName = translateUpper(smiNode->name);
       fprintf(f, "#define %-32s %d\n", cName,
          smiNode->oid[smiNode->oidlen-1]);
       xfree(cName);
   }
    }
    fprintf(f, "\n");

    if (num) {
   fprintf(f, "static oid __%s_base[] = {", cGroupName);
   for (i = 0; i < grpNode->oidlen; i++) {
       fprintf(f, "%s%d", i ? ", " : "", grpNode->oid[i]);
   }
   fprintf(f, "};\n\n");

   if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT)
   {
      fprintf(f, "struct variable %s_variables[] = {\n", cGroupName);
   }
   else  /* ESMI_NEW_SYN_NETSNMP_OPT & only for non-scalar tables */
   if ((esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT) && (num))
   {
      SmiNode *parentSmiNode = NULL;
      SmiNode *tempSmiNode = NULL;
      char    *cParentName = NULL;

      /* Get the Parent Node */
      parentSmiNode = smiGetParentNode(grpNode); 
      cParentName = esmiTranslateHungarian(parentSmiNode->name);
       
      if (grpNode->indexkind != SMI_INDEX_UNKNOWN)
      {
         fprintf(f, "static oid __%s_base[] = {", cParentName);
         for (i = 0; i < grpNode->oidlen-1; i++) {
            fprintf(f, "%s%d", i ? ", " : "", grpNode->oid[i]);
         }
         fprintf(f, "};\n\n");

         /* Print VALID column information */
         for (smiNode = smiGetFirstChildNode(grpNode);
              smiNode;
              smiNode = smiGetNextChildNode(smiNode))
         {
            if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
               && (smiNode->access == SMI_ACCESS_READ_ONLY
                   || smiNode->access == SMI_ACCESS_READ_WRITE) 
#if (NCS_NETSNMP != 0)
               && (smiNode->status != SMI_STATUS_OBSOLETE)
#endif
             )
            {
#if 0
               /* Don't print the info for index objects */
               if (esmiIsObjIndexType(grpNode, smiNode))
                  continue;
#endif
               if (tempSmiNode)
                  fprintf(f, "%d, ", tempSmiNode->oid[tempSmiNode->oidlen-1]);
               else
               {
                  fprintf(f, "\n/* Pl. note that this array should not have index objects, obsolete objects */"
                     "\nstatic unsigned int __%s_valid_columns[] = {", cParentName);
              }

               tempSmiNode = smiNode;
               valid++;
            }
         }

         if (tempSmiNode)
         fprintf(f, "%d};\n", tempSmiNode->oid[tempSmiNode->oidlen-1]);

         /* Generate the following registration functions only if table has atleast one (get) accessible object. */
         if (valid)
         {
            fprintf(f, "\nstatic netsnmp_table_registration_info  *__%s_tbl_info;",
                       parentSmiNode->name);
            fprintf(f, "\nNetsnmp_Node_Handler                    __%s_handler;", cParentName);
            fprintf(f, "\nstatic netsnmp_handler_registration     *__%s_reg;", parentSmiNode->name);
            fprintf(f, "\nstatic u_char __%s_validate_set_varbinds(int colnum,"
                       "\n                                       netsnmp_request_info *cur_req);", cParentName);
         }
      }
      else
      {
         for (smiNode = smiGetFirstChildNode(grpNode);
              smiNode;
              smiNode = smiGetNextChildNode(smiNode))
         {
            if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
               && (smiNode->access == SMI_ACCESS_READ_ONLY
                   || smiNode->access == SMI_ACCESS_READ_WRITE) 
#if (NCS_NETSNMP != 0)
               && (smiNode->status != SMI_STATUS_OBSOLETE)
#endif
             )
            {
               valid++;
            }
         }
         if (valid)
         {
            /* Generate the following lines only if scalars group has accessible (get) objects */
            fprintf(f, "\nNetsnmp_Node_Handler                 __%s_handler;", cGroupName);
            fprintf(f, "\nstatic netsnmp_handler_registration  *__%s_reg;", grpNode->name);
            fprintf(f, "\nstatic u_char __%s_validate_set_varbinds(int colnum,"
                       "\n                                       netsnmp_request_info *cur_req);", cGroupName);
         }
      }

      fprintf(f, "\n\nstruct variable  __%s_variables[] = {\n", cGroupName);

      xfree(cParentName);
   }
       
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode)) {
       if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
      && (smiNode->access == SMI_ACCESS_READ_ONLY
          || smiNode->access == SMI_ACCESS_READ_WRITE || smiNode->access == SMI_ACCESS_NOTIFY || 
       ((esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT) && 
        (smiNode->access == SMI_ACCESS_NOT_ACCESSIBLE)))
#if (NCS_NETSNMP != 0)
                && (smiNode->status != SMI_STATUS_OBSOLETE)
#endif
               ) {
      smiType = smiGetNodeType(smiNode);
      if (!smiType) {
          continue;
      }
      cName = translateUpper(smiNode->name);
#if (NCS_NETSNMP != 0)
    if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT)
    {
       fprintf(f, "    { %s, %s, %s, read_%s_stub, %d, {%d} },\n",
          cName, esmiGetAsnTypeString(smiType->asntype),
          getAccessString(smiNode->access),
          cGroupName, 1, smiNode->oid[smiNode->oidlen-1]);
    }
    else
    if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
    {
       if (smiGetNextChildNode(smiNode))
          fprintf(f, "              { %s, %s, %s, 0, 0, {0} },\n", cName, esmiGetAsnTypeString(smiType->asntype), getAccessString(smiNode->access));
       else
          fprintf(f, "              { %s, %s, %s, 0, 0, {0} }\n", cName, esmiGetAsnTypeString(smiType->asntype), getAccessString(smiNode->access));
    }
    else
    {
#endif
       fprintf(f, "    { %s, %s, %s, read_%s_stub, %d, {%d} },\n",
          cName, getBaseTypeString(smiType->basetype),
          getAccessString(smiNode->access),
          cGroupName, 1, smiNode->oid[smiNode->oidlen-1]);
#if (NCS_NETSNMP != 0)
    }
#endif
      xfree(cName);
       }
   }
   fprintf(f, "};\n\n");
    }

    xfree(cGroupName);
}

static void printTrapDefinesInImportedModule(FILE *f, SmiModule *module)
{
    SmiImport *currentImport;
    SmiModule *smiModule;
    SmiNode   *smiNode, *tempNode;
    int       cnt = 0;

    /* for all the imported modules */ 
    for(currentImport = smiGetFirstImport(module); currentImport;
        currentImport = smiGetNextImport(currentImport))
    {
        /* get the node */ 
       tempNode = esmiFindTableAnyModule(currentImport->name, 0);   
       if (tempNode == NULL)
           continue; 

        /* get the Module */ 
       smiModule = smiGetNodeModule(tempNode); 
       if (smiModule == NULL)
           continue; 

        /* do we need to generate the code this import? */ 
        if (smiModule->esmi.imported == 0)
            continue; 

        /* for all the nodes in this imported module */ 
        for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
        smiNode;
        smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
        {
            if ((esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT) ||
                (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT))
            {
               /* Generate the TRAP base OIDs */
               if (isTrap(smiNode)) /* For TRAP table */
               {
                    int  i = 0;
                    char *cTrapGrpName = esmiTranslateHungarian(smiNode->name);

                    fprintf(f, "static oid __%s_trap_base[] = {", cTrapGrpName);
                    for (i = 0; i < smiNode->oidlen; i++) {
                        fprintf(f, "%s%d", i ? ", " : "", smiNode->oid[i]);
                    }
                    fprintf(f, "};\n\n");

                    /* Free the allotted memory for cTrapGrpName */
                    xfree(cTrapGrpName);
               }
            }
        } /* for(smiNode...) */ 
    }
    return; 
}
static void printAgtDefines(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
    smiNode;
    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
   if (isGroup(smiNode)) 
       printAgtDefinesGroup(f, smiNode, ++cnt);
#if (NCS_NETSNMP != 0)
        else
        if ((esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT) ||
            (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT))
        {
           /* Generate the TRAP base OIDs */
           if (isTrap(smiNode)) /* For TRAP table */
           {
              int  i = 0;
              char *cTrapGrpName = esmiTranslateHungarian(smiNode->name);

         fprintf(f, "static oid __%s_trap_base[] = {", cTrapGrpName);
         for (i = 0; i < smiNode->oidlen; i++) {
            fprintf(f, "%s%d", i ? ", " : "", smiNode->oid[i]);
              }
         fprintf(f, "};\n\n");

              /* Free the allotted memory for cTrapGrpName */
              xfree(cTrapGrpName);
           }
        }
#endif /* NCS_NETSNMP */
    } /* End of for loop */

#if (NCS_NETSNMP != 0)
     printTrapDefinesInImportedModule(f, smiModule); 
#endif /* NCS_NETSNMP */
    
    if (cnt) {
   fprintf(f, "\n");
    }
}


static void  printNewAgtScalarHandlerFunc(FILE *fd, SmiNode *grpNode)
{
   char *cGroupName = NULL;
   char *cName      = NULL;
   SmiNode *smiNode = NULL;
   SmiType *smiType = NULL;
   
   cGroupName = esmiTranslateHungarian(grpNode->name);

   fprintf(fd, "\n\nint __%s_handler(netsnmp_mib_handler          *handler,"
               "\n                               netsnmp_handler_registration *reginfo,"
               "\n                               netsnmp_agent_request_info   *reqinfo,"
               "\n                               netsnmp_request_info         *requests)"
               "\n{", cGroupName);

   fprintf(fd, "\n   netsnmp_request_info  *cur_req = NULL;"
               "\n   netsnmp_variable_list *var = NULL;"
               "\n   u_char       *val_str = NULL; "
               "\n   size_t       val_len = 0;"
               "\n   u_char       type = 0;"
               "\n   u_char       error = 0;"
               "\n   NCSMIB_ARG   mib_arg;"
               "\n   uns8         space[1024];"
               "\n   NCSMEM_AID   ma;"
               "\n   uns32        status = NCSCC_RC_FAILURE;");
            
   fprintf(fd, "\n\n   m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): entered\");", cGroupName);

   fprintf(fd, "\n\n   for (cur_req = requests; cur_req; cur_req = cur_req->next)\n   {");

   fprintf(fd, "\n      if (cur_req->processed != 0)"
               "\n         continue;");

   fprintf(fd, "\n\n      var = cur_req->requestvb;"
               "\n\n      /* assign the type of the object */"
               "\n      switch (var->name[OID_LENGTH(__%s_base)])"
               "\n      {", cGroupName);

   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
      if ((smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR))
          && ((smiNode->access == SMI_ACCESS_READ_ONLY) ||
              (smiNode->access == SMI_ACCESS_READ_WRITE) ||
              (smiNode->access == SMI_ACCESS_NOT_ACCESSIBLE))
          && (smiNode->status != SMI_STATUS_OBSOLETE))
      {
          smiType = smiGetNodeType(smiNode);
          if (!smiType)
             continue;

          cName = translateUpper(smiNode->name);
          if (smiNode->access == SMI_ACCESS_NOT_ACCESSIBLE)
          {
             fprintf(fd, "\n      case %s:"
                         "\n          if ((reqinfo->mode == MODE_SET_RESERVE1) || (reqinfo->mode == MODE_SET_ACTION))"
                         "\n             netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_NOTWRITABLE);"
                         "\n          else"
                         "\n             netsnmp_set_request_error(reqinfo, cur_req, SNMP_NOSUCHOBJECT);"
                         "\n          continue;"
                         "\n          break;\n", cName); 
          }
          else
          {
             fprintf(fd, "\n      case %s:", cName);
             fprintf(fd, "\n          type = %s;", esmiGetAsnTypeString(smiType->asntype));
             fprintf(fd, "\n          break;\n"); 
          }

          xfree(cName);
      }
   }

   fprintf(fd, "\n      default:"
               "\n          if ((reqinfo->mode == MODE_SET_RESERVE1) || (reqinfo->mode == MODE_SET_ACTION))"
               "\n             netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_NOTWRITABLE);"
               "\n          else"
               "\n             netsnmp_set_request_error(reqinfo, cur_req, SNMP_NOSUCHOBJECT);"
               "\n          continue;"
               "\n          break;\n      }"); 

   fprintf(fd, "\n\n      m_NCS_OS_MEMSET(&mib_arg, 0, sizeof(NCSMIB_ARG));"
               "\n      m_NCS_OS_MEMSET(space, 0, sizeof(space));"
               "\n      ncsmem_aid_init(&ma, space, 1024);");

   fprintf(fd, "\n\n      /* process the request */"
               "\n      switch (reqinfo->mode)\n      {"
               "\n      case MODE_GET:"
               "\n          {");
               
   fprintf(fd, "\n             m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): got a GET request\");"
               "\n\n             /* Send the message to MAC */"
               "\n             status = snmpsubagt_mab_mac_msg_send(&mib_arg, %s,"
               "\n                                             NULL, 0, var->name[OID_LENGTH(__%s_base)],"
               "\n                                             0/* type */, NCSMIB_OP_REQ_GET, NULL, 0,"
               "\n                                             NCS_SNMPSUBAGT_MAC_TIMEOUT, &ma, space, 1024,"
               "\n                                             __%s_variables, sizeof(__%s_variables)/sizeof(struct variable),", cGroupName, grpNode->esmi.info.tableInfo.tableId, cGroupName, cGroupName, cGroupName);
   if (! grpNode->esmi.info.tableInfo.isSparseTbl)
      fprintf(fd, "\n                                             FALSE);");
   else
      fprintf(fd, "\n                                             TRUE);");

   fprintf(fd, "\n             if (status != NCSCC_RC_SUCCESS)"
               "\n             {"
               "\n                m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send() failed\", status);"
               "\n\n                /* set the error code and return/continue on to the next request?? */"
               "\n                netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_GENERR);"
               "\n                continue;"
               "\n             }"
               "\n\n             /* process the status of the MIB request */"
               "\n             if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS)"
               "\n             {"
               "\n                m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send(): Error code from Appn:\", mib_arg.rsp.i_status);  "
               "\n\n                /* set the error and continue */"
               "\n                netsnmp_set_request_error(reqinfo, cur_req, "
               "\n                                 snmpsubagt_mab_error_code_map(reqinfo->asp->mode==MODE_GET?mib_arg.rsp.i_status:NCSCC_RC_NO_INSTANCE, reqinfo->mode));"
               "\n                continue;"
               "\n             }"
               "\n\n             /* update the result */"
               "\n             val_str  = snmpsubagt_mab_mibarg_resp_process(&mib_arg.rsp.info.get_rsp.i_param_val,"
               "\n                                                           &val_len, type);"
               "\n             if (val_str == NULL)"
               "\n             {"
               "\n                m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mibarg_resp_process():error (type from Appn)\", mib_arg.rsp.info.get_rsp.i_param_val.i_fmat_id);"
               "\n\n                /* set the error code and continue to the next PDU */"
               "\n                netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_GENERR);"
               "\n                continue;"
               "\n             }"
               "\n\n             /* upload the response from application to the varbind */" 
               "\n             if (snmp_set_var_typed_value(var, type, val_str, val_len) != 0)"
               "\n             {"
               "\n                /* set the error code and continue to the next PDU */"
               "\n                m_SNMPSUBAGT_GEN_STR(\"snmp_set_var_typed_value(): failed\");"
               "\n                continue;"
               "\n             }"
               "\n          }"
               "\n          break;  /* for GET/GETNEXT(will be converted to GET by library) */"
               "\n\n      case MODE_SET_RESERVE1:"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): Got RESERVE1 request\");"
               "\n\n          /* validate the type, length of the value to be set, and the value to be set */"
               "\n          error = __%s_validate_set_varbinds("
               "\n                                  var->name[OID_LENGTH(__%s_base)], cur_req);"
               "\n          if (error != SNMP_ERR_NOERROR)"
               "\n          {"
               "\n             m_SNMPSUBAGT_GEN_ERR(\"%s:__%s_validate_set_varbinds(): failed with error code\", error);"
               "\n\n             /* set the error code and return */"
               "\n             netsnmp_set_request_error(reqinfo, cur_req, error);"
               "\n             continue;"
               "\n          }"
               "\n\n          /* send the TEST request */"
               "\n          status = snmpsubagt_mab_mac_msg_send(&mib_arg, %s,"
               "\n                                               NULL, 0, var->name[OID_LENGTH(__%s_base)],"
               "\n                                               cur_req->requestvb->type,"
               "\n                                               NCSMIB_OP_REQ_TEST,"
               "\n                                               (void*)cur_req->requestvb->val.string, cur_req->requestvb->val_len,"
               "\n                                               NCS_SNMPSUBAGT_MAC_TIMEOUT, &ma, space, 1024,"
               "\n                                               __%s_variables, sizeof(__%s_variables)/sizeof(struct variable),", cGroupName, cGroupName, cGroupName, cGroupName, cGroupName, cGroupName, cGroupName, cGroupName,
                     grpNode->esmi.info.tableInfo.tableId, cGroupName, cGroupName, cGroupName);
   if (! grpNode->esmi.info.tableInfo.isSparseTbl)
      fprintf(fd, "\n                                             FALSE);");
   else
      fprintf(fd, "\n                                             TRUE);");
   fprintf(fd,"\n          if (status != NCSCC_RC_SUCCESS)"
               "\n          {"
               "\n             m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send()[TEST Req] failed\", status);"
               "\n             netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_GENERR);"
               "\n             continue;"
               "\n          }"
               "\n\n          /* process the status of the MIB request */"
               "\n          if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS)"
               "\n          {"
               "\n             m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send()[TEST Req]:error code from Appn\", mib_arg.rsp.i_status);"
               "\n             netsnmp_set_request_error(reqinfo, cur_req,"
               "\n                               snmpsubagt_mab_error_code_map(mib_arg.rsp.i_status, reqinfo->mode));"
               "\n             continue;"
               "\n          }"
               "\n          break;", cGroupName, cGroupName);

   fprintf(fd, "\n\n      case MODE_SET_RESERVE2:"
               "\n          continue;"
               "\n          break;"
               "\n\n      case MODE_SET_ACTION:"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler():got ACTION request\");"
               "\n\n          /* send the SET request */"
               "\n          status = snmpsubagt_mab_mac_msg_send(&mib_arg, %s,"
               "\n                                      NULL, 0, var->name[OID_LENGTH(__%s_base)],"
               "\n                                      cur_req->requestvb->type,"
               "\n                                      NCSMIB_OP_REQ_SET,"
               "\n                                      (void*)cur_req->requestvb->val.string, cur_req->requestvb->val_len,"
               "\n                                      NCS_SNMPSUBAGT_MAC_TIMEOUT, &ma, space, 1024,"
               "\n                                      __%s_variables, sizeof(__%s_variables)/sizeof(struct variable),", cGroupName, grpNode->esmi.info.tableInfo.tableId, cGroupName, cGroupName, cGroupName);
   if (! grpNode->esmi.info.tableInfo.isSparseTbl)
      fprintf(fd, "\n                                             FALSE);");
   else
      fprintf(fd, "\n                                             TRUE);");
   fprintf(fd, "\n\n          if (status != NCSCC_RC_SUCCESS)"
               "\n          {"
               "\n             m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send()[SET Req] failed\", status);"
               "\n\n             netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_GENERR);"
               "\n             continue;"
               "\n          }", cGroupName);

   fprintf(fd, "\n\n          /* process the status of the MIB request */"
               "\n          if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS)"
               "\n          {"
               "\n             m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send()[SET Req]:error code from Appn\", mib_arg.rsp.i_status);"
               "\n             netsnmp_set_request_error(reqinfo, cur_req,"
               "\n                          snmpsubagt_mab_error_code_map(mib_arg.rsp.i_status, reqinfo->mode));"
               "\n             continue;"
               "\n          }"
               "\n          break;", cGroupName);

   fprintf(fd, "\n\n      case MODE_SET_COMMIT:"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler():got COMMIT request\");"
               "\n          continue;"
               "\n          break;"
               "\n\n      case MODE_SET_FREE:"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler():got FREE request\");"
               "\n          continue;"
               "\n          break;"
               "\n\n      case MODE_SET_UNDO:"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): Got UNDO request\");"
               "\n          continue;"
               "\n          break;"
               "\n\n      case MODE_GETNEXT:"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): Invalid request GETNEXT received\");  "
               "\n          break;"
               "\n\n      default:"  
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): Invalid request received\");  "  
               "\n          break;"
               "\n      }/* end of switch */\n   } /* end of for() loop */", cGroupName, cGroupName, cGroupName, cGroupName, cGroupName);

   fprintf(fd, "\n\n   return SNMP_ERR_NOERROR;"
               "\n} /* __%s_handler() */", cGroupName);
   return;
}


static void  printCheckNotAccessibleObjs(FILE *fd, SmiNode *grpNode)
{
   SmiNode *smiNode = NULL;
   SmiType *smiType = NULL;
   char    *cName = NULL;
   int     count = 0;


   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
      if ((smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR))
          && (smiNode->access == SMI_ACCESS_NOT_ACCESSIBLE) 
          && (smiNode->status != SMI_STATUS_OBSOLETE))
      {
          smiType = smiGetNodeType(smiNode);
          if (!smiType)
             continue;

          cName = translateUpper(smiNode->name);
          if (!count)
             fprintf(fd, "\n             if (table_info->colnum == %s", cName);
          else
             fprintf(fd, "\n                || table_info->colnum == %s", cName);
          xfree(cName);

          count++;
      }
   }
  
   if (count)
   {
      fprintf(fd, ")");
      fprintf(fd, "\n             {"
                  "\n                netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_NOACCESS);"
                  "\n                continue;"
                  "\n             }");
   }
       
   return;
}


static void  printNewAgtTblHandlerFunc(FILE *fd, SmiNode *grpNode)
{
   char *cGroupName = NULL;
   char *cParentName = NULL;
   char *cName = NULL;
   SmiNode *smiNode = NULL;
   SmiNode *parentSmiNode = NULL;
   SmiType *smiType = NULL;

   parentSmiNode = smiGetParentNode(grpNode); 
   cParentName = esmiTranslateHungarian(parentSmiNode->name);
   cGroupName = esmiTranslateHungarian(grpNode->name);

   fprintf(fd, "\n\nint __%s_handler(netsnmp_mib_handler          *handler,"
               "\n                               netsnmp_handler_registration *reginfo,"
               "\n                               netsnmp_agent_request_info   *reqinfo,"
               "\n                               netsnmp_request_info         *requests)"
               "\n{", cParentName);

   fprintf(fd, "\n   netsnmp_request_info       *cur_req = NULL;"
               "\n   netsnmp_table_request_info *table_info = NULL;"        
               "\n   netsnmp_variable_list      *var = NULL;"
               "\n   NCSMIB_PARAM_VAL l_rsp_param_val;"
               "\n   NCSMIB_ARG       mib_arg;"
               "\n   NCSMIB_OP        req_type;"
               "\n   NCSMEM_AID       ma;"
               "\n   uns8     space[1024];"
               "\n   u_char   *val_str = NULL; "
               "\n   size_t   val_len = 0;"
               "\n   u_char   type = 0;"
               "\n   u_char   error = 0;"
               "\n   uns32    instance[MAX_OID_LEN] = {0};"
               "\n   uns32    index = 0;"
               "\n   uns32    status = NCSCC_RC_FAILURE;");
            
   fprintf(fd, "\n\n   m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): Start.\");"
               "\n\n   for (cur_req = requests; cur_req; cur_req = cur_req->next)"
               "\n   {"
               "\n      if (cur_req->processed != 0)"
               "\n          continue;"
               "\n\n      /* get the table information/details given by the table helper */" 
               "\n      table_info = netsnmp_extract_table_info(cur_req);"
               "\n      if (table_info == NULL)"
               "\n         continue;"
               "\n\n      var = cur_req->requestvb;"
               "\n      /* clean the MIBARG vehicle */"
               "\n      type = 0;"
               "\n      val_str = NULL; "
               "\n      val_len = 0; ", cParentName);

   fprintf(fd, "\n\n      m_NCS_OS_MEMSET(&mib_arg, 0, sizeof(NCSMIB_ARG));"
               "\n      m_NCS_OS_MEMSET(space, 0, sizeof(space));"
               "\n      ncsmem_aid_init(&ma, space, 1024);"
               "\n\n      /* compose the index */"
               "\n      m_NCS_OS_MEMSET(instance, 0, (MAX_OID_LEN*sizeof(uns32)));"
               "\n      for (index = 0; index < table_info->index_oid_len; index++)"
               "\n         instance[index] = (uns32)table_info->index_oid[index];");

   fprintf(fd, "\n\n      switch (reqinfo->mode)"
               "\n      {"
               "\n      case MODE_GET:"
               "\n      case MODE_GETNEXT:"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): Got GET/GETNEXT request\");"
               "\n\n          if (var->type == ASN_NULL)"
               "\n          {", cParentName);

   printCheckNotAccessibleObjs(fd, grpNode);

   fprintf(fd, "\n\n             if (reqinfo->mode == MODE_GET)"
               "\n             {"
               "\n                req_type = NCSMIB_OP_REQ_GET;"
               "\n                m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): Got GET request\");"
               "\n             }"
               "\n             else"
               "\n             {"
               "\n                req_type = NCSMIB_OP_REQ_NEXT;"
               "\n                m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): Got GETNEXT request\");"
               "\n             }"
               "\n\n             /* send the message to MAC */"
               "\n             status = snmpsubagt_mab_mac_msg_send(&mib_arg, %s,"
               "\n                                               instance, table_info->index_oid_len,"
               "\n                                               table_info->colnum,"
               "\n                                               0/* type */, req_type, NULL, 0,"
               "\n                                               NCS_SNMPSUBAGT_MAC_TIMEOUT, &ma, space, 1024,"
               "\n                                               __%s_variables, sizeof(__%s_variables)/sizeof(struct variable),", cParentName, cParentName, grpNode->esmi.info.tableInfo.tableId, cGroupName, cGroupName);
   if (! grpNode->esmi.info.tableInfo.isSparseTbl)
      fprintf(fd, "\n                                             FALSE);");
   else
      fprintf(fd, "\n                                             TRUE);");
   fprintf(fd, "\n             if (status != NCSCC_RC_SUCCESS)"
               "\n             {"
               "\n                m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send() failed\", status);"
               "\n                if (reqinfo->mode == MODE_GETNEXT)"
               "\n                  netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_NOERROR);"
               "\n                else"
               "\n\n                netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_GENERR);"
               "\n                continue;"
               "\n             }"
               "\n\n             /* process the status of the MIB request */"
               "\n             if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS)"
               "\n             {"
               "\n                m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send():error code from Appn:\",mib_arg.rsp.i_status);"
               "\n\n                /* set the error and continue */"
               "\n                if(reqinfo->mode == MODE_GETNEXT)"
               "\n                  netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_NOERROR);"
               "\n                else"
               "\n                  netsnmp_set_request_error(reqinfo, cur_req,"
               "\n                             snmpsubagt_mab_error_code_map(mib_arg.rsp.i_status, reqinfo->mode));"
               "\n                continue;"
               "\n             }", cParentName, cParentName);

   fprintf(fd, "\n\n           if (reqinfo->mode == MODE_GET)"
               "\n             {"
               "\n                 m_NCS_OS_MEMSET(&l_rsp_param_val, 0, sizeof(NCSMIB_PARAM_VAL));"
               "\n                 memcpy(&l_rsp_param_val,"
               "\n                                 &mib_arg.rsp.info.get_rsp.i_param_val,"
               "\n                                 sizeof(NCSMIB_PARAM_VAL));"
               "\n             }"
               "\n             else /* FOR GETNEXT */"
               "\n             {"
               "\n                m_NCS_OS_MEMSET(&l_rsp_param_val, 0, sizeof(NCSMIB_PARAM_VAL));"
               "\n                memcpy(&l_rsp_param_val,"
               "\n                                &mib_arg.rsp.info.next_rsp.i_param_val,"
               "\n                                sizeof(NCSMIB_PARAM_VAL));"
               "\n\n                /* compose the OID with the new instance */"
               "\n                status = snmpsubagt_mab_oid_compose(cur_req->requestvb->name,"
               "\n                                                    &cur_req->requestvb->name_length,"
               "\n                                                    __%s_base,"
               "\n                                                    OID_LENGTH(__%s_base),"
               "\n                                                    &mib_arg.rsp.info.next_rsp);"
               "\n                if (status != NCSCC_RC_SUCCESS)"
               "\n                {"
               "\n                   m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_oid_compose() failed\", status);"
               "\n                   netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_GENERR);"
               "\n                   continue;"
               "\n                }"
               "\n             } /* for GETNEXT */", cGroupName, cGroupName, cParentName);

   fprintf(fd, "\n\n             /* assign the type of the object */ "
              "\n             switch (l_rsp_param_val.i_param_id)"
              "\n             {"
              "\n                /* repeat the same case for all the objects in this table */");

   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
      if ((smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR))
          && ((smiNode->access == SMI_ACCESS_READ_ONLY) ||
              (smiNode->access == SMI_ACCESS_READ_WRITE))
          && (smiNode->status != SMI_STATUS_OBSOLETE))
      {
          smiType = smiGetNodeType(smiNode);
          if (!smiType)
             continue;

          cName = translateUpper(smiNode->name);
          fprintf(fd, "\n             case %s:", cName);
          fprintf(fd, "\n                 type = %s;", esmiGetAsnTypeString(smiType->asntype));
          fprintf(fd, "\n                 break;\n"); 

          xfree(cName);
      }
   }
                    
   fprintf(fd, "\n             default:"
               "\n                 /* set the error code, and continue to the next varbind */"
               "\n                 if ((reqinfo->mode == MODE_SET_RESERVE1) || (reqinfo->mode == MODE_SET_ACTION))"
               "\n                    netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_NOTWRITABLE);"
               "\n                 else"
               "\n                    netsnmp_set_request_error(reqinfo, cur_req, SNMP_NOSUCHOBJECT);"
               "\n                 continue;"
               "\n                 break;"
               "\n             }/* switch (table_info->colnum) */"); 

   fprintf(fd, "\n\n           /* update the result */"
               "\n             val_str  = snmpsubagt_mab_mibarg_resp_process(&l_rsp_param_val,"
               "\n                                                           &val_len, type);"
               "\n             if (val_str == NULL)"
               "\n             {"
               "\n                /* set the error code and continue to the next PDU */ "
               "\n                m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mibarg_resp_process():error (type from Appn)\", l_rsp_param_val.i_fmat_id);"
               "\n                netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_GENERR);"
               "\n                continue;"
               "\n             }", cParentName);

   fprintf(fd, "\n\n           /* upload the response from application to the varbind */ "
               "\n             if (snmp_set_var_typed_value(var, type, val_str, val_len) != 0)"
               "\n             {"
               "\n                /* set the error code and continue to the next PDU */"
               "\n                m_SNMPSUBAGT_GEN_STR(\"snmp_set_var_typed_value(): failed\");"
               "\n                continue;"
               "\n             }"
               "\n          }/* end if (var->type == ASN_NULL) */"
               "\n          break; /* MODE_GET/GETNEXT */"
               "\n\n      case MODE_SET_RESERVE1:"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): Got RESERVE1 request\");"
               "\n\n          /* validate the type, length of the value to be set, and the value to be set */"
               "\n          error = __%s_validate_set_varbinds(table_info->colnum, cur_req);"
               "\n          if (error != SNMP_ERR_NOERROR)"
               "\n          {"
               "\n             m_SNMPSUBAGT_GEN_ERR(\"%s:__%s_validate_set_varbinds(): failed with error code\", error);"
               "\n             /* set the error code and return */"
               "\n             netsnmp_set_request_error(reqinfo, cur_req, error);"
               "\n             continue;"
               "\n          }"
               "\n\n          /* send the TEST request */"
               "\n          status = snmpsubagt_mab_mac_msg_send(&mib_arg, %s,"
               "\n                                               instance, table_info->index_oid_len, table_info->colnum,"
               "\n                                               cur_req->requestvb->type,"
               "\n                                               NCSMIB_OP_REQ_TEST,"
               "\n                                               (void*)cur_req->requestvb->val.string, cur_req->requestvb->val_len,"
               "\n                                               NCS_SNMPSUBAGT_MAC_TIMEOUT, &ma, space, 1024,"
               "\n                                               __%s_variables, sizeof(__%s_variables)/sizeof(struct variable),", cParentName, cParentName, cParentName, cParentName, grpNode->esmi.info.tableInfo.tableId, cGroupName, cGroupName);
   if (! grpNode->esmi.info.tableInfo.isSparseTbl)
      fprintf(fd, "\n                                             FALSE);");
   else
      fprintf(fd, "\n                                             TRUE);");
   fprintf(fd, "\n          if (status != NCSCC_RC_SUCCESS)"
               "\n          {"
               "\n             m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send()[TEST Req] failed\", status);"
               "\n             netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_GENERR);"
               "\n             continue;"
               "\n          }"
               "\n\n          /* process the status of the MIB request */"
               "\n          if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS)"
               "\n          {"
               "\n             m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send()[TEST Req]:error code from Appn\", mib_arg.rsp.i_status);"
               "\n             netsnmp_set_request_error(reqinfo, cur_req,"
               "\n                            snmpsubagt_mab_error_code_map(mib_arg.rsp.i_status, reqinfo->mode));"
               "\n             continue;"
               "\n          }"
               "\n          break;", cParentName, cParentName);

   fprintf(fd, "\n\n      case MODE_SET_RESERVE2:"
               "\n          /* allocate memory, if any.  We do not support this */"
               "\n          continue;"
               "\n          break;");

   fprintf(fd, "\n\n      case MODE_SET_ACTION:"
               "\n          /* Send the SET request */"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler():got ACTION request\");"
               "\n          status = snmpsubagt_mab_mac_msg_send(&mib_arg, %s,"
               "\n                             instance, table_info->index_oid_len, table_info->colnum,"
               "\n                             cur_req->requestvb->type,"
               "\n                             NCSMIB_OP_REQ_SET,"
               "\n                             (void*)cur_req->requestvb->val.string, cur_req->requestvb->val_len,"
               "\n                             NCS_SNMPSUBAGT_MAC_TIMEOUT, &ma, space, 1024,"
               "\n                             __%s_variables, sizeof(__%s_variables)/sizeof(struct variable), ", cParentName, grpNode->esmi.info.tableInfo.tableId, cGroupName, cGroupName);
   if (! grpNode->esmi.info.tableInfo.isSparseTbl)
      fprintf(fd, "\n                                             FALSE);");
   else
      fprintf(fd, "\n                                             TRUE);");
      
   fprintf(fd, "\n          if (status != NCSCC_RC_SUCCESS)"
               "\n          {"
               "\n             m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send()[SET Req] failed\", status);"
               "\n             netsnmp_set_request_error(reqinfo, cur_req, SNMP_ERR_GENERR);"
               "\n             continue;"
               "\n          }"

               "\n\n          /* process the status of the MIB request */"
               "\n          if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS)"
               "\n          {"
               "\n             m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_mab_mac_msg_send()[SET Req]:error code from Appn\", mib_arg.rsp.i_status);"
               "\n             netsnmp_set_request_error(reqinfo, cur_req," 
               "\n                           snmpsubagt_mab_error_code_map(mib_arg.rsp.i_status, reqinfo->mode));"
               "\n             continue;"
               "\n          }"
               "\n          break;", cParentName, cParentName);

   fprintf(fd, "\n\n      case MODE_SET_FREE:"
               "\n          /** Forget undo data, if exists */"
               "\n         m_SNMPSUBAGT_GEN_STR(\"__%s_handler():got FREE request\");"
               "\n          continue;"
               "\n          break;", cParentName);

   fprintf(fd, "\n\n      case MODE_SET_COMMIT:"
               "\n          /** Forget undo data, if exists */"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler():got COMMIT request\");"
               "\n          continue;"
               "\n          break;", cParentName);

   fprintf(fd, "\n\n      case MODE_SET_UNDO:"
               "\n          /** Forget undo data, if exists */"
               "\n          m_SNMPSUBAGT_GEN_STR(\"__%s_handler(): Got UNDO request\");"
               "\n          continue;"
               "\n          break;", cParentName);
                                                       
   fprintf(fd, "\n\n      default:"
               "\n         m_SNMPSUBAGT_GEN_STR(\"__%s_handler():got UNKNOWN request\");"
               "\n          break;"
               "\n      }/* end of switch */"
               "\n   }/* end of  for(cur_req) */"
               "\n\n   return SNMP_ERR_NOERROR;"
               "\n}/* end __%s_handler() */", cParentName, cParentName);
   return;
}


static void  printNewValidateSetVarBinds(FILE *fd, SmiNode *grpNode)
{
   char *cGroupName = NULL;
   char *cParentName = NULL;
   char *cName = NULL;
   SmiType *smiType = NULL;
   SmiNode *smiNode = NULL;
   SmiNode *parentSmiNode = NULL;
   Object  *obj = NULL;

   parentSmiNode = smiGetParentNode(grpNode); 
   cParentName = esmiTranslateHungarian(parentSmiNode->name);
   cGroupName = esmiTranslateHungarian(grpNode->name);

   if (grpNode->indexkind == SMI_INDEX_UNKNOWN)
   {
      fprintf(fd, "\n\n\nstatic u_char __%s_validate_set_varbinds(int colnum,"
                  "\n                                            netsnmp_request_info *cur_req)"
                  "\n{"
                  "\n   u_char  snmp_err = SNMP_ERR_NOERROR;"
                  "\n\n   m_SNMPSUBAGT_GEN_STR(\"__%s_validate_set_varbinds()\");"
                  "\n\n   switch (colnum)"
                  "\n   {", cGroupName, cGroupName);
   }
   else
   {
      fprintf(fd, "\n\nstatic u_char __%s_validate_set_varbinds(int colnum,"
                  "\n                                            netsnmp_request_info *cur_req)"
                  "\n{"
                  "\n   u_char  snmp_err = SNMP_ERR_NOERROR;"
                  "\n\n   m_SNMPSUBAGT_GEN_STR(\"__%s_validate_set_varbinds()\");"
                  "\n\n   switch (colnum)"
                  "\n   {", cParentName, cParentName);
   }

   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
      if ((smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR))
          && ((smiNode->access == SMI_ACCESS_READ_ONLY) ||
              (smiNode->access == SMI_ACCESS_READ_WRITE) ||
              (smiNode->access == SMI_ACCESS_NOT_ACCESSIBLE))
          && (smiNode->status != SMI_STATUS_OBSOLETE))
      {
          smiType = smiGetNodeType(smiNode);
          if (!smiType)
             continue;

          cName = translateUpper(smiNode->name);
          if (smiNode->access == SMI_ACCESS_READ_WRITE)
          {
             fprintf(fd, "\n   case %s:"
                         "\n       /* Validate the type */"
                         "\n       if (cur_req->requestvb->type != %s)"
                         "\n       {"
                         "\n          snmp_err = SNMP_ERR_WRONGTYPE;"
                         "\n          break;"
                         "\n       }", cName, esmiGetAsnTypeString(smiType->asntype)); 

             obj = (Object *)smiNode;

             switch(smiType->asntype)
             {
             case ESMI_ASN_INTEGER:
                 fprintf(fd, "\n\n       /* validate the value length */"
                             "\n       if (cur_req->requestvb->val_len != sizeof(long))"
                             "\n       {"
                             "\n          snmp_err = SNMP_ERR_WRONGLENGTH;"
                             "\n          break;"
                             "\n       }");
                 
                 /* Do the range with the values specified in MIB file for an object or with
                  * MIB default values as per object type. The range is done only for 
                  * non-discrete values.
                  */
                 if ((obj->typePtr->listPtr) &&
                     (!obj->typePtr->listPtr->nextPtr))
                 {
                    if ( (((Range *)obj->typePtr->listPtr->ptr)->export.minValue.value.integer32 == 0x80000000) &&
                         (((Range *)obj->typePtr->listPtr->ptr)->export.maxValue.value.integer32 == 0x80000000) )
                       fprintf(fd, "\n       if ((*((long*)cur_req->requestvb->val.string) < 0x80000000) ||" 
                                   "\n           (*((long*)cur_req->requestvb->val.string) > 0x80000000))"
                                   "\n       {"
                                   "\n          snmp_err = SNMP_ERR_WRONGVALUE;"
                                   "\n          break;"
                                   "\n       }");
                    else if (((Range *)obj->typePtr->listPtr->ptr)->export.minValue.value.integer32 == 0x80000000) 
                       fprintf(fd, "\n       if ((*((long*)cur_req->requestvb->val.string) < 0x80000000) ||" 
                                   "\n           (*((long*)cur_req->requestvb->val.string) > %d))"
                                   "\n       {"
                                   "\n          snmp_err = SNMP_ERR_WRONGVALUE;"
                                   "\n          break;"
                                   "\n       }",
                                ((Range *)obj->typePtr->listPtr->ptr)->export.maxValue.value.integer32);
                    else if (((Range *)obj->typePtr->listPtr->ptr)->export.maxValue.value.integer32 == 0x80000000)
                       fprintf(fd, "\n       if ((*((long*)cur_req->requestvb->val.string) < %d) ||" 
                                   "\n           (*((long*)cur_req->requestvb->val.string) > 0x80000000))"
                                   "\n       {"
                                   "\n          snmp_err = SNMP_ERR_WRONGVALUE;"
                                   "\n          break;"
                                   "\n       }",
                                ((Range *)obj->typePtr->listPtr->ptr)->export.minValue.value.integer32);
                    else
                       fprintf(fd, "\n       if ((*((long*)cur_req->requestvb->val.string) < %d) ||" 
                                   "\n           (*((long*)cur_req->requestvb->val.string) > %d))"
                                   "\n       {"
                                   "\n          snmp_err = SNMP_ERR_WRONGVALUE;"
                                   "\n          break;"
                                   "\n       }",
                                ((Range *)obj->typePtr->listPtr->ptr)->export.minValue.value.integer32,
                                ((Range *)obj->typePtr->listPtr->ptr)->export.maxValue.value.integer32);
                 }
                 break;
                 
             case ESMI_ASN_IPADDRESS:
                 fprintf(fd, "\n\n       /* validate the value length */"
                             "\n       if (cur_req->requestvb->val_len != sizeof(long))"
                             "\n       {"
                             "\n          snmp_err = SNMP_ERR_WRONGLENGTH;"
                             "\n          break;"
                             "\n       }");
                 break;
                 
             case ESMI_ASN_UNSIGNED:
             case ESMI_ASN_TIMETICKS:
             case ESMI_ASN_GAUGE:
             case ESMI_ASN_COUNTER:
                 fprintf(fd, "\n\n       /* validate the value length */"
                             "\n       if (cur_req->requestvb->val_len != sizeof(long))"
                             "\n       {"
                             "\n          snmp_err = SNMP_ERR_WRONGLENGTH;"
                             "\n          break;"
                             "\n       }");
                 
                 /* Do the range with the values specified in MIB file for an object or with
                  * MIB default values as per object type. The range is done only for 
                  * non-discrete values.
                  */
                 if ((obj->typePtr->listPtr) &&
                     (!obj->typePtr->listPtr->nextPtr))
                 {
                    fprintf(fd, "\n       if ((*((unsigned long*)cur_req->requestvb->val.string) < 0x%x) ||" 
                                "\n           (*((unsigned long*)cur_req->requestvb->val.string) > 0x%x))"
                                "\n       {"
                                "\n          snmp_err = SNMP_ERR_WRONGVALUE;"
                                "\n          break;"
                                "\n       }",
                            ((Range *)obj->typePtr->listPtr->ptr)->export.minValue.value.unsigned32,
                            ((Range *)obj->typePtr->listPtr->ptr)->export.maxValue.value.unsigned32);
                 }
                 break;

             case ESMI_ASN_OCTET_STR:
             case ESMI_ASN_OPAQUE:
                 /* Do the range with the values specified in MIB file for an object or with
                  * MIB default values as per object type. The range is done only for 
                  * non-discrete values.
                  */
                 if ((obj->typePtr->listPtr) &&
                     (!obj->typePtr->listPtr->nextPtr))
                 {
                    fprintf(fd, "\n\n       /* validate the value length */"
                                "\n       if ((cur_req->requestvb->val_len < %d) ||"
                                "\n           (cur_req->requestvb->val_len > %d))"
                                "\n       {"
                                "\n          snmp_err = SNMP_ERR_WRONGLENGTH;"
                                "\n          break;"
                                "\n       }", 
                          ((Range *)obj->typePtr->listPtr->ptr)->export.minValue.value.integer32,
                          ((Range *)obj->typePtr->listPtr->ptr)->export.maxValue.value.integer32);
                 }
                 else
                 {
                    fprintf(fd, "\n\n       /* validate the value length */"
                                "\n       if ((cur_req->requestvb->val_len < 0) ||"
                                "\n           (cur_req->requestvb->val_len > 0xffff))"
                                "\n       {"
                                "\n          snmp_err = SNMP_ERR_WRONGLENGTH;"
                                "\n          break;"
                                "\n       }");
                 }
                 break;

             case ESMI_ASN_COUNTER64:
                 fprintf(fd, "\n\n       /* validate the value length */"
                             "\n       if (cur_req->requestvb->val_len != 8)"
                             "\n       {"
                             "\n          snmp_err = SNMP_ERR_WRONGLENGTH;"
                             "\n          break;"
                             "\n       }");
                 break;
              
             default:
                 break;
             }
          }
          else if (smiNode->access == SMI_ACCESS_READ_ONLY)
          {
             fprintf(fd, "\n   /* read-only object */"
                         "\n   case %s:"
                         "\n       snmp_err = SNMP_ERR_NOTWRITABLE;\n", cName);
          }
          else /* SMI_ACCESS_NOT_ACCESSIBLE */
          {
             fprintf(fd, "\n   /* NOTE: No verification against the values is not done, since there is no"
                         "\n    * mention about the range in the MIB definition."
                         "\n    */");
             fprintf(fd, "\n   case %s:"
                         "\n       snmp_err = SNMP_ERR_NOTWRITABLE;", cName);
          }
          
          fprintf(fd, "\n       break;\n");
          xfree(cName);
      }
   } /* End of for() loop */

   fprintf(fd, "\n   default:"
               "\n       snmp_err = SNMP_ERR_NOTWRITABLE;"
               "\n       break;\n");

   fprintf(fd, "   }\n\n   return snmp_err;\n}\n");

   xfree(cGroupName);
   xfree(cParentName);

   return;
}


static void printNewAgtScalarRegister(FILE *fd, SmiNode *grpNode)
{
   char    *cGroupName = NULL;
   char    *cName = NULL;
   int     firstNode = FALSE;
   SmiNode *smiNode = NULL;
   SmiNode *tempSmiNode = NULL;


   cGroupName = esmiTranslateHungarian(grpNode->name);

   fprintf(fd, "\n   __%s_reg = netsnmp_create_handler_registration(\"%s\","
               "\n                             __%s_handler, __%s_base,"
               "\n                             oid_len, %s);"
               "\n   if (__%s_reg == NULL)"
               "\n   {"
               "\n      m_SNMPSUBAGT_GEN_STR(\"%s: Registration handler creation failed\");"
               "\n      return snmp_err;"
               "\n   }", grpNode->name, grpNode->name, cGroupName, cGroupName, esmiGetAccessString(grpNode),
                         grpNode->name, grpNode->name);
               
   fprintf(fd, "\n\n   /* check the return code */"
               "\n   snmp_err = netsnmp_register_scalar_group(__%s_reg,", grpNode->name);

   /* Print the min_column value & max column value */
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
      if ((smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR))
           && ((smiNode->access == SMI_ACCESS_READ_ONLY) ||
               (smiNode->access == SMI_ACCESS_READ_WRITE) ||
               (smiNode->access == SMI_ACCESS_NOT_ACCESSIBLE)) 
           && (smiNode->status != SMI_STATUS_OBSOLETE))
      {
         tempSmiNode = smiNode;

         if (firstNode)
            continue;

          /* Print the first Node column */
          cName = translateUpper(smiNode->name);
          fprintf(fd, "\n                                  %s,", cName); 
          xfree(cName);
          firstNode = TRUE;
      }
   }
  
   /* Print the last Node column */
   if (tempSmiNode) 
   {    
      cName = translateUpper(tempSmiNode->name);
      fprintf(fd, "\n                                  %s);", cName); 
      xfree(cName);
   }

   fprintf(fd, "\n   if (snmp_err != SNMP_ERR_NOERROR)"
               "\n   {"
               "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:netsnmp_register_scalar_group(): failed\", snmp_err);"
               "\n\n      /* May be some cleanup is required for reginfo "
               "\n      * It is done as part of the netsnmp_subtree_free()"
               "\n      */"
               "\n      __%s_reg = NULL;"
               "\n      return snmp_err;"
               "\n   }"
               "\n\n   m_SNMPSUBAGT_GEN_STR(\"%s: Registered\");"
               "\n\n   memset(&log_oid, 0, sizeof(NCSFL_MEM));"
               "\n   log_oid.len = sizeof(__%s_base);"
               "\n   log_oid.addr = log_oid.dump = (char*)__%s_base;"
               "\n\n   m_SNMPSUBAGT_GEN_OID_LOG(log_oid);"
               "\n\n   /* Adds the tableId to the (TABLE/TRAP ID) patricia tree */"
               "\n    status = snmpsubagt_table_oid_add(%s,"
               "\n                                      __%s_base,"
               "\n                                      oid_len,"
               "\n                                      __%s_variables,"
               "\n                                      var_count);"
               "\n   if (status != NCSCC_RC_SUCCESS)"
               "\n   {"
               "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_table_oid_add(): failed:\", status);  "
               "\n      return SNMPERR_GENERR;"
               "\n   }", cGroupName, grpNode->name, grpNode->name, cGroupName, cGroupName,
                     grpNode->esmi.info.tableInfo.tableId, cGroupName, cGroupName, cGroupName);

   fprintf(fd, "\n\n   return snmp_err;\n");

   xfree(cGroupName);

   return;
}


static void printNewAgtAddIndices(FILE *fd, SmiNode *grpNode)  
{
   List   *listPtr = NULL;
   int    fixedLenStr = FALSE;
   SmiNode *smiNode = NULL;
   SmiType *smiType = NULL;
   Object *obj = NULL;
   Object *objPtr = (Object *)grpNode;


   /* Check whether the indices are defined for this table */
   if ((!objPtr->listPtr) || (grpNode->indexkind == SMI_INDEX_AUGMENT))
   {
      /* No indices are defined for this table */
      fprintf(fd, " ASN_PRIV_IMPLIED_OCTET_STR, 0);");
      return;
   }

   /* Check if the indices has a fixed length string in it, Yet to decide 
    * for OBJECT_ID
    */ 
   for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr) 
   {
      smiNode = (SmiNode *)&((Object *)listPtr->ptr)->export;
      
      smiType = smiGetNodeType(smiNode);
      if (!smiType) continue;
     
      if ((smiType->basetype == SMI_BASETYPE_OCTETSTRING) &&
          (smiType->asntype != ESMI_ASN_IPADDRESS))
      {
         obj = (Object *)smiNode;
         if (((Range *)obj->typePtr->listPtr->ptr)->export.minValue.value.integer32 ==
             ((Range *)obj->typePtr->listPtr->ptr)->export.minValue.value.integer32)
         {
            fixedLenStr = TRUE;
         }
      }
   }

   if (fixedLenStr)
   {
      fprintf(fd, " ASN_PRIV_IMPLIED_OCTET_STR,");  
   }
   else
   {
      for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr) 
      {
         smiNode = (SmiNode *)&((Object *)listPtr->ptr)->export;
      
         smiType = smiGetNodeType(smiNode);
         if (!smiType) continue;
    
         fprintf(fd, " %s,", esmiGetAsnTypeString(smiType->asntype)); 
      }
   }

   fprintf(fd, " 0);");

   return;
}


static void printNewAgtTblRegister(FILE *fd, SmiNode *grpNode)
{
   SmiNode *smiNode = NULL;
   SmiNode *parentSmiNode = NULL;
   SmiNode *tempSmiNode = NULL;
   char    *cGroupName = NULL;
   char    *cParentName = NULL;
   char    *cName = NULL;
   int     firstNode = FALSE;


   parentSmiNode = smiGetParentNode(grpNode); 
   cParentName = esmiTranslateHungarian(parentSmiNode->name);
   cGroupName = esmiTranslateHungarian(grpNode->name);
  
   fprintf(fd, "\n   __%s_reg = netsnmp_create_handler_registration(\"%s\","
               "\n                                          __%s_handler,"
               "\n                                          __%s_base,"
               "\n                                          base_oid_len,"
               "\n                                          %s);"
               "\n   if (__%s_reg == NULL)"
               "\n   {"
               "\n      m_SNMPSUBAGT_GEN_STR(\"%s: Create handler failed\");"
               "\n      return snmp_err;"
               "\n   }"
               "\n\n   __%s_tbl_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);"
               "\n   if (__%s_tbl_info == NULL)"
               "\n   {"
               "\n      m_SNMPSUBAGT_GEN_STR(\"%s: Malloc for table info failed\");"
               "\n      /* free __%s_reg */"
               "\n      /* a void function */"
               "\n      netsnmp_handler_registration_free(__%s_reg);"
               "\n      __%s_reg = NULL;"
               "\n      return snmp_err;"
               "\n   }"
               "\n\n   /* following is void function provided by the NET-SNMP library */"
               "\n   netsnmp_table_helper_add_indexes(__%s_tbl_info,",
                     parentSmiNode->name, parentSmiNode->name, cParentName, cParentName, esmiGetAccessString(grpNode),
                     parentSmiNode->name, parentSmiNode->name, parentSmiNode->name, parentSmiNode->name,
                     parentSmiNode->name, parentSmiNode->name, parentSmiNode->name, parentSmiNode->name,
                     parentSmiNode->name);
                    
   printNewAgtAddIndices(fd, grpNode);             

   fprintf(fd, "\n\n   /* NOTE: for SMIDUMP"
               "\n   If one of the indices of a table is a fixed length string, then let SMIDUMP generate"
               "\n   the following code. This is because, this API does not take the length of the octet"
               "\n   string. To avoid any changes to the API, let SMIDUMP generate,"
               "\n   netsnmp_table_helper_add_indexes(__%s_tbl_info,"
               "\n                                    ASN_PRIV_IMPLIED_OCTET_STR, 0);"
               "\n   Same as for a fixed length  OID type of index, it would be"       
               "\n   netsnmp_table_helper_add_indexes(__%s_tbl_info,"
               "\n                                    ASN_PRIV_IMPLIED_OBJECT_ID, 0);"
               "\n   If a table has variable length strings for eg str1, str2(1..10), int, str3(1..10),"
               "\n   then SMIDUMP would dump the following line."
               "\n   netsnmp_table_helper_add_indexes(__%s_tbl_info, ASN_OCTET_STR,"
               "\n                               ASN_OCTET_STR, ASN_INTEGER, ASN_OCTET_STR,0);"
               "\n   */",
                parentSmiNode->name, parentSmiNode->name, parentSmiNode->name);

   /* Print the min_column value & max column value */
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
      if ((smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR))
           && ((smiNode->access == SMI_ACCESS_READ_ONLY) ||
               (smiNode->access == SMI_ACCESS_READ_WRITE)) 
           && (smiNode->status != SMI_STATUS_OBSOLETE))
      {
         tempSmiNode = smiNode;

         if (firstNode)
            continue;

          /* Print the first Node column */
          cName = translateUpper(smiNode->name);
          fprintf(fd, "\n   __%s_tbl_info->min_column = %s;", parentSmiNode->name, cName); 
          xfree(cName);
          firstNode = TRUE;
      }
   }
  
   /* Print the last Node column */
   if (tempSmiNode) 
   {    
      cName = translateUpper(tempSmiNode->name);
      fprintf(fd, "\n   __%s_tbl_info->max_column = %s;", parentSmiNode->name, cName); 
      xfree(cName);
   }

   fprintf(fd, "\n\n   /* build the valid columns */"
               "\n   __%s_tbl_info->valid_columns = (netsnmp_column_info*) malloc(sizeof(netsnmp_column_info));"
               "\n   if (__%s_tbl_info->valid_columns == NULL)"
               "\n   {"
               "\n      /* without this, we can not support SNMP GETNEXT operation  */"
               "\n      m_SNMPSUBAGT_GEN_STR(\"%s: malloc for valid columns failed\"); "
               "\n      netsnmp_handler_registration_free(__%s_reg);"
               "\n      __%s_reg = NULL;"
               "\n      SNMP_FREE(__%s_tbl_info);"                            
               "\n      return snmp_err;"
               "\n   }", parentSmiNode->name, parentSmiNode->name, parentSmiNode->name,
                         parentSmiNode->name, parentSmiNode->name, parentSmiNode->name);
   
   fprintf(fd, "\n\n   memset(__%s_tbl_info->valid_columns, 0,"
               "\n          sizeof(netsnmp_column_info));", parentSmiNode->name);
                                    
   fprintf(fd, "\n\n   __%s_tbl_info->valid_columns->isRange = 0;"
               "\n   __%s_tbl_info->valid_columns->list_count ="
               "\n                 sizeof(__%s_valid_columns)/sizeof(unsigned int);"
               "\n   __%s_tbl_info->valid_columns->details.list = __%s_valid_columns;",
                     parentSmiNode->name, parentSmiNode->name, cParentName,
                     parentSmiNode->name, cParentName);
                                             
   fprintf(fd, "\n\n   /* register the handler with library, and the table with the Agent */"
               "\n   snmp_err = netsnmp_register_table(__%s_reg, __%s_tbl_info);"
               "\n   if (snmp_err != SNMPERR_SUCCESS)"
               "\n   {"
               "\n      m_SNMPSUBAGT_GEN_ERR(\"%s: Registration failed\", snmp_err);"
               "\n\n      /* free the table details */"
               "\n      free(__%s_tbl_info->valid_columns);"
               "\n\n      /* free the index columns */"
               "\n      snmp_free_varbind(__%s_tbl_info->indexes);"
               "\n\n      /* free the node */"
               "\n      SNMP_FREE(__%s_tbl_info);"                            
               "\n      __%s_reg = NULL;"
               "\n      return snmp_err;"
               "\n   }", parentSmiNode->name, parentSmiNode->name, cParentName, parentSmiNode->name,
                         parentSmiNode->name, parentSmiNode->name, parentSmiNode->name,
                         parentSmiNode->name);

   fprintf(fd, "\n\n   m_SNMPSUBAGT_GEN_STR(\"%s: Registered\");"
               "\n\n   memset(&log_oid, 0, sizeof(NCSFL_MEM));"
               "\n   log_oid.len = sizeof(__%s_base);"
               "\n   log_oid.addr = log_oid.dump = (char*)__%s_base;"
               "\n   m_SNMPSUBAGT_GEN_OID_LOG(log_oid);", parentSmiNode->name, cParentName, cParentName);
                                                                     
   fprintf(fd, "\n\n   /* Adds the tableId to the (TABLE/TRAP ID) patricia tree */"
               "\n   status = snmpsubagt_table_oid_add(%s,"
               "\n                                     __%s_base,"
               "\n                                     oid_len,"
               "\n                                     __%s_variables,"
               "\n                                     var_count);"
               "\n   if (status != NCSCC_RC_SUCCESS)"
               "\n   {"
               "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_table_oid_add(): failed\", status);"
               "\n      return SNMPERR_GENERR;"
               "\n   }", grpNode->esmi.info.tableInfo.tableId, cGroupName, cGroupName, cParentName);

   fprintf(fd, "\n\n   return snmp_err;\n");

   xfree(cParentName);
   xfree(cGroupName); 
                                                                                     
   return;
}

static void printAgtRegister(FILE *f, SmiNode *grpNode, int cnt)
{
    SmiNode *smiNode;
    char    *cGroupName;
    int     num = 0;
#if (NCS_NETSNMP != 0)
    SmiNode  *parentSmiNode = NULL;
    char     *cParentName = NULL;

    parentSmiNode = smiGetParentNode(grpNode); 
    cParentName = esmiTranslateHungarian(parentSmiNode->name);
#endif

    for (smiNode = smiGetFirstChildNode(grpNode);
    smiNode;
    smiNode = smiGetNextChildNode(smiNode)) {
   if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
       && (smiNode->access == SMI_ACCESS_READ_ONLY
      || smiNode->access == SMI_ACCESS_READ_WRITE)
#if (NCS_NETSNMP != 0)
            && (smiNode->status != SMI_STATUS_OBSOLETE)
#endif
           ) {
       num++;
   }
    }

#if (NCS_NETSNMP != 0)
        /* No read-only/read-write/read-create obj's exists in this table */ 
        if (! num) 
        {
           if (checkColumnsMaxAccess(grpNode) == ESMI_SNMP_FAILURE)
              return;

           /* Generate register function for this table which actually doesn't register it with snmpd, but adds 
              the table oid to the database. */
           cGroupName = esmiTranslateHungarian(grpNode->name);

           fprintf(f, "\n/* Register function for the table: %s */", grpNode->name);
           fprintf(f, "\nstatic int __register_%s()\n{\n", cGroupName);
           fprintf(f, "   int       snmp_err  = SNMPERR_SUCCESS;\n"); 
           fprintf(f, "   uns32     status    = NCSCC_RC_FAILURE;\n" 
                      "   uns32     var_count = sizeof(__%s_variables)/sizeof(struct variable);\n"
                      "   uns32     oid_len   = OID_LENGTH(__%s_base);\n", cGroupName, cGroupName);
           
           if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
              fprintf(f, "   NCSFL_MEM log_oid;\n");
           if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT)
           {  /* print the routine that adds the table/trap ID to the patricia tree */
              fprintf(f, "\n   /* Adds the tableId to the (TABLE/TRAP ID) patricia tree */\n");
              fprintf(f, "   status = snmpsubagt_table_oid_add(%s,\n"
                         "                                     __%s_base,\n"
                         "                                     oid_len,\n"
                         "                                     %s_variables, \n"
                         "                                     var_count);\n",
                             grpNode->esmi.info.tableInfo.tableId, cGroupName,
                             cGroupName);
              fprintf(f, "\n   if (status != NCSCC_RC_SUCCESS)"
                         "\n      return SNMPERR_GENERR;\n");
              fprintf(f, "\n   return snmp_err;\n\n");
           }
           else if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
           {
                  if (grpNode->indexkind != SMI_INDEX_UNKNOWN)
                  {
                      fprintf(f, "\n\n   m_SNMPSUBAGT_GEN_STR(\"%s: Registered\");"
                                  "\n\n   memset(&log_oid, 0, sizeof(NCSFL_MEM));"
                                      "\n   log_oid.len = sizeof(__%s_base);"
                                  "\n   log_oid.addr = log_oid.dump = (char*)__%s_base;"
                                  "\n   m_SNMPSUBAGT_GEN_OID_LOG(log_oid);", parentSmiNode->name, cParentName, cParentName);
                                                                     
                      fprintf(f, "\n\n   /* Adds the tableId to the (TABLE/TRAP ID) patricia tree */"
                                  "\n   status = snmpsubagt_table_oid_add(%s,"
                                  "\n                                     __%s_base,"
                                  "\n                                     oid_len,"
                                  "\n                                     __%s_variables,"
                                  "\n                                     var_count);"
                                  "\n   if (status != NCSCC_RC_SUCCESS)"
                                  "\n   {"
                                  "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_table_oid_add(): failed\", status);"
                                  "\n      return SNMPERR_GENERR;"
                                  "\n   }", grpNode->esmi.info.tableInfo.tableId, cGroupName, cGroupName, cParentName);
       
                      fprintf(f, "\n\n   return snmp_err;\n");
                  }
                  else
                  {
                      fprintf(f, "\n\n   m_SNMPSUBAGT_GEN_STR(\"%s: Registered\");"
                                  "\n\n   memset(&log_oid, 0, sizeof(NCSFL_MEM));"
                                      "\n   log_oid.len = sizeof(__%s_base);"
                                  "\n   log_oid.addr = log_oid.dump = (char*)__%s_base;"
                                  "\n   m_SNMPSUBAGT_GEN_OID_LOG(log_oid);", grpNode->name, cGroupName, cGroupName);
                                                                     
                      fprintf (f, "\n\n   /* Adds the tableId to the (TABLE/TRAP ID) patricia tree */"
                      "\n    status = snmpsubagt_table_oid_add(%s,"
                      "\n                                      __%s_base,"
                      "\n                                      oid_len,"
                      "\n                                      __%s_variables,"
                      "\n                                      var_count);"
                      "\n   if (status != NCSCC_RC_SUCCESS)"
                      "\n   {"
                      "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_table_oid_add(): failed:\", status);  "
                      "\n      return SNMPERR_GENERR;"
                      "\n   }", grpNode->esmi.info.tableInfo.tableId, cGroupName, cGroupName, cGroupName);

                      fprintf(f, "\n\n   return snmp_err;\n");
                  }
           }
           fprintf(f, "}\n\n");

           /* Print the routine that deletes the table/trap ID from the patricia tree
              along with the unregister code */
           fprintf(f, "\n/* De-register function for the table: %s */", grpNode->name);
           fprintf(f, "\nstatic int  __unregister_%s()\n{\n", cGroupName); 
           fprintf(f, "   uns32 status   = NCSCC_RC_FAILURE;\n"); 
           fprintf(f, "   int   snmp_err = SNMPERR_SUCCESS;"); 
           if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT) 
           {
              fprintf(f, "\n   NCSFL_MEM    log_oid;");
              if (grpNode->indexkind != SMI_INDEX_UNKNOWN)
              {
                 fprintf(f, "\n\n   m_SNMPSUBAGT_GEN_STR(\"%s: UnRegistered\");"
                            "\n\n   memset(&log_oid, 0, sizeof(NCSFL_MEM));"
                               "\n   log_oid.len = sizeof(__%s_base);"
                            "\n   log_oid.addr = log_oid.dump = (char*)__%s_base;"
                            "\n\n   m_SNMPSUBAGT_GEN_OID_LOG(log_oid);", parentSmiNode->name, cParentName, cParentName);
              }
              else
              {
                 fprintf(f, "\n\n   m_SNMPSUBAGT_GEN_STR(\"%s: UnRegistered\");"
                            "\n\n   memset(&log_oid, 0, sizeof(NCSFL_MEM));"
                            "\n   log_oid.len = sizeof(__%s_base);"
                            "\n   log_oid.addr = log_oid.dump = (char*)__%s_base;"
                            "\n\n   m_SNMPSUBAGT_GEN_OID_LOG(log_oid);", grpNode->name, cGroupName, cGroupName);
              }
           }
           fprintf(f, "\n   /* Deletes the tableId from the (TABLE/TRAP ID) patricia tree */\n");
           fprintf(f, "   status = snmpsubagt_table_oid_del(%s);",
                                               grpNode->esmi.info.tableInfo.tableId);
           fprintf(f, "\n   if (status != NCSCC_RC_SUCCESS)"
                      "\n   {");
       
           if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT) 
           {
              fprintf(f, "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_table_oid_del(): failed\", status);", cGroupName);
           }
           fprintf(f, "\n      return SNMPERR_GENERR;"
                      "\n   }");
           fprintf(f, "\n\n   return snmp_err;\n");
           fprintf(f, "}\n\n");
           return;
        }
#endif

    fprintf(f, "\n");

    if (cnt == 1) {
   fprintf(f,
      "/****************************************************************************\n"
      "  Registration & De-registration functions for the various MIB groups.\n"
      "****************************************************************************/\n");
    }
    
#if (NCS_NETSNMP != 0)
    if ((esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT) ||
        (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT))
    {
       cGroupName = esmiTranslateHungarian(grpNode->name);
       cParentName = esmiTranslateHungarian(parentSmiNode->name);
    }
    else
#endif
    cGroupName = translate(grpNode->name);

    fprintf(f, "\n/* Register function for the table: %s */", grpNode->name);
    fprintf(f, "\nstatic int __register_%s()\n{\n", cGroupName);

    fprintf(f, "   int       snmp_err  = SNMPERR_GENERR;\n"); 
#if (NCS_NETSNMP != 0) 
    if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT) 
    {  
       fprintf(f, "   uns32 status    = NCSCC_RC_FAILURE;\n" 
                  "   uns32 var_count = sizeof(%s_variables)/sizeof(struct variable);\n",
                      cGroupName);
       fprintf(f, "   uns32 oid_len   = OID_LENGTH(__%s_base);\n\n", cGroupName);

    }
    else if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
    {
       fprintf(f, "   uns32     status    = NCSCC_RC_FAILURE;\n" 
                  "   uns32     var_count = sizeof(__%s_variables)/sizeof(struct variable);\n"
                  "   uns32     oid_len   = OID_LENGTH(__%s_base);\n"
                  "   NCSFL_MEM log_oid;\n", cGroupName, cGroupName);
       
       if (grpNode->indexkind != SMI_INDEX_UNKNOWN)
       {
          fprintf(f, "   uns32     base_oid_len = OID_LENGTH(__%s_base);\n\n", cParentName);
       }
    }
#endif /* NCS_NETSNMP */

    if (esmiNetSnmpOpt != ESMI_NEW_SYN_NETSNMP_OPT)
    {
       fprintf(f,
          "   snmp_err = register_mib(\"%s\",\n"
          "                       %s_variables,\n"
          "                       sizeof(struct variable),\n"
          "                       var_count,\n"
          "                       __%s_base,\n"
          "                       oid_len);\n",
          cGroupName, cGroupName, cGroupName, cGroupName, cGroupName);
          fprintf(f, "\n   if (snmp_err != SNMPERR_SUCCESS)"
                     "\n      return snmp_err;\n\n");
    }

#if (NCS_NETSNMP != 0) 
    if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT)
    {  /* print the routine that adds the table/trap ID to the patricia tree */
       fprintf(f, "\n   /* Adds the tableId to the (TABLE/TRAP ID) patricia tree */\n");
       fprintf(f, "   status = snmpsubagt_table_oid_add(%s,\n"
                  "                                     __%s_base,\n"
                  "                                     oid_len,\n"
                  "                                     %s_variables, \n"
                  "                                     var_count);\n",
                      grpNode->esmi.info.tableInfo.tableId, cGroupName,
                      cGroupName, cGroupName, cGroupName);
       fprintf(f, "\n   if (status != NCSCC_RC_SUCCESS)"
                  "\n      return SNMPERR_GENERR;\n");
       fprintf(f, "\n   return snmp_err;\n\n");
    }
    else
    if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
    {
       fprintf(f, "\n   m_SNMPSUBAGT_GEN_STR(\"__register_%s()\");\n\n", cGroupName);

       if (grpNode->indexkind != SMI_INDEX_UNKNOWN)
       {
          printNewAgtTblRegister(f, grpNode);
       }
       else
       {
          printNewAgtScalarRegister(f, grpNode);
       }
    }
#endif /* NCS_NETSNMP */
    
    fprintf(f, "}\n\n");

#if (NCS_NETSNMP != 0) 
    if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
    {
       if (grpNode->indexkind == SMI_INDEX_UNKNOWN)
       {
          /* Print the handler routine for SCALAR table */
          printNewAgtScalarHandlerFunc(f, grpNode);
       }
       else
       {
          /* Print the handler routine for NON-SCALAR table */
          printNewAgtTblHandlerFunc(f, grpNode);
       }

       printNewValidateSetVarBinds(f, grpNode);
    }

    if ((esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT) ||
        (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT))
    {
       /* Print the routine that deletes the table/trap ID from the patricia tree
          along with the unregister code */
       fprintf(f, "\n/* De-register function for the table: %s */", grpNode->name);
       fprintf(f, "\nstatic int  __unregister_%s()\n{\n", cGroupName); 
       fprintf(f, "   uns32 status   = NCSCC_RC_FAILURE;\n"); 
       fprintf(f, "   int   snmp_err = SNMPERR_GENERR;"); 

       if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT) 
       {
          fprintf(f, "\n\n   snmp_err =  unregister_mib(__%s_base,\n"
           "                         OID_LENGTH(__%s_base));", cGroupName, cGroupName);
          fprintf(f, "\n   if (snmp_err != SNMPERR_SUCCESS)"
                     "\n      return snmp_err;\n\n");
       }
       else if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT) 
       {
          fprintf(f, "\n   NCSFL_MEM    log_oid;");
          fprintf(f, "\n\n   m_SNMPSUBAGT_GEN_STR(\"__unregister_%s()\");", cGroupName);
          
          if (grpNode->indexkind != SMI_INDEX_UNKNOWN)
          {
             fprintf(f, "\n\n   snmp_err =  snmpsubagt_mab_unregister_mib(__%s_reg);"
                        "\n   if (snmp_err != SNMPERR_SUCCESS)"
                        "\n   {"
                        "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:__unregister_%s(): failed with error code\", snmp_err);"
                        "\n   }"
                        "\n   __%s_reg = NULL;", parentSmiNode->name, cParentName, cGroupName, parentSmiNode->name);

             fprintf(f, "\n   m_SNMPSUBAGT_GEN_STR(\"%s: UnRegistered\");"
                        "\n\n   memset(&log_oid, 0, sizeof(NCSFL_MEM));"
                        "\n   log_oid.len = sizeof(__%s_base);"
                        "\n   log_oid.addr = log_oid.dump = (char*)__%s_base;"
                        "\n\n   m_SNMPSUBAGT_GEN_OID_LOG(log_oid);", grpNode->name, cParentName, cParentName);

             fprintf(f, "\n\n   /* free the table details */"
                        "\n   free(__%s_tbl_info->valid_columns);"
                        "\n\n   /* free the index columns */"
                        "\n   snmp_free_varbind(__%s_tbl_info->indexes);"
                        "\n\n   /* free the node */"
                        "\n   SNMP_FREE(__%s_tbl_info);", parentSmiNode->name,
                        parentSmiNode->name, parentSmiNode->name);
          }
          else
          {
             fprintf(f, "\n   snmp_err =  snmpsubagt_mab_unregister_mib(__%s_reg);"
                        "\n   if (snmp_err != SNMPERR_SUCCESS)"
                        "\n   {"
                        "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:__unregister_%s(): failed with error code\", snmp_err);"
                        "\n   }"
                        "\n   __%s_reg = NULL;", grpNode->name, cGroupName, cGroupName, grpNode->name);

             fprintf(f, "\n\n   m_SNMPSUBAGT_GEN_STR(\"%s: UnRegistered\");"
                        "\n\n   memset(&log_oid, 0, sizeof(NCSFL_MEM));"
                        "\n   log_oid.len = sizeof(__%s_base);"
                        "\n   log_oid.addr = log_oid.dump = (char*)__%s_base;"
                        "\n\n   m_SNMPSUBAGT_GEN_OID_LOG(log_oid);", grpNode->name, cGroupName, cGroupName);
          }
       } 
       
       fprintf(f, "\n   /* Deletes the tableId from the (TABLE/TRAP ID) patricia tree */\n");
       fprintf(f, "   status = snmpsubagt_table_oid_del(%s);",
                                           grpNode->esmi.info.tableInfo.tableId);
       fprintf(f, "\n   if (status != NCSCC_RC_SUCCESS)"
                  "\n   {");
       
       if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT) 
       {
          fprintf(f, "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_table_oid_del(): failed\", status);", cGroupName);
       }
       fprintf(f, "\n      return SNMPERR_GENERR;"
                  "\n   }");
       fprintf(f, "\n\n   return snmp_err;\n");
       fprintf(f, "}\n\n");
    }

    xfree(cParentName);

#endif /* NCS_NETSNMP */

    xfree(cGroupName);
}


static void printAgtInit(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;

    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
    smiNode;
    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
   if (isGroup(smiNode)) {
       printAgtRegister(f, smiNode, ++cnt);
   }
    }

    if (cnt) {
   fprintf(f, "\n");
    }
}

#if (NCS_NETSNMP != 0) 
static void printAddTableOidOfTrapsInImportedModule(FILE *f, SmiModule *module, int regFlag)
{
    SmiImport *currentImport;
    SmiModule *smiModule;
    SmiNode   *smiNode, *tempNode;
    int       lineNum;
    char      *cGroupName = NULL; 

    /* for all the imported modules */ 
    for(currentImport = smiGetFirstImport(module); currentImport;
        currentImport = smiGetNextImport(currentImport))
    {
        /* get the node */ 
       tempNode = esmiFindTableAnyModule(currentImport->name, 0);   
       if (tempNode == NULL)
           continue; 

        /* get the Module */ 
       smiModule = smiGetNodeModule(tempNode); 
       if (smiModule == NULL)
           continue; 

        /* do we need to generate the code this import? */ 
        if (smiModule->esmi.imported == 0)
            continue; 

        /* for all the nodes in this imported module */ 
        for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
        smiNode;
        smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
        {
            if ((esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT) ||
                (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT))
            {
               /* Generate the function call. */
               if (isTrap(smiNode)) /* For TRAP table */
               {
                    cGroupName = esmiTranslateHungarian(smiNode->name);

                    /* Check whether table id is given or not. */
                    if(smiNode->esmi.info.tableInfo.tableId[0] == '\0')
                    {
                        /* Generate warning. */
                        lineNum = ((Object *)smiNode)->line;
                      
                        fprintf(stderr, "\n%s:%d  WARNING: ESMI TableId is not defined for the Traps:  %s\n", esmiTempMibFile, lineNum, smiNode->name);
                                                                      
                        continue;
                    }
                    /* Print the routine that adds / deletes the TRAP OID to/from the
                        patricia tree (IOD database) */
                    if (regFlag == ESMI_REGISTER_FLAG)
                    {
                        fprintf(f, "\n\n   /* Adds the tableId to the (TABLE/TRAP ID) patricia tree */");
                        fprintf(f, "\n   subagt_status = snmpsubagt_table_oid_add(%s,\n"
                                    "                                              __%s_trap_base,\n"
                                    "                                              OID_LENGTH(__%s_trap_base),\n"
                                    "                                              NULL,\n"
                                    "                                              0); \n",
                                smiNode->esmi.info.tableInfo.tableId, cGroupName, 
                                cGroupName);
                        fprintf(f, "\n   if (subagt_status != NCSCC_RC_SUCCESS)"
                                    "\n   {");
                        if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
                        {
                        fprintf(f, "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_table_oid_add(): failed\", subagt_status);", cGroupName);
                        }
                        fprintf(f, "\n      return subagt_status;"
                                    "\n   }"); 
                    }
                    else /* Deregister flag set */
                    {
                        fprintf(f, "\n\n   /* Deletes the tableId from the (TABLE/TRAP ID) patricia tree */");
                        fprintf(f, "\n   subagt_status = snmpsubagt_table_oid_del(%s);",
                                           smiNode->esmi.info.tableInfo.tableId);
                        fprintf(f, "\n   if (subagt_status != NCSCC_RC_SUCCESS)"
                                    "\n   {");
                        if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
                        {
                        fprintf(f, "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_table_oid_del(): failed\", subagt_status);", cGroupName);
                        }
                        fprintf(f, "\n      return subagt_status;"
                                    "\n   }"); 
                    }
                    xfree(cGroupName);
               }
            }
        } /* for(smiNode...) */ 
    }
    return; 
}

/**************************************************************************
Name          :  printAgtNcsModuleRegister

Description   :  This function generates a common register (or) unregister  
                 routine (at MIB/Module level) in which all the table register
                 routines are called (for the tables defined in this MIB file).

Arguments     :  fd -  File descriptor of a file to which the common  
                       register/unregister routine need to generate.
                 smiModule - Pointer to the SmiModule.
                 regFlag - '1' to generate common register routine.
                           '0' to generate common un-register routine.

Return values :  Nothing  

Notes         :     
**************************************************************************/
static void printAgtNcsModuleRegister(FILE *fd,
                                      SmiModule *smiModule,
                                      int regFlag)
{
   SmiNode *smiNode, *tempNode;
   char    *cGroupName = NULL; 
   char    *cModuleName = NULL; 
   int     trapFnd = 0;
   int  lineNum;
   SmiImport *currentImport;
   SmiModule *importModule;
   
   cModuleName = translateLower(smiModule->name);

   if (regFlag == 1) /* generate routine for Registration purpose */
   {
      /* Print the function header with appropriate information for register
         routine */
      fprintf(fd, "\n/**************************************************************************"
               "\nName          :  __register_%s_module"
               "\n\nDescription   :  This function is a common registration routine where it" 
               "\n                 calls all the table registration routines of a module(MIB)."
               "\n\nArguments     :  - NIL -"
               "\n\nReturn values :  Nothing"
               "\n\nNotes         :  Basically it registers the module (for all the TABLEs"
               "\n                 defined the MIB) with SNMP agent"   
               "\n**************************************************************************/", 
                cModuleName);
      fprintf(fd, "\nuns32 __register_%s_module()\n{\n", cModuleName); 
   }
   else /* generate routine for unregistration purpose */
   {
      /* Print the function header with appropriate information for unregister
         routine */
      fprintf(fd, "\n/**************************************************************************"
               "\nName          :  __unregister_%s_module"
               "\n\nDescription   :  This function is a common unregistration routine where it" 
               "\n                 calls all the table unregistration routines of a module(MIB)."
               "\n\nArguments     :  - NIL -"
               "\n\nReturn values :  Nothing"
               "\n\nNotes         :  Basically it deregisters the module (for all the TABLEs"
               "\n                 defined the MIB) with SNMP agent"   
               "\n**************************************************************************/", 
                cModuleName);
      fprintf(fd, "\nuns32 __unregister_%s_module()\n{\n", cModuleName); 
   }
         
   for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
        smiNode;
        smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
   {
      if (isTrap(smiNode))  
      {
         trapFnd = 1;
         break;
      }
   }

    /* If trap node is not defined in this module, check if it is imported from any module. */
    if(trapFnd == 0)
    {
        /* for all the imported modules */ 
        for(currentImport = smiGetFirstImport(smiModule); currentImport;
            currentImport = smiGetNextImport(currentImport))
        {
            /* get the node */ 
            tempNode = esmiFindTableAnyModule(currentImport->name, 0);   
            if (tempNode == NULL)
            continue; 

            /* get the Module */ 
            importModule = smiGetNodeModule(tempNode); 
            if (importModule == NULL)
                continue; 

                /* do we need to generate the code this import? */ 
                if (importModule->esmi.imported == 0)
                    continue; 

                /* for all the nodes in this imported module */ 
                for (smiNode = smiGetFirstNode(importModule, SMI_NODEKIND_ANY);
                     smiNode;
                     smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
                {
                     /* Generate the function call. */
                     if (isTrap(smiNode)) /* For TRAP table */
                     {
                        trapFnd = 1;
                        break;
                     }
                }
            }
    }

   fprintf(fd, "   int   status = SNMPERR_SUCCESS;\n");

   /* subagt_status is used only if traps exits */
   if (trapFnd)
      fprintf(fd, "   uns32 subagt_status = NCSCC_RC_FAILURE;\n\n");
                                      
   if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
   {
      if (regFlag == 1) /* generate routine for Registration purpose */
      {
         fprintf(fd, "\n   m_SNMPSUBAGT_GEN_STR(\"__register_%s_module()\");", cModuleName);
      }
      else
      {
         fprintf(fd, "\n   m_SNMPSUBAGT_GEN_STR(\"__unregister_%s_module()\");", cModuleName);
      }
   }
          
   /* Free the allotted memory */
   xfree(cModuleName); 

   /* Traverse through all the table objects defined in this MIB and 
      call their respective register/unregister routines */                                   
   for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
        smiNode;
        smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
   {
      cGroupName = esmiTranslateHungarian(smiNode->name);

      if (isGroup(smiNode))
      {
         SmiNode *childNode;
         int     accessible_tbl = 0;
         
         /* Check whether all the objects of the table are not-accessible.
            In this case the related register_<table>() should not call in
            register_module() routine in the generated stubs. */
         for (childNode = smiGetFirstChildNode(smiNode);
              childNode;
              childNode = smiGetNextChildNode(childNode))
         {
             if (childNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
                 && (childNode->access == SMI_ACCESS_READ_ONLY
                     || childNode->access == SMI_ACCESS_READ_WRITE || childNode->access == SMI_ACCESS_NOTIFY)
                 && (childNode->status != SMI_STATUS_OBSOLETE))
             {
                 /* has a read-write / read-only / read-create objects */
                 accessible_tbl = 1; 
                 break;
              }
         }

         /* No read-only/read-write/read-create obj's exists in this table */ 
         if (! accessible_tbl)  continue;

         if (regFlag == ESMI_REGISTER_FLAG)
         {
            fprintf(fd, "\n\n   /* Calling the registration routine for  %s  table */\n", smiNode->name);
            fprintf(fd, "   status =  __register_%s();", cGroupName);
            fprintf(fd, "\n   if (status != SNMPERR_SUCCESS)"
                        "\n   {");
            if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
            {
               fprintf(fd, "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:__register_%s(): failed\", status);", cGroupName, cGroupName);
            }
            fprintf(fd, "\n      return NCSCC_RC_FAILURE;"
                        "\n   }"); 
         }
         else /* Deregister flag set */
         {
            fprintf(fd, "\n\n   /* Calling the de-registration routine for  %s  table */\n", smiNode->name);
            fprintf(fd, "   status =  __unregister_%s();", cGroupName);
            fprintf(fd, "\n   if (status != SNMPERR_SUCCESS)"
                        "\n   {");
            if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
            {
               fprintf(fd, "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:__unregister_%s(): failed\", status);", cGroupName, cGroupName);
            }
            fprintf(fd, "\n      return NCSCC_RC_FAILURE;"
                        "\n   }"); 
         }
      }
      else if (isTrap(smiNode))  
      {
         /* Check whether table id is given or not. */
         if(smiNode->esmi.info.tableInfo.tableId[0] == '\0')
         {
             /* Generate warning. */
             lineNum = ((Object *)smiNode)->line;
                      
             fprintf(stderr, "\n%s:%d  WARNING: ESMI TableId is not defined for the Traps:  %s\n", esmiTempMibFile, lineNum, smiNode->name);
                                                                      
             continue;
         }
         /* Print the routine that adds / deletes the TRAP OID to/from the
            patricia tree (IOD database) */
         if (regFlag == ESMI_REGISTER_FLAG)
         {
            fprintf(fd, "\n\n   /* Adds the tableId to the (TABLE/TRAP ID) patricia tree */");
            fprintf(fd, "\n   subagt_status = snmpsubagt_table_oid_add(%s,\n"
                        "                                              __%s_trap_base,\n"
                        "                                              OID_LENGTH(__%s_trap_base),\n"
                        "                                              NULL,\n"
                        "                                              0); \n",
                      smiNode->esmi.info.tableInfo.tableId, cGroupName, 
                      cGroupName);
            fprintf(fd, "\n   if (subagt_status != NCSCC_RC_SUCCESS)"
                        "\n   {");
            if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
            {
               fprintf(fd, "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_table_oid_add(): failed\", subagt_status);", cGroupName);
            }
            fprintf(fd, "\n      return subagt_status;"
                        "\n   }"); 
         }
         else /* Deregister flag set */
         {
            fprintf(fd, "\n\n   /* Deletes the tableId from the (TABLE/TRAP ID) patricia tree */");
            fprintf(fd, "\n   subagt_status = snmpsubagt_table_oid_del(%s);",
                                           smiNode->esmi.info.tableInfo.tableId);
            fprintf(fd, "\n   if (subagt_status != NCSCC_RC_SUCCESS)"
                        "\n   {");
            if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
            {
               fprintf(fd, "\n      m_SNMPSUBAGT_GEN_ERR(\"%s:snmpsubagt_table_oid_del(): failed\", subagt_status);", cGroupName);
            }
            fprintf(fd, "\n      return subagt_status;"
                        "\n   }"); 
         }
      }
      
      xfree(cGroupName);
   } /* End of for loop */

   /* print table OID add/del function calls for the trap node imported in this module. */
   printAddTableOidOfTrapsInImportedModule(fd, smiModule, regFlag);

   fprintf(fd, "\n\n   return NCSCC_RC_SUCCESS; \n");
   fprintf(fd, "}\n\n");

   return;
}


/**************************************************************************
Name          :  printAgtNcsSetWriteMethod

Description   :  This function generates a common "write_method_set" routine 
                 for a table in which all the objects write methods of this
                 TABLE are called. 

Arguments     :  fd - File descriptor of a file to which write_method_set 
                      routine is generated.
                 smiModule - Pointer to the SmiModule.

Return values :  Nothing 

Notes         :     
**************************************************************************/
static void printAgtNcsSetWriteMethod(FILE *fd, SmiModule *smiModule)
{
   SmiNode *grpNode = NULL;
   SmiNode *smiChild = NULL; 
   char    *grpName, *childName;


   /* Traverse through all the table objects defined in this MIB and 
      call their write_methods */                                   
   for (grpNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
        grpNode;
        grpNode = smiGetNextNode(grpNode, SMI_NODEKIND_ANY)) 
   {
       if (isGroup(grpNode) && isWritable(grpNode)) 
       {
          grpName = esmiTranslateHungarian(grpNode->name);

          /* For a TABLE object, generate a common write_method_set routine 
             in which the write_method for each of its objects are called */
          fprintf(fd, "\nvoid %s_write_method_set(uns32 object_id,\n", grpName); 
          fprintf(fd, "                                  WriteMethod **write_method)\n{\n");
          fprintf(fd, "   switch(object_id)\n   {\n");

          /* Free the allotted memory for grpName */
          xfree(grpName);

          /* Generates a write_method (calls) for all of the objects defined in
             a table */    
          for (smiChild = smiGetFirstChildNode(grpNode); smiChild; 
               smiChild = smiGetNextChildNode(smiChild))
          {
              /* Generates a write method if and only if the object is of 
                 readCreate (or) readWrite type */
              if (smiChild->access == SMI_ACCESS_READ_WRITE)
              {
                 childName = esmiTranslateHungarian(smiChild->name);
                 fprintf(fd, "   case %s:\n", translateUpper(smiChild->name));
                 fprintf(fd, "       *write_method = (WriteMethod *)&write_%s_stub;\n", childName);
                 fprintf(fd, "       break;\n\n");
                 xfree(childName);
              }
          }
   
          fprintf(fd, "   default:\n        *write_method = NULL;\n"
                      "        break;\n   }\n");
          fprintf(fd, "\n   return;\n}\n\n");

       }/* end of the group node */
   }/* end of for loop */
   
   return; 
}


/**************************************************************************
Name          :  printNcsAgtReadMethod

Description   :  This function generates a read method w.r.t the TABLE. 
                 It does SNMP GET/NEXT operation on an object of this table.
                 For an SNMP SET operation it calls the corresponding 
                 write_method. 

Arguments     :  fd - File descriptor of a file to which read_method 
                      routine is generated.
                 grpNode - Pointer to the SmiNode   

Return values :  Nothing 

Notes         :     
**************************************************************************/
static void printNcsAgtReadMethod(FILE *fd, SmiNode *grpNode)
{
   SmiType *smiType;
   char    *sName;
   char    *cGroupName = NULL;
                                                                                
   sName = esmiTranslateHungarian(grpNode->name);
                                                                                
   fprintf(fd,
        "static unsigned char *\nread_%s_stub(struct variable *vp,\n"
        "               oid     *name,\n"
        "               size_t  *length,\n"
        "               int     exact,\n"
        "               size_t  *var_len,\n"
        "               WriteMethod **write_method)\n"
        "{\n", sName);
   
   /* print the local variables */
   fprintf(fd, "   NCSMIB_ARG  mib_arg;"
               "\n   NCSMEM_AID  ma;"
               "\n   uns8        space[1024];"
               "\n   uns32       status = NCSCC_RC_FAILURE;"); 
   fprintf(fd, "\n   uns32       instance[MAX_OID_LEN]/* TBD NCS Instance Length */;\n   uns32       instance_len = 0;\n"); 

   fprintf(fd, "   NCSMIB_OP   req_type;\n");

   if (areScalars(grpNode))
   {
      fprintf(fd, "\n\n   /* Do the generic checks  */");
      fprintf(fd, "\n   status = snmpsubagt_given_oid_validate(vp, name, length, exact,"
                  " var_len, write_method);");

      fprintf(fd, "\n   if (status != NCSCC_RC_SUCCESS)\n       return NULL;\n");
   }

   /* SET request or not?? */
   fprintf(fd, "\n   /* SET request */ \n"); 
   fprintf(fd, "   if (exact == 2)\n   {\n");

   if (isWritable(grpNode))/* does this group have writable objects?? */
   {
       fprintf(fd, "       /* SET the write_Method, and return NULL */\n"); 
       fprintf(fd, "       %s_write_method_set(vp->magic, write_method);\n", sName);
   }
   else
   {
      fprintf(fd, "       /* there are no writable objects in this table */\n"); 
      fprintf(fd, "       write_method = NULL;\n");
   }

   fprintf(fd, "       return NULL;\n   }\n\n");

   fprintf(fd, "   memset(&mib_arg, 0, sizeof(NCSMIB_ARG));"
               "\n   m_NCS_OS_MEMSET(space, 0, sizeof(space));"
               "\n   ncsmem_aid_init(&ma, space, 1024);\n\n");
   fprintf(fd, "   memset(&rsp_param_val, 0, sizeof(NCSMIB_PARAM_VAL));\n");
   fprintf(fd, "   memset(instance, 0, (MAX_OID_LEN*sizeof(uns32)));\n\n");

   if (! areScalars(grpNode))
   {
       fprintf(fd, "   /* extract the index from oid */\n");
       fprintf(fd, "   status = snmpsubagt_mab_index_extract(name, *length,\n");
       fprintf(fd, "                                     __%s_base,\n", sName); 
       fprintf(fd, "                                     OID_LENGTH(__%s_base),\n", 
                       sName);
       fprintf(fd, "                                     instance, &instance_len);\n");
       fprintf(fd, "   if (status != NCSCC_RC_SUCCESS)\n       return NULL;\n\n");
   }
   else
   {
      fprintf(fd, "   /* Instance Length for Scalars */\n");
      fprintf(fd, "   instance_len = 0;\n");
   }
   
   if (! areScalars(grpNode))
   {
      fprintf(fd, "   if (exact == 1)\n       req_type = NCSMIB_OP_REQ_GET;\n");
      fprintf(fd, "   else if (exact == 0)\n           req_type = NCSMIB_OP_REQ_NEXT;\n\n");
   }
   else  /* For scalar table */
   {
      fprintf(fd, "\n   req_type = NCSMIB_OP_REQ_GET;\n\n");
   }

   cGroupName = esmiTranslateHungarian(grpNode->name);
   fprintf(fd, "   /* Send the message to MAC */\n");
   fprintf(fd, "   status = snmpsubagt_mab_mac_msg_send(&mib_arg, %s, \n",
      grpNode->esmi.info.tableInfo.tableId);
   fprintf(fd, "                              instance, instance_len, vp->magic,\n");
   fprintf(fd, "                              vp->type, req_type, NULL, 0,\n");
   fprintf(fd, "                              NCS_SNMPSUBAGT_MAC_TIMEOUT, &ma, space, 1024,"
               "\n                             __%s_variables, sizeof(__%s_variables)/sizeof(struct variable), ", cGroupName, cGroupName);
   if (! grpNode->esmi.info.tableInfo.isSparseTbl)
      fprintf(fd, "\n                                             FALSE);");
   else
      fprintf(fd, "\n                                             TRUE);");

   xfree(cGroupName);
   
   fprintf(fd, "   if (status != NCSCC_RC_SUCCESS)\n       return NULL;\n\n");

   fprintf(fd, "   /* process the status of the MIB request */\n");
   fprintf(fd, "   if (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS)\n");
   fprintf(fd, "      return NULL;\n\n");

   if (! areScalars(grpNode))
   {
      fprintf(fd, "   /* extract the param_val based on the type of the request */\n");
      fprintf(fd, "   if (exact == 1) /* GET request */\n   {\n");
      fprintf(fd, "      memcpy(&rsp_param_val, \n"); 
      fprintf(fd, "             &mib_arg.rsp.info.get_rsp.i_param_val, \n");
      fprintf(fd, "             sizeof(NCSMIB_PARAM_VAL));\n   }\n");
      fprintf(fd, "   else /* GETNEXT request */\n   {\n");
      fprintf(fd, "      memcpy(&rsp_param_val,\n");
      fprintf(fd, "             &mib_arg.rsp.info.next_rsp.i_param_val,\n");
      fprintf(fd, "             sizeof(NCSMIB_PARAM_VAL));\n\n");
                                                                                    
      fprintf(fd, "      /* compose the OID with the new instance */");
      fprintf(fd, "\n      status = snmpsubagt_mab_oid_compose(name, length, __%s_base,"
                  "\n                                    OID_LENGTH(__%s_base),"
                  "\n                                    &mib_arg.rsp.info.next_rsp);", sName, sName); 
      fprintf(fd, "\n      if (status != NCSCC_RC_SUCCESS)\n      {");
      fprintf(fd, "\n         /* Log the ERROR */");
      fprintf(fd, "\n         return NULL;\n      }\n");
      fprintf(fd, "   }\n\n");   
   }
   else  /* For scalar table */
   {
      fprintf(fd, "   /* extract the param_val based on the type of the request */\n");
      fprintf(fd, "   memcpy(&rsp_param_val, \n"); 
      fprintf(fd, "          &mib_arg.rsp.info.get_rsp.i_param_val, \n");
      fprintf(fd, "          sizeof(NCSMIB_PARAM_VAL));\n");
   }

   fprintf(fd, "\n   return (snmpsubagt_mab_mibarg_resp_process(&rsp_param_val,var_len,vp->type));\n}\n\n");   
   xfree(sName);

   return;
}


/**************************************************************************
Name          :  printNcsAgtWriteMethod

Description   :  This function generates write method for each of the 
                 readWrite / readCreate object defined in a MIB.

Arguments     :  fd - File descriptor of a file to which write_method is 
                      generated.
                 smiNode - Pointer to a  SMI Node

Return values :  Nothing 

Notes         :     
**************************************************************************/
static void printNcsAgtWriteMethod(FILE *fd, SmiNode *smiNode)
{
   SmiNode  *parentPtr = NULL; 
   char     *cName, *cGrpName;

   /* Get the Parent Node */
   parentPtr = smiGetParentNode(smiNode); 

   if (parentPtr == NULL)
   {
      fprintf(stderr, "WARNING: Does not have a parent\n"); 
      return; 
   }

   cName = translateUpper(smiNode->name);
   cGrpName = esmiTranslateHungarian(parentPtr->name);

   /* Following lines dumps the code that does SNMP SET operation */
   fprintf(fd, "{\n"); 
   fprintf(fd, "   NCSMIB_ARG  io_mib_arg;\n");
   fprintf(fd, "   NCSMEM_AID  ma;"
               "\n   uns8        space[1024];\n");
   fprintf(fd, "   uns32       i_inst[MAX_OID_LEN] = {0};\n");
   fprintf(fd, "   uns32       i_inst_len = 0;\n");
   fprintf(fd, "   uns32       status = NCSCC_RC_FAILURE;\n\n"); 
   fprintf(fd, "   m_NCS_OS_MEMSET(&io_mib_arg, 0, sizeof(NCSMIB_ARG));"
               "\n   m_NCS_OS_MEMSET(space, 0, sizeof(space));"
               "\n   ncsmem_aid_init(&ma, space, 1024);\n\n");
   
   if (smiNode->nodekind != SMI_NODEKIND_SCALAR)
   {
      fprintf(fd, "   status = snmpsubagt_mab_index_extract(name, name_len,\n");
      fprintf(fd, "                                     __%s_base,\n", cGrpName);
      fprintf(fd, "                                     OID_LENGTH(__%s_base),\n", cGrpName);
      fprintf(fd, "                                     i_inst, &i_inst_len);\n");
      fprintf(fd, "   if (status != NCSCC_RC_SUCCESS)\n   {\n");
      fprintf(fd, "       printf(\"Extract index FAILED.... \\n\");\n"); 
      fprintf(fd, "       return SNMP_ERR_GENERR;\n   }\n\n");
   }
   
   fprintf(fd, "   /* Send the SET request */\n"); 
   fprintf(fd, "   switch(action)\n   {\n"); 
   fprintf(fd, "   case RESERVE1:\n"); 
   fprintf(fd, "       /* send the TEST request */\n"); 
   fprintf(fd, "       status = snmpsubagt_mab_mac_msg_send(&io_mib_arg,%s,\n", 
                                 parentPtr->esmi.info.tableInfo.tableId);
   fprintf(fd, "                                      i_inst, i_inst_len,\n"
               "                                      %s,\n", cName);
   fprintf(fd, "                                      var_val_type,\n"
               "                                      NCSMIB_OP_REQ_TEST,\n");
   fprintf(fd, "                                      (void*)var_val, var_val_len,\n");
   fprintf(fd, "                                      NCS_SNMPSUBAGT_MAC_TIMEOUT, &ma, space, 1024,"
               "\n                                    __%s_variables, sizeof(__%s_variables)/sizeof(struct variable),", cGrpName, cGrpName);
   if (! parentPtr->esmi.info.tableInfo.isSparseTbl)
      fprintf(fd, "\n                                             FALSE);");
   else
      fprintf(fd, "\n                                             TRUE);");
   fprintf(fd, "       if (status != NCSCC_RC_SUCCESS)\n       {\n");
   fprintf(fd, "          printf(\"TEST request to MAC failed ....\");\n");
   fprintf(fd, "          return SNMP_ERR_GENERR;\n       }\n\n");
   
   fprintf(fd, "       /* process the status of the MIB request */\n");
   fprintf(fd, "       if (io_mib_arg.rsp.i_status != NCSCC_RC_SUCCESS)\n");
   fprintf(fd, "          return SNMP_ERR_GENERR;\n\n");
   fprintf(fd, "       break;\n\n");

   fprintf(fd, "   case RESERVE2:\n");
   fprintf(fd, "       /* allocate memory, if any.  We do not support this */\n");
   fprintf(fd, "       break;\n\n");
   
   fprintf(fd, "   case ACTION:\n");
   fprintf(fd, "       /* Send the SET request */\n");
   fprintf(fd, "       status = snmpsubagt_mab_mac_msg_send(&io_mib_arg,%s,\n",
                parentPtr->esmi.info.tableInfo.tableId);
   fprintf(fd, "                                            i_inst, i_inst_len,\n"
               "                                            %s,\n", cName);
   fprintf(fd, "                                            var_val_type,\n"
               "                                            NCSMIB_OP_REQ_SET,\n");
   fprintf(fd, "                                            (void*)var_val, var_val_len,\n");
   fprintf(fd, "                                            NCS_SNMPSUBAGT_MAC_TIMEOUT, &ma, space, 1024,"
               "\n                                            __%s_variables, sizeof(__%s_variables)/sizeof(struct variable),", cGrpName, cGrpName);
   if (! parentPtr->esmi.info.tableInfo.isSparseTbl)
      fprintf(fd, "\n                                             FALSE);");
   else
      fprintf(fd, "\n                                             TRUE);");

   fprintf(fd, "       if (status != NCSCC_RC_SUCCESS)\n       {\n");
   fprintf(fd, "          printf(\"SET request to MAC failed ....\");\n");
   fprintf(fd, "          return SNMP_ERR_GENERR;\n       }\n\n");
   fprintf(fd, "       if (io_mib_arg.rsp.i_status != NCSCC_RC_SUCCESS)\n");
   fprintf(fd, "          return SNMP_ERR_GENERR;\n");
   fprintf(fd, "       break;\n\n");
   
   fprintf(fd, "    case UNDO:\n"
               "        /* We do not support this */\n"
               "        break;\n\n");
   fprintf(fd, "    case FREE:\n"
               "       /* we do not support this */\n"
               "       break;\n\n");
   fprintf(fd, "    default:\n"
               "       break;\n\n    }\n\n");
   fprintf(fd, "    return SNMP_ERR_NOERROR;\n}\n\n");
   
   xfree(cName);
   xfree(cGrpName);

   return; 
}

#endif /* NCS_NETSNMP */


static void printAgtReadMethod(FILE *f, SmiNode *grpNode)
{
    SmiNode   *smiNode;
    SmiType   *smiType;
    char      *cName, *sName, *lName;

    sName = translate(grpNode->name);

    fprintf(f,
       "static unsigned char *\nread_%s_stub(struct variable *vp,\n"
       "    oid     *name,\n"
       "    size_t  *length,\n"
       "    int     exact,\n"
       "    size_t  *var_len,\n"
       "    WriteMethod **write_method)\n"
       "{\n", sName);

    fprintf(f, "    static %s_t %s;\n\n", sName, sName);
    
    smiNode = smiGetFirstChildNode(grpNode);
    if (smiNode && smiNode->nodekind == SMI_NODEKIND_SCALAR) {
   fprintf(f,
      "    /* check whether the instance identifier is valid */\n"
      "\n"
      "    if (header_generic(vp, name, length, exact, var_len,\n"
      "                       write_method) == MATCH_FAILED) {\n"
      "        return NULL;\n"
      "    }\n"
      "\n");
    }

    fprintf(f,
       "    /* call the user supplied function to retrieve values */\n"
       "\n"
       "    read_%s(&%s);\n"
       "\n", sName, sName);

    fprintf(f,
       "    /* return the current value of the variable */\n"
       "\n"
       "    switch (vp->magic) {\n"
       "\n");

    for (smiNode = smiGetFirstChildNode(grpNode);
    smiNode;
    smiNode = smiGetNextChildNode(smiNode)) {
   if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
       && (smiNode->access == SMI_ACCESS_READ_ONLY
      || smiNode->access == SMI_ACCESS_READ_WRITE)) {
       cName = translateUpper(smiNode->name);
       lName = translate(smiNode->name);
       smiType = smiGetNodeType(smiNode);
       if (! smiType) {
      continue;
       }
       fprintf(f, "    case %s:\n", cName);
       switch (smiType->basetype) {
       case SMI_BASETYPE_OBJECTIDENTIFIER:
      fprintf(f,
         "        *var_len = %s._%sLength;\n"
         "        return (unsigned char *) %s.%s;\n",
         sName, lName, sName, lName);
      break;
       case SMI_BASETYPE_OCTETSTRING:
       case SMI_BASETYPE_BITS:
      fprintf(f,
         "        *var_len = %s._%sLength;\n"
         "        return (unsigned char *) %s.%s;\n",
         sName, lName, sName, lName);
      break;
       case SMI_BASETYPE_ENUM:
       case SMI_BASETYPE_INTEGER32:
       case SMI_BASETYPE_UNSIGNED32:
      fprintf(f,
         "        return (unsigned char *) &%s.%s;\n",
         sName, lName);
      break;
       default:
      fprintf(f,
         "        /* add code to return the value here */\n");
       }
       fprintf(f, "\n");
       xfree(cName);
       xfree(lName);
   }
    }

    fprintf(f,
       "    default:\n"
       "         ERROR_MSG(\"\");\n"
       "    }\n"
       "\n"
       "    return NULL;\n"
       "}\n"
       "\n");

    xfree(sName);
}



static void printAgtReadMethods(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
    smiNode;
    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
   if (isGroup(smiNode) && isAccessible(smiNode)) {
       cnt++;
       if (cnt == 1) {
      fprintf(f,
         "/*\n"
         " * Read methods for groups of scalars and tables:\n"
         " */\n\n");
       }
#if (NCS_NETSNMP != 0)
            if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT) 
               printNcsAgtReadMethod(f, smiNode); 
            else
#endif
               printAgtReadMethod(f, smiNode);
   }
    }
    
    if (cnt) {
   fprintf(f, "\n");
    }
}


static void printAgtWriteMethods(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;
    char      *cName;
   
 
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
    smiNode;
    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
    {
   if (smiNode->access == SMI_ACCESS_READ_WRITE) 
        {
#if (NCS_NETSNMP != 0)
           if (smiNode->status == SMI_STATUS_OBSOLETE)
               continue;
#endif
       cnt++;
       if (cnt == 1) {
      fprintf(f,
         "/*\n"
         " * Forward declaration of write methods for writable objects:\n"
         " */\n\n");
       }

            cName = esmiTranslateHungarian(smiNode->name);
       fprintf(f,
          "static int\nwrite_%s_stub(int action,\n"
          "               u_char   *var_val,\n"
          "               u_char   var_val_type,\n"
          "               size_t   var_val_len,\n"
          "               u_char   *statP,\n"
          "               oid      *name,\n"
          "               size_t   name_len)\n", cName);
            xfree(cName);

#if (NCS_NETSNMP != 0)
      if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT)
      {
                   printNcsAgtWriteMethod(f, smiNode); 
      }
      else
#endif /* NCS_NETSNMP */
      {
                    fprintf(f, "{\n    return SNMP_ERR_NOERROR;\n"
                               "}\n\n");
      }
   }
    } /* End of for loop */
    
    if (cnt) {
   fprintf(f, "\n");
    }
}


static void dumpAgtStub(SmiModule *smiModule, char *baseName)
{
    char   *stubModuleName;
    FILE   *f;

    stubModuleName = xmalloc(strlen(baseName) + 10);
    strcpy(stubModuleName, baseName);
    
   f = createFile(stubModuleName, ".c");
    if (! f) {
   xfree(stubModuleName);
        return;
    }

    fprintf(f,
       "/*\n"
       " * This C file has been generated by smidump "
       SMI_VERSION_STRING ".\n"
       " * It is intended to be used with the NET-SNMP agent library.\n"
       " *\n"
       " * This C file is derived from the %s module.\n"
       " *\n * $I" "d$\n"
       " */\n\n", smiModule->name );
    fprintf(f,
       "#include <stdio.h>\n"
       "#include <string.h>\n"
       "#include <malloc.h>\n"
       "\n"
       "#include \"%s.h\"\n"
       "\n"
       "#include <ucd-snmp/asn1.h>\n"
       "#include <ucd-snmp/snmp.h>\n"
       "#include <ucd-snmp/snmp_api.h>\n"
       "#include <ucd-snmp/snmp_impl.h>\n"
       "#include <ucd-snmp/snmp_vars.h>\n"
       "\n",
       baseName);
   
    
   printAgtReadMethodDecls(f, smiModule);
    printAgtWriteMethodDecls(f, smiModule);
    printAgtDefines(f, smiModule);
    printAgtInit(f, smiModule);
    printAgtReadMethods(f, smiModule);
    printAgtWriteMethods(f, smiModule);

    fclose(f);
    xfree(stubModuleName);
}

static void printMgrOidDefinitions(FILE *f, SmiModule *smiModule)
{
    SmiNode      *smiNode;
    char         *cName;
    unsigned int i;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
    smiNode;
    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
   if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
       && smiNode->access != SMI_ACCESS_NOTIFY) {
       cName = translate(smiNode->name);
         fprintf(f, "static oid %s[] = {", cName);
       for (i = 0; i < smiNode->oidlen; i++) {
      fprintf(f, "%s%u", i ? ", " : "", smiNode->oid[i]);
       }
       fprintf(f, "};\n");
       xfree(cName);
   }
    }
    fprintf(f, "\n");
}



static void printMgrGetScalarAssignement(FILE *f, SmiNode *grpNode)
{
    SmiNode *smiNode;
    SmiType *smiType;
    char    *cGroupName, *cName;
    unsigned maxSize, minSize;

    cGroupName = translate(grpNode->name);

    for (smiNode = smiGetFirstChildNode(grpNode);
    smiNode;
    smiNode = smiGetNextChildNode(smiNode)) {
   if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
       && (smiNode->access == SMI_ACCESS_READ_ONLY
      || smiNode->access == SMI_ACCESS_READ_WRITE)) {

       smiType = smiGetNodeType(smiNode);
       if (!smiType) {
      continue;
       }
       
       cName = translate(smiNode->name);
       fprintf(f,
          "        if (vars->name_length > sizeof(%s)/sizeof(oid)\n"
          "            && memcmp(vars->name, %s, sizeof(%s)) == 0) {\n",
          cName, cName, cName);
       switch (smiType->basetype) {
       case SMI_BASETYPE_INTEGER32:
       case SMI_BASETYPE_UNSIGNED32:
       case SMI_BASETYPE_ENUM:
      fprintf(f,
         "            (*%s)->__%s = *vars->val.integer;\n"
         "            (*%s)->%s = &((*%s)->__%s);\n",
         cGroupName, cName,
         cGroupName, cName, cGroupName, cName);
      break;
       case SMI_BASETYPE_OCTETSTRING:
       case SMI_BASETYPE_BITS:
      maxSize = getMaxSize(smiType);
      minSize = getMinSize(smiType);
      fprintf(f,
         "            memcpy((*%s)->__%s, vars->val.string, vars->val_len);\n",
         cGroupName, cName);
      if (minSize != maxSize) {
          fprintf(f,
             "            (*%s)->_%sLength = vars->val_len;\n",
             cGroupName, cName);
      }
      fprintf(f,
         "            (*%s)->%s = (*%s)->__%s;\n",
         cGroupName, cName, cGroupName, cName);
      break;
       case SMI_BASETYPE_OBJECTIDENTIFIER:
      break;
       default:
      break;
       }
       fprintf(f,
          "        }\n");
       xfree(cName);
   }
    }

    xfree(cGroupName);
}



static void printMgrGetMethod(FILE *f, SmiModule *smiModule,
               SmiNode *grpNode)
{
    SmiNode *smiNode;
    char    *cModuleName, *cGroupName;

    cModuleName = translateLower(smiModule->name);
    cGroupName = translate(grpNode->name);

    fprintf(f,
       "int %s_mgr_get_%s(struct snmp_session *s, %s_t **%s)\n"
       "{\n"
       "    struct snmp_session *peer;\n"
       "    struct snmp_pdu *request, *response;\n"
       "    struct variable_list *vars;\n"
       "    int status;\n"
       "\n",
       cModuleName, cGroupName, cGroupName, cGroupName);

    fprintf(f,
       "    request = snmp_pdu_create(SNMP_MSG_GETNEXT);\n");
       
    for (smiNode = smiGetFirstChildNode(grpNode);
    smiNode;
    smiNode = smiGetNextChildNode(smiNode)) {
   if (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)
       && (smiNode->access == SMI_ACCESS_READ_ONLY
      || smiNode->access == SMI_ACCESS_READ_WRITE)) {
       fprintf(f,
       "    snmp_add_null_var(request, %s, sizeof(%s)/sizeof(oid));\n",
          smiNode->name, smiNode->name);
   }
    }

    fprintf(f,
       "\n"
       "    peer = snmp_open(s);\n"
       "    if (!peer) {\n"
       "        return -1;\n"
       "    }\n"
       "\n"
       "    status = snmp_synch_response(peer, request, &response);\n"
       "    if (status != STAT_SUCCESS) {\n"
       "        return -2;\n"
       "    }\n"
       "\n");

    /* generate code for error checking and handling */

    fprintf(f,
       "    *%s = (%s_t *) malloc(sizeof(%s_t));\n"
       "    if (! *%s) {\n"
       "        return -4;\n"
       "    }\n"
       "\n",
       cGroupName, cGroupName, cGroupName, cGroupName);

    fprintf(f,
       "    for (vars = response->variables; vars; vars = vars->next_variable) {\n");
    printMgrGetScalarAssignement(f, grpNode);
    fprintf(f,
       "    }\n"
       "\n");


#if 0
    if (response->errstat != SNMP_ERR_NOERROR) {
   return -3;
    }

    /* copy to data structures */

    /* cleanup */

#endif

    fprintf(f,
       "    if (response) snmp_free_pdu(response);\n"
       "\n"
       "    if (snmp_close(peer) == 0) {\n"
       "        return -5;\n"
       "    }\n"
       "\n"
       "    return 0;\n"
       "}\n\n");

    xfree(cGroupName);
    xfree(cModuleName);
}
 



static void printMgrGetMethods(FILE *f, SmiModule *smiModule)
{
    SmiNode   *smiNode;
    int       cnt = 0;
    
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
    smiNode;
    smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) {
   if (isGroup(smiNode) && isAccessible(smiNode)) {
       cnt++;
       printMgrGetMethod(f, smiModule, smiNode);
   }
    }
    
    if (cnt) {
   fprintf(f, "\n");
    }
}



static void dumpMgrStub(SmiModule *smiModule, char *baseName)
{
    char   *stubModuleName;
    FILE   *f;

    stubModuleName = xmalloc(strlen(baseName) + 10);
    strcpy(stubModuleName, baseName);
    strcat(stubModuleName, "-mgr-stub");
    
    f = createFile(stubModuleName, ".c");
    if (! f) {
   xfree(stubModuleName);
        return;
    }

    fprintf(f,
       "/*\n"
       " * This C file has been generated by smidump "
       SMI_VERSION_STRING ".\n"
       " * It is intended to be used with the NET-SNMP library.\n"
       " *\n"
       " * This C file is derived from the %s module.\n"
       " *\n * $I" "d$\n"
       " */\n\n", smiModule->name );
   
    fprintf(f,
       "#include <stdlib.h>\n"
       "\n"
       "#include <ucd-snmp/asn1.h>\n"
       "#include <ucd-snmp/snmp.h>\n"
       "#include <ucd-snmp/snmp_api.h>\n"
       "#include <ucd-snmp/snmp_client.h>\n"
       "\n"
       "#include \"%s.h\"\n"
       "\n",
       baseName);

    printMgrOidDefinitions(f, smiModule);
    
    printMgrGetMethods(f, smiModule);
    
    if (fflush(f) || ferror(f)) {
   perror("smidump: write error");
   exit(1);
    }

    fclose(f);
    xfree(stubModuleName);
}



static void dumpAgtImpl(SmiModule *smiModule, char *baseName)
{
    char   *stubModuleName, *cModuleName;
    FILE   *f;

    stubModuleName = xmalloc(strlen(baseName) + 10);
    strcpy(stubModuleName, baseName);
    strcat(stubModuleName, "-agt");
    

    f = createFile(stubModuleName, ".c");
    if (! f) {
   xfree(stubModuleName);
        return;
    }

    cModuleName = translateLower(smiModule->name);

    fprintf(f,
       "/*\n"
       " * This C file has been generated by smidump "
       SMI_VERSION_STRING ".\n"
       " * It is intended to be used with the NET-SNMP agent library.\n"
       " *\n"
       " * This C file is derived from the %s module.\n"
       " *\n * $I" "d$\n"
       " */\n\n", smiModule->name );
   
    fprintf(f,
       "#include <stdio.h>\n"
       "#include <string.h>\n"
       "#include <malloc.h>\n"
       "\n"
       "#include \"%s.h\"\n"
       "\n"
       "#include <ucd-snmp/asn1.h>\n"
       "#include <ucd-snmp/snmp.h>\n"
       "#include <ucd-snmp/snmp_api.h>\n"
       "#include <ucd-snmp/snmp_impl.h>\n"
       "#include <ucd-snmp/snmp_vars.h>\n"
       "\n",
       baseName);

    fprintf(f,
       "static oid %s_caps[] = {0,0};\n"
       "\n",
       cModuleName);

    fprintf(f,
       "void init_%s(void)\n"
       "{\n"
#if 0
       /* create an entry in the sysORTable */
       
       register_sysORTable(if_mib_caps, sizeof(if_mib_caps),
            "IF-MIB implementation version 0.0.");
       
       /* register the various parts of the MIB */
       
       register_interfaces();
       register_ifEntry();
       
       /* register essential callbacks */
       
       snmp_register_callback(SNMP_CALLBACK_LIBRARY,
               SNMP_CALLBACK_SHUTDOWN,
               term_if_mib, NULL);
#endif
       "}\n"
       "\n",
       cModuleName);


    fprintf(f,
       "void deinit_%s()\n"
       "{\n"
       "    unregister_sysORTable(%s_caps, sizeof(%s_caps));\n"
       "}\n"
       "\n",
       cModuleName, cModuleName, cModuleName);

    fprintf(f,
       "int term_%s()\n"
       "{\n"
       "    deinit_%s();\n"
       "    return 0;\n"
       "}\n"
       "\n",
       cModuleName, cModuleName);

    xfree(cModuleName);
    
    if (fflush(f) || ferror(f)) {
   perror("smidump: write error");
   exit(1);
    }

    fclose(f);
    xfree(stubModuleName);
}



static void dumpNetSnmp(int modc, SmiModule **modv, int flags, char *output)
{
    char   *baseName;
    int      i;

    if (flags & SMIDUMP_FLAG_UNITE) {
   /* not implemented yet */
    } else {
   for (i = 0; i < modc; i++) {
       baseName = output ? output : translateFileName(modv[i]->name);
       dumpHeader(modv[i], baseName);
       if (! noAgtStubs) {
      dumpAgtStub(modv[i], baseName);
      dumpAgtImpl(modv[i], baseName);
       }
       if (! noMgrStubs) {
      dumpMgrStub(modv[i], baseName);
       }
       if (! output) xfree(baseName);
   }
    }

}



void initNetsnmp()
{
    static SmidumpDriverOption opt[] = {
   { "no-mgr-stubs", OPT_FLAG, &noMgrStubs, 0,
     "do not generate manager stub code"},
   { "no-agt-stubs", OPT_FLAG, &noAgtStubs, 0,
     "do not generate agent stub code"},
        { 0, OPT_END, 0, 0 }
    };

    static SmidumpDriver driver = {
   "netsnmp",
   dumpNetSnmp,
   SMI_FLAG_NODESCR,
   SMIDUMP_DRIVER_CANT_UNITE,
   "ANSI C code for the NET-SNMP package",
   opt,
   NULL
    };

    smidumpRegisterDriver(&driver);
}

/* NCS - Start Change 
 * Added the following init function to initliaze the 
 * driver routine for integration with MAB
 */

#if (NCS_NETSNMP != 0)
/**************************************************************************
Name          :  dumpNcsHeader
                                                                                
Description   : This function generates the header(*.h) file of a Module(MIB). 
                The generated header file will have the complete information of
                of objects/tables that are defined in a MIB (Module) file.
                                                                                
Arguments     : smiModule - Pointer to the SmiModule structure.
                baseName - Pointer to the string that represents for the name
                        of the header file. User defined name of derived 
                         from the MIB file.
                                                                                
Return values : ESMI_SNMP_SUCCESS / ESMI_SNMP_FAILURE
                                                                                
Notes         :
**************************************************************************/
static int dumpNcsHeader(SmiModule *smiModule)
{
   char  *pModuleName, *hModuleName;
   FILE  *fd;
   char  *baseName = translateLower(smiModule->name);

   pModuleName = translateUpper(smiModule->name);

   /* Create an header file */
   fd = esmiCreateFile(baseName, ESMI_MODULE_H_FILE, subAgtDirPath);
   if (!fd) 
   {
      /* Log the message : Not able to create stub file */
      xfree(pModuleName);
      xfree(baseName);
      return ESMI_SNMP_FAILURE;
   }

   /* Print the Motorola's copy right information */
   esmiPrintMotoCopyRight(fd, baseName, ESMI_MODULE_H_FILE);

   /* Free the allotted memory for baseName */
   xfree(baseName);
    
   fprintf(fd, "#ifndef _%s_H_\n", pModuleName);
   fprintf(fd, "#define _%s_H_\n\n\n", pModuleName);

   fprintf(fd,
           "/*\n"
      " * This C header file has been generated by smidump "
      SMI_VERSION_STRING ".\n"
      " * It is intended to be used with the NET-SNMP package \n"
      " * for integration with NCS MASv.\n"
      " *\n"
      " * This header is derived from the %s module.\n"
      " *\n * $I" "d$\n"
      " */\n\n", smiModule->name);

   fprintf(fd, "#include <stdlib.h>\n\n");

   fprintf(fd,
           "#ifdef HAVE_STDINT_H\n"
      "#include <stdint.h>\n"
      "#endif\n\n");

   fprintf(fd, "#include \"subagt_mab.h\"\n"
               "#include \"subagt_oid_db.h\"\n");

   if (esmiGlbHdrFile)
      fprintf(fd, "#include \"%s\"\n", esmiGlbHdrFile);

   if (esmiNetSnmpOpt == ESMI_NEW_SYN_NETSNMP_OPT)
   {
      fprintf(fd, "\n#include <net-snmp/net-snmp-includes.h>"
                  "\n#include <net-snmp/agent/net-snmp-agent-includes.h>"
                  "\n#include <net-snmp/agent/table.h>"
                  "\n#include <net-snmp/agent/instance.h>"
                  "\n#include <net-snmp/agent/table_data.h>"
                  "\n#include <net-snmp/agent/table_dataset.h>");
   }

   fprintf(fd,
            "\n\n\n/*"
            "\n * Initialization/Termination functions:\n"
            " */");

   hModuleName = translateLower(smiModule->name);

   fprintf(fd, "\n\n/* Common Registration routine */\n");
   fprintf(fd, "extern uns32 __register_%s_module(void);", hModuleName);
   fprintf(fd, "\n\n/* Common Unregistration routine */\n");
   fprintf(fd, "extern uns32 __unregister_%s_module(void);", hModuleName);

   fprintf(fd, "\n\n/* This variable is imported from subAgent module */\n");
   fprintf(fd, "extern NCSMIB_PARAM_VAL rsp_param_val;\n\n");

   xfree(hModuleName);

   fprintf(fd, "#endif /* _%s_H_ */\n", pModuleName);

   fclose(fd);
   xfree(pModuleName);

   /* Print onto console about the file generated information*/
   hModuleName = esmiTranslateLower(smiModule->name, ESMI_MODULE_H_FILE);
   fprintf(stdout, "\nFile Generated:  %s\n", hModuleName);
   xfree(hModuleName);

   return ESMI_SNMP_SUCCESS;
}


static void  printNewAgtDecls(FILE *fd, SmiModule *smiModule)
{
   return;
}


/**************************************************************************
Name          : printAgtNcsSetWriteMethodDecls
                                                                                
Description   : This function prints the declarations of write methods for
                all the accessible tables defined in a MIB file.
                                                                                
Arguments     : fd - file descriptor of the file to which these routines
                     will be printed.
                smiModule - Pointer to the SmiModule structure.
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void printAgtNcsSetWriteMethodDecls(FILE *fd, SmiModule *smiModule)
{
   SmiNode *smiNode = NULL;
   int     cnt = 0;
   char    *grpName;
  
   /* Traverse through all the tables defined in a MIB and print 
      write_method_set declaration if the the table has atleast one 
      child object of read-write type */
   for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
        smiNode;
        smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY))
   {
      if (isGroup(smiNode) && isWritable(smiNode))
      {  
         cnt++;
         if (cnt == 1) 
         {
            fprintf(fd,
                     "\n\n/****************************************************************************\n"
                     "   Forward declarations of write methods for tables \n"
                     "   which has object/objects of read-write/read-create type\n"  
                     " ***************************************************************************/\n\n");
         }

         grpName = esmiTranslateHungarian(smiNode->name);
    fprintf(fd, "static void %s_write_method_set(uns32 object_id, \n"
         "                                WriteMethod **write_method);\n", 
                     grpName);

         /* Free the allotted memory for grpName */
         xfree(grpName);
      }
   } /* End of for loop */

   if (cnt) 
      fprintf(fd, "\n");

   return; 
}


/**************************************************************************
Name          :  dumpNcsAgtStub
                                                                                
Description   : This function generates the stubs that are required for
                NCS subAgent. It generates the stubs that provide synchronous
                /asynchronous interface between MASv and NCS subAgent. 
                                                                                
Arguments     : smiModule - Pointer to the SmiModule structure.
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static int dumpNcsAgtStub(SmiModule *smiModule)
{
   FILE   *fd;
   char *cModuleName, *hModuleName;
   char *baseName = translateLower(smiModule->name);
   
   /* Create a stub file, generated w.r.t Module (MIB) */ 
   fd = esmiCreateFile(baseName, ESMI_MODULE_C_FILE, subAgtDirPath);
   if (! fd)
   {
      /* Log the message: not able to create "<basename>ESMI_MODULE_C_FILE" 
         file */
      xfree(baseName);
      return ESMI_SNMP_FAILURE;
   }

   /* Print the Motorola's copy right information */
   esmiPrintMotoCopyRight(fd, baseName, ESMI_MODULE_C_FILE);

   fprintf(fd,
            "\n\n\n/*\n"
       " * This C file has been generated by smidump "
       SMI_VERSION_STRING ".\n"
       " * It is intended to be used with the NET-SNMP agent library.\n"
       " *\n"
       " * This C file is derived from the %s module.\n"
       " *\n * $I" "d$\n"
       " */\n\n", smiModule->name );

   hModuleName = esmiTranslateLower(smiModule->name, ESMI_MODULE_H_FILE);
   fprintf(fd, "#include \"%s\"\n", hModuleName); 
   if(hModuleName)  xfree(hModuleName);

   /* Free the allotted memory for baseName */
   xfree(baseName);

   if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT)
   {
      /* Print write_method_set declarations for all tables defined 
         in a MIB file */
      printAgtNcsSetWriteMethodDecls(fd, smiModule); 
   
      /* Print read_method declarations */ 
      printAgtReadMethodDecls(fd, smiModule);

      /* Print write_method declarations */ 
      printAgtWriteMethodDecls(fd, smiModule);
   }
   else  /* ESMI_NEW_SYN_NETSNMP_OPT */
   {
      printNewAgtDecls(fd, smiModule);
   }

   /* Print Agent definitions like Macros, structures etc.. */
   printAgtDefines(fd, smiModule);

   /* Print register routines for all tables defined in a 
      Module */
   printAgtInit(fd, smiModule);
   
   /* generate a common routine for registration with Agent */
   printAgtNcsModuleRegister(fd, smiModule, ESMI_REGISTER_FLAG);    
   
   /* generate a common routine for unregistration with Agent */
   printAgtNcsModuleRegister(fd, smiModule, ESMI_UNREGISTER_FLAG);    

   if (esmiNetSnmpOpt == ESMI_SYN_NETSNMP_OPT)
   {
      /* genarate a write method set for read-write/read-create objects */   
      printAgtNcsSetWriteMethod(fd, smiModule); 

      /* Print read_methods */
      printAgtReadMethods(fd, smiModule);
  
      /* Print write_methods */
      printAgtWriteMethods(fd, smiModule);
   }

   fclose(fd);
  
   /* Print onto console about the file generated information*/
   cModuleName = esmiTranslateLower(smiModule->name, ESMI_MODULE_C_FILE);
   fprintf(stdout, "\nFile Generated:  %s\n", cModuleName);
   xfree(cModuleName);

   return ESMI_SNMP_SUCCESS;
}


/**************************************************************************
Name          :  printMapiTableObjIds
                                                                                
Description   :  This function dumps the Object_IDs w.r.t to table into 
                 MAPI header file. 
                                                                                
Arguments     :  fd - File descriptor of the MAPI header file.
                 grpNode - pointer to the SmiModule structure.
                                                                                
Return values :  ESMI_SNMP_SUCCESS / ESMI_SNMP_FAILURE 
                                                                                
Notes         :
**************************************************************************/
static int  printMapiTableObjIds(FILE *fd, SmiNode *grpNode)
{
   SmiNode *smiNode = NULL;
   char    *tblNameBig = NULL;
   char    *tblNameSmall = NULL;
   int     count = 0;
   int     value = 0;

   tblNameSmall = esmiTranslateHungarian(grpNode->name);
   tblNameBig   = translateUpper(tblNameSmall);

   fprintf(fd, "\n\n/* Object ID enums for the  \"%s\"  table */", grpNode->name);
   
   fprintf(fd, "\ntypedef enum %s_%s \n{", tblNameSmall, (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_id":"id");

   /* Traverse through all of the objects in the table */
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {   
       if (esmiNetSnmpOpt == ESMI_PSSV_OPT)
       {
           /* Enum generation for PSSv need not have NOTIFICATION objects */ 
           if (! (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)))
              continue;
       }
       else
       {
           if (! (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR
                                       | SMI_NODEKIND_NOTIFICATION)))
              continue;
       }

       count++;
       value = smiNode->oid[smiNode->oidlen-1];
      
       /* for the first object of the table */
       if (count == 1)
          fprintf(fd, "\n   %s_%s = %d,", smiNode->name, 
                  (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_ID":"ID", value);     
       else /* for the remaining objects of the table */
       { 
          /* If there is a mistmatch between the count and obj_ID value
             then update the obj_ID with the value */
          if (count != value)
          {
             fprintf(fd, "\n   %s_%s = %d,", smiNode->name, 
                     (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_ID":"ID", value);     
             count = value;
          }
          else
          {
             /* count & value equal, so just updating with obj_ID is 
                enough, as enum value will be incremented for the follower */
             fprintf(fd, "\n   %s_%s = %d,", smiNode->name, 
                     (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_ID":"ID", value);     
          }
       }      
   } 

   fprintf(fd, "\n   %s%s \n} %s_%s; \n", grpNode->name, 
            (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_Max_ID":"Max_ID",
            tblNameBig, 
            (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_ID":"ID");

   xfree(tblNameSmall);
   xfree(tblNameBig);

   return;
}

static int checkColumnsMaxAccess (SmiNode *grpNode)
{
   SmiNode *smiNode;
   List *listPtr;
   Object *objPtr = (Object *)grpNode, *indexPtr;
   Node *nodePtr;
   int aux = 0, cols = 0, readonly = 0;
   
   for (nodePtr = objPtr->nodePtr->firstChildPtr; nodePtr; nodePtr = nodePtr->nextPtr) 
   {
     cols++;
   }
   /* Check if all the columns of this table are indices. If so, then check if atleast one of them is read-only. 
      Else check if all the columns are not-accessible, in which case, generate error */

   for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr)
   {
       indexPtr = (Object *)listPtr->ptr;
       for (nodePtr = objPtr->nodePtr->firstChildPtr; nodePtr; nodePtr = nodePtr->nextPtr)
          if (indexPtr == nodePtr->lastObjectPtr)
          {
             aux++;
             if (indexPtr->export.access == SMI_ACCESS_READ_ONLY)
               readonly++;
          } 
   }
  
   if ((aux == cols) && (readonly == 0))
   {
      printf("ERROR: Atleast one of the index must be read-only\n");
      return ESMI_SNMP_FAILURE;   
   }

   for(smiNode = smiGetFirstChildNode(grpNode);
       smiNode;
       smiNode = smiGetNextChildNode(smiNode))
   {
       if (smiNode->access != SMI_ACCESS_NOT_ACCESSIBLE)
          return ESMI_SNMP_SUCCESS;
   }
   printf ("ERROR: All the columns of %s are defined as not-accessible\n", grpNode->name);
   return ESMI_SNMP_FAILURE;
}

/**************************************************************************
Name          : dumpMapiHeader
                                                                                
Description   : This function dumps the Object_ID data w.r.t to table 
                into MAPI header file. 
                                                                                
Arguments     : fd - File descriptor of the MAPI header file.
                smiModule - pointer to the SmiModule structure.
                                                                                
Return Values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void dumpMapiHeader(FILE *fd, SmiModule *smiModule)
{
   SmiNode   *smiNode;
   int       count = 0;

   /* Traverse through all the smiNodes of a Module and dump the object_IDs
      object_IDs w.r.t table */
   for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
        smiNode;
        smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
   {
       /* If smiNode is of table type */
       if (esmiIsTable(smiNode)) 
       {
          if (checkColumnsMaxAccess(smiNode) == ESMI_SNMP_FAILURE)
             continue;
          count++;

          if(count == 1)
          {
             fprintf(fd, "\n\n/***********************************************************************");
             fprintf(fd, "\n           Object ID enums for  \"%s\"  Module", smiModule->name);
             fprintf(fd, "\n***********************************************************************/");
          }
          printMapiTableObjIds(fd, smiNode);
       }
   }
 
   return;
}


/**************************************************************************
Name          :  createNcsMapiFile
                                                                                
Description   :  This function create a MAPI header file 
                                                                                
Arguments     : applName - Appl. name used to frame Mapi header file name.
                                                                                
Return values : fd - File descriptor of the MAPI header file
                                                                                
Notes         :
**************************************************************************/
static FILE *createNcsMapiFile(char *applName)
{
   char *mapiName, *hdrName;
   FILE *fd;

   /* Translate the application name to lower case letters */
   mapiName = translateLower(applName);
   hdrName = translateUpper(applName);
       
   /* Create a application specific MAPI file */
   fd = esmiCreateFile(mapiName, ESMI_MAPI_H_FILE, mapiDirPath); 
   if (!fd)
   {
      /* Log the Message : Not able to create MAPI header file */
      xfree(mapiName);
      xfree(hdrName);
      return NULL;
   }
   
   /* Print the copy right information into MAPI header file */
   esmiPrintMotoCopyRight(fd, mapiName, ESMI_MAPI_H_FILE);

   fprintf(fd, "\n\n#ifndef  %s_MAPI_H", hdrName);
   fprintf(fd, "\n#define  %s_MAPI_H", hdrName);

   xfree(mapiName);
   xfree(hdrName);

   return fd;
}


/**************************************************************************
Name          : deleteNcsAgtStub
                                                                                
Description   : This function deletes all the generated stubs (*.c, *.h)
                that are specific to modules (MIBs).
                                                                                
Arguments     : mibNum - Number of MIB stubs to be deleted.
                modv - Has the module (MIBs) names, where the related 
                       stubs need to be deleted.
                                                                                
Return values : Nothing 
                                                                                
Notes         :
**************************************************************************/
static void deleteNcsAgtStub(int mibNum, SmiModule **modv)
{
   char *cModuleName, *hModuleName;
   int  mibCount;

   /* Traverse through all the input modules and delete the module specific 
      stub*/
   for (mibCount = 0; mibCount <= mibNum; mibCount++)
   {
      /* Frame the names for module stubs "<module_name>ESMI_MODULE_C_FILE" &
         "<module_name>ESMI_MODULE_H_FILE */
      cModuleName = esmiTranslateLower(modv[mibCount]->name, ESMI_MODULE_C_FILE);
      hModuleName = esmiTranslateLower(modv[mibCount]->name, ESMI_MODULE_H_FILE);

      /* Delete the generated stubs if they exists */
      ESMI_FILE_DELETE(cModuleName);
      ESMI_FILE_DELETE(hModuleName);

      /* Free the allotted the memory for cModuleName & hModuleName */
      xfree(cModuleName);
      xfree(hModuleName);
   } /* End of the for loop */

   return;
}


/**************************************************************************
Name          : printApplIncludes
                                                                                
Description   : This function prints/dumps the includes of all the modules
                that belongs to this application.
                                                                                
Arguments     : cFd - File descriptor of the appl. specific stub file.
                modc - Number of Input modules (MIBs)
                modv - array of Input module (MIB) names.
                                                                                
Return values : nothing
                                                                                
Notes         :
**************************************************************************/
static void  printApplIncludes(FILE *cFd,
                               int modc, 
                               SmiModule **modv)
{
   int  moduleNum = 0;
   char *moduleName = NULL;

   /* Print all the header files (includes) of modules that belongs to 
      this application */
   for (moduleNum = 0; moduleNum < modc; moduleNum++)
   {
      moduleName = esmiTranslateLower(modv[moduleNum]->name, ESMI_MODULE_H_FILE);
      fprintf(cFd, "\n#include  \"%s\"", moduleName);
      xfree(moduleName);
   } /* End of for loop */

   fprintf(cFd, "\n\n");

   return;
}

/**************************************************************************
Name          : printApplRegister
                                                                                
Description   : This function prints/dumps the application specific common
                register/unregister routines into application spec SNMP 
                file.
                                                                                
Arguments     : cFd - File descriptor of the appl. specific stub *.c file.
                hFd - File descriptor of the appl. specific stub *.h file.
                applInitName - name of the application 
                modc - Number of Input modules (MIBs)
                modv - array of Input module (MIB) names.
                regFlag - says whether to print register/unregister
                          routines
                                                                                
Return values : ESMI_SNMP_SUCCESS / ESMI_SNMP_FAILURE
                                                                                
Notes         :
**************************************************************************/
static int  printApplRegister(FILE *cFd,
                              FILE *hFd,
                              char *applInitName,
                              int modc, 
                              SmiModule **modv, 
                              int regFlag)
{
   int  moduleNum = 0;
   char *moduleName = NULL;

   switch (regFlag)
   {
   case ESMI_REGISTER_FLAG:
       fprintf(hFd, "\n/* Application specific common REGISTER routine */");
       fprintf(hFd, "\n\nextern uns32 subagt_register_%s_subsys(void);\n", applInitName);
      fprintf(cFd, "\n\n/**************************************************************************"
               "\nName          :  subagt_register_%s_subsys"
               "\n\nDescription   :  This function is a common registration routine (application"
               "\n                 specific) where it calls all the Module (MIBs defined for"
               "\n                 an application) registration routines"
               "\n\nArguments     :  - NIL -"
               "\n\nReturn values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE"
               "\n\nNotes         :  Basically it registers all the modules (for all the TABLEs"
               "\n                 defined the MIB) with SNMP agent"   
               "\n**************************************************************************/", 
                applInitName);

       fprintf(cFd, "\nuns32 subagt_register_%s_subsys()\n{\n", applInitName);
       fprintf(cFd, "   uns32 status = NCSCC_RC_FAILURE;\n");

       for (moduleNum = 0; moduleNum < modc; moduleNum++)
       {
          moduleName = translateLower(modv[moduleNum]->name);
          fprintf(cFd, "\n   /* Register '%s' module (MIB) with SNMP Agent */", moduleName);
          fprintf(cFd, "\n   status = ncs_snmpsubagt_init_deinit_msg_post(\"__register_%s_module\");", moduleName);
          fprintf(cFd, "\n   if (status != NCSCC_RC_SUCCESS)\n       return status;\n");
          xfree(moduleName);
       } /* End of for loop */

       fprintf(cFd, "\n   return NCSCC_RC_SUCCESS;\n}\n");

       break;

   case ESMI_UNREGISTER_FLAG:
       fprintf(hFd, "\n/* Application specific common UNREGISTER routine */");
       fprintf(hFd, "\n\nextern uns32 subagt_unregister_%s_subsys(void);\n", applInitName);
       fprintf(cFd, "\n\n/**************************************************************************"
               "\nName          :  subagt_unregister_%s_subsys"
               "\n\nDescription   :  This function is a common unregistration routine (application"
               "\n                 specific) where it calls all the Module (MIBs defined for"
               "\n                 an application) unregistration routines"
               "\n\nArguments     :  - NIL -"
               "\n\nReturn values :  Nothing"
               "\n\nNotes         :  Basically it de-registers all the modules (for all the TABLEs"
               "\n                 defined the MIB) from the SNMP agent"   
               "\n**************************************************************************/", 
                applInitName);
       fprintf(cFd, "\nuns32 subagt_unregister_%s_subsys()\n{\n", applInitName);
       fprintf(cFd, "   uns32 status = NCSCC_RC_FAILURE;\n");

       for (moduleNum = 0; moduleNum < modc; moduleNum++)
       {
          moduleName = translateLower(modv[moduleNum]->name);
          fprintf(cFd, "\n   /* De-register '%s' module (MIB) from SNMP Agent */", moduleName);
          fprintf(cFd, "\n   status = ncs_snmpsubagt_init_deinit_msg_post(\"__unregister_%s_module\");", moduleName);
          fprintf(cFd, "\n   if (status != NCSCC_RC_SUCCESS)\n       return status;\n");
          xfree(moduleName);
       } /* End of for loop */

       fprintf(cFd, "\n   return NCSCC_RC_SUCCESS;\n}\n");

       break;

   default:
       return ESMI_SNMP_FAILURE;
   }
   
   return ESMI_SNMP_SUCCESS;
}

/**************************************************************************
Name          : dumpNcsApplSubAgtInit
                                                                                
Description   : This function creates a stub which is an application specific
                that has a common register & unregister routines used to
                register & deregister the NCS subAgent with SNMP agent.
                Basically these routines registers/unregisters all the MIBs
                which are specific to an application with SNMP agent (in
                the perspective of NCS subAgent). 
                                                                                
Arguments     : applName - Ptr to the string, stands for an application name.
                modc - Number of input arguments.
                modv - Array of input arguments
            
Return values : ESMI_SNMP_SUCCESS / ESMI_SNMP_FAILURE
                                                                                
Notes         :  
**************************************************************************/
static int dumpNcsApplSubAgtInit(char *applName, int modc, SmiModule **modv)
{
   int  status = ESMI_SNMP_SUCCESS;
   FILE *cFd, *hFd;
   char *tempApplName, *tempHdrApplName, *tempApplNameBig;


   /* Have the file name, to delete the file if something goes wrong */ 
   tempApplName = esmiTranslateLower(applName, ESMI_APPL_NAME_C_FILE);

   /* Create a application specific subAgent init stub file */
   cFd = esmiCreateFile(applName, ESMI_APPL_NAME_C_FILE, subAgtDirPath); 
   if (! cFd)
   {
      /* Log the Msg : Not able to create appl. specific subAgent init file */
      return ESMI_SNMP_FAILURE; 
   }

   /* Create a application specific subAgent init stub file */
   hFd = esmiCreateFile(applName, ESMI_APPL_NAME_H_FILE, subAgtDirPath); 
   if (! hFd)
   {
      /* Log the Msg : Not able to create appl. specific subAgent init file */

      /* Close the "<applName>ESMI_APPL_NAME_C_FILE" file */
      fclose(cFd);
      /* If some thing goes wrong, delete the generated stub file */
      ESMI_FILE_DELETE(tempApplName);     
      xfree(tempApplName);

      return ESMI_SNMP_FAILURE;
   }

   /* Print the copy right information into Application Init file */
   esmiPrintMotoCopyRight(cFd, applName, ESMI_APPL_NAME_C_FILE);

   tempHdrApplName = esmiTranslateLower(applName, ESMI_APPL_NAME_H_FILE);

   /* Print the include header file directive into '.c' file */
   fprintf(cFd, "\n\n#include \"%s\" \n", tempHdrApplName);
   xfree(tempHdrApplName);

   /* Print the copy right information into Application Init file */
   esmiPrintMotoCopyRight(hFd, applName, ESMI_APPL_NAME_H_FILE);

   tempApplNameBig = translateUpper(applName);

   fprintf(hFd, "\n#ifndef _%s_SUBAGT_INIT_H_\n", tempApplNameBig);
   fprintf(hFd, "#define _%s_SUBAGT_INIT_H_\n", tempApplNameBig);

   fprintf(hFd, "\n\n#include \"subagt_mab.h\"");
   fprintf(hFd, "\n#include \"subagt_oid_db.h\"\n\n");

   /* Prints all the required includes (header files of all the modules of this 
      application) */
   printApplIncludes(cFd, modc, modv);

   /* Print the application specific common register routine */
   status = printApplRegister(cFd, hFd, applName, modc, modv, ESMI_REGISTER_FLAG);

   if (status == ESMI_SNMP_SUCCESS)
   {
      /* Print the application specific common unregister routine */
      status = printApplRegister(cFd, hFd, applName, modc, modv, ESMI_UNREGISTER_FLAG);
   }

   fprintf(hFd, "\n#endif /* _%s_SUBAGT_INIT_H_ */ \n", tempApplNameBig);
   
   /* Free the allotted memory */
   xfree(tempApplNameBig);

   fprintf(cFd, "  \n\n");
   fprintf(hFd, "  \n\n");

   /* Close the application specific subAgent init stub files */
   fclose(cFd);
   fclose(hFd);

   if (status != ESMI_SNMP_SUCCESS)
   {
      /* If some thing goes wrong, delete the generated stub file (*.c) */
      ESMI_FILE_DELETE(tempApplName);     
 
      memset(tempApplName, '\0', sizeof(tempApplName));

      /* Have the file name, to delete the file if something goes wrong */ 
      strcpy(tempApplName, applName);
      strcat(tempApplName, ESMI_APPL_NAME_H_FILE);

      /* If some thing goes wrong, delete the generated stub file (*.h)*/
      ESMI_FILE_DELETE(tempApplName);     
   }
   else
   {
      fprintf(stdout, "\nFile Generated:  %s\n", tempApplName); 

      memset(tempApplName, '\0', sizeof(tempApplName));

      /* Have the file name, to delete the file if something goes wrong */ 
      strcpy(tempApplName, applName);
      strcat(tempApplName, ESMI_APPL_NAME_H_FILE);

      fprintf(stdout, "\nFile Generated:  %s\n", tempApplName); 
   }

   xfree(tempApplName);

   return status;
}


/**************************************************************************
Name          : esmiCheckModuleTables

Description   : This function validates all the tables data defined in a 
                module. 

Arguments     : smiModule - Ptr to the SmiModule structure.

Return Values : ESMI_SNMP_SUCCESS / ESMI_SNMP_FAILURE

Notes         :
**************************************************************************/
static int esmiCheckModuleTables(SmiModule *smiModule)
{
   SmiNode *smiNode;

   for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
        smiNode;
        smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY))
   {
       if (isGroup(smiNode)) {
           if (! esmiCheckTblObject(smiNode))
              return ESMI_SNMP_FAILURE;
       }
   }

   return ESMI_SNMP_SUCCESS;
}


/**************************************************************************
Name          : printNcsMapi
                                                                                
Description   : For one execution of SMIDUMP command one MAPI header file
                will be generated that contains all the object information
                (enum vals for objects with respect to table) of all the
                input Modules (MIBs). 
                                                                                
Arguements    : modc - Number of arguments given for SMIDUMP command
                modv - Pointer to the SmiModule, contains MIB file name
                     information that was given in an SMIDUMP command. 
                flags - Defined in Driver definition
                applName -  Application name              
            
Return values : Nothing
                                                                                
Notes         :  
**************************************************************************/
static void printNcsMapi(int modc, SmiModule **modv, int flags, char *applName)
{
   char  *tempApplName; 
   int   mibNum;
   FILE  *mapiFd;

   /* Check whether MAPI directory path is defined */
   if (! mapiDirPath)
   {
      fprintf(stderr, "\n ERROR: MAPI directory path is not defined in your application"
                      "\n        ESMI MIB file. Please define the ESMI tag for MAPI dir"
                      "\n        path. One ESMI tag definition for an application is enough.\n\n");
      return;
   }

   /* Create a MAPI header file */
   mapiFd = createNcsMapiFile(applName);
   if (!mapiFd)
   {
      /* Log the message : Not able to create a MAPI file */
      return;
   }
   
   /* to be able to include in C++code */   
   fprintf(mapiFd, "\n\n#ifdef  __cplusplus"); 
   fprintf(mapiFd, "\n\textern \"C\" {"); 
   fprintf(mapiFd,"\n#endif"); 

   /* Traverse through all of the Modules */
   for (mibNum = 0; mibNum < modc; mibNum++) 
   {
#if 0
       /* Commenting this ...*/
      if (! esmiCheckModuleTables(modv[mibNum]))
      { 
         /* Module has some invalid TABLE data */ 
         esmiStatus = ESMI_SNMP_FAILURE;
         break;
      }
#endif
      
      /* Dump the MAPI header file */
      dumpMapiHeader(mapiFd, modv[mibNum]);
   }

   /* to be able to include in C++code */   
   fprintf(mapiFd, "\n#ifdef  __cplusplus"); 
   fprintf(mapiFd, "\n}"); 
   fprintf(mapiFd, "\n#endif");
   
   /* close the MAPI file */
   fprintf(mapiFd, "\n\n\n#endif \n\n");

   /* Close the mapi header file */
   fclose(mapiFd);

   /* Frame the mapi file name */
   tempApplName = esmiTranslateLower(applName, ESMI_MAPI_H_FILE);

   fprintf(stdout, "\nFile Generated:  %s\n", tempApplName);
   xfree(tempApplName);

   return;
}


/**************************************************************************
Name          : dumpNcsNetSnmp
                                                                                
Description   : This function generates the stubs/code that provides 
                sync/async interface between MASv and NCS subAgent.
                This routine also generates MAPI header file with respect to 
                the application. 
                                                                                
Arguements    : modc - Number of arguments given for SMIDUMP command
                modv - Pointer to the SmiModule, contains MIB file name
                     information that was given in an SMIDUMP command. 
                flags - Defined in Driver definition
                applName -  Application name              
            
Return values : Nothing
                                                                                
Notes         :  
**************************************************************************/
static void dumpNcsNetSnmp(int modc, SmiModule **modv, int flags, char *applName)
{
   char  *hdrName;
   char  *tempApplName; 
   int   mibNum, esmiStatus = ESMI_SNMP_SUCCESS;
  
   
   /* Check whether SUBAGT directory path is defined */
   if (! subAgtDirPath)
   {
      fprintf(stderr, "\n ERROR: SUBAGT directory path is not defined in your application"
                      "\n        ESMI MIB file. Please define the ESMI tag for SUBAGT dir path."
                      "\n        One ESMI tag definition for an application is enough.\n\n");
      return;
   }

   if (flags & SMIDUMP_FLAG_UNITE)
   {
      /* not implemented yet */
   }
   else
   {  
      /* Traverse through all of the Modules */
      for (mibNum = 0; mibNum < modc; mibNum++) 
      {
          if (! esmiCheckModuleTables(modv[mibNum]))
          { 
             /* Module has some invalid TABLE data */ 
             esmiStatus = ESMI_SNMP_FAILURE;
             break;
          }
          /* Create a stub "*.h" for NCS subAgent w.r.t  Module */
          if (! dumpNcsHeader(modv[mibNum]))
          { 
             /* Not able to create an header file for a Module,
                so update the esmiStatus and come out of the loop */ 
             esmiStatus = ESMI_SNMP_FAILURE;
             break;
          }
         
          /* Create a stub "*.c" for NCS subAgent w.r.t Module */
          if (! dumpNcsAgtStub(modv[mibNum]))
          { 
             /* Not able to create the respective stub file,
                so come out of the loop */ 
             esmiStatus = ESMI_SNMP_FAILURE;
             break;
          }
      }
   } 

   /* If SMIDUMP doesn't encountered any erros, generate an application
      initialization subAgent stub */  
   if (esmiStatus == ESMI_SNMP_SUCCESS)
   {  
      tempApplName = translateLower(applName); 

      esmiStatus = dumpNcsApplSubAgtInit(tempApplName, modc, modv);
      xfree(tempApplName);
   }

   if (esmiStatus != ESMI_SNMP_SUCCESS)
   {
      /* If some thing went wrong then delete all the generated stubs */
      deleteNcsAgtStub(mibNum, modv);
      esmiError = TRUE;
   }

   return;
}


/**************************************************************************
Name          : dumpSynNcsNetSnmp
                                                                                
Description   : Driver routine to generate the stubs/code that provides 
                synchronous interface between MASv and NCS subAgent.
                This routine also generates MAPI header file with respect to 
                the application. For one execution of SMIDUMP command one MAPI
                header file will be generated that contains all the object 
                information (enum vals for objects with respect to table) of
                all the input Modules (MIBs). 
                                                                                
Arguments     : modc - Number of arguments given for SMIDUMP command
                modv - Pointer to the SmiModule, contains MIB file name
                       information that was given in an SMIDUMP command. 
                flags - Defined in Driver definition
                ouput -  -TBD-              
            
Return values : Nothing
                                                                                
Notes         :  
**************************************************************************/
static void dumpSynNcsNetSnmp(int modc, SmiModule **modv, int flags, char *output)
{
   /* set the MAB Option to Synchronous Interface option */
   esmiNetSnmpOpt =  ESMI_SYN_NETSNMP_OPT; 

   /* If application name is not mentioned in the SMIDUMP command, just
      return with out generating any stubs */ 
   if (! output)
   {
      fprintf(stderr, "\nPlease enter the application name with '-o<appl_name>' option."
                      "\nSMIDUMP doesn't generate any stubs for NCS subAgent.\n");

      fprintf(stderr, "\nSyntax:./smidump -fsynncsnetsnmp -o<appl_name> [MIB1] {MIB2] ..\n\n");
      return;
   }

   dumpNcsNetSnmp(modc, modv, flags, output);
  
   /* Free the allocated memory for directory paths */ 
   smiFree(mapiDirPath);
   smiFree(subAgtDirPath);
   smiFree(esmiDirPath);
   smiFree(esmiGlbHdrFile);

   return;
}


/**************************************************************************
Name          : dumpNewSynNcsNetSnmp
                                                                                
Description   : Driver routine to generate the stubs/code that provides 
                synchronous interface between MASv and NCS subAgent.
                This routine also generates MAPI header file with respect to 
                the application. For one execution of SMIDUMP command one MAPI
                header file will be generated that contains all the object 
                information (enum vals for objects with respect to table) of
                all the input Modules (MIBs). 
                                                                                
Arguments     : modc - Number of arguments given for SMIDUMP command
                modv - Pointer to the SmiModule, contains MIB file name
                       information that was given in an SMIDUMP command. 
                flags - Defined in Driver definition
                ouput -  -TBD-              
            
Return values : Nothing
                                                                                
Notes         :  
**************************************************************************/
static void dumpNewSynNcsNetSnmp(int modc, SmiModule **modv, int flags,
                                 char *output)
{
   /* set the MAB Option to Synchronous Interface option */
   esmiNetSnmpOpt =  ESMI_NEW_SYN_NETSNMP_OPT; 

   /* If application name is not mentioned in the SMIDUMP command, just
      return with out generating any stubs */ 
   if (! output)
   {
      fprintf(stderr, "\nPlease enter the application name with '-o<appl_name>' option."
                      "\nSMIDUMP doesn't generate any stubs for NCS subAgent.\n");

      fprintf(stderr, "\nSyntax:./smidump -fsynncsnetsnmp -o<appl_name> [MIB1] {MIB2] ..\n\n");
      return;
   }

   dumpNcsNetSnmp(modc, modv, flags, output);
  
   /* Free the allocated memory for directory paths */ 
   smiFree(mapiDirPath);
   smiFree(subAgtDirPath);
   smiFree(esmiDirPath);
   smiFree(esmiGlbHdrFile);

   return;
}


/**************************************************************************
Name          : dumpNcsMapi
                                                                                
Description   : Driver routine to generate  MAPI header file with respect to 
                the application. For one execution of SMIDUMP command one MAPI
                header file will be generated that contains all the object 
                information (enum vals for objects with respect to table) of
                all the input Modules (MIBs). 
                                                                                
Arguments     : modc - Number of arguments given for SMIDUMP command
                modv - Pointer to the SmiModule, contains MIB file name
                       information that was given in an SMIDUMP command. 
                flags - Defined in Driver definition
                ouput -  -TBD-              
            
Return values : Nothing
                                                                                
Notes         :  
**************************************************************************/
static void dumpNcsMapi(int modc, SmiModule **modv, int flags, char *output)
{
   /* set the Option to MAPI option */
   esmiNetSnmpOpt =  ESMI_MAPI_OPT; 

   /* If application name is not mentioned in the SMIDUMP command, just
      return with out generating any stubs */ 
   if (! output)
   {
      fprintf(stderr, "\nPlease enter the application name with '-o<appl_name>' option."
                      "\nSMIDUMP doesn't generate any stubs for NCS MAPI.\n");

      fprintf(stderr, "\nSyntax:./smidump -f ncsmapi -o <appl_name> [MIB1] {MIB2] ..\n\n");
      return;
   }

   printNcsMapi(modc, modv, flags, output);
  
   /* Free the allocated memory for directory paths */ 
   smiFree(mapiDirPath);
   smiFree(subAgtDirPath);
   smiFree(esmiDirPath);
   smiFree(esmiGlbHdrFile);

   return;
}


/**************************************************************************
Name          : dumpAsynNcsNetSnmp
                                                                                
Description   : Driver routine to generate the stubs/code that provides
                asynchronous interface between MASv and NCS subAgent.
                This routine also generates MAPI header file with respect to
                the application. For one execution of SMIDUMP command one MAPI
                header file will be generated that contains all the object
                information (enum vals for objects with respect to table) of
                all the input Modules (MIBs).
                                                                                
Arguments     : modc - Number of arguments given for SMIDUMP command
                modv - Pointer to the SmiModule, contains MIB file name
                     information that was given in an SMIDUMP command.
                flags - Defined in Driver definition
                ouput -  -TBD-
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void dumpAsynNcsNetSnmp(int modc, SmiModule **modv, int flags, char *output)
{
   /* Don't delete the following lines, once async interface implementation
      starts.. then we can use this code.. by modifying it according to the
      requirements.. until then comment it as it is. */
   /*
   esmiNetSnmpOpt =  ESMI_ASYN_NETSNMP_OPT; 
   dumpNcsNetSnmp(modc, modv, flags, output);
   */
   
   return;
}

#endif /* NCS_NETSNMP */

#if (NCS_MIBLIB != 0)
/**************************************************************************
Name          : getListFromParentType
                                                                                
Description   : This function returns the list ptr from parent type for 
                given type ptr. It finds a parent type for which list 
                ptr is valid and returns it. If no parent type is found
                with valid list ptr, NULL will be returned.
                                                                                
Arguments     : typePtr - pointer to the Type structure.
                
Return Values : Pointer to List.
                                                                                
Notes         :
**************************************************************************/
static List *getListFromParentType (Type *typePtr)
{
    Type *tempPtr = typePtr->parentPtr;
    
    while((tempPtr) && (tempPtr->listPtr == NULL))
    {
        tempPtr = tempPtr->parentPtr;
    }
    if(tempPtr)
        return (tempPtr->listPtr);
    else
        return NULL;
}

/**************************************************************************
Name          : getRangeForType
                                                                                
Description   : This function returns the range ptr for the given type ptr.
                                                                                
Arguments     : typePtr - pointer to the Type structure.
                
Return Values : Pointer to Range.
                                                                                
Notes         :
**************************************************************************/
static Range *getRangeForType (Type *typePtr)
{
    Type *tempPtr = typePtr->parentPtr;
    
    if(typePtr->listPtr)
    {
        return (Range *)(typePtr->listPtr->ptr);
    }
    else
    {
        while((tempPtr) && (tempPtr->listPtr == NULL))
        {
            tempPtr = tempPtr->parentPtr;
        }
        if(tempPtr)
            return ((Range *)tempPtr->listPtr->ptr);
        else
            return NULL;
    }
}

/**************************************************************************
Name          : esmiIsObjIndexType
                                                                                
Description   : This function checks whether the object is a part of the
                Index list of the table.
                                                                                
Arguments     : grpNode - Pointer to the SmiNode of the Table.
                smiNode - Pointer to the SmiNode of the Object
                                                                                
Return values : TRUE / FALSE
                                                                                
Notes         :
**************************************************************************/
static int esmiIsObjIndexType(SmiNode *grpNode, SmiNode *smiNode)
{
   List   *listPtr = NULL;
   Object *objPtr  = (Object *)grpNode;
  
   /* Get the number of indices exists for this table */ 
   for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr) 
   {  
      /* If the object is the part of the TABLE index list then.. return
         TRUE */
      if (! strcmp(((Object*)listPtr->ptr)->export.name, smiNode->name))
         return TRUE; 
   }

   return FALSE;
}

static void printSmiNodeDiscreteValues(FILE *fd, SmiNode *smiNode)
{
   List    *listPtr = NULL;
   List    *parentListPtr = NULL;
   Object  *objPtr  = NULL;
   int     discreteNum = 0, value = 0; 
   SmiType *smiType = NULL;
   Range   *rangePtr = NULL;

       discreteNum = 0;

       if (! (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)))
          return;

       if ( !(smiType = smiGetNodeType(smiNode)))
          return;
       
       objPtr = (Object *)smiNode;

       if (objPtr->typePtr->listPtr)
          listPtr = objPtr->typePtr->listPtr->nextPtr;
       else if(parentListPtr = getListFromParentType(objPtr->typePtr)) /* check for parent's listPtr. This is required for textual conventions. */
           listPtr = parentListPtr->nextPtr;

       /* If the range has -ve and +ve integers, then it will be generated as discrete range object */
       /*********************************************************************************************/
       /* Following is for objects of type Integer32 without any range specified */
       else if (smiType->basetype == SMI_BASETYPE_INTEGER32) 
       {
          rangePtr = getRangeForType(objPtr->typePtr);
          if ((rangePtr == NULL) || ((rangePtr->export.minValue.value.integer32 < 0) && (rangePtr->export.maxValue.value.integer32 >= 0)))
          {
             discreteNum += 2;

             fprintf(fd, "\nstatic NCSMIB_INT_OBJ_RANGE %s_%s[%d] = {", smiNode->name, 
                          (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_values":"values", discreteNum); 
            
             if (rangePtr == NULL)
             {
                fprintf(fd, "\n                                          {0x80000000, 0xffffffff},");
                fprintf(fd, "\n                                          {0x0, 0x7fffffff}");
             }
             else
             {
                fprintf(fd, "\n                                          {0x%x, 0x%x},", rangePtr->export.minValue.value.integer32, -1);
                fprintf(fd, "\n                                          {0x0, 0x%x}", rangePtr->export.maxValue.value.integer32);
             }
             fprintf(fd, "\n                                        };\n");
             return;
          }
          else 
            return;
       }
       else
          return;

       /* Following is for objects of type Integer32(-ve, +ve) */
       if ((listPtr == NULL) && (smiType->basetype == SMI_BASETYPE_INTEGER32))
       {
          rangePtr = getRangeForType(objPtr->typePtr);
          if ((rangePtr == NULL) || ((rangePtr->export.minValue.value.integer32 < 0) && (rangePtr->export.maxValue.value.integer32 >= 0)))
          {
             discreteNum += 2;

             fprintf(fd, "\nstatic NCSMIB_INT_OBJ_RANGE %s_%s[%d] = {", smiNode->name, 
                          (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_values":"values", discreteNum); 
            
             if (rangePtr == NULL)
             {
                fprintf(fd, "\n                                          {0x80000000, 0xffffffff},");
                fprintf(fd, "\n                                          {0x0, 0x7fffffff}");
             }
             else
             {
                fprintf(fd, "\n                                          {0x%x, 0x%x},", rangePtr->export.minValue.value.integer32, -1);
                fprintf(fd, "\n                                          {0x0, 0x%x}", rangePtr->export.maxValue.value.integer32);
             }
             fprintf(fd, "\n                                        };\n");
             return;
          }
          else 
            return;
       }

       /*********************************************************************************************/
       while (listPtr)
       {
          discreteNum++;
          /* Check if the range has a -ve integer and a +ve integers */
          if ((smiType->basetype == SMI_BASETYPE_INTEGER32) &&
                  (((Range *)listPtr->ptr)->export.minValue.value.integer32 < 0) && 
                  (((Range *)listPtr->ptr)->export.maxValue.value.integer32 >= 0))
               discreteNum++;
          listPtr = listPtr->nextPtr;
       }
       
       if (! discreteNum)
          return;     

       discreteNum++;
       
       fprintf(fd, "\nstatic %s %s_%s[%d] = {", ((smiType->basetype == SMI_BASETYPE_OCTETSTRING) || (smiType->basetype == SMI_BASETYPE_BITS))?
                        "NCSMIB_OCT_OBJ":"NCSMIB_INT_OBJ_RANGE",
                        smiNode->name, (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_values":"values", discreteNum);
       /* Get the number of indices exists for this table */ 

       if (objPtr->typePtr->listPtr)
           listPtr = objPtr->typePtr->listPtr;
       else
           listPtr = parentListPtr;

       for ( ; listPtr; listPtr = listPtr->nextPtr) 
       {
          if (listPtr->nextPtr != NULL)
          {
              if((((Range *)listPtr->ptr)->export.minValue.len) &&
                 (! (((Range *)listPtr->ptr)->export.maxValue.basetype)))
              {
                  
                 value = (int)(((Range *)listPtr->ptr)->export.minValue.value.integer64 >> 32);
                 fprintf(fd, "\n                                                   {0x%x, 0x%x},", value, value);

              }
              else
              {
                    if ((smiType->basetype == SMI_BASETYPE_INTEGER32) &&
                        (((Range *)listPtr->ptr)->export.minValue.value.integer32 < 0) && 
                        (((Range *)listPtr->ptr)->export.maxValue.value.integer32 >= 0))
                    {
                       fprintf(fd, "\n                                                   {0x%x, 0x%x},", 
                                  ((Range *)listPtr->ptr)->export.minValue.value.integer32, -1);
                       fprintf(fd, "\n                                                   {0x0, 0x%x},", 
                                  ((Range *)listPtr->ptr)->export.maxValue.value.integer32);
                    }
                    else
                       fprintf(fd, "\n                                                   {0x%x, 0x%x},", 
                                  ((Range *)listPtr->ptr)->export.minValue.value.integer32, ((Range *)listPtr->ptr)->export.maxValue.value.integer32);
              }
          }
          else /* Last Discrete value */
          {
              if((((Range *)listPtr->ptr)->export.minValue.len) &&
                 (! (((Range *)listPtr->ptr)->export.maxValue.basetype)))
              {
                  
                 value = (int)(((Range *)listPtr->ptr)->export.minValue.value.integer64 >> 32); 
                 fprintf(fd, "\n                                                   {0x%x, 0x%x}", value, value);
              }
              else
              {
                    if ((smiType->basetype == SMI_BASETYPE_INTEGER32) &&
                        (((Range *)listPtr->ptr)->export.minValue.value.integer32 < 0) && 
                        (((Range *)listPtr->ptr)->export.maxValue.value.integer32 >= 0))
                    {
                       fprintf(fd, "\n                                                   {0x%x, 0x%x},", 
                                  ((Range *)listPtr->ptr)->export.minValue.value.integer32, -1);
                       fprintf(fd, "\n                                                   {0x0, 0x%x}", 
                                  ((Range *)listPtr->ptr)->export.maxValue.value.integer32);
                    }
                    else
                       fprintf(fd, "\n                                                   {0x%x, 0x%x}", 
                                ((Range *)listPtr->ptr)->export.minValue.value.integer32, ((Range *)listPtr->ptr)->export.maxValue.value.integer32);
              }
          }
       }

       fprintf(fd, "\n                                                      };\n");
       return;
}

/**************************************************************************
Name          : printObjDiscreteValues
                                                                                
Description   : This function prints the indices (if they exists) of the
                table
                                                                                
Arguments     : fd - File descriptor of the file to which the indices should 
                be printed.
                grpNode - Pointer to the SmiNode of the Table.
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void printObjDiscreteValues(FILE *fd, SmiNode *grpNode)
{
   List    *listPtr = NULL;
   List    *parentListPtr = NULL;
   Object  *objPtr  = NULL;
   SmiNode *smiNode = NULL;
   int     discreteNum = 0, value = 0; 
   SmiType *smiType = NULL;
   Range   *rangePtr = NULL;


   /* Traverse through all of the objects in the table */
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
       discreteNum = 0;

       if (! (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)))
          continue;

       if ( !(smiType = smiGetNodeType(smiNode)))
          continue;
       
       objPtr = (Object *)smiNode;

       if (objPtr->typePtr->listPtr)
          listPtr = objPtr->typePtr->listPtr->nextPtr;
       else if(parentListPtr = getListFromParentType(objPtr->typePtr)) /* check for parent's listPtr. This is required for textual conventions. */
           listPtr = parentListPtr->nextPtr;

       /* If the range has -ve and +ve integers, then it will be generated as discrete range object */
       /*********************************************************************************************/
       /* Following is for objects of type Integer32 without any range specified */
       else if (smiType->basetype == SMI_BASETYPE_INTEGER32) 
       {
          rangePtr = getRangeForType(objPtr->typePtr);
          if ((rangePtr == NULL) || ((rangePtr->export.minValue.value.integer32 < 0) && (rangePtr->export.maxValue.value.integer32 >= 0)))
          {
             discreteNum += 2;

             fprintf(fd, "\nstatic NCSMIB_INT_OBJ_RANGE %s_%s[%d] = {", smiNode->name, 
                          (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_values":"values", discreteNum); 
            
             if (rangePtr == NULL)
             {
                fprintf(fd, "\n                                          {0x80000000, 0xffffffff},");
                fprintf(fd, "\n                                          {0x0, 0x7fffffff}");
             }
             else
             {
                fprintf(fd, "\n                                          {0x%x, 0x%x},", rangePtr->export.minValue.value.integer32, -1);
                fprintf(fd, "\n                                          {0x0, 0x%x}", rangePtr->export.maxValue.value.integer32);
             }
             fprintf(fd, "\n                                        };\n");
             continue;
          }
          else 
            continue;
       }
       else
          continue;

       /* Following is for objects of type Integer32(-ve, +ve) */
       if ((listPtr == NULL) && (smiType->basetype == SMI_BASETYPE_INTEGER32))
       {
          rangePtr = getRangeForType(objPtr->typePtr);
          if ((rangePtr == NULL) || ((rangePtr->export.minValue.value.integer32 < 0) && (rangePtr->export.maxValue.value.integer32 >= 0)))
          {
             discreteNum += 2;

             fprintf(fd, "\nstatic NCSMIB_INT_OBJ_RANGE %s_%s[%d] = {", smiNode->name, 
                          (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_values":"values", discreteNum); 
            
             if (rangePtr == NULL)
             {
                fprintf(fd, "\n                                          {0x80000000, 0xffffffff},");
                fprintf(fd, "\n                                          {0x0, 0x7fffffff}");
             }
             else
             {
                fprintf(fd, "\n                                          {0x%x, 0x%x},", rangePtr->export.minValue.value.integer32, -1);
                fprintf(fd, "\n                                          {0x0, 0x%x}", rangePtr->export.maxValue.value.integer32);
             }
             fprintf(fd, "\n                                        };\n");
             continue;
          }
          else 
            continue;
       }
      
       /*********************************************************************************************/
       while (listPtr)
       {
          discreteNum++;
          /* Check if the range has a -ve integer and a +ve integers */
          if ((smiType->basetype == SMI_BASETYPE_INTEGER32) &&
                  (((Range *)listPtr->ptr)->export.minValue.value.integer32 < 0) && 
                  (((Range *)listPtr->ptr)->export.maxValue.value.integer32 >= 0))
               discreteNum++;
          listPtr = listPtr->nextPtr;
       }
       
       if (! discreteNum)
          continue;     

       discreteNum++;
       
       fprintf(fd, "\nstatic %s %s_%s[%d] = {", ((smiType->basetype == SMI_BASETYPE_OCTETSTRING) || (smiType->basetype == SMI_BASETYPE_BITS))?
                        "NCSMIB_OCT_OBJ":"NCSMIB_INT_OBJ_RANGE",
                        smiNode->name, (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_values":"values", discreteNum);
       /* Get the number of indices exists for this table */ 

       if (objPtr->typePtr->listPtr)
           listPtr = objPtr->typePtr->listPtr;
       else
           listPtr = parentListPtr;

       for ( ; listPtr; listPtr = listPtr->nextPtr) 
       {
          if (listPtr->nextPtr != NULL)
          {
              if (smiType->basetype == SMI_BASETYPE_ENUM)
              {
                value = (int)(((NamedNumber *)listPtr->ptr)->export.value.value.integer32);
                fprintf(fd, "\n                                                   {0x%x, 0x%x},", value, value);
              }
              else if (smiType->basetype == SMI_BASETYPE_BITS)
              {
                value = (int)(((NamedNumber *)listPtr->ptr)->export.value.value.unsigned32);
                fprintf(fd, "\n                                                   {0x%x, 0x%x},", value, value);
              }
              else if((((Range *)listPtr->ptr)->export.minValue.len) &&
                 (! (((Range *)listPtr->ptr)->export.maxValue.basetype)))
              {
                  
                 value = (int)(((Range *)listPtr->ptr)->export.minValue.value.integer64 >> 32);
                 fprintf(fd, "\n                                                   {0x%x, 0x%x},", value, value);

              }
              else
              {
                    if ((smiType->basetype == SMI_BASETYPE_INTEGER32) &&
                        (((Range *)listPtr->ptr)->export.minValue.value.integer32 < 0) && 
                        (((Range *)listPtr->ptr)->export.maxValue.value.integer32 >= 0))
                    {
                       fprintf(fd, "\n                                                   {0x%x, 0x%x},", 
                                  ((Range *)listPtr->ptr)->export.minValue.value.integer32, -1);
                       fprintf(fd, "\n                                                   {0x0, 0x%x},", 
                                  ((Range *)listPtr->ptr)->export.maxValue.value.integer32);
                    }
                    else
                       fprintf(fd, "\n                                                   {0x%x, 0x%x},", 
                                  ((Range *)listPtr->ptr)->export.minValue.value.integer32,
                                  ((Range *)listPtr->ptr)->export.maxValue.value.integer32);
              }
          }
          else /* Last Discrete value */
          {
              if (smiType->basetype == SMI_BASETYPE_ENUM)
              {
                value = (int)(((NamedNumber *)listPtr->ptr)->export.value.value.integer32);
                fprintf(fd, "\n                                                   {0x%x, 0x%x},", value, value);
              }
              else if (smiType->basetype == SMI_BASETYPE_BITS)
              {
                value = (int)(((NamedNumber *)listPtr->ptr)->export.value.value.unsigned32);
                fprintf(fd, "\n                                                   {0x%x, 0x%x},", value, value);
              }
              else if((((Range *)listPtr->ptr)->export.minValue.len) &&
                 (! (((Range *)listPtr->ptr)->export.maxValue.basetype)))
              {
                  
                 value = (int)(((Range *)listPtr->ptr)->export.minValue.value.integer64 >> 32); 
                 fprintf(fd, "\n                                                   {0x%x, 0x%x}", value, value);
              }
              else
              {
                    if ((smiType->basetype == SMI_BASETYPE_INTEGER32) &&
                        (((Range *)listPtr->ptr)->export.minValue.value.integer32 < 0) && 
                        (((Range *)listPtr->ptr)->export.maxValue.value.integer32 >= 0))
                    {
                       fprintf(fd, "\n                                                   {0x%x, 0x%x},", 
                                  ((Range *)listPtr->ptr)->export.minValue.value.integer32, -1);
                       fprintf(fd, "\n                                                   {0x0, 0x%x}", 
                                  ((Range *)listPtr->ptr)->export.maxValue.value.integer32);
                    }
                    else
                       fprintf(fd, "\n                                                   {0x%x, 0x%x}", 
                                ((Range *)listPtr->ptr)->export.minValue.value.integer32, ((Range *)listPtr->ptr)->export.maxValue.value.integer32);
              }
          }
       }

       fprintf(fd, "\n                                                      };\n");
   }
   
   /* For pssv, check if augmented/imported index is a discrete obj type */
   /* From grpNode, get the indexes imported/augmented from another table */
   if (esmiNetSnmpOpt == ESMI_PSSV_OPT)
   {
      objPtr = (Object *) grpNode;
      Object *parentObjPtr = NULL;
      if (grpNode->indexkind == SMI_INDEX_AUGMENT)
      {
         /* Related ptr in Object points to the table from which this table is augmented. */
         objPtr = objPtr->relatedPtr;
         if (objPtr->export.indexkind == SMI_INDEX_INDEX)
         {
            for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr)
            {
               parentObjPtr = ((Object *)listPtr->ptr)->nodePtr->parentPtr->firstObjectPtr; /* this is group node for augmented index */
               /* status = print_var_info_pssv (fd, &(parentObjPtr->export), &(((Object *)listPtr->ptr)->export), nonLocalIndices); */
               smiNode = &(((Object *)listPtr->ptr)->export);
               printSmiNodeDiscreteValues(fd, smiNode);
            }
         }
         /* Else, Table from which we have augmented doesn't have SMI_INDEX_INDEX...*/
      }
      else if (grpNode->indexkind == SMI_INDEX_INDEX)
      {
         for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr)
         {
            parentObjPtr = ((Object *)listPtr->ptr)->nodePtr->parentPtr->firstObjectPtr;
            if (parentObjPtr == objPtr)
               continue; /* Index is of the local table, don't print now. */

            /* status = print_var_info_pssv (fd, &(parentObjPtr->export), &(((Object *)listPtr->ptr)->export), nonLocalIndices); */
            smiNode = &(((Object *)listPtr->ptr)->export);
            printSmiNodeDiscreteValues(fd, smiNode);
         }
 
      }
   }

   return;
}


/**************************************************************************
Name          : printMibLibOrPssvIndices
                                                                                
Description   : This function prints the indices (if they exists) of the
                table
                                                                                
Arguments     : fd - File descriptor of the file to which the indices should 
                be printed.
                grpNode - Pointer to the SmiNode of the Table.
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void printMibLibOrPssvIndices(FILE *fd, SmiNode *grpNode)
{
   List   *listPtr = NULL;
   Object *objPtr = (Object *)grpNode;
   int    indicesNum = 0; 
   char   *tblNameSmall;


   /* Check whether the indices are defined for this table */
   if (!objPtr->listPtr)
   {
      /* No indices are defined for this table */
      return;
   }

   if (esmiCheckIndexInThisTable(grpNode, &indicesNum) != ESMI_SNMP_SUCCESS)
      return;

   tblNameSmall = translateLower(grpNode->name);

   /* Get the number of indices exists for this table */ 
   for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr) 
       indicesNum++;

   /* Only one index defined for this table */
   if (indicesNum == 1)
   {
      listPtr = objPtr->listPtr;
      fprintf(fd, "\nstatic uns16 %s_%s[%d] = { %s_ID };", tblNameSmall,
                            (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_indices":"indices",
                            indicesNum, ((Object *)listPtr->ptr)->export.name);
   }
   else /* has more than one index */
   {
      fprintf(fd, "\nstatic uns16 %s_%s[%d] = {", tblNameSmall, 
                            (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_indices":"indices",
                            indicesNum);

      /* Traverse through all the indices defined for this table */
      for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr) 
      { 
          if (listPtr->nextPtr != NULL)
             fprintf(fd, "\n                                     %s_ID,",
                                         ((Object*)listPtr->ptr)->export.name); 
          else /* Last Index */
             fprintf(fd, "\n                                     %s_ID };",
                               ((Object *)listPtr->ptr)->export.name); 
      }
   }
   /* Free the alloted memory for tblNameSmall */ 
   xfree(tblNameSmall);

   return;
}


void printMaxTblParamsMacro (FILE *fd, SmiNode *grpNode)
{
   SmiNode *smiNode = NULL;
   Object *objPtr = (Object *) grpNode, *parentObjPtr = NULL;
   List *listPtr = NULL;
   int status = ESMI_SNMP_SUCCESS;
   int nonLocalIndices = 0, localObjects = 0, i, count;
   char *tblNameBig = translateUpper (grpNode->name);

   /* From grpNode, get the indexes imported/augmented from another table and print var_info for them */
   if (grpNode->indexkind == SMI_INDEX_AUGMENT)
   {
         /* Related ptr in Object points to the table from which this table is augmented. */
         objPtr = objPtr->relatedPtr;
         if (objPtr->export.indexkind == SMI_INDEX_INDEX)
         {
            for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr)
            {
               nonLocalIndices++;
            }
         }
         /* Else, Table from which we have augmented doesn't have SMI_INDEX_INDEX...*/
   }
   else if (grpNode->indexkind == SMI_INDEX_INDEX)
   {
         for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr)
         {
            parentObjPtr = ((Object *)listPtr->ptr)->nodePtr->parentPtr->firstObjectPtr;
            if (parentObjPtr == objPtr)
               continue; /* Index is of the local table. */

            nonLocalIndices++;
         }
   }

   /* Traverse through all of the objects in the table */
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
         if (! (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)))
            continue;

         if (nonLocalIndices == 0) /* (scalars) or tables having only local indices */
            count = smiNode->oid[smiNode->oidlen-1]; 
         else
            localObjects++;
   }

   if (nonLocalIndices == 0)
      fprintf(fd, "\n\n#define %s_PSSV_TBL_MAX_PARAMS   %u\n", tblNameBig, count); 
   else
      fprintf(fd, "\n\n#define %s_PSSV_TBL_MAX_PARAMS   %u\n", tblNameBig, nonLocalIndices+localObjects); 

   return;
}

 
/**************************************************************************
Name          :  printMibLibOrPssvDefines
                                                                                
Description   : This function prints the function definitions, macros, data
                structs  to be defined in the generated MIBLIB stub.
                                                                                
Arguments     : fd - File descriptor of the file to which the code should be
                dumped.
                grpNode - Pointer to the SmiNode of the table.
                objCount - Number of objects present in this table 
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void printMibLibOrPssvDefines(FILE *fd, SmiNode *grpNode)
{
   char *tblNameBig = translateUpper(grpNode->name); 
   char *tblNameSmall = translateLower(grpNode->name);
   char *tblName = esmiTranslateHungarian(grpNode->name);

   if (esmiNetSnmpOpt == ESMI_PSSV_OPT)
   {
      fprintf(fd, "\n#include \"%s\"", "psr_mib_load.h");
      if (grpNode->esmi.info.tableInfo.hdrFile[0] != '\0') /* Table Header file is optional for PSSV */
         fprintf(fd, "\n#include \"%s\"", grpNode->esmi.info.tableInfo.hdrFile);
      /* print the enum for this table only */ 
      printMaxTblParamsMacro(fd, grpNode); 
   }

   if (esmiNetSnmpOpt != ESMI_PSSV_OPT)
   {
       /* Print the id of the table */   
       fprintf(fd, "\n#include \"%s\"", grpNode->esmi.info.tableInfo.hdrFile);

       fprintf(fd, "\n\n#define  %s_TBL_MAX_PARAMS   (%sMax_ID - 1)", tblNameBig, grpNode->name);

       fprintf(fd, "\n\nstatic NCSMIB_VAR_INFO  %s_var_info[%s_TBL_MAX_PARAMS];", 
                   tblNameSmall, tblNameBig); 
       fprintf(fd, "\nstatic NCSMIB_OBJ_INFO  %s_obj_info;", tblNameSmall);
   }

   if (esmiNetSnmpOpt == ESMI_PSSV_OPT)
   {
      fprintf(fd, "\n\n/* forward declaration for register function */");
      fprintf(fd, "\nuns32  __%s_pssv_reg(NCSCONTEXT);", tblNameSmall);
   }

   /* Print indices of the table, if they exists */
   if (esmiNetSnmpOpt == ESMI_MIBLIB_OPT)
      printMibLibOrPssvIndices(fd, grpNode);

   /* Print Object discrete values in static ARRAY formats */
   printObjDiscreteValues(fd, grpNode);

   if (esmiNetSnmpOpt == ESMI_PSSV_OPT) 
   {
      unsigned int i;
  
      fprintf(fd, "\n\n/* Table base oid */");
      fprintf(fd, "\nstatic uns32  __%s_base_pssv[] = {", tblName);
      for (i = 0; i < grpNode->oidlen; i++)
      {
         fprintf(fd, "%s%d", i ? ", " : "", grpNode->oid[i]);
      }
      fprintf(fd, "};\n\n");
   }
   else
   {
      fprintf(fd, "\n\n/* To register the '%s' table information with MibLib */", tblNameSmall);
      fprintf(fd, "\nEXTERN_C uns32  %s_tbl_reg(void);\n", tblNameSmall);
   }

   fprintf(fd, "\n/* To initialize the '%s' table information */", tblNameSmall);
   fprintf(fd, "\nstatic void %s_%s(NCSMIB_TBL_INFO *);", tblNameSmall,
               (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_tbl_init":"tbl_init");

   fprintf(fd, "\n\n/* To initialize the '%s' table objects */", tblNameSmall);
   fprintf(fd, "\nstatic void %s_%s(NCSMIB_VAR_INFO *);", tblNameSmall,
               (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_obj_init":"obj_init");

   /* Free the allotted memory for table names */
   xfree(tblNameBig);
   xfree(tblNameSmall);
   xfree(tblName);

   return;
}


/**************************************************************************
Name          : printObjInitHdr
                                                                                
Description   : This function prints the header information of object
                init routine and the routine itself. 
                                                                                
Arguments     : fd - File descriptor of the file to which the information  
                     should be dumped.
                objInitName - Sting ptr to the name of the obj init routine. 
                                                                                
Return Values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void printObjInitHdr(FILE *fd, char *objInitName)
{
   char *tblNameBig;
   char *tblNameSmall;

   tblNameBig   = translateUpper(objInitName);
   tblNameSmall = translateLower(objInitName);

   /* Print the function header with appropriate information */
   fprintf(fd, "\n\n\n/**************************************************************************"
               "\nName          :  %s_%s"
               "\n\nDescription   :  Initialize the objects of the  %s  table" 
               "\n\nArguments     :  %s_var_info - Ptr to the NCSMIB_VAR_INFO structure"
               "\n\nReturn values :  Nothing"
               "\n\nNotes         :"     
               "\n**************************************************************************/", 
                tblNameSmall, (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_obj_init":"obj_init", tblNameSmall, tblNameSmall);

   /* Print the function */
   fprintf(fd, "\nstatic void  %s_%s(NCSMIB_VAR_INFO *var_info)\n{", tblNameSmall,
           (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_obj_init":"obj_init");
   fprintf(fd, "\n   m_NCS_OS_MEMSET(var_info," 
               "\n                   0,"
               "\n                   sizeof(NCSMIB_VAR_INFO)*%s_%s);", 
               tblNameBig,  
               (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"PSSV_TBL_MAX_PARAMS":"TBL_MAX_PARAMS");

   /* Free the alloted memory for table names */
   xfree(tblNameBig);
   xfree(tblNameSmall);
   
   return;
}


/**************************************************************************
Name          : printConvFromActualToRelative
                                                                                
Description   : This function prints the param_id conversion function (from
                relative linear value to actual value defined as per MIB)
                for the table which has non-linear objects in it. 
                                                                                
Arguments     : fd - File descriptor of the file to which the code  
                     should be dumped.
                grpNode - Pointer to the SmiNode.
                                                                                
Return Values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void  printConvFromActualToRelative(FILE *fd, SmiNode *grpNode)
{
   SmiNode *smiNode = NULL;
   SmiType *smiType = NULL;
   int     objNum = 0;

   /* Traverse through all of the objects in the table */
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
      if (! (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)))
         continue;

      smiType = smiGetNodeType(smiNode);
      if (!smiType) continue;

      objNum++;
    
      fprintf(fd, "\n      case %s_ID:", smiNode->name);
      fprintf(fd, "\n          *param_id = %d;", objNum);
      fprintf(fd, "\n          break;\n");
   }

   fprintf(fd, "\n      default:");
   fprintf(fd, "\n          break;");

   return; 
}


/**************************************************************************
Name          : printConvFromRelativeToActual
                                                                                
Description   : This function prints the param_id conversion function (from
                actual value defined as per MIB to relative linear value 
                which is used by MibLib for indexing a static VAR_INFO
                array) for the table which has non-linear objects in it. 
                                                                                
Arguments     : fd - File descriptor of the file to which the code  
                     should be dumped.
                grpNode - Pointer to the SmiNode.
                                                                                
Return Values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void  printConvFromRelativeToActual(FILE *fd, SmiNode *grpNode)
{
   SmiNode *smiNode = NULL;
   SmiType *smiType = NULL;
   int     objNum = 0;

   /* Traverse through all of the objects in the table */
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
      if (! (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)))
         continue;

      smiType = smiGetNodeType(smiNode);
      if (!smiType) continue;

      objNum++;
    
      fprintf(fd, "\n      case %d:", objNum);
      fprintf(fd, "\n          *param_id = %s_ID;", smiNode->name);
      fprintf(fd, "\n          break;\n");
   }
  
   fprintf(fd, "\n      default:");
   fprintf(fd, "\n          break;");

   return; 
}


/**************************************************************************
Name          : printParamIdConvFunc
                                                                                
Description   : This function prints the param_id conversion function (from
                actual value defined as per MIB to relative linear value 
                which is used by MibLib for indexing a static VAR)INFO
                array) for the table which has non-linear objects in it. 
                                                                                
Arguments     : fd - File descriptor of the file to which the code  
                     should be dumped.
                grpNode - Pointer to the SmiNode.
                                                                                
Return Values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void printParamIdConvFunc(FILE *fd, SmiNode *grpNode)
{
   char *tblNameSmall;

   tblNameSmall = translateLower(grpNode->name);

   /* Print the function header with appropriate information */
   fprintf(fd, "\n\n\n/**************************************************************************"
               "\nName          :  %s_conv_param_id"
               "\n\nDescription   :  Convert the object's param_id (of MIB_ARG) values of"
               "\n                 %s table from actual value defined as per MIB to"
               "\n                 relative linear value which is used by NCS MibLib for indexing"
               "\n                 a static VAR_INFO array. This function is only be called if" 
               "\n                 the table has non-linear object values."
               "\n\nArguments     :  %s_var_info - Ptr to the NCSMIB_VAR_INFO structure"
               "\n\nReturn values :  Nothing"
               "\n\nNotes         :  After NCS MibLib completes its processing, the param_id"
               "\n                   value is replaced back to the actual value"     
               "\n**************************************************************************/", 
                tblNameSmall, tblNameSmall, tblNameSmall);

   /* Print the function */
   fprintf(fd, "\nvoid  %s_conv_param_id(uns16 *param_id,"
               "\n                       NCSMIB_PARAM_ID_CONV_FLAG flag)\n{", tblNameSmall);

   fprintf(fd, "\n\n   /* Converting args->param_id to relative linear value */");
   fprintf(fd, "\n   if (flag == NCSMIB_PARAM_ID_ACTUAL_TO_RELATIVE)\n   {" 
               "\n      switch(*param_id)\n      {");

   printConvFromActualToRelative(fd, grpNode);

   fprintf(fd, "\n      }\n   }\n   else /* Converting args->param_id to actual value */");
   fprintf(fd, "\n   if (flag == NCSMIB_PARAM_ID_ACTUAL_TO_RELATIVE)\n   {" 
               "\n      switch(*param_id)\n      {");

   printConvFromRelativeToActual(fd, grpNode);

   fprintf(fd, "\n      }\n   }\n");
   fprintf(fd, "\n   return;\n}\n");

   /* Free the alloted memory for table names */
   xfree(tblNameSmall);
   
   return;
}


/**************************************************************************
Name          : esmiCheckIsDiscreteObject
                                                                                
Description   : This function check whether discrete values are set to an
                object. If they are set then return the number of discrete
                values set. 
                                                                                
Arguments     : smiNode - pointer to the SmiNode data structs, holds
                          object information.
                                                                                
Return values : Number of discrete values defined for an object.
                                                                                
Notes         :
**************************************************************************/
static int esmiCheckIsDiscreteObject(SmiNode *smiNode) 
{
   List   *lstPtr = NULL;
   int    discreteCount = 0;
   Object *obj = (Object *)smiNode;
   Range *rangePtr = NULL;
   SmiType *smiType = NULL;
   char tempStr[64];

   /* Reset the memory contents of these variable */
   memset(tempStr, 0, sizeof(tempStr));
   smiType = smiGetNodeType(smiNode);

   lstPtr = (List *)(obj->typePtr->listPtr);

   if (! lstPtr)
   {
      /* Get the listPtr from the parentType. */
      lstPtr = getListFromParentType(obj->typePtr); 
      if(! lstPtr)
      {
         rangePtr = getRangeForType(obj->typePtr);
         if ((smiType->basetype == SMI_BASETYPE_INTEGER32) && 
             ((rangePtr == NULL) || 
             ((rangePtr->export.minValue.value.integer32 < 0) &&
             (rangePtr->export.maxValue.value.integer32 >= 0))))
         {
             memset(smiNode->esmi.info.objectInfo.objRangeType,
                    0,
                    sizeof(smiNode->esmi.info.objectInfo.objRangeType));
             strcpy(smiNode->esmi.info.objectInfo.objRangeType, "NCSMIB_INT_DISCRETE_OBJ_TYPE");
             smiNode->esmi.info.objectInfo.objSpecType = ESMI_OBJ_NUM_TYPE;

             strcpy(tempStr, smiNode->name);
      
             if (esmiNetSnmpOpt == ESMI_PSSV_OPT) 
                strcat(tempStr, "_pssv_values");
             else
                strcat(tempStr, "_values");

             strcpy(smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1, tempStr);
             discreteCount = 2;
         }
         return discreteCount;
      }
   }
   
   if ((lstPtr->nextPtr == NULL) && (smiType->basetype == SMI_BASETYPE_INTEGER32))
   {
      rangePtr = getRangeForType(obj->typePtr);
      if ((rangePtr == NULL) || 
             ((rangePtr->export.minValue.value.integer32 < 0) &&
             (rangePtr->export.maxValue.value.integer32 >= 0)))
         {
             memset(smiNode->esmi.info.objectInfo.objRangeType,
                    0,
                    sizeof(smiNode->esmi.info.objectInfo.objRangeType));
             strcpy(smiNode->esmi.info.objectInfo.objRangeType, "NCSMIB_INT_DISCRETE_OBJ_TYPE");
             smiNode->esmi.info.objectInfo.objSpecType = ESMI_OBJ_NUM_TYPE;

             strcpy(tempStr, smiNode->name);
      
             if (esmiNetSnmpOpt == ESMI_PSSV_OPT) 
                strcat(tempStr, "_pssv_values");
             else
                strcat(tempStr, "_values");

             strcpy(smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1, tempStr);
             discreteCount = 2;
         }
         return discreteCount;
   }

   /* Get the number of discrete values defined for an object */   
   while (lstPtr->nextPtr)
   {
      discreteCount++;
      /* Check if the range has a -ve integer and a +ve integers */
      if ((smiType->basetype == SMI_BASETYPE_INTEGER32) &&
          (((Range *)lstPtr->ptr)->export.minValue.value.integer32 < 0) && 
          (((Range *)lstPtr->ptr)->export.maxValue.value.integer32 >= 0))
         discreteCount++;
      lstPtr = lstPtr->nextPtr;
   }
  
   /* If the object was set with DISCRETE values then set the 
      objRangeType of smiNode to the corresponding MACRO definition */ 
   if(discreteCount)
   {

      discreteCount++;

      memset(smiNode->esmi.info.objectInfo.objRangeType,
             0,
             sizeof(smiNode->esmi.info.objectInfo.objRangeType));

      /* Copy the corresponding objRangeType string */
      
      if((smiType->basetype == SMI_BASETYPE_OCTETSTRING) || (smiType->basetype == SMI_BASETYPE_BITS))
            strcpy(smiNode->esmi.info.objectInfo.objRangeType,
             "NCSMIB_OCT_DISCRETE_OBJ_TYPE");
      else /* any integer type */
      strcpy(smiNode->esmi.info.objectInfo.objRangeType,
             "NCSMIB_INT_DISCRETE_OBJ_TYPE");

      smiNode->esmi.info.objectInfo.objSpecType = ESMI_OBJ_NUM_TYPE;

      strcpy(tempStr, smiNode->name);
      
      if (esmiNetSnmpOpt == ESMI_PSSV_OPT) 
         strcat(tempStr, "_pssv_values");
      else
         strcat(tempStr, "_values");

      strcpy(smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1,
             tempStr);
   }
  
   /* return the number of discrete values defined for an object */ 
   return discreteCount;
}

static int print_var_info_pssv (FILE *fd, SmiNode *grpNode, SmiNode *smiNode, int var_info_index)
{
   char    tempStr[64];
   SmiType *smiType = NULL;
   int     objTypeDefined = FALSE; 
   int     numDiscreteValues = 0;
   char    isPssv[15] = {0};
   Range *rangePtr = NULL;
   int value = 0;
  
   memset(tempStr, '\0', sizeof(tempStr));

   if (! (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)))
      return ESMI_SNMP_SUCCESS;

   smiType = smiGetNodeType(smiNode);
   if (!smiType) return ESMI_SNMP_SUCCESS;

   value = smiNode->oid[smiNode->oidlen-1];

   /* Print param_id */ 
   fprintf(fd, "\n   var_info[%u].param_id = %u;", 
                                     var_info_index, value); 

       
   /* Print offset */       
   fprintf(fd, "\n   var_info[%u].offset = 0;", var_info_index);
       
   /* Print object length */
   /* Check whether the object length is defined as a string or as an 
   integer value */

   if (smiNode->esmi.info.objectInfo.objLenType == ESMI_OBJ_STR_TYPE)
   {
         fprintf(fd, "\n   var_info[%u].len = %s;",
                 var_info_index, smiNode->esmi.info.objectInfo.objLenInfo.objLen); 
   }
   else if (smiNode->esmi.info.objectInfo.objLenType == ESMI_OBJ_NUM_TYPE) /* Length is defined as an interger value */
   {
         fprintf(fd, "\n   var_info[%u].len = %d;",
                    var_info_index, smiNode->esmi.info.objectInfo.objLenInfo.val); 
   }
   else /* object length tag is not specified. */
   {
      if((smiType->basetype == SMI_BASETYPE_OCTETSTRING) || 
         (smiType->basetype == SMI_BASETYPE_BITS)        ||
         (smiType->basetype == SMI_BASETYPE_OBJECTIDENTIFIER))
      {
         fprintf(fd, "\n   var_info[%u].len = 0;",
                       var_info_index); 
      }
      else if ((smiType->basetype == SMI_BASETYPE_UNSIGNED64) || (smiType->basetype == SMI_BASETYPE_INTEGER64))
      {
         fprintf(fd, "\n   var_info[%u].len = 8;",
                       var_info_index); 
      }
      else
      {
         fprintf(fd, "\n   var_info[%u].len = %s;",
                       var_info_index, "sizeof(uns32)"); 
      }
   }
   /* Print is_index_id flag */
   if (! esmiIsObjIndexType(grpNode, smiNode))
      fprintf(fd, "\n   var_info[%u].is_index_id = FALSE;", var_info_index); 
   else
      fprintf(fd, "\n   var_info[%u].is_index_id = TRUE;", var_info_index); 

   /* Print object access */
   switch (smiNode->access)
   {
       case SMI_ACCESS_NOT_ACCESSIBLE:
           fprintf(fd, "\n   var_info[%u].access = NCSMIB_ACCESS_NOT_ACCESSIBLE;",
                             var_info_index); 
           break;

       case SMI_ACCESS_NOTIFY:
           fprintf(fd, "\n   var_info[%u].access = NCSMIB_ACCESS_ACCESSIBLE_FOR_NOTIFY;",
                             var_info_index); 
           break;

       case SMI_ACCESS_READ_ONLY:
           fprintf(fd, "\n   var_info[%u].access = NCSMIB_ACCESS_READ_ONLY;",
                             var_info_index); 
           break;

       case SMI_ACCESS_READ_WRITE:
           if (!grpNode->create)
              fprintf(fd, "\n   var_info[%u].access = NCSMIB_ACCESS_READ_WRITE;",
                             var_info_index); 
           else
              fprintf(fd, "\n   var_info[%u].access = NCSMIB_ACCESS_READ_CREATE;", 
                             var_info_index); 
           break;

       default:
           fprintf(stderr, "\nERROR: Unrecognized access type defined for the object: %s \n", smiNode->name);
           return ESMI_SNMP_FAILURE;
   }

   /* Print object status */
   switch (smiNode->status)
   {
       case SMI_STATUS_CURRENT:
       case SMI_STATUS_MANDATORY:
           fprintf(fd, "\n   var_info[%u].status = NCSMIB_OBJ_CURRENT;", var_info_index); 
           break;

       case SMI_STATUS_DEPRECATED:
           fprintf(fd, "\n   var_info[%u].status = NCSMIB_OBJ_DEPRECATE;", var_info_index); 
           break;

       case SMI_STATUS_OBSOLETE:
           fprintf(fd, "\n   var_info[%u].status = NCSMIB_OBJ_OBSOLETE;", var_info_index); 
           break;
    
       default:
           fprintf(stderr, "\nERROR: Unrecognized status type defined for the object: %s \n", smiNode->name);
           return ESMI_SNMP_FAILURE;
   }
      
   /* Print set_when_down flag */
   if (smiNode->esmi.info.objectInfo.setWhenDown) 
      fprintf(fd, "\n   var_info[%u].set_when_down = TRUE;", var_info_index); 
   else /* default is FALSE */
      fprintf(fd, "\n   var_info[%u].set_when_down = FALSE;", var_info_index);

   /* Print fmat_id type */
   switch(smiType->basetype)
   {
       case SMI_BASETYPE_ENUM:
       case SMI_BASETYPE_INTEGER32:
       case SMI_BASETYPE_UNSIGNED32:
           fprintf(fd, "\n   var_info[%u].fmat_id = NCSMIB_FMAT_INT;", var_info_index);
           break;
       
       case SMI_BASETYPE_BITS:
       case SMI_BASETYPE_OCTETSTRING:
       case SMI_BASETYPE_INTEGER64:
       case SMI_BASETYPE_UNSIGNED64:
       case SMI_BASETYPE_OBJECTIDENTIFIER:
           fprintf(fd, "\n   var_info[%u].fmat_id = NCSMIB_FMAT_OCT;", var_info_index);
           break;

       default:
           fprintf(stderr, "\nERROR: Unrecognized base type defined for the object: %s \n", smiNode->name);
           return ESMI_SNMP_FAILURE;
   }

   numDiscreteValues = 0;
   numDiscreteValues = esmiCheckIsDiscreteObject(smiNode);
       
   /* Print object (range) type */
   if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, tempStr))
   {
          objTypeDefined = TRUE;

          fprintf(fd, "\n   var_info[%u].obj_type = %s;",
                    var_info_index, smiNode->esmi.info.objectInfo.objRangeType);
   }
   else  /* Take the DEFAULT values based on the object base type */
   {
      objTypeDefined = FALSE;

      switch(smiType->basetype)
      {
          case SMI_BASETYPE_BITS:
          case SMI_BASETYPE_OCTETSTRING:
          case SMI_BASETYPE_INTEGER64:
          case SMI_BASETYPE_UNSIGNED64:
          case SMI_BASETYPE_OBJECTIDENTIFIER:
              /* Print object (range) type */
              fprintf(fd, "\n   var_info[%u].obj_type = NCSMIB_OCT_OBJ_TYPE;", var_info_index);
              break;

          default: /* Default is considered to be as INT_RANGE_TYPE */
              /* Print object (range) type */
              fprintf(fd, "\n   var_info[%u].obj_type = NCSMIB_INT_RANGE_OBJ_TYPE;", var_info_index);
              break;
      }
   }

   /* Print object specification type */
   switch (smiNode->esmi.info.objectInfo.objSpecType)
   {
       case ESMI_OBJ_STR_TYPE:
           if(! objTypeDefined) 
           {
              switch(smiType->basetype)
              {
              case SMI_BASETYPE_BITS:
              case SMI_BASETYPE_OCTETSTRING:
                  fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.min_len = %s;",
                          var_info_index,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);

                  fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.max_len = %s;",
                          var_info_index,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str2);
                  break;

              case SMI_BASETYPE_INTEGER64:
              case SMI_BASETYPE_UNSIGNED64:
                  fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.min_len = 8;",
                             var_info_index); 
                  fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.max_len = 8;",
                             var_info_index); 
                  break;

              default: /* Default is considered to be as INT_RANGE_TYPE */
                  fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.min = %s;", 
                          var_info_index,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);

                  fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.max = %s;",
                          var_info_index, 
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str2);
                  break;
              }
           }
           else
           if (! strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_INT_RANGE_OBJ_TYPE"))
           {
              fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.min = %s;", 
                     var_info_index,  smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
              fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.max = %s;",
                     var_info_index, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str2);
           }
           else  if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_INT_DISCRETE_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%u].obj_spec.value_spec.num_values = %s;",
                          var_info_index, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
              fprintf(fd, "\n   var_info[%u].obj_spec.value_spec.values = %s;",
                         var_info_index, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str2);
           }
           else  if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_OCT_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.min_len = %s;",
                            var_info_index, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
              fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.max_len = %s;",
                            var_info_index, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str2);
           }
           else  if ((strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_OTHER_OCT_OBJ_TYPE") == 0) ||
                     (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_OTHER_INT_OBJ_TYPE") == 0))
           {
                     fprintf(fd, "\n   var_info[%u].obj_spec.validate_obj = %s;",
                            var_info_index, 
                            smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
           }
           else  /* Invalid input for obj_type (objRangeType), don't generate stub */
              return ESMI_SNMP_FAILURE;

           break;

       case ESMI_OBJ_NUM_TYPE:
           if(! objTypeDefined) 
           {
              switch(smiType->basetype)
              {
              case SMI_BASETYPE_BITS:
              case SMI_BASETYPE_OCTETSTRING:
                  fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.min_len = %d;",
                          var_info_index,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val1);

                  fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.max_len = %d;",
                          var_info_index,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val2);
                  break;

              case SMI_BASETYPE_INTEGER64:
              case SMI_BASETYPE_UNSIGNED64:
                  fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.min_len = 8;",
                             var_info_index); 
                  fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.max_len = 8;",
                             var_info_index); 
                  break;

              default: /* Default is considered to be as INT_RANGE_TYPE */
                  fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.min = %d;", 
                          var_info_index,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val1);

                  fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.max = %d;",
                          var_info_index,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val2);
                  break;
              }
           }
           else
           if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_INT_RANGE_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.min = %d;", 
                         var_info_index, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val1);
              fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.max = %d;", 
                         var_info_index, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val2);
           }
           else
           if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_INT_DISCRETE_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%u].obj_spec.value_spec.num_values = %d;",
                           var_info_index, numDiscreteValues);
              fprintf(fd, "\n   var_info[%u].obj_spec.value_spec.values = %s;",
                           var_info_index,  smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
           }
           else
           if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_OCT_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.min_len = %d;",
                          var_info_index,  smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val1);
              fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.max_len = %d;",
                          var_info_index, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val2);
           }
           else
           if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_OCT_DISCRETE_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%u].obj_spec.disc_stream_spec.num_values = %d;",
                           var_info_index, numDiscreteValues);
              fprintf(fd, "\n   var_info[%u].obj_spec.disc_stream_spec.values = %s;",
                           var_info_index,  smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
           }
           else  /* Invalid input for obj_type (objRangeType), don't generate stub */
              return ESMI_SNMP_FAILURE;

           break;

       default: /* If not specified thru ESMI tags, 
                   then take the range values from the object definitions */
          { 
             Object *obj = (Object *)smiNode;
             rangePtr = getRangeForType(obj->typePtr);

             switch(smiType->basetype)
             {
             case SMI_BASETYPE_BITS:
             case SMI_BASETYPE_OCTETSTRING:
                 if (rangePtr)
                 {
                    fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.min_len = %d;",
                          var_info_index, rangePtr->export.minValue.value.integer32);
                    fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.max_len = %d;",
                          var_info_index, rangePtr->export.maxValue.value.integer32);
                 }
                 else
                 {  /* listPtr of typePtr is NULL, so get the min and max lengths from its parentType's listPtr. */
                    fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.min_len = 0;",
                          var_info_index);
                    fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.max_len = 0xffff;",
                          var_info_index); 
                 }
                 break;

             case SMI_BASETYPE_INTEGER64:
             case SMI_BASETYPE_UNSIGNED64:
                 fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.min_len = 8;",
                          var_info_index);
                 fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.max_len = 8;",
                          var_info_index);
                 break;
                 
             case SMI_BASETYPE_INTEGER32:
                 if (rangePtr)
                 {
                    if (var_info_index, rangePtr->export.minValue.value.integer32 == 0x80000000)
                       fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.min = -2147483647 - 1;", var_info_index); 
                    else
                       fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.min = %d;", 
                            var_info_index, rangePtr->export.minValue.value.integer32);
                    if (var_info_index, rangePtr->export.maxValue.value.integer32 == 0x80000000)
                       fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.max = -2147483647 - 1;", var_info_index); 
                    else
                       fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.max = %d;", 
                            var_info_index, rangePtr->export.maxValue.value.integer32);
                 }
                 else
                 {  /* Frankly speaking min value should be -2147483648 */
                    fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.min = -2147483647 - 1;",
                          var_info_index);
                    fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.max = 2147483647;",
                          var_info_index); 
                 }
                break;

             case SMI_BASETYPE_OBJECTIDENTIFIER:
                fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.min_len = %d;",
                      var_info_index, 0);
                fprintf(fd, "\n   var_info[%u].obj_spec.stream_spec.max_len = %d;",
                      var_info_index, 512);
                break;

             default: /* Default is considered to be as INT_RANGE_TYPE */
                 if (rangePtr)
                 {
                    fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.min = 0x%x;", 
                         var_info_index, rangePtr->export.minValue.value.unsigned32);
                    fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.max = 0x%x;", 
                         var_info_index,  rangePtr->export.maxValue.value.unsigned32);
                 }
                 else
                 {
                    fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.min = 0;", 
                         var_info_index);
                    fprintf(fd, "\n   var_info[%u].obj_spec.range_spec.max = 0xffffffff;", 
                         var_info_index);
                 }
                 break;
             }
          }
          break;
   }
   
   /* Print is_persistent flag */ 
   if (smiNode->esmi.info.objectInfo.isPersistent)
         fprintf(fd, "\n   var_info[%u].is_readonly_persistent = TRUE;", var_info_index); 
   else
         fprintf(fd, "\n   var_info[%u].is_readonly_persistent = FALSE;", var_info_index); 

   /* print objects MIB name, only for PSSv driver */
   fprintf(fd, "\n   var_info[%u].mib_obj_name = \"%s\";", var_info_index, smiNode->name); 

   return ESMI_SNMP_SUCCESS;

}  /* End of print_var_info () */   
  
static int print_var_info_miblib (FILE *fd, SmiNode *grpNode, SmiNode *smiNode)
{
   char    tempStr[64];
   SmiType *smiType = NULL;
   int     objTypeDefined = FALSE; 
   int     numDiscreteValues = 0;
   char    isPssv[15] = {0};
   Range *rangePtr = NULL;
  
   memset(tempStr, '\0', sizeof(tempStr));

   if (esmiNetSnmpOpt == ESMI_PSSV_OPT) 
      strcpy(isPssv, "pssv_ID"); 
   else
      strcpy(isPssv, "ID");


       if (! (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)))
          return ESMI_SNMP_SUCCESS;

       smiType = smiGetNodeType(smiNode);
       if (!smiType) return ESMI_SNMP_SUCCESS;

       fprintf(fd, "\n\n   /* %s_%s */", smiNode->name, isPssv); 
       /* Print param_id */ 
       fprintf(fd, "\n   var_info[%s_%s - 1].param_id = %s_%s;", 
                                         smiNode->name, isPssv, smiNode->name, isPssv); 

       
       /* Print offset */       
       if (esmiNetSnmpOpt == ESMI_PSSV_OPT) 
       {
          fprintf(fd, "\n   var_info[%s_pssv_ID - 1].offset = 0;", smiNode->name);
       }
       else 
       {
          if((smiNode->status != SMI_STATUS_OBSOLETE) &&
             (strcmp(smiNode->esmi.info.objectInfo.baseAddress, ESMI_NULL_STR)))
          {
             fprintf(fd, "\n   var_info[%s_ID - 1].offset = "
                         "\n            (uns16)((long)&%s->%s - (long)%s);", 
                                      smiNode->name,
                                      smiNode->esmi.info.objectInfo.baseAddress,
                                      smiNode->esmi.info.objectInfo.member,
                                      smiNode->esmi.info.objectInfo.baseAddress);
          }
          else
          {
             fprintf(fd, "\n   var_info[%s_ID - 1].offset = 0;", smiNode->name);
          }
       }
       
       /* Print object length */
       /* Check whether the object length is defined as a string or as an 
          integer value */

       if (smiNode->esmi.info.objectInfo.objLenType == ESMI_OBJ_STR_TYPE)
       {
          fprintf(fd, "\n   var_info[%s_%s - 1].len = %s;",
                    smiNode->name, isPssv, smiNode->esmi.info.objectInfo.objLenInfo.objLen); 
       }
       else if (smiNode->esmi.info.objectInfo.objLenType == ESMI_OBJ_NUM_TYPE) /* Length is defined as an interger value */
       {
          fprintf(fd, "\n   var_info[%s_%s - 1].len = %d;",
                    smiNode->name, isPssv, smiNode->esmi.info.objectInfo.objLenInfo.val); 
       }
       else /* object length tag is not specified. */
       {
          if((smiType->basetype == SMI_BASETYPE_OCTETSTRING) || 
             (smiType->basetype == SMI_BASETYPE_BITS)        ||
             (smiType->basetype == SMI_BASETYPE_OBJECTIDENTIFIER))
          {
             fprintf(fd, "\n   var_info[%s_%s - 1].len = 0;",
                       smiNode->name, isPssv); 
          }
          else if ((smiType->basetype == SMI_BASETYPE_UNSIGNED64) || (smiType->basetype == SMI_BASETYPE_INTEGER64))
          {
             fprintf(fd, "\n   var_info[%s_%s - 1].len = 8;",
                       smiNode->name, isPssv); 
          }
          else
          {
             fprintf(fd, "\n   var_info[%s_%s - 1].len = %s;",
                       smiNode->name, isPssv, "sizeof(uns32)"); 
          }
       }
       /* Print is_index_id flag */
       if (! esmiIsObjIndexType(grpNode, smiNode))
          fprintf(fd, "\n   var_info[%s_%s - 1].is_index_id = FALSE;", smiNode->name, isPssv); 
       else
          fprintf(fd, "\n   var_info[%s_%s - 1].is_index_id = TRUE;", smiNode->name, isPssv); 

       /* Print object access */
       switch (smiNode->access)
       {
       case SMI_ACCESS_NOT_ACCESSIBLE:
           fprintf(fd, "\n   var_info[%s_%s - 1].access = NCSMIB_ACCESS_NOT_ACCESSIBLE;",
                             smiNode->name, isPssv); 
           break;

       case SMI_ACCESS_NOTIFY:
           fprintf(fd, "\n   var_info[%s_%s - 1].access = NCSMIB_ACCESS_ACCESSIBLE_FOR_NOTIFY;",
                             smiNode->name, isPssv); 
           break;

       case SMI_ACCESS_READ_ONLY:
           fprintf(fd, "\n   var_info[%s_%s - 1].access = NCSMIB_ACCESS_READ_ONLY;",
                             smiNode->name, isPssv); 
           break;

       case SMI_ACCESS_READ_WRITE:
           if (!grpNode->create)
              fprintf(fd, "\n   var_info[%s_%s - 1].access = NCSMIB_ACCESS_READ_WRITE;",
                             smiNode->name, isPssv); 
           else
              fprintf(fd, "\n   var_info[%s_%s - 1].access = NCSMIB_ACCESS_READ_CREATE;", 
                             smiNode->name, isPssv); 
           break;

       default:
           fprintf(stderr, "\nERROR: Unrecognized access type defined for the object: %s \n", smiNode->name);
           return ESMI_SNMP_FAILURE;
       }

       /* Print object status */
       switch (smiNode->status)
       {
       case SMI_STATUS_CURRENT:
           fprintf(fd, "\n   var_info[%s_%s - 1].status = NCSMIB_OBJ_CURRENT;", smiNode->name, isPssv); 
           break;

       case SMI_STATUS_DEPRECATED:
           fprintf(fd, "\n   var_info[%s_%s - 1].status = NCSMIB_OBJ_DEPRECATE;", smiNode->name, isPssv); 
           break;

       case SMI_STATUS_OBSOLETE:
           fprintf(fd, "\n   var_info[%s_%s - 1].status = NCSMIB_OBJ_OBSOLETE;", smiNode->name, isPssv); 
           break;
    
       default:
           fprintf(stderr, "\nERROR: Unrecognized status type defined for the object: %s \n", smiNode->name);
           return ESMI_SNMP_FAILURE;
       }
      
       /* Print set_when_down flag */
       if (smiNode->esmi.info.objectInfo.setWhenDown) 
          fprintf(fd, "\n   var_info[%s_%s - 1].set_when_down = TRUE;", smiNode->name, isPssv); 
       else /* default is FALSE */
          fprintf(fd, "\n   var_info[%s_%s - 1].set_when_down = FALSE;", smiNode->name, isPssv);

       /* Print fmat_id type */
       switch(smiType->basetype)
       {
       case SMI_BASETYPE_ENUM:
       case SMI_BASETYPE_INTEGER32:
       case SMI_BASETYPE_UNSIGNED32:
           fprintf(fd, "\n   var_info[%s_%s - 1].fmat_id = NCSMIB_FMAT_INT;", smiNode->name, isPssv);
           break;
       
       case SMI_BASETYPE_BITS:
       case SMI_BASETYPE_OCTETSTRING:
       case SMI_BASETYPE_INTEGER64:
       case SMI_BASETYPE_UNSIGNED64:       
       case SMI_BASETYPE_OBJECTIDENTIFIER:
           fprintf(fd, "\n   var_info[%s_%s - 1].fmat_id = NCSMIB_FMAT_OCT;", smiNode->name, isPssv);
           break;

       default:
           fprintf(stderr, "\nERROR: Unrecognized base type defined for the object: %s \n", smiNode->name);
           return ESMI_SNMP_FAILURE;
       }

       numDiscreteValues = 0;
       numDiscreteValues = esmiCheckIsDiscreteObject(smiNode);
       
       /* Print object (range) type */
       if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, tempStr))
       {
          objTypeDefined = TRUE;

          fprintf(fd, "\n   var_info[%s_%s - 1].obj_type = %s;",
                    smiNode->name, isPssv, smiNode->esmi.info.objectInfo.objRangeType);
       }
       else  /* Take the DEFAULT values based on the object base type */
       {
          objTypeDefined = FALSE;

          switch(smiType->basetype)
          {
          case SMI_BASETYPE_BITS:
          case SMI_BASETYPE_OCTETSTRING:
          case SMI_BASETYPE_INTEGER64:
          case SMI_BASETYPE_UNSIGNED64:
          case SMI_BASETYPE_OBJECTIDENTIFIER:
              /* Print object (range) type */
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_type = NCSMIB_OCT_OBJ_TYPE;", smiNode->name, isPssv);
              break;

          default: /* Default is considered to be as INT_RANGE_TYPE */
              /* Print object (range) type */
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_type = NCSMIB_INT_RANGE_OBJ_TYPE;", smiNode->name, isPssv);
              break;
          }
       }

       /* Print object specification type */
       switch (smiNode->esmi.info.objectInfo.objSpecType)
       {
       case ESMI_OBJ_STR_TYPE:
           if(! objTypeDefined) 
           {
              switch(smiType->basetype)
              {
              case SMI_BASETYPE_BITS:
              case SMI_BASETYPE_OCTETSTRING:
                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.min_len = %s;",
                          smiNode->name, isPssv,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);

                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.max_len = %s;",
                          smiNode->name, isPssv,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str2);
                  break;

              case SMI_BASETYPE_INTEGER64:
              case SMI_BASETYPE_UNSIGNED64:
                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.min_len = 8;",
                             smiNode->name, isPssv); 
                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.max_len = 8;",
                             smiNode->name, isPssv); 
                  break;

              default: /* Default is considered to be as INT_RANGE_TYPE */
                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.min = %s;", 
                          smiNode->name, isPssv,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);

                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.max = %s;",
                          smiNode->name,isPssv, 
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str2);
                  break;
              }
           }
           else
           if (! strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_INT_RANGE_OBJ_TYPE"))
           {
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.min = %s;", 
                     smiNode->name, isPssv,  smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.max = %s;",
                     smiNode->name,  isPssv, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str2);
           }
           else  if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_INT_DISCRETE_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.value_spec.num_values = %s;",
                          smiNode->name, isPssv, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.value_spec.values = %s;",
                         smiNode->name,  isPssv, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str2);
           }
           else  if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_OCT_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.min_len = %s;",
                            smiNode->name, isPssv, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.max_len = %s;",
                            smiNode->name, isPssv, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str2);
           }
           else  if ((strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_OTHER_OCT_OBJ_TYPE") == 0) ||
                     (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_OTHER_INT_OBJ_TYPE") == 0))
           {
                     fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.validate_obj = %s;",
                            smiNode->name,isPssv, 
                            smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
           }
           else  /* Invalid input for obj_type (objRangeType), don't generate stub */
              return ESMI_SNMP_FAILURE;

           break;

       case ESMI_OBJ_NUM_TYPE:
           if(! objTypeDefined) 
           {
              switch(smiType->basetype)
              {
              case SMI_BASETYPE_BITS:
              case SMI_BASETYPE_OCTETSTRING:
                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.min_len = %d;",
                          smiNode->name, isPssv,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val1);

                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.max_len = %d;",
                          smiNode->name, isPssv,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val2);
                  break;

              case SMI_BASETYPE_INTEGER64:
              case SMI_BASETYPE_UNSIGNED64:
                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.min_len = 8;",
                             smiNode->name, isPssv); 
                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.max_len = 8;",
                             smiNode->name, isPssv); 
                  break;

              default: /* Default is considered to be as INT_RANGE_TYPE */
                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.min = %d;", 
                          smiNode->name, isPssv,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val1);

                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.max = %d;",
                          smiNode->name, isPssv,
                          smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val2);
                  break;
              }
           }
           else
           if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_INT_RANGE_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.min = %d;", 
                         smiNode->name,isPssv, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val1);
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.max = %d;", 
                         smiNode->name,isPssv, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val2);
           }
           else
           if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_INT_DISCRETE_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.value_spec.num_values = %d;",
                           smiNode->name, isPssv, numDiscreteValues);
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.value_spec.values = %s;",
                           smiNode->name,isPssv,  smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
           }
           else
           if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_OCT_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.min_len = %d;",
                          smiNode->name,isPssv,  smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val1);
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.max_len = %d;",
                          smiNode->name,isPssv, smiNode->esmi.info.objectInfo.objSpecInfo.objSpecVal.val2);
           }
           else
           if (strcmp(smiNode->esmi.info.objectInfo.objRangeType, 
                     "NCSMIB_OCT_DISCRETE_OBJ_TYPE") == 0)
           {
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.disc_stream_spec.num_values = %d;",
                           smiNode->name, isPssv, numDiscreteValues);
              fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.disc_stream_spec.values = %s;",
                           smiNode->name,isPssv,  smiNode->esmi.info.objectInfo.objSpecInfo.objSpecStr.str1);
           }
           else  /* Invalid input for obj_type (objRangeType), don't generate stub */
              return ESMI_SNMP_FAILURE;

           break;

       default: /* If not specified thru ESMI tags, 
                   then take the range values from the object definitions */
          { 
             Object *obj = (Object *)smiNode;
             rangePtr = getRangeForType(obj->typePtr);

             switch(smiType->basetype)
             {
             case SMI_BASETYPE_BITS:
             case SMI_BASETYPE_OCTETSTRING:
                 if (rangePtr)
                 {
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.min_len = %d;",
                          smiNode->name, isPssv, rangePtr->export.minValue.value.integer32);
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.max_len = %d;",
                          smiNode->name, isPssv, rangePtr->export.maxValue.value.integer32);
                 }
                 else
                 {  /* listPtr of typePtr is NULL, so get the min and max lengths from its parentType's listPtr. */
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.min_len = 0;",
                          smiNode->name, isPssv);
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.max_len = 0xffff;",
                          smiNode->name, isPssv); 
                 }
                 break;

             case SMI_BASETYPE_INTEGER64:
             case SMI_BASETYPE_UNSIGNED64:
                 fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.min_len = 8;",
                          smiNode->name, isPssv);
                 fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.max_len = 8;",
                          smiNode->name, isPssv);
                 break;
                 
             case SMI_BASETYPE_INTEGER32:
                 if (rangePtr)
                 {
                    if (rangePtr->export.minValue.value.integer32 == 0x80000000)
                       fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.min = -2147483647 - 1;", 
                            smiNode->name, isPssv);
                    else
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.min = %d;", 
                         smiNode->name, isPssv, rangePtr->export.minValue.value.integer32);

                    if (rangePtr->export.maxValue.value.integer32 == 0x80000000)
                       fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.max = -2147483647 - 1;", 
                            smiNode->name, isPssv);
                    else
                       fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.max = %d;", 
                            smiNode->name, isPssv, rangePtr->export.maxValue.value.integer32);
                 }
                 else
                 {  /* Frankly speaking min value should be -2147483648 */
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.min = -2147483647 - 1;",
                          smiNode->name, isPssv);
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.max = 2147483647;",
                          smiNode->name, isPssv); 
                 }
                 break;

              case SMI_BASETYPE_OBJECTIDENTIFIER:
                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.min_len = %d;",
                          smiNode->name, isPssv, 0);

                  fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.stream_spec.max_len = %d;",
                          smiNode->name, isPssv, 512);
                          
                  break; 

             default: /* Default is considered to be as INT_RANGE_TYPE */
                 if (rangePtr)
                 {
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.min = 0x%x;", 
                         smiNode->name, isPssv, rangePtr->export.minValue.value.unsigned32);
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.max = 0x%x;", 
                         smiNode->name, isPssv,  rangePtr->export.maxValue.value.unsigned32);
                 }
                 else
                 {
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.min = 0;", 
                         smiNode->name, isPssv);
                    fprintf(fd, "\n   var_info[%s_%s - 1].obj_spec.range_spec.max = 0xffffffff;", 
                         smiNode->name, isPssv);
                 }
                 break;
             }
          }
          break;
       }
   
      /* Print is_persistent flag */ 
      if (smiNode->esmi.info.objectInfo.isPersistent)
         fprintf(fd, "\n   var_info[%s_%s - 1].is_readonly_persistent = TRUE;", smiNode->name, isPssv); 
      else
         fprintf(fd, "\n   var_info[%s_%s - 1].is_readonly_persistent = FALSE;", smiNode->name, isPssv); 

      /* print objects MIB name, only for PSSv driver */
      fprintf(fd, "\n   var_info[%s_%s - 1].mib_obj_name = \"%s\";", smiNode->name, isPssv,smiNode->name); 

   return ESMI_SNMP_SUCCESS;

}  /* End of print_var_info () */   
  
/**************************************************************************
Name          : printMibLibOrPssvObjInit
                                                                                
Description   : This function prints the objects init routine of a 
                table into the generated MIBLIB stub.
                                                                                
Arguments     : fd - File descriptor of the file to which the code  
                     should be dumped.
                grpNode - Pointer to the SmiNode.
                objCount - Number of objects present in this table 
                                                                                
Return Values : Nothing
                                                                                
Notes         :
**************************************************************************/
static int  printMibLibOrPssvObjInit(FILE *fd, SmiNode *grpNode, int objCount)
{ 
   SmiNode *smiNode = NULL;
   Object *objPtr = (Object *) grpNode, *parentObjPtr = NULL;
   List *listPtr = NULL;
   int status = ESMI_SNMP_SUCCESS;
   int nonLocalIndices = 0, localObjects = 0, i;
   char *tblNameSmall = translateLower (grpNode->name);

   /* Print the function descriptive header */ 
   printObjInitHdr(fd, grpNode->name);

   /* From grpNode, get the indexes imported/augmented from another table and print var_info for them */
   if (esmiNetSnmpOpt == ESMI_PSSV_OPT)
   {
      if (grpNode->indexkind == SMI_INDEX_AUGMENT)
      {
         /* Related ptr in Object points to the table from which this table is augmented. */
         objPtr = objPtr->relatedPtr;
         if (objPtr->export.indexkind == SMI_INDEX_INDEX)
         {
            for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr)
            {
               parentObjPtr = ((Object *)listPtr->ptr)->nodePtr->parentPtr->firstObjectPtr;
               status = print_var_info_pssv (fd, &(parentObjPtr->export), &(((Object *)listPtr->ptr)->export), nonLocalIndices); 
               if (status != ESMI_SNMP_SUCCESS)
                  return status;
               nonLocalIndices++;
            }
         }
         /* Else, Table from which we have augmented doesn't have SMI_INDEX_INDEX...*/
      }
      else if (grpNode->indexkind == SMI_INDEX_INDEX)
      {
         for (listPtr = objPtr->listPtr; listPtr; listPtr = listPtr->nextPtr)
         {
            parentObjPtr = ((Object *)listPtr->ptr)->nodePtr->parentPtr->firstObjectPtr;
            if (parentObjPtr == objPtr)
               continue; /* Index is of the local table, don't print var_info now. */

            status = print_var_info_pssv (fd, &(parentObjPtr->export), &(((Object *)listPtr->ptr)->export), nonLocalIndices); 
            if (status != ESMI_SNMP_SUCCESS)
               return status;
            nonLocalIndices++;
         }
 
      }
   }

   /* Traverse through all of the objects in the table */
   for (smiNode = smiGetFirstChildNode(grpNode);
        smiNode;
        smiNode = smiGetNextChildNode(smiNode))
   {
      if (esmiNetSnmpOpt == ESMI_PSSV_OPT)
      {
         if (nonLocalIndices == 0) /* (scalars) or tables having only local indices */
         status = print_var_info_pssv (fd, grpNode, smiNode, smiNode->oid[smiNode->oidlen-1] - 1); 
         else
         status = print_var_info_pssv (fd, grpNode, smiNode, nonLocalIndices+localObjects);
      }
      else
         status = print_var_info_miblib (fd, grpNode, smiNode);

      if (status != ESMI_SNMP_SUCCESS)
         return status;
      localObjects++;
   }

   fprintf(fd, "\n\n   return;\n}"); 

   if (nonLocalIndices != 0)
   {
      /* Print the array of local objects located in var_info */
      fprintf (fd, "\n\nstatic uns16 %s_pssv_objects_local_to_tbl[%u] = { %u", tblNameSmall, localObjects+1, localObjects);
      for (i = 0; i < localObjects; i++)
      {
         fprintf (fd, ", %u", nonLocalIndices+i);
      }
      fprintf (fd, " }; \n\n");
   }
   return ESMI_SNMP_SUCCESS;
}

/**************************************************************************
Name          :  printMibLibOrPssvTblInit
                                                                                
Description   : This function prints the table initialization routine into 
                the generated MIBLIB stub.
                                                                                
Arguments     : fd - File descriptor of the file to which the code should 
                be dumped.
                grpNode - Pointer to the SmiNode.
                objCount - Number of objects present in this table 
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static int  printMibLibOrPssvTblInit(FILE *fd, SmiNode *grpNode, int objCount)
{
   char *tblNameSmall;
   char *tblNameBig;
   char *tblName;
   int  indexExists = ESMI_SNMP_FAILURE;
   int  indexCount = 0;
   SmiNode *rowStatusNode = NULL;


   /* Convert the table name to small & CAPITAL letters string and conver
      '-' to '_' if they exists */
   tblNameSmall = translateLower(grpNode->name);
   tblNameBig   = translateUpper(grpNode->name);
   tblName = esmiTranslateHungarian(grpNode->name);
    
   /* Print the function header with appropriate information */
   fprintf(fd, "\n\n\n/**************************************************************************"
               "\nName          :  %s_%s"
               "\n\nDescription   :  Initialize  %s  table" 
               "\n\nArguments     :  tbl_info - Pointer to the NCSMIB_TBL_INFO structure" 
               "\n\nReturn values :  Nothing"
               "\n\nNotes         :     "
               "\n**************************************************************************/", 
                tblNameSmall, (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_tbl_init":"tbl_init", tblNameSmall);

   /* Print the function */
   fprintf(fd, "\nstatic void  %s_%s(NCSMIB_TBL_INFO *tbl_info)\n{", tblNameSmall, 
           (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"pssv_tbl_init":"tbl_init");

   fprintf(fd, "\n   m_NCS_OS_MEMSET(tbl_info, 0, sizeof(NCSMIB_TBL_INFO));"); 

   /* Check if the Index exists in this table */
   if (esmiCheckIndexInThisTable(grpNode, &indexCount) == ESMI_SNMP_SUCCESS)
   {
      indexExists = ESMI_SNMP_SUCCESS;

      if (esmiNetSnmpOpt == ESMI_MIBLIB_OPT)
      {
         fprintf(fd, "\n\n   /* Indices for %s table */", tblNameSmall);
         fprintf(fd, "\n   NCSMIB_INDEX_PARAM_IDS  index_data;");
         fprintf(fd, "\n   /* Initialize the index information */");
         fprintf(fd, "\n   index_data.num_index_ids = %d;", indexCount);
         fprintf(fd, "\n   index_data.index_param_ids = %s_indices;\n\n", tblNameSmall);
      }
   }

   fprintf(fd, "\n\n   /* Update the table info */");

   /* Print the id of the table */   
   fprintf(fd, "\n   tbl_info->table_id = %s;", grpNode->esmi.info.tableInfo.tableId);

   /* Print the version of the table */
   if (grpNode->esmi.info.tableInfo.tableVersion == 0)
   {
      /* dump the default value */
      fprintf(fd, "\n   tbl_info->table_version = %d;", 1);
   }
   else
   {
      fprintf(fd, "\n   tbl_info->table_version = %d;", grpNode->esmi.info.tableInfo.tableVersion);
   }

   /* Print the rank of the table */
   if (grpNode->esmi.info.tableInfo.psrRank[0] == 0)
   {
      /* dump the default value */
      fprintf(fd, "\n   tbl_info->table_rank = %s;", "NCSMIB_TBL_RANK_DEFAULT");
   }
   else
   {
      fprintf(fd, "\n   tbl_info->table_rank = %s;", grpNode->esmi.info.tableInfo.psrRank);
   }

   /* Print the is_static flag */
   if (! grpNode->esmi.info.tableInfo.isStatic)
      fprintf(fd, "\n   tbl_info->is_static = FALSE;");
   else /* default is FALSE */
      fprintf(fd, "\n   tbl_info->is_static = TRUE;");

   /* Print the is_persistent flag */
   if (! grpNode->esmi.info.tableInfo.isPersistent)
      fprintf(fd, "\n   tbl_info->is_persistent = FALSE;");
   else /* default is FALSE */
      fprintf(fd, "\n   tbl_info->is_persistent = TRUE;");

   /* Print the is sparse_table flag */
   if (! grpNode->esmi.info.tableInfo.isSparseTbl)
      fprintf(fd, "\n   tbl_info->sparse_table = FALSE;");
   else /* default is FALSE */
      fprintf(fd, "\n   tbl_info->sparse_table = TRUE;");
      
   /* Print the table_of_scalars flag */
   if (grpNode->indexkind == SMI_INDEX_UNKNOWN)
      fprintf(fd, "\n   tbl_info->table_of_scalars = TRUE;");
   else
      fprintf(fd, "\n   tbl_info->table_of_scalars = FALSE;");

   /* Print index_in_this_tbl */
   if (indexExists)
   {
      fprintf(fd, "\n   tbl_info->index_in_this_tbl = TRUE;");
      if (esmiNetSnmpOpt == ESMI_MIBLIB_OPT)
         fprintf(fd, "\n   tbl_info->idx_data.index_info = index_data;");
   }
   else
   {
      fprintf(fd, "\n   tbl_info->index_in_this_tbl = FALSE;");
      if ((grpNode->indexkind != SMI_INDEX_UNKNOWN) && (esmiNetSnmpOpt == ESMI_MIBLIB_OPT))
      {
         fprintf(fd, "\n   tbl_info->idx_data.validate_idx = %s_verify_instance;", tblNameSmall);
      }
   }

   /* Print num_objects of the table */
   fprintf(fd, "\n   tbl_info->num_objects = %s_%s;", tblNameBig,
               (esmiNetSnmpOpt == ESMI_PSSV_OPT)?"PSSV_TBL_MAX_PARAMS":"TBL_MAX_PARAMS");

    /* Get the smiNode of row status type */
    rowStatusNode = esmiGetRowStatusNode(grpNode);

    if (rowStatusNode != NULL)
    {
         /* Print status_field of the table */
         if (esmiNetSnmpOpt == ESMI_PSSV_OPT)
         {
            fprintf(fd, "\n   tbl_info->status_field = %d;", rowStatusNode->oid[rowStatusNode->oidlen-1]);
         }
         else
         {
            fprintf(fd, "\n   tbl_info->status_field = %s_ID;", rowStatusNode->name);
         }

        /* Following statements are onlu required for NCS MibLib */
        if (esmiNetSnmpOpt != ESMI_PSSV_OPT) 
        {
             if (strcmp(rowStatusNode->esmi.info.objectInfo.baseAddress, ESMI_NULL_STR))
             {
                /* Print status_offset of the table */
                fprintf(fd, "\n   tbl_info->status_offset = "
                            "\n            (uns16)((long)&%s->%s - (long)%s);",
                                      rowStatusNode->esmi.info.objectInfo.baseAddress,
                                      rowStatusNode->esmi.info.objectInfo.member,
                                      rowStatusNode->esmi.info.objectInfo.baseAddress); 
             }
             else
             {
                fprintf(fd, "\n   tbl_info->status_offset = 0;");
             }
        }
    }

   if (esmiRowInOneDataStruct(grpNode))
   {
      /* Print row_in_one_data_struct flag */
      fprintf(fd, "\n\n   /* All row objects are in one data structure */"); 
      fprintf(fd, "\n   tbl_info->row_in_one_data_struct = TRUE;");
   }
   else
   {
      /* Print row_in_one_data_struct flag */
      fprintf(fd, "\n\n   /* All row objects are not in one data structure */"); 
      fprintf(fd, "\n   tbl_info->row_in_one_data_struct = FALSE;");
   }

   /* Following statements are onlu required for NCS MibLib */
   if (esmiNetSnmpOpt != ESMI_PSSV_OPT) 
   {
      fprintf(fd, "\n\n   /* %s TABLE's GET/SET/NEXT/.. function pointers */", tblNameSmall); 

      /* Print SET routine */
      fprintf(fd, "\n   tbl_info->set     = %s_set;", tblNameSmall);

      /* Print GET routine */
      fprintf(fd, "\n   tbl_info->get     = %s_get;", tblNameSmall);
               
      /* Print EXTRACT routine */
      fprintf(fd, "\n   tbl_info->extract = %s_extract;", tblNameSmall);

      /* Print NEXT routine */
      fprintf(fd, "\n   tbl_info->next    = %s_next;", tblNameSmall);

      /* Print SETROW routine */
      fprintf(fd, "\n   tbl_info->setrow  = %s_setrow;", tblNameSmall);

      /* generate this, if there is no ROW-STATUS object in this table */ 
      /* table is requested to be persistent */ 
      if ((rowStatusNode == NULL) &&
          (grpNode->esmi.info.tableInfo.isPersistent == TRUE) && (grpNode->indexkind != SMI_INDEX_AUGMENT))
      {
          /* Print RMVROW routine */
          fprintf(fd, "\n   tbl_info->rmvrow  = %s_rmvrow;", tblNameSmall);

      }
   }

   /* print table name given in mib for PSSv driver */
   if (esmiNetSnmpOpt == ESMI_PSSV_OPT)
   {
      /* TBD Print the MIB module name */ 
      fprintf(fd, "\n\n   /* initialize the PSS related stuff for this table */");
      fprintf(fd, "\n   tbl_info->mib_tbl_name = \"%s\";", grpNode->name);
      fprintf(fd, "\n   tbl_info->base_oid = __%s_base_pssv;", tblName);
      fprintf(fd, "\n   tbl_info->base_oid_len = sizeof(__%s_base_pssv)/sizeof(uns32);", tblName);
      if (grpNode->esmi.info.tableInfo.capability[0] == 0)
      {
         /* dump the default value */ 
         fprintf(fd, "\n   tbl_info->capability = %s;", "NCSMIB_CAPABILITY_SETROW");
      }
      else
         fprintf(fd, "\n   tbl_info->capability = %s;", grpNode->esmi.info.tableInfo.capability);
   }
   
   /* print return and closing brace of the function */
   fprintf(fd, "\n\n   return;\n}");

   /* Free the alloted memory for table name in small/CAPITAL letters */ 
   xfree(tblNameSmall);
   xfree(tblNameBig);
   xfree(tblName);

   return ESMI_SNMP_SUCCESS;
}


/**************************************************************************
Name          : printMibLibTblRegister
                                                                                
Description   : This function prints the register routine of a TABLE into the
                MIBLIB stub. 
                                                                                
Arguments     : fd - File descriptor of the file to which the code should be
                     dumped.
                grpNode - Pointer to the SmiNode.
                objCount - Number of objects present in this table 
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void  printMibLibTblRegister(FILE *fd, SmiNode *grpNode, int objCount)
{
   char  *tblNameSmall;

   tblNameSmall = translateLower(grpNode->name);

   /* Print the function header with appropriate information */
   fprintf(fd, "\n\n\n/**************************************************************************"
               "\nName          :  %s_tbl_reg"
               "\n\nDescription   :  Registering the %s table data and"
               "\n                 its object info with the NCS MibLib."
               "\n\nArguments     :  Nothing"
               "\n\nReturn values :  status"
               "\n\nNotes         :"     
               "\n**************************************************************************/", 
                tblNameSmall, tblNameSmall); 
   fprintf(fd, "\nuns32  %s_tbl_reg()\n{", tblNameSmall);
   fprintf(fd, "\n   NCSMIBLIB_REQ_INFO  req_info;");
   fprintf(fd, "\n   uns32               status = NCSCC_RC_SUCCESS;");
   fprintf(fd, "\n\n   m_NCS_OS_MEMSET(&req_info, 0, sizeof(NCSMIBLIB_REQ_INFO));");
   
   fprintf(fd, "\n\n   /* Update the object data */");
   fprintf(fd, "\n   %s_obj_init(%s_var_info);", tblNameSmall, tblNameSmall);
   fprintf(fd, "\n   %s_obj_info.var_info = %s_var_info;", tblNameSmall, tblNameSmall);

   fprintf(fd, "\n\n   /* Update the table data */");
   fprintf(fd, "\n   %s_tbl_init(&%s_obj_info.tbl_info);", tblNameSmall,tblNameSmall);   

   fprintf(fd, "\n\n   req_info.req = NCSMIBLIB_REQ_REGISTER;");
   fprintf(fd, "\n   req_info.info.i_reg_info.obj_info = &%s_obj_info;", tblNameSmall);

   fprintf(fd, "\n\n   status = ncsmiblib_process_req(&req_info);");
   
   fprintf(fd, "\n\n   return status;\n}");

   return;
}


/**************************************************************************
Name          : printPssvTblRegister
                                                                                
Description   : This function prints the register routine of a TABLE into 
                the PSSV stub. 
                                                                                
Arguments     : fd - File descriptor of the file to which the code should 
                     be dumped.
                grpNode - Pointer to the SmiNode.
                objCount - Number of objects present in this table 
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void  printPssvTblRegister(FILE *fd, SmiNode *grpNode, int objCount)
{
   char  *tblNameSmall;
   char  *tblNameBig;
   int indexCount;

   tblNameSmall = translateLower(grpNode->name);
   tblNameBig = translateUpper(grpNode->name); 

   /* Print the function header with appropriate information */
   fprintf(fd, "\n\n\n/**************************************************************************"
               "\nName          :  __%s_pssv_reg"
               "\n\nDescription   :  Registering the %s table data and"
               "\n                 its object info with the NCS PSSV."
               "\n\nArguments     :  Context information"
               "\n\nReturn values :  status"
               "\n\nNotes         :"     
               "\n**************************************************************************/", 
                tblNameSmall, tblNameSmall); 
   fprintf(fd, "\nuns32  __%s_pssv_reg(NCSCONTEXT i_cntxt)\n{", tblNameSmall);
   fprintf(fd, "\n   NCSPSS_SS_ARG  pss_arg;");
   fprintf(fd, "\n\tNCSMIB_VAR_INFO  %s_pssv_var_info[%s_PSSV_TBL_MAX_PARAMS];",tblNameSmall, tblNameBig);  
   fprintf(fd, "\n\tNCSMIB_OBJ_INFO  %s_pssv_obj_info;", tblNameSmall); 
   fprintf(fd, "\n   uns32          status = NCSCC_RC_SUCCESS;");
   fprintf(fd, "\n\n   m_NCS_OS_MEMSET(&pss_arg, 0, sizeof(NCSPSS_SS_ARG));");
   fprintf(fd, "\n\n   m_NCS_OS_MEMSET(&%s_pssv_obj_info, 0, sizeof(NCSMIB_OBJ_INFO));", tblNameSmall);
   
   fprintf(fd, "\n\n   /* Update the object data */");
   fprintf(fd, "\n   %s_pssv_obj_init(%s_pssv_var_info);", tblNameSmall, tblNameSmall);
   fprintf(fd, "\n   %s_pssv_obj_info.var_info = %s_pssv_var_info;", tblNameSmall, tblNameSmall);

   fprintf(fd, "\n\n   /* Update the table data */");
   fprintf(fd, "\n   %s_pssv_tbl_init(&%s_pssv_obj_info.tbl_info);", tblNameSmall, tblNameSmall);   

   fprintf(fd, "\n   pss_arg.i_op = NCSPSS_SS_OP_TBL_LOAD;");
   fprintf(fd, "\n   pss_arg.i_cntxt = i_cntxt;");
   fprintf(fd, "\n   pss_arg.info.tbl_id = %s;", grpNode->esmi.info.tableInfo.tableId);
   fprintf(fd, "\n   pss_arg.info.tbl_info.mib_tbl = &%s_pssv_obj_info;", tblNameSmall);

   if ((esmiCheckIndexInThisTable (grpNode, &indexCount) != ESMI_SNMP_SUCCESS) && (grpNode->indexkind != SMI_INDEX_UNKNOWN))
   fprintf(fd, "\n   pss_arg.info.tbl_info.objects_local_to_tbl = %s_pssv_objects_local_to_tbl;", tblNameSmall);

   fprintf(fd, "\n   status = ncspss_ss(&pss_arg);\n   {");
#if 0
   fprintf(fd, "\n      /* log the failure */\n\t  m_LOG_PSS_ERR_STR(\"__%s_pssv_reg(): ncspss_ss() failed\", status);", tblNameSmall); 
#endif
   fprintf(fd, "\n      return status;\n   }");  
#if 0
   fprintf(fd, "\n   m_LOG_PSS_STR(\"__%s_pssv_reg(): Successfully loaded\\n\");", tblNameSmall); 
#endif
   
   fprintf(fd, "\n\n   return status;\n}");

   return;
}


/**************************************************************************
Name          :  printMibLibOrPssvStub
                                                                                
Description   : This function traverses through all the smiNodes of a Module 
                (MIB) and generates the stubs for NCS MIBLIB w.r.t tables
                defined in the MIB.
                                                                                
Arguments     : smiModule - pointer to the SmiModule data structs, holds
                            MIB information.
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void printMibLibOrPssvStub(SmiNode *grpNode)
{
   SmiNode *smiNode = NULL;
   FILE    *fd;
   char    *tblName, *tblNameSmall;
   int     objCount = 0;
   int     objsLinear = TRUE;


   /* Frame the table name */
   if (esmiNetSnmpOpt == ESMI_PSSV_OPT) 
   {
      /* this table is not persistent */ 
      if (grpNode->esmi.info.tableInfo.isPersistent == 0)
      {
         return; 
      }
      tblName = esmiTranslateLower(grpNode->name, ESMI_PSSV_C_FILE);
      /* Create a File with name "<table_name>ESMI_PSSV_C_FILE */
      fd = esmiCreateFile(tblName, 0, pssvDirPath);
   }
   else
   {
      tblName = esmiTranslateLower(grpNode->name, ESMI_MIBLIB_C_FILE);
      /* Create a File with name "<table_name>ESMI_MIBLIB_C_FILE */
      fd = esmiCreateFile(tblName, 0, grpNode->esmi.info.tableInfo.dirPath);
   }
  
   if (!fd) /* Not able to create the file */
   {  
      /* Log the message: Not able to create a mib file */
      fprintf(stdout, "printMibLibOrPssvStub(): unable to create the file %s\n", tblName); 
      removeGeneratedFiles = 1; 
      xfree(tblName);
      return;
   }
  
   /* Print the Motorolas copy right information */
   esmiPrintMotoCopyRight(fd, tblName, 0);
     
   /* Get the number of objects present in the table */ 
   objCount = esmiGetTblObjNum(grpNode, &objsLinear);

   /* Prints function, structure.. definitions for this table */
   printMibLibOrPssvDefines(fd, grpNode);

   /* Print table object initialization routine */
   if (printMibLibOrPssvObjInit(fd, grpNode, objCount) != ESMI_SNMP_SUCCESS)
   {
      fprintf(stdout, "printMibLibOrPssvStub(): printMibLibOrPssvObjInit() failed for table %s\n", tblName); 
      fclose(fd);  /* Close the file */ 
      ESMI_FILE_DELETE(tblName); /* Delete the stub file */
      xfree(tblName);   /* Free the memory alloted for table name */
      removeGeneratedFiles = 1; 
      return;
   }

   /* Print table init routine */
   if (printMibLibOrPssvTblInit(fd, grpNode, objCount) != ESMI_SNMP_SUCCESS)
   {
      fprintf(stdout, "printMibLibOrPssvStub(): printMibLibOrPssvTblInit() failed for table %s\n", tblName); 
      fclose(fd);  /* Close the file */ 
      ESMI_FILE_DELETE(tblName); /* Delete the stub file */
      xfree(tblName);   /* Free the memory alloted for table name */
      removeGeneratedFiles = 1; 
      return;
   }

   /* print table register routine */
   if (esmiNetSnmpOpt == ESMI_PSSV_OPT) 
      printPssvTblRegister(fd, grpNode, objCount);
   else
      printMibLibTblRegister(fd, grpNode, objCount);

   fprintf(fd, "\n\n");

   /* Close the file */ 
   fclose(fd);

   fprintf(stdout, "\nFile Generated:  %s\n", tblName);

   /* Free the memory alloted for table name */
   xfree(tblName);

   return;
}


/**************************************************************************
Name          : esmiCheckTblChildObjects
                                                                                
Description   : This function validates the child objects of the table. 
                                                                                
Arguments     : grpNode - pointer to the SmiNode data structs, holds
                          table object information.
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static int esmiCheckTblChildObjects(SmiNode *grpNode) 
{
   SmiNode *smiNode;
   SmiType *smiType;
   char    tempStr[64];

   memset(tempStr, '\0', sizeof(tempStr));

   /* Traverse through all of the child objects of the table and validate
      them  */
   for(smiNode = smiGetFirstChildNode(grpNode);
       smiNode;
       smiNode = smiGetNextChildNode(smiNode))
   {
       if (! (smiNode->nodekind & (SMI_NODEKIND_COLUMN | SMI_NODEKIND_SCALAR)))
          continue;
   
       if ((smiNode->status == SMI_STATUS_OBSOLETE) &&
           (esmiNetSnmpOpt != ESMI_MIBLIB_OPT) &&
           (esmiNetSnmpOpt != ESMI_PSSV_OPT))
          continue;

       smiType = smiGetNodeType(smiNode);
       if (!smiType) continue;

       /* Check whether the object data kind is of ESMI_OBJECT_DATA, else
          return failure */
       if (smiNode->esmi.esmiDataKind != ESMI_OBJECT_DATA)
       {
          fprintf(stderr, "\nERROR: ESMI tag is not defined for the object:  %s ", smiNode->name);
          fprintf(stderr, "\n       Not able to generate the stubs for the table:  %s \n", grpNode->name);
          return ESMI_SNMP_FAILURE;
       }
     
       /* Check whether the base addr(of the base structure) is defined for the object */ 
       if( !strcmp(smiNode->esmi.info.objectInfo.baseAddress, tempStr))
       {
         fprintf(stderr, "\n\n%s:%d  ERROR: Structure Base Address is not defined for the object: %s",
                   esmiTempMibFile, smiNode->esmi.info.objectInfo.line, smiNode->name);
         fprintf(stderr, "\n       Not able to generate the stubs for the table:  %s \n", grpNode->name);
          return ESMI_SNMP_FAILURE;
       }
 
       /* Check whether the member of the base structure is defined for the object */ 
       if( !strcmp(smiNode->esmi.info.objectInfo.member, tempStr))
       {
         fprintf(stderr, "\n\n%s:%d  ERROR: Structure Member is not defined for the object: %s",
                   esmiTempMibFile, smiNode->esmi.info.objectInfo.line, smiNode->name);
         fprintf(stderr, "\n       Not able to generate the stubs for the table:  %s \n", grpNode->name);
          return ESMI_SNMP_FAILURE;
       }

#if 0
       if((smiType->basetype == SMI_BASETYPE_OCTETSTRING) || (smiType->basetype == SMI_BASETYPE_BITS))
       {
         if(smiNode->esmi.info.objectInfo.objLenType) 
         {
            fprintf(stderr, "\n\n%s:%d  ERROR: Object length is defined for the object: %s",
                       esmiTempMibFile, smiNode->esmi.info.objectInfo.line, smiNode->name);
            fprintf(stderr, "\n       Not able to generate the stubs for the table:  %s \n", grpNode->name);
            return ESMI_SNMP_FAILURE;
         }
       }
#endif

   } /* End of for loop */
  
   return ESMI_SNMP_SUCCESS;
}


/**************************************************************************
Name          : esmiCheckObjects
                                                                                
Description   : This function validates the correctness of the data of a 
                table and its child objects.
                                                                                
Arguments     : grpNode- pointer to the SmiNode data structs, holds
                          table information.
                                                                                
Return values : ESMI_SNMP_SUCCESS / ESMI_SNMP_FAILURE
                                                                                
Notes         :
**************************************************************************/
static int esmiCheckObjects(SmiNode *grpNode)
{
   SmiNode *smiNode;

   if (esmiCheckTblObject(grpNode) != ESMI_SNMP_SUCCESS)
      return ESMI_SNMP_FAILURE;

   if (esmiNetSnmpOpt == ESMI_PSSV_OPT)
      return ESMI_SNMP_SUCCESS;

   if (esmiCheckTblChildObjects(grpNode) != ESMI_SNMP_SUCCESS)
      return ESMI_SNMP_FAILURE;

   return ESMI_SNMP_SUCCESS;
}


/**************************************************************************
Name          : dumpMibLibOrPssvStubs
                                                                                
Description   : This function traverses through all the smiNodes (objects) 
                of a Module (MIB) and generates the stubs for NCS MIBLIB
                w.r.t tables defined in the MIB.
                                                                                
Arguments     : smiModule - pointer to the SmiModule data structs, holds
                          MIB information.
                                                                                
Return values : Nothing
                                                                                
Notes         :
**************************************************************************/
static void dumpMibLibOrPssvStubs(SmiModule *smiModule)
{
   SmiNode *smiNode;

   /* Traverse through all the SmiNodes (objects) of a Module, and generate
      the stubs for NCS MIBLIB w.r.t. TABLEs */
   for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
        smiNode;
        smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
   {
       /* Is the smiNode (object) is of TABLE type */
       if (isGroup(smiNode) && (checkColumnsMaxAccess(smiNode) == ESMI_SNMP_SUCCESS) && isAccessible(smiNode)) 
       {
          /* Check the validity of the Table object and its child objects,
             if the table has valid data then only print the stubs for 
             MibLib */

          if ((esmiNetSnmpOpt == ESMI_PSSV_OPT) && (smiNode->esmi.info.tableInfo.isPersistent != 1))
             continue;

          if (esmiCheckObjects(smiNode) == ESMI_SNMP_SUCCESS)
          {
             /* Generate stubs for NCS MIBLIB, for a TABLE */
             printMibLibOrPssvStub(smiNode);
          }
          else
             esmiError = TRUE;
       }
   } /* End of for loop */

   return;
}


/**************************************************************************
Name          : dumpNcsMibLib
                                                                                
Description   : Driver routine to generate the stubs/code for NCS MibLib.
                Stub files are generated with respect to the tables defined 
                in a MIB file.
                                                                                
Arguments     : modc - Number of arguments given for SMIDUMP command
                modv - Pointer to the SmiModule, contains MIB file name
                       information that was given in an SMIDUMP command.
                flags - Defined in Driver definition
                ouput - Not applicable here 
                                                                                
Return values : Nothing
                                                                                
Notes         : In this function there is no means for "output" as the stubs
                generated are named on behalf of the table name.
**************************************************************************/
static void dumpNcsMibLib(int modc, SmiModule **modv, int flags, char *output)
{
   int mibNum;

   esmiNetSnmpOpt = ESMI_MIBLIB_OPT;
 
   if (flags & SMIDUMP_FLAG_UNITE)
   {
      /* not implemented yet */
   }
   else
   {  /* Traverse through all the input MIB files */
      for (mibNum = 0; mibNum < modc; mibNum++) 
      {
          /* Process the MIB file to generate the stubs for NCS MibLib */
          dumpMibLibOrPssvStubs(modv[mibNum]);
      }
   }

   /* Free the allocated memory for directory paths */ 
   smiFree(mapiDirPath);
   smiFree(subAgtDirPath);
   smiFree(esmiDirPath);
   smiFree(esmiGlbHdrFile);
   smiFree(pssvDirPath);

   return;
}

/**************************************************************************
Name          : dumpNcsPssvCommonRegister
                                                                                
Description   : This function creates a stub which is an application specific
                that has a common register routine to register with PSSv. 
                                                                                
Arguments     : modc - Number of input arguments.
                modv - Array of input arguments
                applName - Ptr to the string, stands for an application name.
            
Return values : ESMI_SNMP_SUCCESS / ESMI_SNMP_FAILURE
                                                                                
Notes         :  
**************************************************************************/
static int dumpNcsPssvCommonRegister(int modc, SmiModule **modv, char *applName)
{
   int      i,  status = ESMI_SNMP_SUCCESS;
   FILE     *fd, *hfd;
   char     *tempName = NULL;
   char     *tblNameSmall = NULL;
   int      delete_file = 1; 
   char     *pApplName = NULL; 

   SmiNode   *smiNode = NULL; 
   SmiModule *smiModule = NULL; 

   /* Create a application specific PSSv header file */
   hfd = esmiCreateFile(applName, ESMI_APPL_NAME_PSSV_H_FILE, pssvDirPath); 
   if (! hfd)
   {
      /* Log the Msg : Not able to create appl. specific PSSv init file */
      return ESMI_SNMP_FAILURE; 
   }

   /* Print the copy right information into Application Init file */
   esmiPrintMotoCopyRight(hfd, applName, ESMI_APPL_NAME_PSSV_H_FILE);

   pApplName = translateUpper(applName);
   fprintf(hfd, "#ifndef _%s_PSSV_INIT_H\n", pApplName);
   fprintf(hfd, "#define _%s_PSSV_INIT_H\n", pApplName);
   fprintf(hfd, "\n#include \"%s\"", "ncsgl_defs.h");

   /* for all the modules */ 
   for (i = 0; i<modc; i++)
   {
      /* for each node in this module */
      smiModule = modv[i]; 
      for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
           smiNode;
           smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
      {
        /* Is the smiNode (object) is of TABLE type */
        if (isGroup(smiNode) && isAccessible(smiNode)) 
        {
           /* Check the validity of the Table object and its child objects,
              if the table has valid data then only print the functions for 
              PSSv */
           if ((smiNode->esmi.info.tableInfo.isPersistent == 1)&&
               (esmiCheckObjects(smiNode) == ESMI_SNMP_SUCCESS))
           {
               tempName =  translateLower(modv[i]->name); 
               fprintf(hfd, "\n#include \"%s%s\" ", tempName, ESMI_PSSV_H_FILE);
               xfree(tempName); 
               delete_file = 0; 
               break;  /* no need to print multiple times. Comeout of the innermost loop */ 
           }
        }
      } /* for all the nodes in a module */ 
   }  /* Print the include header file directive */

   if (delete_file == 1)
   {
      tempName = esmigetCompleteName(applName, ESMI_APPL_NAME_PSSV_H_FILE, pssvDirPath); 
      fclose(hfd); 
      ESMI_FILE_DELETE(tempName);
      xfree(tempName); 
      xfree(pApplName);
      esmiError = TRUE;
      return ESMI_SNMP_FAILURE; 
   }

   /* put the extern declaration for the common register routine */ 
   fprintf(hfd, "\n\n/* Common register routine for all the MIBs */"); 
   fprintf(hfd, "\nEXTERN_C uns32 __%s_pssv_reg(NCSCONTEXT   i_cntxt);", applName); 
   fprintf(hfd, "\n\n#endif /* _%s_PSSV_INIT_H */\n\n", pApplName);
   fprintf(hfd, "\n\n"); 
   fclose(hfd); 
   
   /* print a message telling that a file has been generated */ 
   tempName = esmigetCompleteName(applName, ESMI_APPL_NAME_PSSV_H_FILE, pssvDirPath); 
   fprintf(stderr,  "\nFile generated: %s\n", tempName); 
   xfree(tempName); 
   xfree(pApplName); 

   /* Create a application specific PSSv init file */
   fd = esmiCreateFile(applName, ESMI_APPL_NAME_PSSV_C_FILE, pssvDirPath); 
   if (! fd)
   {
      /* Log the Msg : Not able to create appl. specific PSSv init file */
      return ESMI_SNMP_FAILURE; 
   }

   /* Print the copy right information into Application Init file */
   esmiPrintMotoCopyRight(fd, applName, ESMI_APPL_NAME_PSSV_C_FILE);

   fprintf(fd, "\n#include \"%s%s\" ", applName, ESMI_APPL_NAME_PSSV_H_FILE);

   fprintf(fd, "\n/**************************************************************************\n");
   fprintf(fd, "\nName          :  __%s_pssv_reg",  applName);
   fprintf(fd, "\n\nDescription   :  This function is a common registration routine (application"); 
   fprintf(fd, "\n\t\t\t\t\t\tspecific) where it calls all the table registrations with PSSv."); 
   fprintf(fd, "\n\nArguments     :  Context Information"); 
   fprintf(fd, "\n\nReturn values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE"); 
   fprintf(fd, "\n**************************************************************************/"); 
   fprintf(fd, "\nuns32 __%s_pssv_reg(NCSCONTEXT   i_cntxt)\n{", applName); 
   fprintf(fd, "\n\tuns32 status = NCSCC_RC_FAILURE;");

   for (i=0; i <modc; i++)
   {
      /* for each node in this module */
      smiModule = modv[i]; 
      for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
           smiNode;
           smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
      {
        /* Is the smiNode (object) is of TABLE type */
        if (isGroup(smiNode) && isAccessible(smiNode)) 
        {
           /* Check the validity of the Table object and its child objects,
              if the table has valid data then only print the functions for 
              PSSv */
           if ((smiNode->esmi.info.tableInfo.isPersistent == 1)&&
               (esmiCheckObjects(smiNode) == ESMI_SNMP_SUCCESS))
           {
              tblNameSmall = translateLower(smiNode->name);
              fprintf(fd, "\n\n\tstatus = __%s_pssv_reg(i_cntxt);", tblNameSmall);
              fprintf(fd, "\n\tif (status != NCSCC_RC_SUCCESS)\n\t{\n\t\t/* log the message */\n\t}");
              xfree(tblNameSmall); 
           }
        }
      } /* for all the nodes in a module */ 
   } /* for all the modules */ 

   fprintf(fd,  "\n\n\treturn status;\n}/* __%s_pssv_reg(NCSCONTEXT)*/\n\n", applName); 
   tempName = esmigetCompleteName(applName, ESMI_APPL_NAME_PSSV_C_FILE, pssvDirPath); 
   fprintf(stderr,  "\nFile generated: %s\n", tempName); 
   xfree(tempName); 

   /* Close the application specific PSSv init file */
   fclose(fd);
   return status;
}

/**************************************************************************
Name          :  dumpNcsPssvHeader
                                                                                
Description   : This function generates the header(*.h) file of a Module(MIB)
                required for PSSv. 
                                                                                
Arguments     : smiModule - Pointer to the SmiModule structure.
                                                                                
Return values : ESMI_SNMP_SUCCESS / ESMI_SNMP_FAILURE
                                                                                
Notes         :
**************************************************************************/
static int dumpNcsPssvHeader(SmiModule *smiModule)
{
   FILE  *fd;
   char  *baseName = translateLower(smiModule->name);
   char *tblNameSmall = NULL;
   char *pModuleName = NULL; 
   SmiNode *smiNode;
   int     delete_file = 1; 
   char *tempName; 

   pModuleName = translateUpper(smiModule->name);

   /* Create an header file */
   fd = esmiCreateFile(baseName, ESMI_PSSV_H_FILE, pssvDirPath);
   if (!fd) 
   {
      /* Log the message : Not able to create stub file */
      xfree(pModuleName);
      xfree(baseName);
      return ESMI_SNMP_FAILURE;
   }

   /* Print the Motorola's copy right information */
   esmiPrintMotoCopyRight(fd, baseName, ESMI_PSSV_H_FILE);
 
   /* Free the allotted memory for baseName */
    
   fprintf(fd, "#ifndef _%s_PSSV_MIB_H_\n", pModuleName);
   fprintf(fd, "#define _%s_PSSV_MIB_H_\n\n\n", pModuleName);

   fprintf(fd,
           "/*\n"
      " * This C header file has been generated by smidump "
      SMI_VERSION_STRING ".\n"
      " * It is intended to be used with NCS PSSv.\n"
      " * This header is derived from the %s module.\n"
      " */\n", smiModule->name);

   fprintf(fd,
            "\n/*"
            "\n * Registration functions with PSSv for this module:\n"
            " */");

    /* Traverse through all the SmiNodes (objects) of a Module, and generate
       the stubs for NCS MIBLIB w.r.t. TABLEs */
    for (smiNode = smiGetFirstNode(smiModule, SMI_NODEKIND_ANY);
         smiNode;
         smiNode = smiGetNextNode(smiNode, SMI_NODEKIND_ANY)) 
    {
        /* Is the smiNode (object) is of TABLE type */
        if (isGroup(smiNode) && isAccessible(smiNode)) 
        {
           /* Check the validity of the Table object and its child objects,
              if the table has valid data then only print the functions for 
              PSSv */
           if ((smiNode->esmi.info.tableInfo.isPersistent == 1)&&
               (esmiCheckObjects(smiNode) == ESMI_SNMP_SUCCESS)) 
           {
              tblNameSmall = translateLower(smiNode->name);
              fprintf(fd, "\n\n/* To register the '%s' table information with PSSV */", tblNameSmall);
              fprintf(fd, "\nEXTERN_C uns32  __%s_pssv_reg(NCSCONTEXT);\n", tblNameSmall);
              delete_file = 0; 
           }
        }
    } /* End of for loop */

    if (delete_file == 1)
    {
      fclose(fd);
      tempName = esmigetCompleteName(baseName, ESMI_APPL_NAME_PSSV_H_FILE, pssvDirPath); 
      ESMI_FILE_DELETE(tempName);
      xfree(tempName); 
      xfree(pModuleName);
      xfree(baseName);
      esmiError = TRUE;
      return ESMI_SNMP_FAILURE; 
    }

   fprintf(fd, "\n\n#endif /* _%s_PSSV_MIB_H_ */\n\n", pModuleName);

   fclose(fd);

   /* Print onto console about the file generated information*/
   fprintf(stdout, "\nFile Generated:  %s%s\n", baseName, ESMI_PSSV_H_FILE);
   xfree(baseName);
   xfree(pModuleName);

   return ESMI_SNMP_SUCCESS;
}

/**************************************************************************
Name          : dumpNcsPssv
                                                                                
Description   : Driver routine to generate the stubs/code for NCS PSSV.
                Stub files are generated with respect to the tables defined 
                in a MIB file.
                                                                                
Arguments     : modc - Number of arguments given for SMIDUMP command
                modv - Pointer to the SmiModule, contains MIB file name
                       information that was given in an SMIDUMP command.
                flags - Defined in Driver definition
                ouput - Not applicable here 
                                                                                
Return values : Nothing
                                                                                
Notes         : In this function there is no means for "output" as the stubs
                generated are named on behalf of the table name.
**************************************************************************/
static void dumpNcsPssv(int modc, SmiModule **modv, int flags, char *output)
{
   int mibNum;
   int esmiStatus; 
   char     *tempName = NULL;

   esmiNetSnmpOpt = ESMI_PSSV_OPT;
 
   if (flags & SMIDUMP_FLAG_UNITE)
   {
      /* not implemented yet */
   }
   else
   {
      /* dump the common registration routine */ 
      esmiStatus = dumpNcsPssvCommonRegister(modc, modv, output);  
      if (esmiStatus == ESMI_SNMP_FAILURE)
      {
          printf("\n Problems in generating the Common Register routine for PSSv\n"); 
          cleanUpDriverGeneratedFiles(pssvDirPath); 
          return; 
      }

        /* Traverse through all the input MIB files */
      for (mibNum = 0; mibNum < modc; mibNum++) 
      {
          /* generate the header fie */ 
          esmiStatus = dumpNcsPssvHeader(modv[mibNum]); 
          if (esmiStatus == ESMI_SNMP_FAILURE)
          { 
                printf("\n Problems in generating the Header file for module: %s\n", 
                       modv[mibNum]->name); 
                break; 
          }

          /* Process the MIB file to generate the stubs for NCS PSSV,
           * The following function is also used for generating MibLib stubs.
           * This is because the stubs generated is almost same except the
           * registration function which is different for PSSV & MibLib.
           */
          dumpMibLibOrPssvStubs(modv[mibNum]);
      }

      /* make sure there is no problem */ 
      if (removeGeneratedFiles == 1)
      {
          cleanUpDriverGeneratedFiles(pssvDirPath); 
          removeGeneratedFiles = 0; 
      }
   }

   /* Free the allocated memory for directory paths */ 
   smiFree(mapiDirPath);
   smiFree(subAgtDirPath);
   smiFree(esmiDirPath);
   smiFree(esmiGlbHdrFile);
   smiFree(pssvDirPath);

   return;
}

#endif /* NCS_MIBLIB */


#if (NCS_NETSNMP != 0)
/**************************************************************************
Name          : initNcsSynNetsnmp
                                                                                
Description   : Driver initialization routine. In this funtion, driver
                properties are defined and adds the driver to the driver list
                (nothing but registering the driver). This information is 
                used by SMIDUMP tool to generate the stubs that provides
                synchronous interface between MASv and NCS subAgent.
                                                                                
Arguements    : Nothing
                                                                                
Return values : Nothing.
                                                                                
Notes         : Inside the function, define the driver and register with
                SMIDUMP tool. 
**************************************************************************/
void initNcsSynNetsnmp()
{
   /* Define the driver properties */
   static SmidumpDriver driver = {
   "synncsnetsnmp",     /* driver name mentioned in SMIDUMP command */
   dumpSynNcsNetSnmp,   /* driver call back func to generate stubs */
   SMI_FLAG_NODESCR,
   SMIDUMP_DRIVER_CANT_UNITE,
   "ANSI C code for the NCS Synchronous NET-SNMP package",
   NULL /* opt */,
   NULL
   };

   /* registers the driver, basically it adds the driver to the driver 
    * list.
    */
   smidumpRegisterDriver(&driver);

   return;
}


/**************************************************************************
Name          : initNcsSynNetsnmpNew
                                                                                
Description   : Driver initialization routine. In this funtion, driver
                properties are defined and adds the driver to the driver list
                (nothing but registering the driver). This information is 
                used by SMIDUMP tool to generate the stubs that provides
                synchronous interface between MASv and NCS subAgent.
                                                                                
Arguements    : Nothing
                                                                                
Return values : Nothing.
                                                                                
Notes         : Inside the function, define the driver and register with
                SMIDUMP tool. 
**************************************************************************/
void initNcsSynNetsnmpNew()
{
   /* Define the driver properties */
   static SmidumpDriver driver = {
   "synncsnetsnmp_new",   /* driver name mentioned in SMIDUMP command */
   dumpNewSynNcsNetSnmp,  /* driver call back func to generate stubs */
   SMI_FLAG_NODESCR,
   SMIDUMP_DRIVER_CANT_UNITE,
   "ANSI C code for the NCS Synchronous NET-SNMP 5.1.1 package",
   NULL /* opt */,
   NULL
   };

   /* registers the driver, basically it adds the driver to the driver 
    * list.
    */
   smidumpRegisterDriver(&driver);

   return;
}


/**************************************************************************
Name          : initNcsMapi
                                                                                
Description   : Driver initialization routine. In this funtion, driver
                properties are defined and adds the driver to the driver list
                (nothing but registering the driver). This information is 
                used by SMIDUMP tool to generate the stubs that provides
                synchronous interface between MASv and NCS subAgent.
                                                                                
Arguements    : Nothing
                                                                                
Return values : Nothing.
                                                                                
Notes         : Inside the function, define the driver and register with
                SMIDUMP tool. 
**************************************************************************/
void initNcsMapi()
{
   /* Define the driver properties */
   static SmidumpDriver driver = {
   "ncsmapi",     /* driver name mentioned in SMIDUMP command */
   dumpNcsMapi,   /* driver call back func to generate stubs */
   SMI_FLAG_NODESCR,
   SMIDUMP_DRIVER_CANT_UNITE,
   "Generates NCS MAPI header files",
   NULL /* opt */,
   NULL
   };

   /* registers the driver, basically it adds the driver to the driver 
    * list.
    */
   smidumpRegisterDriver(&driver);

   return;
}


/**************************************************************************
Name          : initNcsAsynNetsnmp
                                                                                
Description   : Driver initialization routine. In this funtion, driver
                properties are defined and adds the driver to the driver list
                (nothing but registering the driver). This information is 
                used by SMIDUMP tool to generate the stubs that provides
                asynchronous interface between MASv and NCS subAgent.
                                                                                
Arguments     : Nothing 
                                                                                
Return values : Nothing.
                                                                                
Notes         : Inside the function, define the driver and register with
                SMIDUMP tool
**************************************************************************/
void initNcsAsynNetsnmp()
{
   static SmidumpDriver driver = {
   "asynncsnetsnmp",     /* driver name mentioned in SMIDUMP command */
   dumpAsynNcsNetSnmp,   /* driver call back func to generate stubs */
   SMI_FLAG_NODESCR,
   SMIDUMP_DRIVER_CANT_UNITE,
   "ANSI C code for the NCS Asynchronous NET-SNMP package",
   NULL /* opt */,
   NULL
   };

   /* registers the driver, basically it adds the driver to the driver 
    * list.
    */
   smidumpRegisterDriver(&driver);

   return;
}

#endif /* NCS_NETSNMP */

#if (NCS_MIBLIB != 0)
/**************************************************************************
Name          : initNcsMibLib
                                                                                
Description   : Driver initialization routine. In this funtion, driver
                properties are defined and adds the driver to the driver list
                (nothing but registering the driver). This information is 
                used by SMIDUMP tool to generate the stubs for NCS MibLib
                                                                                
Arguments     :  Nothing
                                                                                
Return values : Nothing.
                                                                                
Notes         : Inside the function, define the driver and register with
                SMIDUMP tool
**************************************************************************/
void initNcsMibLib()
{
   static SmidumpDriver driver = {
   "ncsmiblib",     /* driver name mentioned in SMIDUMP command */
   dumpNcsMibLib,   /* driver call back func to generate stubs */
   SMI_FLAG_NODESCR,
   SMIDUMP_DRIVER_CANT_UNITE,
   "ANSI C code for the NCS MibLib package",
   NULL /* opt */,
   NULL
   };

   /* registers the driver, basically it adds the driver to the driver 
    * list.
    */
   smidumpRegisterDriver(&driver);

   return;
}


/**************************************************************************
Name          : initNcsPssv
                                                                                
Description   : Driver initialization routine. In this funtion, driver
                properties are defined and adds the driver to the driver list
                (nothing but registering the driver). This information is 
                used by SMIDUMP tool to generate the stubs for NCS PSSV
                                                                                
Arguments     :  Nothing
                                                                                
Return values : Nothing.
                                                                                
Notes         : Inside the function, define the driver and register with
                SMIDUMP tool
**************************************************************************/
void initNcsPssv()
{
   static SmidumpDriver driver = {
   "ncspssv",     /* driver name mentioned in SMIDUMP command */
   dumpNcsPssv,   /* driver call back func to generate stubs */
   SMI_FLAG_NODESCR,
   SMIDUMP_DRIVER_CANT_UNITE,
   "ANSI C code for the NCS PSSV package",
   NULL /* opt */,
   NULL
   };

   /* registers the driver, basically it adds the driver to the driver 
    * list.
    */
   smidumpRegisterDriver(&driver);

   return;
}

#endif /* NCS_MIBLIB */


