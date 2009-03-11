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

 MODULE NAME:  CLITRAV.C

  ..............................................................................
  DESCRIPTION:

  Source file for Command Tree traversing

*******************************************************************************/

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                    Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include "cli.h"

#if (NCS_CLI == 1)
static void clean_arg(NCSCLI_ARG_SET *arg);
static NCS_BOOL is_any_optional_node(CLI_CMD_ELEMENT **ptr);
static NCS_BOOL is_any_wildcard_node(CLI_CMD_ELEMENT **ptr);
static void convert_to_community(NCSCLI_ARG_VAL *argval, int8* str);
static void convert_to_macaddr(NCSCLI_ARG_VAL *arg_val, int8* str);
static void cli_update_cmd_arg(NCSCLI_ARG_SET *ctok, CLI_EXECUTE_PARAM *io_param);
static void cli_help_str_fill(CLI_CB *pCli, CLI_CMD_HELP *buf);

#if(NCS_IPV6 == 1)
static void cli_str_to_ipv6addr(NCSCLI_ARG_VAL *arg_val, int8* str);
static void cli_str_to_ipv6pfx(NCSCLI_ARG_VAL *arg_val, int8* str);
/* Function to get delimit count from Ipv6 address */
static uns32 cli_get_ipv6_dlmt_cnt (int8 *i_str, uns8 *o_dlimt_cnt, 
                                                     NCS_BOOL *o_dbl_dlimt);
#endif


#define  m_BGP_UTIL_PFX_FROM_BITS(track, bits) \
         (track & (0xFFFFFFFF << (32-bits)))


/*****************************************************************************

  PROCEDURE NAME:        cli_get_ipv6_dlmt_cnt

  DESCRIPTION:           This function get delimit count from Ipv6 address 

  ARGUMENTS:
    int8 *              A string containing IPv6 address
    o_dlimt_cnt         Dilimit count (output)
    o_dbl_dlimt         Double delimit flag (Output)

  RETURNS: 
   NCSCC_RC_SUCCESS      If the string validation is successful.
   NCSCC_RC_FAILURE      If the string validation failed.
*****************************************************************************/
#if(NCS_IPV6 == 1)
static uns32 cli_get_ipv6_dlmt_cnt (int8 *i_str, uns8 *o_dlimt_cnt, 
                                                     NCS_BOOL *o_dbl_dlimt)
{

   uns32    i=0, str_len=0;
   uns8     char_cnt=0, dlimt_cnt=0;
   NCS_BOOL dlimt_flag=FALSE, dbl_dlimt=FALSE;

   str_len = strlen(i_str);

   /* String lenght should be at least two characters */
   if(str_len <2) return NCSCC_RC_FAILURE;

   /* Check for delimiter at the begin */
   if(i_str[0] == ':') {
      if(i_str[1] == ':') {
         dbl_dlimt = TRUE;
         i=2;

         /* Check for IPV4 compatable IPV6 address format */
         if(CLI_SUCCESSFULL_MATCH == cli_check_ipv4addr(&i_str[i])) {
            *o_dbl_dlimt = dbl_dlimt;
            return NCSCC_RC_SUCCESS;
         }
      }
      else return NCSCC_RC_FAILURE;
   }
   
   while(i<str_len) {
      if(i_str[i] == ':') {
         /* Delimiter found, set char_cnt = 0 */
         char_cnt = 0;

         if(dlimt_flag == FALSE) {
            dlimt_cnt++;
            dlimt_flag = TRUE;
         }
         else {   /* Double deleimiter (::) case */         
            if(dbl_dlimt == FALSE) dbl_dlimt = TRUE;            
            else return NCSCC_RC_FAILURE; /* Don't allow more that one double delimiter */            
         }
      }
      else if(((i_str[i] >= '0') && (i_str[i] <= '9')) ||
              ((i_str[i] >= 'A') && (i_str[i] <= 'F')) ||
              ((i_str[i] >= 'a') && (i_str[i] <= 'f'))) {
         
         /* 0-9, A-F, and a-f are Possible characters in Ipv6 address */
         char_cnt++;
         dlimt_flag = FALSE;
      }
      else return NCSCC_RC_FAILURE;

      if(char_cnt > 4) return NCSCC_RC_FAILURE;
      i++;
   }

   /* Last character should not be delimiter(:) unless it 
   is double delimiter(::) */
   if((i_str[i-1] == ':') && (i_str[i-2] != ':')) return NCSCC_RC_FAILURE;

   *o_dlimt_cnt = dlimt_cnt;
   *o_dbl_dlimt = dbl_dlimt;

   return NCSCC_RC_SUCCESS;
}
#endif

/*****************************************************************************

  PROCEDURE NAME:        CLI_CHECK_IPV6ADDR

  DESCRIPTION:           This function checks if the given IPv6 address
                         is valid.

  ARGUMENTS:
    int8 *                A string containing IPv6 address

  RETURNS: 
   Match Code
    CLI_NO_MATCH          If it is an invalid IPv6 address
    CLI_PARTIAL_MATCH     If the given IPv6 address is not complete
    CLI_SUCCESSFULL_MATCH If the IPv6 address is valid
  
*****************************************************************************/
#if(NCS_IPV6 == 1)
uns32 
cli_check_ipv6addr (int8 *i_str)
{
   uns8     dlimt_cnt=0;
   NCS_BOOL  dbl_dlimt = FALSE;
   uns32    rc = NCSCC_RC_FAILURE;

   rc = cli_get_ipv6_dlmt_cnt(i_str, &dlimt_cnt, &dbl_dlimt);
   if(rc != NCSCC_RC_SUCCESS) return CLI_NO_MATCH;

   if((dbl_dlimt == TRUE) && (dlimt_cnt < NCS_IPV6_ADDR_UNS16_CNT))
      return CLI_SUCCESSFULL_MATCH;
   else if((dbl_dlimt == FALSE) && (dlimt_cnt == (NCS_IPV6_ADDR_UNS16_CNT-1)))
      return CLI_SUCCESSFULL_MATCH;
   return CLI_NO_MATCH;
}
#endif

/*****************************************************************************

  PROCEDURE NAME:        CLI_CHECK_IPV6PREFIX

  DESCRIPTION:           This function checks if the given IPv6 address
                         prefix is valid.

  ARGUMENTS:
    int8 *                A string containing IPv6 address

  RETURNS: 
   Match Code
    CLI_NO_MATCH          If it is an invalid IPv6 address
    CLI_PARTIAL_MATCH     If the given IPv6 address is not complete
    CLI_SUCCESSFULL_MATCH If the IPv6 address is valid
  
*****************************************************************************/
#if(NCS_IPV6 == 1)
uns32 
cli_check_ipv6prefix(int8 *i_str)
{
   NCS_BOOL  dbl_dlimt= FALSE;
   int8     buffer[CLI_BUFFER_SIZE];
   uns8     i=0, str_len=0, dlimt_cnt=0;
   uns32    rc = NCSCC_RC_FAILURE;

   /* Get the string length of the input string */
   str_len = (uns8) strlen(i_str);

   memset(buffer, 0, sizeof(buffer));

   strcpy(buffer, i_str);

   /* Look for the '/' in the string*/
   while(i<str_len) {
      if(buffer[i] == '/') {
         buffer[i] = 0;
         rc = cli_get_ipv6_dlmt_cnt (buffer, &dlimt_cnt, &dbl_dlimt);
         if(rc != NCSCC_RC_SUCCESS)
            return CLI_NO_MATCH;
         break;
      }
      i++;
   }

   /* IF '/' not found */
   if(i == str_len) return CLI_NO_MATCH;

   if(dlimt_cnt >= NCS_IPV6_ADDR_UNS16_CNT) return CLI_NO_MATCH;

   /* Now validate the prefix length fields*/
   i++;

   /* Prefix length field should not be more than 3 bytes
   and should not be zero */
   if(((str_len-i) > 3) || ((str_len-i) == 0)) return CLI_NO_MATCH;

   /* Validate the pfx lenght charecters */
   while(i<str_len) {
      if((i_str[i] >= '0') && (i_str[i] <= '9')) i++;
      else return CLI_NO_MATCH;
   }
   
   /* All the tests passed, return success */
   return CLI_SUCCESSFULL_MATCH;
}
#endif

/*****************************************************************************

  PROCEDURE NAME:        CLI_CHECK_MACADDR

  DESCRIPTION:           This function checks if the given mac address is valid.

  ARGUMENTS:
    int8 *                A string containing mac address value

  RETURNS: 
   Match Code
    CLI_NO_MATCH          If it is an invalid IPv6 address
    CLI_PARTIAL_MATCH     If the given IPv6 address is not complete
    CLI_SUCCESSFULL_MATCH If the IPv6 address is valid
  
*****************************************************************************/
uns32
cli_check_macaddr(int8 *i_str)
{
   int8  *delimiter = ":";
   int8  buffer[CLI_BUFFER_SIZE];
   int8  *token = 0;
   uns32 addr_cnt = 0;
   uns32 ret_val = CLI_NO_MATCH;

   memset(buffer, 0, sizeof(buffer));

   strcpy(buffer, i_str);
   token = sysf_strtok(buffer, delimiter);      
   
   while(token) {
      addr_cnt++;
      token = sysf_strtok(0, delimiter);      
   }

   if(NCSCLI_MAC_ADDR_CNT == addr_cnt) ret_val = CLI_SUCCESSFULL_MATCH;
   return ret_val;
}

/*****************************************************************************

  PROCEDURE NAME:        CLI_CHECK_COMMUNITY

  DESCRIPTION:           This function checks if the given community is valid.

  ARGUMENTS:
    int8 *                A string containing community value

  RETURNS: 
   Match Code
    CLI_NO_MATCH          If it is an invalid IPv6 address
    CLI_PARTIAL_MATCH     If the given IPv6 address is not complete
    CLI_SUCCESSFULL_MATCH If the IPv6 address is valid
  
*****************************************************************************/
uns32 
cli_check_community(int8 *i_str)
{
   int8  *delimiter = ":";
   int8  buffer[CLI_BUFFER_SIZE];
   int8  *token = 0;
   uns32 ret_val = CLI_NO_MATCH,
         ret = CLI_NO_MATCH;
      
   memset(buffer, 0, sizeof(buffer));
   strcpy(buffer, i_str);
   token = sysf_strtok(buffer, delimiter);
   if(token) {
      /* Get firt part of community */ 
      ret = cli_check_ipv4addr(token);
      if(CLI_SUCCESSFULL_MATCH != ret) {
         /* Check if the first part is number */
         if(CLI_SUCCESSFULL_MATCH != cli_check_number(token))
            return ret_val;                 
      }
      
      /* Get next part of community */
      while(0 != token) {
        token = sysf_strtok(0, delimiter);
        if(token) {
           if(CLI_SUCCESSFULL_MATCH != cli_check_number(token)) return ret_val;           
           else ret_val = CLI_SUCCESSFULL_MATCH;
        }
      }
   }

   return ret_val;
}


/*****************************************************************************

  PROCEDURE NAME:        CLI_CHECK_NUMBER

  DESCRIPTION:           This function checks if the given number is valid.

  ARGUMENTS:
    int8 *                A string 

  RETURNS: 
   Match Code
    CLI_NO_MATCH          If it is an invalid number or no number    
    CLI_SUCCESSFULL_MATCH If its valid number
  
*****************************************************************************/
uns32 cli_check_number(int8 *i_str)
{
   uns32 num = 0;
   uns32 rc = CLI_NO_MATCH; 
   int8  *buffer = 0;

   sscanf(i_str, "%u", &num);
   if(num) {
      if(0 == (buffer = (int8 *)m_MMGR_ALLOC_CLI_DEFAULT_VAL(strlen(i_str)+1)))
         return NCSCC_RC_FAILURE;
      
      memset(buffer, 0, strlen(i_str));
      sysf_sprintf(buffer, "%d", num);      
      if(strlen(buffer) == strlen(i_str)) {
         m_MMGR_FREE_CLI_DEFAULT_VAL(buffer);
         return CLI_SUCCESSFULL_MATCH;
      }
      if(buffer) m_MMGR_FREE_CLI_DEFAULT_VAL(buffer);
   }      
   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:        CLI_CHECK_IPV4ADDR

  DESCRIPTION:           This function checks if the given IPv4 address
                         is valid.

  ARGUMENTS:
    int8 *                A string containing IPv4 address

  RETURNS: 
   Match Code
    CLI_NO_MATCH          If it is an invalid IPv4 address
    CLI_PARTIAL_MATCH     If the given IPv4 address is not complete
    CLI_AMBIGUOUS_MATCH   It the IPv4 address string contains characters
    CLI_SUCCESSFULL_MATCH If the IPv4 address is valid
  
*****************************************************************************/
uns32 
cli_check_ipv4addr (int8 *i_str)
{
   int8    *ptr = 0;
   uns32   dots = 0, 
           digit = 0;
   NCS_BOOL is_valid = FALSE;
   int8    ipbuf[4];        
      

   /* If NULL then return */
   if(0 == i_str) return CLI_NO_MATCH;
   
   for(;;) {
      memset (ipbuf,'\0', sizeof (ipbuf));
      ptr = i_str;
      
      while('\0' != *i_str) {        
         /* check if it is a digit or dot */
         if(isdigit((uns32)*i_str)) {
            if(digit > 3) return CLI_NO_MATCH;
            if(dots == 3) is_valid = TRUE;
            i_str++;
            digit++;
            continue;
         }
         else {
            /* Check if it is a dot */
            if('.' == *i_str) {               
               /* There should be minimum of 1 digit between dots */
               if('.' == *(i_str + 1)) return CLI_NO_MATCH;
               
               /* If IP address has more then 3 dots return error */
               if(dots >= 3 || !digit) return CLI_NO_MATCH;
               
               dots++;
               digit = 0;               
               break;
            }
            else {
               /* Neither digit nor dot */
               return CLI_AMBIGUOUS_MATCH;
            }
         }
         i_str++;
      }
      
      /* Check if digit exceeds 255 chars */
      strncpy (ipbuf, ptr, i_str - ptr);
      if(atoi(ipbuf) > 255) return CLI_NO_MATCH;
      
      /* check if end of ipaddr */
      if ('\0' != *i_str) i_str++;
      else break;
   }     

   if(dots < 3 || !is_valid) return CLI_PARTIAL_MATCH;   
   return CLI_SUCCESSFULL_MATCH;
}


/*****************************************************************************

  PROCEDURE NAME:        CLI_CHECK_IPV4PREFIX

  DESCRIPTION:           This function checks if the given IPv4 address
                         prefix is valid.

  ARGUMENTS:
    int8 *                A string containing IPv4 address

  RETURNS: 
   Match Code
    CLI_NO_MATCH          If it is an invalid IPv4 address
    CLI_PARTIAL_MATCH     If the given IPv4 address is not complete
    CLI_AMBIGUOUS_MATCH   It the IPv4 address string contains characters
    CLI_SUCCESSFULL_MATCH If the IPv4 address is valid
  
*****************************************************************************/
uns32 cli_check_ipv4prefix (int8 *i_cmdstring)
{
   int8 *str = i_cmdstring;
   uns32 ipaddress=0;
   int8 iptoken[32];
   
   /* Check if there is '/' to indicate mask */
   while('\0' != *str) {
      if('/' == *str) break;
      
      /* If not . or number return */
      if(('.' != *str) && (!isdigit((uns32)*str))) return CLI_AMBIGUOUS_MATCH;
      
      iptoken[ipaddress] = *str;
      ipaddress++;
      str++;
   }
   
   iptoken[ipaddress] = '\0';
   
   /* Check is given token is valid ip address */
   if(cli_check_ipv4addr (iptoken) != CLI_SUCCESSFULL_MATCH) return CLI_NO_MATCH;   
   str++;
   
   /* If exceeds range then return error */
   if(atoi (str) > 32 || atoi(str) < 0) return CLI_NO_MATCH;   
   return CLI_SUCCESSFULL_MATCH;
}


/*****************************************************************************

  PROCEDURE NAME:   CLI_IPV4_TO_INT

  DESCRIPTION:      Converts IPv4 address ie NUM.NUM.NUM.NUM into an 
                    integer


  ARGUMENTS:        Integer value of IPv4 address
   uns32                  

  RETURNS: 
   uns32            IP Address as Integer
*****************************************************************************/
uns32 cli_ipv4_to_int(int8 *i_string)
{
   int8     seps[] = ".";
   int8     *token, str_addr[16];
   uns32    ipaddress = 0;   
   
   strcpy(str_addr, i_string);
   
   token = sysf_strtok( str_addr, seps );
   ipaddress |= atoi(token);
   
   while(token) {       
      /* Get next token: */
      token = sysf_strtok(0, seps);
      if(token != 0) {
         /* Left shift 8 bits and OR with token */
         ipaddress <<= 8;         
         ipaddress |= atoi(token);
      }            
   }
   return ipaddress;
}

/*****************************************************************************

  PROCEDURE NAME:        CLI_MATCHTOKEN

  DESCRIPTION:           This function finds if str2 is present in str1.
  
  ARGUMENTS:
   int8 *                i_str1
   int8 *                i_str2
   CLI_CMD_ERROR         *o_cli_cmd_error
  RETURNS: 
   Match Code
   CLI_NO_MATCH          If the two strings are not equal
   CLI_PARTIAL_MATCH     If str2 is a subset of str1
   CLI_SUCCESSFULL_MATCH If both strings are equal.
  
*****************************************************************************/
uns32 
cli_matchtoken(int8 *i_str1, int8 *i_str2, CLI_CMD_ERROR *o_cli_cmd_error)
{
    int8    *s1 = 0, 
            *s2 = 0;
    uns16   pos = 0;

    /* check if string is 0 */
    if(!*i_str2 ) return CLI_NO_MATCH;

    s1 = (int8 *)i_str1;
    s2 = (int8 *) i_str2;    

    /* Loop and compare each character, also count num of characters
       matched */
    while(*s1 && tolower(*s2) && !(*s1-tolower(*s2))) {
        s1++, s2++;
        pos++;        
    }

    /* At the end of loop both strings are same */
    if(!*s1 && !*s2) {
        /* reset error string */
        memset(o_cli_cmd_error->errstring, '\0', sizeof(o_cli_cmd_error->errstring));    
        o_cli_cmd_error->errorpos = 0;
        return CLI_SUCCESSFULL_MATCH;
    }

    /* s2 is less then s1 its a partial match */
    if(!*s2) {
        /* reset error string */
        memset(o_cli_cmd_error->errstring, 0, 
            sizeof(o_cli_cmd_error->errstring));    
        o_cli_cmd_error->errorpos = 0;
        return CLI_PARTIAL_MATCH;
    }

    /* get the best match token, as if a match is not found we will
       traverse thru other nodes, the err pos can be reset 
    */
    if(pos > o_cli_cmd_error->errorpos) o_cli_cmd_error->errorpos = pos;

    /* both strings are not equal, copy error string */
    strcpy(o_cli_cmd_error->errstring,i_str2);
    return CLI_NO_MATCH;
}


/*****************************************************************************

  PROCEDURE NAME:        CLI_GET_TOKENTYPE

  DESCRIPTION:           This function finds and returns the token type of
                         string passed
  
  ARGUMENTS:
   int8 *                token

  RETURNS: 
   Token type
            NCSCLI_STRING
            NCSCLI_IPv4  
            CLI_IPv4_ERROR
  
*****************************************************************************/
uns32 
cli_get_tokentype(int8 * i_token)
{
   uns32   type = NCSCLI_NUMBER;
   int8    *buf = i_token;
   uns32   ret = 0;
   
   while(0 != *i_token) {
      /* Initially type is int, check is there are any alphabets */
      if(!isdigit((uns32)*i_token)) type = NCSCLI_STRING;      
      i_token++;
   }  
   
   /* If type is NCSCLI_NUMBER then return */
   if(NCSCLI_NUMBER == type) return NCSCLI_NUMBER;

   /* Check if this string is the IPv6 Address */
#if (NCS_IPV6 == 1)
   ret = cli_check_ipv6addr(buf);
   if(CLI_SUCCESSFULL_MATCH == ret) return NCSCLI_IPv6;

   /* Check if it is IPv6 Prefix (CIDR) */
   ret = cli_check_ipv6prefix(buf);   
   if(CLI_SUCCESSFULL_MATCH == ret) return NCSCLI_CIDRv6;
#endif
   
   /* Check MAC Address value */
   ret = cli_check_macaddr(buf); 
   if(CLI_SUCCESSFULL_MATCH == ret) return NCSCLI_MACADDR;

   /* Check community value */
   ret = cli_check_community(buf);
   if(CLI_SUCCESSFULL_MATCH == ret) return NCSCLI_COMMUNITY;
   
   /* Check if token is IPv4 address */   
   ret = cli_check_ipv4addr(buf);   
   if(CLI_SUCCESSFULL_MATCH == ret) return NCSCLI_IPv4;
   
   /* Its a partial/ Invalid IP addr so return error */
   if(CLI_NO_MATCH == ret || CLI_PARTIAL_MATCH == ret) return CLI_IPv4_ERROR;
   
   /* Not IPv4 either can be IPv4 prefix or string */       
   
   ret = cli_check_ipv4prefix(buf);
   
   if(CLI_SUCCESSFULL_MATCH == ret) return NCSCLI_CIDRv4;   
   return NCSCLI_STRING;
}


/*****************************************************************************

  PROCEDURE NAME:        CLI_SET_ARG

  DESCRIPTION:           This function fills the cmd_tokens struct while parsing the
                         command user has entered into tokens. It stores the command
                         into tokens
  
  ARGUMENTS
   int8 *                argument to be set
   NCSCLI_ARG_SET *         The arg structure to which the tokens are copied

  RETURNS: 
   Sucess or Error
    CLI_SUCESS
    CLI_FAILURE
    
*****************************************************************************/
/*uns32 cli_set_arg(int8 * i_arg, NCSCLI_ARG_SET *o_cmd_tokens) */
uns32 cli_set_arg(int8 * i_arg, NCSCLI_ARG_SET *o_cmd_tokens, CLI_CB *pCli) /* Fxi for the bug 59854 , CLI_CB* extra argument passed */
{
   uns32 count = 0, bit_pos, len = 0;
   int num = 0;
   unsigned long long int longnum = 0; /* Added to fix the bug 59854 */
   uns32 hvalue;

   
   /* increment the i_pos_value and also set the last element to one */
   o_cmd_tokens->i_pos_value <<= 1;
   o_cmd_tokens->i_pos_value |= 1;
   
   /*  find out number of elements and set array of arg accordingly */
   bit_pos = o_cmd_tokens->i_pos_value;
   
   /* Get index of last token */
   m_CLI_GET_LAST_TOKEN(bit_pos);  
   
   /* get token type */ 
   o_cmd_tokens->i_arg_record[count].i_arg_type = cli_get_tokentype(i_arg);
   
   /* Copy String/Integer/IpPfx into NCSCLI_ARG_VAL
   based on type */
   
   if(NCSCLI_NUMBER == o_cmd_tokens->i_arg_record[count].i_arg_type) {
      sscanf(i_arg, "%u", &num);
      o_cmd_tokens->i_arg_record[count].cmd.intval = num;     
      /*  (Boundary flag is set on crossing ulimit) */
      sscanf(i_arg, "%llu", &longnum);
      hvalue = NCSCLI_UNS32_HLIMIT;
      /*hvalue = 0xffffffff;*/
      if(longnum > hvalue)
      {
          /*pCli->outOfBoundaryToken = (int8 *)malloc(strlen(i_arg)); */
          pCli->outOfBoundaryToken = m_MMGR_ALLOC_OUTOFBOUNDARY_TOKEN(strlen(i_arg) + 1);
          if(!pCli->outOfBoundaryToken) return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
          /*m_NCS_OS_MEMALLOC(strlen(i_arg),pCli->outOfBoundaryToken);*/
           strcpy(pCli->outOfBoundaryToken,i_arg);
      }
   }
   else {
      if(o_cmd_tokens->i_arg_record[count].cmd.strval)
         m_MMGR_FREE_CLI_DEFAULT_VAL(o_cmd_tokens->i_arg_record[count].cmd.strval);
      
      len = strlen(i_arg) + 1;
      o_cmd_tokens->i_arg_record[count].cmd.strval = m_MMGR_ALLOC_CLI_DEFAULT_VAL(len);
      if(!o_cmd_tokens->i_arg_record[count].cmd.strval) return NCSCC_RC_FAILURE;
      memset(o_cmd_tokens->i_arg_record[count].cmd.strval, '\0', len);
      strncpy((int8 *)o_cmd_tokens->i_arg_record[count].cmd.strval, 
         i_arg, len-1);
   }
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:        CLI_GETCOMMAND_TOKENS

  DESCRIPTION:           This function Parses the cmdbuf obtained from CLI 
                         input and parses them into tokens. Fills and returns
                         the NCSCLI_ARG_SET struct with parsed tokens.
  
  ARGUMENTS
   int8 *                string to be tokenized.

  RETURNS: 
   uns32                 NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
    
*****************************************************************************/
/*uns32 cli_getcommand_tokens(int8 *i_cmdstring, NCSCLI_ARG_SET *ctok)*/
uns32 cli_getcommand_tokens(int8 *i_cmdstring, NCSCLI_ARG_SET *ctok, CLI_CB *pCli)/* CLI_CB arg ment added extra to fix the bug 59854 */
{
   int8 *curptr = 0, *start = 0, token[CLI_BUFFER_SIZE];
   uns32 slen;
   
   if(!i_cmdstring) return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);   
   curptr = i_cmdstring;
   
   /* check if there are any white spaces in begining or
   cmd string is NULL */
   while(((uns32)*curptr == CLI_CONS_BLANK_SPACE) && '\0' != *curptr)
      curptr++;
   
   /* If empty then return NULL */
   if('\0' == *curptr) return NCSCC_RC_FAILURE;
   
   ctok->i_pos_value = 0;
   
   /* for each token, copy into cmd_token struct */
   for(;;) {
      start = curptr;
      
      /* extract each token untill space or end of string */
      while (!((uns32)*curptr == CLI_CONS_BLANK_SPACE) && '\0' != *curptr)
         curptr++;
      
      slen = curptr - start;
      strncpy (token, start, slen);
      *(token + slen) = '\0';
      
      /* Fill the cmd_token structure with args */
      /*if(NCSCC_RC_SUCCESS != cli_set_arg(token, ctok)) return NCSCC_RC_FAILURE;*/ /*  ( pCli extra argument passed to fix)*/
     if(NCSCC_RC_SUCCESS != cli_set_arg(token, ctok, pCli)) return NCSCC_RC_FAILURE; 
      while (((uns32)*curptr == CLI_CONS_BLANK_SPACE) && '\0' != *curptr)
         curptr++;
      
      if('\0' == *curptr) break;
   }
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:        CLI_CHECK_TOKEN

  DESCRIPTION:           This function checks the type of token, it
                         compares the token type from arg struct with
                         that present in command tree.
  
  ARGUMENTS
   CLI_CMD_ELEMENT *     Pointer to Command Element in command tree
   NCSCLI_ARG_VAL *         The Arg element which needs to be checked with CmdElement

  RETURNS: 
   Match Code
   CLI_NO_MATCH          If the two strings are not equal
   CLI_PARTIAL_MATCH     If str2 is a subset of str1
   CLI_SUCCESSFULL_MATCH If both strings are equal.
        
*****************************************************************************/
uns32
cli_check_token(CLI_CB *pCli, NCSCLI_ARG_VAL *i_argval, CLI_CMD_ERROR *o_cli_cmd_error)
{
   uns32   ret = CLI_NO_MATCH, uns_ipaddr;
   int8    passwdbuf[CLI_ARG_LEN], ipmask[32], *iptoken;
   CLI_CMD_NODE    *trvmarker=0;
   int8 seps[]   = "/\0";
   
   /* check token type, if dummy node then do nothing */   
   if(NCSCLI_OPTIONAL == pCli->ctree_cb.cmdElement->tokType) {
      ret = CLI_NO_MATCH;
   }
   else if (NCSCLI_GROUP == pCli->ctree_cb.cmdElement->tokType) {
      ret = CLI_NO_MATCH;
   }
   else if (NCSCLI_CONTINOUS_EXP == pCli->ctree_cb.cmdElement->tokType) {
      ret = CLI_NO_MATCH;
   }
   else if(NCSCLI_WILDCARD == pCli->ctree_cb.cmdElement->tokType) {
      ret = CLI_NO_MATCH;
   }
   else if(NCSCLI_KEYWORD == pCli->ctree_cb.cmdElement->tokType) {        
      if(NCSCLI_STRING == i_argval->i_arg_type || NCSCLI_KEYWORD == i_argval->i_arg_type)
         ret = cli_matchtoken(pCli->ctree_cb.cmdElement->tokName, 
            (int8 *)i_argval->cmd.strval, o_cli_cmd_error);
      else
         /* Arg var is not a string, it can be a IP address or Int or ..*/
         m_CHECK_IF_NUMBER(i_argval->i_arg_type, i_argval->cmd, o_cli_cmd_error->errstring);
      
         /* if keyword matched, then update token type in arg struct to
            indicate keyword. When input buffer was tokenized it was treated
            as NCSCLI_STRING type 
         */      
      if(CLI_SUCCESSFULL_MATCH == ret || CLI_PARTIAL_MATCH == ret) {
         i_argval->i_arg_type =  NCSCLI_KEYWORD;           
      }
   }    
   else if(NCSCLI_STRING == pCli->ctree_cb.cmdElement->tokType) {
        /*  arg type check for number is added extra*/
      if(NCSCLI_STRING == i_argval->i_arg_type || NCSCLI_NUMBER == i_argval->i_arg_type) {
         if(NCSCLI_NUMBER == i_argval->i_arg_type) 
         {
           uns32 temp = 0 ,len =0 ;
           temp = i_argval->cmd.intval;
           while(temp)
           {
               len++;
               temp/=10;
           }
           temp = i_argval->cmd.intval;
           i_argval->cmd.intval = 0;
           if(i_argval->cmd.strval)
               m_MMGR_FREE_CLI_DEFAULT_VAL(i_argval->cmd.strval);
           i_argval->cmd.strval = m_MMGR_ALLOC_CLI_DEFAULT_VAL(len+1);
           if(!i_argval->cmd.strval) return NCSCC_RC_FAILURE;
           memset(i_argval->cmd.strval,'\0',len+1); 
           sprintf(i_argval->cmd.strval,"%d",temp);
           i_argval->i_arg_type = NCSCLI_STRING;
         }
         ret = CLI_SUCCESSFULL_MATCH;
      }
      else {
         /* Arg var is not a string, it can be a IP address or Int or ..*/
         m_CHECK_IF_NUMBER(i_argval->i_arg_type, i_argval->cmd, o_cli_cmd_error->errstring);
         ret = CLI_NO_MATCH;
      }
   }
   else if(NCSCLI_MACADDR == pCli->ctree_cb.cmdElement->tokType) {
      if(NCSCLI_MACADDR == i_argval->i_arg_type) {
         convert_to_macaddr(i_argval, i_argval->cmd.strval);         
         ret = CLI_SUCCESSFULL_MATCH;
      }
      else {
         /* Arg variable is an invalid number, so update error struct */
         strcpy(o_cli_cmd_error->errstring, i_argval->cmd.strval);
         ret = CLI_NO_MATCH;
      }      
   }
   else if(NCSCLI_COMMUNITY == pCli->ctree_cb.cmdElement->tokType) {
      if(NCSCLI_COMMUNITY == i_argval->i_arg_type) {
         convert_to_community(i_argval, i_argval->cmd.strval);         
         ret = CLI_SUCCESSFULL_MATCH;
      }
      else {
         /* Arg variable is an invalid number, so update error struct */
         if(NCSCLI_NUMBER == i_argval->i_arg_type)
            sysf_sprintf(o_cli_cmd_error->errstring, "%d", i_argval->cmd.intval);
         else
            strcpy(o_cli_cmd_error->errstring, i_argval->cmd.strval);
         ret = CLI_NO_MATCH;
      }
   }
   else if(NCSCLI_NUMBER == pCli->ctree_cb.cmdElement->tokType) {
      if(NCSCLI_NUMBER == i_argval->i_arg_type) {
         ret = CLI_SUCCESSFULL_MATCH;
      }
      else {
         /* Arg variable is an invalid number, so update error struct */
         strcpy(o_cli_cmd_error->errstring, i_argval->cmd.strval);
         ret = CLI_NO_MATCH;
      }
   }
   else if((NCSCLI_IPv4 == pCli->ctree_cb.cmdElement->tokType)
      || (NCSCLI_MASKv4 == pCli->ctree_cb.cmdElement->tokType)) {

      /* Check if arg val type is also IPv4 address */
      if(NCSCLI_IPv4 == i_argval->i_arg_type) {
         uns_ipaddr = cli_ipv4_to_int((int8 *)i_argval->cmd.strval);
         m_MMGR_FREE_CLI_DEFAULT_VAL(i_argval->cmd.strval);
         i_argval->cmd.intval = uns_ipaddr;         
         ret = CLI_SUCCESSFULL_MATCH;
      }
      else {
         /* Arg var is not a string, it can be a IP6 address or Int or ..*/
         m_CHECK_IF_NUMBER (i_argval->i_arg_type, i_argval->cmd, o_cli_cmd_error->errstring);
         ret = CLI_NO_MATCH;
      }
   }   
   else if(NCSCLI_CIDRv4 == pCli->ctree_cb.cmdElement->tokType) {

      /* Check if arg val type is also IPv4 address */
      if(NCSCLI_CIDRv4 == i_argval->i_arg_type) {
         /* Convert mask to IP address and store mask in ncscli_ipv4_pfx 
            structure. 
         */
         if(NCSCLI_CIDRv4 == i_argval->i_arg_type) {
            strcpy(ipmask, (int8 *)i_argval->cmd.strval);                 
            
            iptoken = sysf_strtok(ipmask, seps);
            if(iptoken != 0) {
               iptoken = sysf_strtok(0, seps);
               ret = CLI_SUCCESSFULL_MATCH;
            }                
            
            if(ret == CLI_SUCCESSFULL_MATCH) {
               m_MMGR_FREE_CLI_DEFAULT_VAL(i_argval->cmd.strval);
               i_argval->cmd.ip_pfx.ip_addr = cli_ipv4_to_int(ipmask);
               if(iptoken) i_argval->cmd.ip_pfx.len = (uns8)atoi(iptoken);
               else i_argval->cmd.ip_pfx.len = 0;               
            }
         }
      }
      else {
         /* Arg var is not a string, it can be a IP6 address or Int or ..*/
         m_CHECK_IF_NUMBER (i_argval->i_arg_type, i_argval->cmd, o_cli_cmd_error->errstring);
         ret = CLI_NO_MATCH;
      }
   }
   else if(NCSCLI_IPv6 == pCli->ctree_cb.cmdElement->tokType) {
      /* Check if arg val type is also IPv4 address */
      if(NCSCLI_IPv6 == i_argval->i_arg_type) {
#if(NCS_IPV6 == 1)
         cli_str_to_ipv6addr (i_argval, i_argval->cmd.strval);
         ret = CLI_SUCCESSFULL_MATCH;
#else
         ret = CLI_NO_MATCH;
#endif         
      }
      else {
         /* Arg var is not a string, it can be a IP6 address or Int or ..*/
         m_CHECK_IF_NUMBER (i_argval->i_arg_type, i_argval->cmd, 
            o_cli_cmd_error->errstring);
         ret = CLI_NO_MATCH;
      }
   }
   else if(NCSCLI_CIDRv6 == pCli->ctree_cb.cmdElement->tokType) {
      /* Check if arg val type is also IPv4 address */
      if(NCSCLI_CIDRv6 == i_argval->i_arg_type) {
#if(NCS_IPV6 == 1)
         cli_str_to_ipv6pfx (i_argval, i_argval->cmd.strval);
         ret = CLI_SUCCESSFULL_MATCH;
#else
         ret = CLI_NO_MATCH;
#endif
      }
      else {
         /* Arg var is not a string, it can be a IP6 address or Int or ..*/
         m_CHECK_IF_NUMBER (i_argval->i_arg_type, i_argval->cmd, 
            o_cli_cmd_error->errstring);
         ret = CLI_NO_MATCH;
      }
   }
   else if(NCSCLI_PASSWORD == pCli->ctree_cb.cmdElement->tokType) {    
      /* Login flag is false, user could have typed command/passwd */
      if(FALSE == pCli->loginFlag) return CLI_NO_MATCH;
      
      /* Check if arg is a number, if so it will convert to a string */
      m_CHECK_IF_NUMBER(i_argval->i_arg_type, i_argval->cmd, passwdbuf);
      
      m_CLI_GET_CURRENT_CONTEXT(pCli, trvmarker);
      
      /* This is a password token, invoke registered function to
      check password validity */
      if(trvmarker->accPswd.i_encrptflag) {                            
         if(trvmarker->accPswd.passwd.i_passfnc) {
            if(trvmarker->accPswd.passwd.i_passfnc(passwdbuf, trvmarker->name))
               ret = CLI_PASSWORD_MATCHED;
            else ret = CLI_INVALID_PASSWD;
         }
      }
      else {
         if(!strcmp(trvmarker->accPswd.passwd.i_passwd, passwdbuf)) {
            ret = CLI_PASSWORD_MATCHED;
         }
         else {
            m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_INVALID_PASSWD);
            ret = CLI_INVALID_PASSWD;
         }
      }
   }
   
   return ret;
}

/*****************************************************************************

  PROCEDURE NAME:        CLI_NODE_CHANGE

  DESCRIPTION:           This function changes the node to new node as
                         specified as part of command desc.
  
  ARGUMENTS
   CLI_CB *              Control block pointer

  RETURNS: 
   CLI_CMD_NODE *       Pointer to CLI_CMD_NODE structure   

  NOTES:
        
*****************************************************************************/
CLI_CMD_NODE *cli_node_change(CLI_CB *pCli)
{
   int8            *token, nodePath[CLI_NODEPATH_LEN];
   CLI_CMD_NODE    *tmpnode = 0;
   NCSCLI_NOTIFY_INFO_TAG info_tag;
   
   /* Copy new nodepath info */
   strcpy(nodePath, pCli->ctree_cb.cmdElement->nodePath);
   token = sysf_strtok(nodePath, CLI_TOK_SEPARATOR);
   
   /* Set tmpnode to root node */
   tmpnode = pCli->ctree_cb.rootMrkr;
   if(!tmpnode) return tmpnode;
   
   if(0 != strcmp(tmpnode->name, token)) return 0;
   
   /* Check if path user has given exists */
   while(0 != token) {                                       
      /* extract next node from nodepath */
      token = sysf_strtok( 0, CLI_TOK_SEPARATOR );
      if(0 != token) {
         /* Get child node and check if it exists */
         tmpnode = tmpnode->pChild; 
         if(!tmpnode) return tmpnode;
         
         if(0 != strcmp(tmpnode->name, token))
         {
            /* If node does not matches, check if there are
               any sibling nodes 
            */
            if(tmpnode->pSibling) {
               tmpnode = tmpnode->pSibling;
               /* Loop thru sibling nodes until a match is found */
               for(;;) {
                  if(0 != strcmp(tmpnode->name, token)) {
                     if(tmpnode->pSibling) tmpnode = tmpnode->pSibling;
                     else {
                        tmpnode = 0;
                        /* Notify user that node does not exist */                                    
                        info_tag.i_notify_hdl = pCli->cli_notify_hdl;
                        info_tag.i_hdl =  pCli->cli_hdl;
                        info_tag.i_cb = pCli;
                        info_tag.i_evt_type = NCSCLI_NOTIFY_EVENT_CLI_INVALID_NODE;
                        pCli->cli_notify(&info_tag);                        
                        break;
                     }
                  }
                  else break;                  
               }
            }
            else {
               /* Reached end of sibling nodes, match not found
                  so return NULL 
               */
               tmpnode = 0;
               break;
            }
         }
      }
   }
   return tmpnode;
}

/*****************************************************************************

  PROCEDURE NAME:       CLI_CHECK_RANGE

  DESCRIPTION:          This function checks if the given Number lies
                        in the range specified.
  
  ARGUMENTS
   CLI_CB *             Pointer to CLI_CB structure
   NCSCLI_ARG_VAL *        Pointer to NCSCLI_ARG_VAL structure
   CLI_CMD_ERROR *      Pointer to CLI_CMD_ERROR structure

  RETURNS: 
   uns32                
        CLI_SUCCESSFULL_MATCH If given number lies in range
        CLI_NO_MATCH          If given number is out of range

  NOTES:
        
*****************************************************************************/
uns32 
cli_check_range(CLI_CB              *pCli, 
                NCSCLI_ARG_VAL       *i_arg_record, 
                CLI_CMD_ERROR       *o_cli_cmd_error)
{
   uns32 ret = CLI_SUCCESSFULL_MATCH;
   uns32 lvalue, hvalue;
   
   /* For Integer elements check if given value lies in the range */
   if(NCSCLI_NUMBER == pCli->ctree_cb.cmdElement->tokType) {
      lvalue = *(uns32 *)pCli->ctree_cb.cmdElement->range->lLimit;
      hvalue = *(uns32 *)pCli->ctree_cb.cmdElement->range->uLimit;
      /* Commented as a part of fixing the bug 59854 */
      /*if ((i_arg_record->cmd.intval < lvalue) || (i_arg_record->cmd.intval > hvalue)) {          
         sysf_sprintf(o_cli_cmd_error->errstring, "%u", i_arg_record->cmd.intval);
         o_cli_cmd_error->errorpos = 0;
         ret = CLI_NO_MATCH;
         m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_RANGE_NOT_INT);   
      } */     
      /*  ( boundary flag is also virified to determine whether range crossed) */
      if ((i_arg_record->cmd.intval < lvalue) || (i_arg_record->cmd.intval > hvalue) || pCli->outOfBoundaryToken) {
          if( pCli->outOfBoundaryToken)
          {
              sysf_sprintf(o_cli_cmd_error->errstring, "%s", pCli->outOfBoundaryToken);
              /*free(pCli->outOfBoundaryToken);*/
              m_MMGR_FREE_OUTOFBOUNDARY_TOKEN(pCli->outOfBoundaryToken);
              pCli->outOfBoundaryToken = NULL;
          }
          else 
              sysf_sprintf(o_cli_cmd_error->errstring, "%u", i_arg_record->cmd.intval);
          o_cli_cmd_error->errorpos = 0;
          ret = CLI_NO_MATCH;
          m_LOG_NCSCLI_HEADLINE(NCSCLI_HDLN_CLI_RANGE_NOT_INT);
      }
   }
   return ret;
}


/*****************************************************************************

  PROCEDURE NAME:       CLI_ADD_DEFAULT_VALUES

  DESCRIPTION:          This function checks if there are any default
                        values that are associated with the token. If the
                        user has not entered the token and there are
                        default values, then the defalut values are
                        sent as part of arg structure to CEF.
                        
  
  ARGUMENTS
   CLI_CMD_ELEMENT *    Pointer to CLI_CMD_ELEMENT structure
   NCSCLI_ARG_SET *        Pointer to NCSCLI_ARG_SET structure
   uns32                Index of token that has default values

  RETURNS: 
   none

  NOTES:
        
*****************************************************************************/

void cli_add_default_values(CLI_CMD_ELEMENT * i_cmdElement, 
                              NCSCLI_ARG_SET * o_cmd_tokens, 
                              uns32 i_index)
{
   uns32 count=0, pos_val=o_cmd_tokens->i_pos_value, len = 0, uns_ipaddr;
   
   /* There is a default value and user has not entered the corresponding
      variable, add the var to cmd_arg struct at index-1 postion 
   */
   
   /* Get args count from arg struct */
   m_CLI_GET_LAST_TOKEN(pos_val);   
   for(;;) {
      /* move last token one position up */
      if(count >= i_index) {
         o_cmd_tokens->i_arg_record[count+1] = o_cmd_tokens->i_arg_record[count];    
         count--;
      }
      else break;
   }
   
   /* Add default values to arg struct */
   if(i_cmdElement->tokType == NCSCLI_KEYWORD) {
      /* If token type is keyword, then set to defaultkeyword. This
         bit-pos is needed to be set to 1 while sending to CEF 
      */
      o_cmd_tokens->i_arg_record[i_index].i_arg_type = NCSCLI_KEYWORD_ARG;
   }
   else {
      o_cmd_tokens->i_arg_record[i_index].i_arg_type = i_cmdElement->tokType;
   }
   
   /* Copy the default values into Arg struct */    
   if(NCSCLI_NUMBER == o_cmd_tokens->i_arg_record[i_index].i_arg_type) {
      /*o_cmd_tokens->i_arg_record[i_index].cmd.intval = *((uns8 *)i_cmdElement->defVal);*/
      o_cmd_tokens->i_arg_record[i_index].cmd.intval = *((uns32 *)i_cmdElement->defVal); 
   }
   else if(NCSCLI_IPv4 == o_cmd_tokens->i_arg_record[i_index].i_arg_type) { 
         uns_ipaddr = cli_ipv4_to_int((int8 *)i_cmdElement->defVal);
         o_cmd_tokens->i_arg_record[i_index].cmd.intval = uns_ipaddr;         
   }
   else {
      if(o_cmd_tokens->i_arg_record[i_index].cmd.strval)
         m_MMGR_FREE_CLI_DEFAULT_VAL(o_cmd_tokens->i_arg_record[i_index].cmd.strval);
      
      len = strlen(i_cmdElement->defVal) + 1;
      o_cmd_tokens->i_arg_record[i_index].cmd.strval = m_MMGR_ALLOC_CLI_DEFAULT_VAL(len);
      if(!o_cmd_tokens->i_arg_record[i_index].cmd.strval) return;
      
      memset(o_cmd_tokens->i_arg_record[i_index].cmd.strval, '\0', len);
      strncpy((int8 *)o_cmd_tokens->i_arg_record[i_index].cmd.strval, 
         (int8 *)i_cmdElement->defVal, len-1);
   }
   
   /* Increment the cmd pos by one and OR it with one */
   o_cmd_tokens->i_pos_value = o_cmd_tokens->i_pos_value << 1;
   o_cmd_tokens->i_pos_value = o_cmd_tokens->i_pos_value | 1;    
   return;
}

/*****************************************************************************

  PROCEDURE NAME:        CLI_CHECK_SYNTAX

  DESCRIPTION:          
  
    This function takes the token structure passed as argument and
    checks command syntax.

    Returns the following conditions
    CLI_SUCCESSFULL_MATCH
    CLI_PARTIAL_MATCH
    CLI_AMBIGUOUS_MATCH
    CLI_NO_MATCH
    CLI_ERROR

    The algorithim for traversing a tree is as follows

    The order of traversing pointers in cmd_element is
    1. Traverse elements in data pointer
    2. Traverse elements in sibling pointer
    3. If a match is found then traverse to child ptr.
    4. If it is an optional node, then traverse to child ptr even
       if a match is not found.

    The following are the rules for traversing pointers in cmd_element struct
    1. If isMand is FALSE it indicates that it is optional node
    2. If dataptr is present,it indicates an optional element.
    3. siblingptr indicates that a OR member is present
    4. The childptr indicates that one more level exists

    The command tree can contains two dummy nodes which are either 
    'optional nodes' which is created when a optional '[' is encountered
    and 'group node' which is created when a '{' brace is encountered.
 
    Command Tree traversing 

    1. Get pointer to command node(CLI_CMD_NODE) structure
    2. From the CLI_CMD_NODE struct.Traverse through the cmdnode pointer to find the command.
       If the first element in CLI_CMD_ELEMENT struct is not the command then
       traverse through 'siblingptr' of CLI_CMD_ELEMENT struct until it is
       found or you have reached end of list.
    3. If the command search results in partial match then add the command to some stack
    4. If the search results in more than 1 partial match, it is ambiguous command
    5. If the command is found then traverse according to order of traversing. In this case go to child.
    6. Check cmd_element type. If it is mandatory then
        - the command is mandatory. 
        - check sibling pointer. If siblingptr is not null it indicates that an exclusive OR condition is present.
        - Loop thru sibling pointers following command traversing rules
        - at each level compare user entered 'token' type with elements present in command tree.
        - If 'keyword' then check for full or partial match. If partial match add to stack.
        - If literal then add into arg struct
    7. If cmd_element is not mandatory, it means it is an optional node
        - Traverse according to the commad tree traversing rules.
        - If node is not found then go to child node of optionalnode


    8. Two or more optional nodes can be combined with OR pointers.
    9. Scan thru order of precedence to find the correct node.
    10. If present then check type i.e. keyword or literal.
    11. When a match is found Check if the command results in mode change.
        - if so then change mode and prompt.

    12. Check if there are default values associated with the cmd_elemnt.
        - if there are default values then if the user has not entered
          then fill the arg struct with default values.

      Based on CLI_CMD_EXECUTE
      If it is CLI_EXECUTE then get the callback function associated with
      last keyword in the command

      If it is CLI_TAB then get the complete command name

      If it is CLI_HELP then get the next parameter name and help string or list
      of all commands present under the node.


  ARGUMENTS
   int8 *                string to be tokenized.

  RETURNS: 
   NCSCLI_ARG_SET *         The arg structure to which the tokens are copied
    
*****************************************************************************/
uns32 cli_check_syntax(CLI_CB             *pCli, 
                       NCSCLI_ARG_SET      *io_cmd_tokens, 
                       CLI_EXECUTE_PARAM  *io_param, 
                       CLI_CMD_ERROR      *cli_cmd_error)
{
    uns32           ret=CLI_NO_MATCH,
                    i=0, traverseCount = 0,
                    cmd_pos=io_cmd_tokens->i_pos_value,
                    default_pos = 0, lowerLimit = 0, upperLimit = 0,
                    optStack_ptr = 0,
                    len=0,
                    prev_optStack_ptr = 0;                         

    NCS_BOOL         partialmatch = FALSE,
                    loopflag = FALSE, 
                    mandatoryelement = FALSE,
                    defaultmatch = FALSE,
                    command_matched = FALSE,
                    endofcmd = FALSE,
                    help_str = FALSE,
                    exit_str = FALSE;

    CLI_CMD_ELEMENT *partialmatchpointer = 0, 
                    *tabnode = 0,
                    *defaultnode = 0,
                    *recursivemarker = 0;           
    CLI_CMD_ELEMENT *optnode_stack[CLI_DUMMY_STACK_SIZE];    

   /* Set Level and current node markers to first command element */
   while(cmd_pos & 1) {  
      ret = cli_check_token(pCli, &io_cmd_tokens->i_arg_record[i], cli_cmd_error);       
 
      /* If No_Match check if there are any default values */
      if(ret == CLI_NO_MATCH && pCli->ctree_cb.cmdElement->defVal) {
         defaultnode = pCli->ctree_cb.cmdElement;
         defaultmatch = TRUE;
         default_pos = i;
      }
      else {
         /* Reset variables */
         defaultnode = 0;
         defaultmatch = FALSE;
         default_pos = 0;
      }
 
      if(CLI_NO_MATCH == ret || CLI_PARTIAL_MATCH == ret) {
         /* If token type is NCSCLI_GROUP and lower range is valid value then
            store marker, need to traverse max times as indicated in range 
         */
         if((pCli->ctree_cb.cmdElement->tokType == NCSCLI_CONTINOUS_EXP)
          && (pCli->ctree_cb.cmdElement->range)) {
            if(!traverseCount) {
               recursivemarker = pCli->ctree_cb.cmdElement;
               lowerLimit = *(uns32 *)pCli->ctree_cb.cmdElement->range->lLimit;          
               upperLimit = *(uns32 *)pCli->ctree_cb.cmdElement->range->uLimit;
            }
       
            /* Keep number of times this node has been traversed */
            traverseCount++;
       
            if(upperLimit && (traverseCount > upperLimit)) {
               ret = CLI_AMBIGUOUS_MATCH;
               break;
            }
       
            if(pCli->ctree_cb.cmdElement->node_ptr.pChild)
               pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pChild;
            continue;
         }            
    
         /* check if there has been a partial match already If so it 
          * indicates a Ambiguous match */
         if(CLI_PARTIAL_MATCH == ret) {
            if(partialmatch) {
               ret = CLI_AMBIGUOUS_MATCH;
               break;
            }
            else {
               /* first partial match encountered, set status and
                  backup pointer position 
               */
               partialmatch = TRUE;
               partialmatchpointer = pCli->ctree_cb.cmdElement;
            }
         }
    
         /* match not found traverse further through pDataPtr, a pDataPtr indicates that
            this is a grp/opt node 
         */
         if(pCli->ctree_cb.cmdElement->node_ptr.pDataPtr) {
            m_TRAVERSE_DATANODE(pCli->ctree_cb.cmdElement, optnode_stack, optStack_ptr);
       
            /* Opt/Grp node, set command-matched to FALSE */
            command_matched = FALSE;
            continue;
         }
    
         /* siblingptr if exists indicates a OR condition */
         if(pCli->ctree_cb.cmdElement->node_ptr.pSibling) {
            pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pSibling;
            continue;
         }
    
         /* Reached end of a node, both data and sibling ptrs are 0 */
         loopflag = FALSE;                     
    
         if(!partialmatch && !defaultmatch) {
            if(mandatoryelement) break;                
       
            /* It is a mandatory command, check if therr are sibling nodes
               to parent grp/opt node 
            */
            if(pCli->ctree_cb.cmdElement->isMand) {
               if(optStack_ptr) mandatoryelement = TRUE;
               else  {
                  if(pCli->ctree_cb.cmdElement->tokType != NCSCLI_WILDCARD) break;                
               }
            }
         }
    
         /* check if there are any elements in opt stack, */
         while(optStack_ptr) {             
            prev_optStack_ptr = optStack_ptr;
       
            /* Go to prev opt/grp node */
            optStack_ptr--;
       
            /* Check if opt/grp node is at same level, if they are at diff levels and
               command is mandatory then need to exit 
               ex: [{regexp expr}|{filter list}] {permit|deny}, expr is mandatory and
               the grp/opt nodes are in parent level 
            */
            if(!partialmatch && mandatoryelement && 
               (pCli->ctree_cb.cmdElement->tokLvl != optnode_stack[optStack_ptr]->tokLvl)) {
               prev_optStack_ptr = optStack_ptr;
               break;
            }
       
            pCli->ctree_cb.cmdElement = optnode_stack[optStack_ptr];
       
            /* Check is there was a match */
            if(!command_matched || partialmatch) {
               
               /* There was no match, so traverse thru sibling ptr */
               if(pCli->ctree_cb.cmdElement->node_ptr.pSibling) {
                  pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pSibling;
                  command_matched = FALSE;
                  loopflag = TRUE;
                  prev_optStack_ptr = optStack_ptr;
                  break;
               }
            }
       
            if(!partialmatch) {
               if(pCli->ctree_cb.cmdElement->isMand) {
                  prev_optStack_ptr = optStack_ptr;
                  break;
               }
            }
       
            /* check if dummy node has childnode, if there was a default match 
               then don't traverse to child 
            */
            if(pCli->ctree_cb.cmdElement->node_ptr.pChild && !defaultmatch) {
               pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pChild; 
               loopflag = TRUE;                    
               prev_optStack_ptr = optStack_ptr;
               break;
            }                                                        
         }
    
         if(loopflag) {
            prev_optStack_ptr = optStack_ptr;
            continue;
         }
    
         /* Give preference to partial match */
         if(defaultmatch && !partialmatch) {
            cli_add_default_values(defaultnode, io_cmd_tokens, default_pos); 
            cmd_pos = cmd_pos << 1;
            cmd_pos = cmd_pos | 1;
       
            /* Since default values are added set status to sucess */
            ret = CLI_SUCCESSFULL_MATCH;                               
            pCli->ctree_cb.cmdElement = defaultnode;
         }
    
         /* If partial match then go to prev partial match node, otherwise exit */
         if(partialmatch) {
            pCli->ctree_cb.cmdElement = partialmatchpointer;
       
            /* Added for Bug : partial keywords */
            len = strlen(pCli->ctree_cb.cmdElement->tokName) + 1;
       
            /* Free if any prev alloc string value */ 
            if(io_cmd_tokens->i_arg_record[i].cmd.strval)
               m_MMGR_FREE_CLI_DEFAULT_VAL(io_cmd_tokens->i_arg_record[i].cmd.strval);
       
            /* Alloc new chunk of memeory */
            io_cmd_tokens->i_arg_record[i].cmd.strval = m_MMGR_ALLOC_CLI_DEFAULT_VAL(len);
            if(!io_cmd_tokens->i_arg_record[i].cmd.strval) return ret;
            memset(io_cmd_tokens->i_arg_record[i].cmd.strval, '\0', len);               
            strncpy(io_cmd_tokens->i_arg_record[i].cmd.strval, 
               pCli->ctree_cb.cmdElement->tokName, len-1);
            partialmatch = FALSE;
            ret = CLI_SUCCESSFULL_MATCH;            
            optStack_ptr = prev_optStack_ptr;/* This means that though token was partiall match
                                                we made it full match, so matain the prevoius 
                                                optional stack count is prevoisly there */
            /* Check if the comand is help string or not */
            if(0 == cli_stricmp(io_cmd_tokens->i_arg_record[i].cmd.strval, CLI_HELP_DESC))
               help_str = TRUE;
       
            if(0 == cli_stricmp(io_cmd_tokens->i_arg_record[i].cmd.strval, CLI_EXIT))
               exit_str = TRUE;
         }
         else {
            /* Still no match found exit */
            if(pCli->ctree_cb.cmdElement->tokType != NCSCLI_WILDCARD)
               if(ret != CLI_SUCCESSFULL_MATCH) {
               ret = CLI_NO_MATCH;
               break;
            }
         }                    
    
         /* Chek if its wildcard */
         if(pCli->ctree_cb.cmdElement->tokType == NCSCLI_WILDCARD) {
            ret = CLI_PARTIAL_MATCH/*CLI_SUCCESSFULL_MATCH*/;
            io_param->o_tokprocs = i;
            io_param->cmd_exec_func = pCli->ctree_cb.cmdElement->cmd_exec_func;
            io_param-> cmd_access_level = pCli->ctree_cb.cmdElement->cmd_access_level;
            io_param->bindery = pCli->ctree_cb.cmdElement->bindery;
            break;
         }
      }            
      else if(CLI_PASSWORD_MATCHED == ret) {
         cmd_pos = cmd_pos >> 1;
         i++;
         io_param->o_status = CLI_PASSWORD_MATCHED;
         ret = CLI_PASSWORD_MATCHED;            
      }
      else if(CLI_INVALID_PASSWD == ret) {
         io_param->o_status = CLI_INVALID_PASSWD;
         break;
      }
  
      if(CLI_SUCCESSFULL_MATCH == ret || CLI_PASSWORD_MATCHED == ret) {   
         /* If 'TAB' operation store command element */
         if(CLI_TAB == io_param->i_cmdtype) {
            tabnode = pCli->ctree_cb.cmdElement;           
            partialmatch = FALSE; /* Reset the previous partiall match flag */
         }
         
         /* Check if there are any range checking to be take care of */
         if(0 != pCli->ctree_cb.cmdElement->range) {
            ret =  cli_check_range(pCli, &io_cmd_tokens->i_arg_record[i], cli_cmd_error);
            if(CLI_NO_MATCH == ret) break;
         }            
         
         /* check if there is a command node change associated with
            this token, if so then on sucessfull completion of command
            change to new node
         */
         if(pCli->ctree_cb.cmdElement->modChg) {
            pCli->ctree_cb.currNode = cli_node_change(pCli);
            if(0 == pCli->ctree_cb.currNode) {
               ret = CLI_NO_MODE; 
               break;
            }              
         }
         
         /* If the parms are continous then set the argtype to NCSCLI_KEYWORD_ARG
            this needs to be sent to CEF.
         */
         if(pCli->ctree_cb.cmdElement->isCont)
            io_cmd_tokens->i_arg_record[i].i_arg_sub_type = NCSCLI_KEYWORD_ARG;
         
         /* If keyword and optional then set argtype to NCSCLI_KEYWORD_ARG
            this needs to be sent to CEF. If level value is > 0
            indicates that it is part of opt parameter 
         */
         if(pCli->ctree_cb.cmdElement->tokType == NCSCLI_KEYWORD)
            if (!pCli->ctree_cb.cmdElement->isMand || optStack_ptr) {
            io_cmd_tokens->i_arg_record[i].i_arg_sub_type = NCSCLI_KEYWORD_ARG;
         }            
            
         /* Increment counter and get next token */
         cmd_pos = cmd_pos >> 1;
         io_cmd_tokens->i_arg_record[i].sym_valid = TRUE;              
         i++;                            
         command_matched = TRUE;
         mandatoryelement = FALSE;
         
         /* match found then go to pChild, if child is present then 
         set flag to partial match */
         
         if(pCli->ctree_cb.cmdElement->node_ptr.pChild) {
            pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pChild; 
            ret = CLI_PARTIAL_MATCH;
         }
         else {
            /* A level value indicates there are Opt/grp nodes */
            if(optStack_ptr) {
               /* Traverse to prev opt node */
               m_TRAVERSE_PREV_OPTNODE(pCli->ctree_cb.cmdElement, optnode_stack, optStack_ptr);
            }
            else {
               /* loop back to main node of this command */
               while(0 == pCli->ctree_cb.cmdElement->prntTokLvl)
                  pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pParent;
            }
            
            /* check if child is present then set flag to partial match */
            if(pCli->ctree_cb.cmdElement->node_ptr.pChild) {
               pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pChild;
               ret = CLI_PARTIAL_MATCH;
            }
            else {
               /* no more child nodes, reached end of tree, check is there are more
                  input tokens if so set to error 
               */
               if(cmd_pos & 1) {
                  /* check is it is recursive node otherwise exit */
                  if(recursivemarker != 0) {
                     pCli->ctree_cb.cmdElement = recursivemarker;
                     continue;
                  }
                  
                  /* Checks is it is a number and if so converts to err string */
                  m_CHECK_IF_NUMBER(io_cmd_tokens->i_arg_record[i].i_arg_type, 
                     io_cmd_tokens->i_arg_record[i].cmd, cli_cmd_error->errstring);                       
                  io_cmd_tokens->i_arg_record[i].sym_valid = FALSE;
                  cli_cmd_error->errorpos = 0;
                  ret = CLI_NO_MATCH;
               }            
               
               /* Set endofcmd flag to true so no need to loop for default values*/
               endofcmd = TRUE;
               
               /* If match is found then copy CEF */
               if((CLI_SUCCESSFULL_MATCH == ret || CLI_PASSWORD_MATCHED == ret) &&
                  CLI_EXECUTE == io_param->i_cmdtype) {
                  if(help_str) {
                     /* User has requested help information */  
                     io_param->i_cmdtype = CLI_HELP;
                     
                     /* Free if any prev alloc string value */ 
                     if(io_cmd_tokens->i_arg_record[0].cmd.strval)
                        m_MMGR_FREE_CLI_DEFAULT_VAL(io_cmd_tokens->i_arg_record[0].cmd.strval);
                     
                     /* Alloc new chunk of memeory */
                     len = strlen("?")+1;
                     io_cmd_tokens->i_arg_record[0].cmd.strval = m_MMGR_ALLOC_CLI_DEFAULT_VAL(len);
                     if(!io_cmd_tokens->i_arg_record[0].cmd.strval) return ret;
                     memset(io_cmd_tokens->i_arg_record[0].cmd.strval, '\0', len);               
                     strncpy(io_cmd_tokens->i_arg_record[0].cmd.strval,"?", len-1);
                     return (io_param->o_status = CLI_SUCCESSFULL_MATCH);
                  }
                  
                  if(exit_str) strncpy(io_param->i_cmdbuf, CLI_EXIT, strlen(CLI_EXIT));
                  
                  io_param->cmd_exec_func = pCli->ctree_cb.cmdElement->cmd_exec_func;
                  io_param-> cmd_access_level = pCli->ctree_cb.cmdElement->cmd_access_level;
                  io_param->bindery = pCli->ctree_cb.cmdElement->bindery;
               }               
               break;
            }
         }
      }     
   }

   /* Check if the user has keyed in the command req no of times. This is
   required for recursive commands where min value is more then 1 */
   if(traverseCount < lowerLimit) {
      if(ret == CLI_SUCCESSFULL_MATCH) ret = CLI_PARTIAL_MATCH;
   }

   /* Reached end of command tree or end of input tokens.
      if not end of cmdtree check if there are any default values 
   */    
   if(CLI_EXECUTE == io_param->i_cmdtype && !endofcmd) {
      /* If cmd element has default values or optional, then
         only you need to check for default values 
      */
      if(pCli->ctree_cb.cmdElement->defVal || !pCli->ctree_cb.cmdElement->isMand) {
         for(;;) {
            /* This fix is added to check if there are mandotry node 
               in the list of optinal nodes 
            */
            if(ret != CLI_SUCCESSFULL_MATCH && pCli->ctree_cb.cmdElement->isMand) {
               break;
            }
       
            if(pCli->ctree_cb.cmdElement->defVal) {
               cli_add_default_values(pCli->ctree_cb.cmdElement, io_cmd_tokens, i); 
               ret = CLI_SUCCESSFULL_MATCH;                        
               i++; /* Fix to move to the next record in case of default values */
          
               if(pCli->ctree_cb.cmdElement->node_ptr.pChild)
               {                
                  pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pChild;
                  continue;
               }
            }
       
            /* If pDataPtr then traverse through datanode */
            if(pCli->ctree_cb.cmdElement->node_ptr.pDataPtr) {
               m_TRAVERSE_DATANODE(pCli->ctree_cb.cmdElement, 
                  optnode_stack, optStack_ptr);
               continue;
            }
       
            /* pSibling if exists indicates a OR condition */
            if(pCli->ctree_cb.cmdElement->node_ptr.pSibling) {
               pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pSibling;
               continue;
            }
       
            /* A level value indicates there are Opt/grp nodes */
            if(optStack_ptr) {
               /* Traverse to prev opt node */
               m_TRAVERSE_PREV_OPTNODE(pCli->ctree_cb.cmdElement, optnode_stack, optStack_ptr);
            }
            else {
               /* loop back to main node of this command */
               while(0 == pCli->ctree_cb.cmdElement->prntTokLvl)
                  pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pParent;
            }
       
            /* check if child is present then set flag to partial match */
            if(pCli->ctree_cb.cmdElement->node_ptr.pChild) {
               pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pChild;
            }
            else {
               /* no more child nodes, reached end of tree */
               if(CLI_SUCCESSFULL_MATCH == ret && CLI_EXECUTE == io_param->i_cmdtype) {
                  io_param->cmd_exec_func = pCli->ctree_cb.cmdElement->cmd_exec_func;
                  io_param-> cmd_access_level = pCli->ctree_cb.cmdElement->cmd_access_level;
                  io_param->bindery = pCli->ctree_cb.cmdElement->bindery;             
               }
               break;
            }
         }
      }
   }
    
   if(CLI_EXECUTE == io_param->i_cmdtype) {
      /* Now check if there is a partial match, scan thru rest of elements
         to see if there are any mandatory elements.
 
         This is required because after otional elements there can be mandatory
         elements. so scan rest of list to find mandatory elements. If present
         it is a 'partial match'.
   
         If not mandatory elements are present it is a sucessfull match 
       */    
      if(CLI_PARTIAL_MATCH == ret) {   
         /* check if child req access restrictions */
         if(NCSCLI_PASSWORD == pCli->ctree_cb.cmdElement->tokType) {
            /* Set status to passreq and break, to allow i/o module to 
               prompt for password 
            */
            io_param->o_status = CLI_PASSWD_REQD;
            ret = CLI_PASSWD_REQD;
         }
         else {
            /* set match to CLI_SUCCESSFULL_MATCH by default, if mandatory
               element is encountered then set back to partial 
            */
            NCS_BOOL is_any_param_optional = FALSE;
            NCS_BOOL is_any_parm_wildcard = FALSE;
            ret = CLI_SUCCESSFULL_MATCH;    
          
            while(pCli->ctree_cb.cmdElement) {
               /* Check for the Wildcard */
               is_any_parm_wildcard = is_any_wildcard_node(&pCli->ctree_cb.cmdElement);
               if(!is_any_parm_wildcard) {
                  /* check if there is any optional node present */             
                  is_any_param_optional = is_any_optional_node(&pCli->ctree_cb.cmdElement);
                  if(!is_any_param_optional) ret = CLI_PARTIAL_MATCH;
               }                   
             
               /* Check is there any child nodes */
               if(pCli->ctree_cb.cmdElement->node_ptr.pChild && !is_any_param_optional) {
                  pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pChild;
               }
               else {
                  /* No more child nodes, reached last node */
                  io_param->cmd_exec_func = pCli->ctree_cb.cmdElement->cmd_exec_func;
                  io_param-> cmd_access_level = pCli->ctree_cb.cmdElement->cmd_access_level;
                  io_param->bindery = pCli->ctree_cb.cmdElement->bindery;                   
                  break;
               }
            }
         }
      }   
   }
    
   /* If user has pressed 'TAB' then need to complete the keyword, if the result is
    either sucessfull or partial match then only needed to complete the word, other
    results indicate improper command 
   */
   if(CLI_TAB == io_param->i_cmdtype) {
      if(CLI_SUCCESSFULL_MATCH == ret || CLI_PARTIAL_MATCH == ret) {
         /* tmp copy to cmdElement as it is global - TBD */
         pCli->ctree_cb.cmdElement = tabnode;
      }
   }
    
   io_param->o_status = ret;
   return ret;
}

/*****************************************************************************

  PROCEDURE NAME:       CLI_FIND_ERRPOS

  DESCRIPTION:          This function finds the actual error postion 
                        in the main string and returns the error position

  ARGUMENTS:
   int8 *                The actual command string user has entered.
   int8 *                The error substring
   int32                 The error position within error substring

  RETURNS: 
    int32                The error position in actual string

*****************************************************************************/
int32 cli_find_errpos(int8 *i_actualstr, int8 *i_errstr, int32 i_errpos)
{
   int8 *token = 0, *prev_tok = 0;
   int8 *s1 = 0, *s2 = 0;
   int32 i;
   
   s1 = i_actualstr;
   s2 = i_errstr;
   
   /* search for string s2 in s1 */    
   if(!strlen(s2) || !strlen(s1)) return 0;
   
   token = strstr(s1, s2);
   if(0 == token) return 0;
   else {
      while(token) {
         prev_tok = token;  
         token = strstr(token+1, s2);
      }
   }
   
   /* find lenght of err position */
   i= prev_tok-i_actualstr + i_errpos;
   return i;
}

/*****************************************************************************

  PROCEDURE NAME:       CLI_TO_POWER

  DESCRIPTION:          Computes X raised to power of y
                          

  ARGUMENTS:
   int32                value
   int32                power to eb raised
   
  RETURNS: 
    int32               Returns raised power of value

*****************************************************************************/
int32 cli_to_power(int32 i_num, int32 i_pow)
{
   int32 index, val=1;
   
   if(i_pow < 0) return 0;
   
   for(index=0; index<i_pow; index++) val *= i_num;   
   return val;
}

/*****************************************************************************

  PROCEDURE NAME:       CLI_MODIFY_CMDARGS

  DESCRIPTION:          This function modifies the NCSCLI_ARG_SET arg values. 
                        It scans thru all the values and sets the bits for 
                        Keywords to zero. This arg struct can be sent to the 
                        sub-system.
  

  ARGUMENTS:
   NCSCLI_ARG_SET *        The arg structure containing the tokens. Initially the
                        struct will contain both keywords and variables. This
                        function will reset the keyword to 0.
   
  RETURNS: 
    none

*****************************************************************************/
void cli_modify_cmdargs(NCSCLI_ARG_SET *io_cmd_tokens)
{
   int32 i=0, cmd_pos = io_cmd_tokens->i_pos_value;
   
   /* Loop through all tokens */
   while(cmd_pos & 1) {
      /* check if it is a keyword, if so then reset bit position and value */
      if(NCSCLI_KEYWORD == io_cmd_tokens->i_arg_record[i].i_arg_type &&
         NCSCLI_KEYWORD_ARG != io_cmd_tokens->i_arg_record[i].i_arg_sub_type) {
         /* Set bit to 0 */
         io_cmd_tokens->i_pos_value = 
            io_cmd_tokens->i_pos_value ^cli_to_power(2, i);
      }        
      
      cmd_pos = cmd_pos >> 1;
      i++;
   }        
}


/*****************************************************************************
  PROCEDURE NAME:        CLI_EXECUTE_COMMAND

    DESCRIPTION:           This function executes the command. The input is
                         the action user has requested and command string.

  ARGUMENTS:
   CLI_CB *              Control Block pointer 
   CLI_EXECUTE_PARAM *   A param structure containing 
   CLI_SESSION_INFO *    Pointer of CLI_SESSION_INFO structure

  RETURNS: 
   param->o_status       Executed command status, can be
   CLI_NO_MATCH          The command did not matched.
   CLI_PARTIAL_MATCH     Incomplete complete
   CLI_AMBIGUOUS_MATCH   The command is ambiguous
   CLI_SUCCESSFULL_MATCH Entered command successfully matched

*****************************************************************************/
uns32 cli_execute_command(CLI_CB            *pCli, 
                          CLI_EXECUTE_PARAM *io_param, 
                          CLI_SESSION_INFO  *o_session)
{
    uns32           status = 0;
    NCSCLI_ARG_SET   ctok;
    CLI_CMD_ERROR   cli_cmd_error;
    NCSCLI_CEF_DATA  cdata;
    CLI_CMD_NODE    *trvmarker = 0;    
    CLI_CMD_ELEMENT *cmdElement = pCli->ctree_cb.cmdElement;
    uns32           rc = NCSCC_RC_SUCCESS;   
    
    pCli->semUsgCnt = 0;
    pCli->cefTmrFlag = FALSE;

    /* Parse the cmdbuf into tokens. returns cmdtokens struct */
    memset(&ctok, 0, sizeof(NCSCLI_ARG_SET));
    memset(&cdata, 0, sizeof(cdata));
   
   /* Commented out as a part of fixing the bug 59854 */
    /*if(NCSCC_RC_SUCCESS != cli_getcommand_tokens(io_param->i_cmdbuf, &ctok)) {
        io_param->o_status = CLI_NO_MATCH;
        rc = NCSCC_RC_FAILURE;
        goto clean_arg;
    }
    */
    /*  arg pCli extra passed */
    if(NCSCC_RC_SUCCESS != cli_getcommand_tokens(io_param->i_cmdbuf, &ctok, pCli))  {
        io_param->o_status = CLI_NO_MATCH;
        rc = NCSCC_RC_FAILURE;
        goto clean_arg;
    }

    /* Temp Reset variables */
    io_param->o_errpos = 0;
    cli_cmd_error.errorpos = 0;
    strcpy(cli_cmd_error.errstring, "");
    pCli->cefStatus = NCSCC_RC_FAILURE;
    pCli->ctree_cb.currNode = 0;

    /* memset tab string */
    memset(io_param->o_hotkey.tabstring, '\0', CLI_ARG_LEN);    

    /* Check user action */

    if(CLI_EXECUTE == io_param->i_cmdtype) {
       /* This function checks the command syntax and returns match code */
       io_param->o_status = cli_check_syntax(pCli, &ctok, io_param, &cli_cmd_error);
       
       if((CLI_SUCCESSFULL_MATCH == io_param->o_status) || 
          (CLI_PASSWORD_MATCHED  == io_param->o_status)) {
          
          /* Help String processing */
          if(CLI_HELP == io_param->i_cmdtype) {

             /* User has requested help information */
             pCli->ctree_cb.cmdElement = cmdElement;
             cli_get_helpstr(pCli, &ctok, io_param, o_session);
             goto clean_arg;              
          }
          
          if(NCSCLI_WILDCARD == pCli->ctree_cb.cmdElement->tokType)
             cli_update_cmd_arg(&ctok, io_param);
          
          /* Now command syntax has been matched, modify the arg structure to 
           * send to subsystem 
          */
          cli_modify_cmdargs(&ctok);
          
          /* get the callback function and execute it - TBD */
          if((0 != io_param->cmd_exec_func) ) {
             m_CLI_GET_CURRENT_CONTEXT(pCli, trvmarker);

             cdata.i_bindery = io_param->bindery;    
             /*if(!pCli->ctree_cb.currNode) 
                pCli->subsys_cb.i_cef_mode = pCli->ctree_cb.trvMrkr->pData; */  
             cdata.i_subsys = &pCli->subsys_cb;
             
             if(io_param->i_execcmd == TRUE)  
             {
             /* Start Timer */
             m_NCS_TMR_START(pCli->cefTmr.tid, CLI_CEF_TIMEOUT, 
                cli_cef_exp_tmr, &(cdata.i_bindery->i_cli_hdl));
             
             /* Increment Semaphore usage count */
             pCli->semUsgCnt = 1;
             /* Fic for 59359 */
             if(!ncscli_user_access_level_authenticate(pCli)) {
                 cli_display(pCli, "\n\t.....You are not authorized to execute this command.\n");
                 rc = NCSCC_RC_FAILURE;
                 /* Stop the timer */
                 m_NCS_TMR_STOP(pCli->cefTmr.tid);
                 goto clean_arg;
             }
             
             /* Invoke CEF */
             if(NCSCC_RC_SUCCESS != io_param->cmd_exec_func(&ctok, &cdata)) {
                cli_display(pCli, "\n..... Command Execution Function failed .....\n");
                rc = NCSCC_RC_FAILURE;
                
                /* Stop the timer */
                m_NCS_TMR_STOP(pCli->cefTmr.tid);
                goto clean_arg;
             }
             
             /* Block on semaphore */
             if(m_NCS_SEM_TAKE(pCli->cefSem) != NCSCC_RC_SUCCESS) {
                m_CLI_DBG_SINK_VOID(NCSCC_RC_FAILURE);
                rc = NCSCC_RC_FAILURE;
                
                /* Stop the timer */
                m_NCS_TMR_STOP(pCli->cefTmr.tid);
                goto clean_arg;
             }
             }
             
             /* End of CEF, free command specific memory */
             if(pCli->subsys_cb.i_cef_data) {
                m_MMGR_FREE_CLI_DEFAULT_VAL(pCli->subsys_cb.i_cef_data);                    
                pCli->subsys_cb.i_cef_data = 0;                
             }
             if(io_param->i_execcmd == FALSE)   
                pCli->cefStatus = NCSCC_RC_SUCCESS;
             
             /* If CEF returned success and currNode is not NULL, then change mode */
             if((NCSCC_RC_SUCCESS == pCli->cefStatus) || 
                (NCSCC_RC_CALL_PEND == pCli->cefStatus)) {
                
                if(pCli->ctree_cb.currNode) {
                   pCli->ctree_cb.trvMrkr = pCli->ctree_cb.currNode;
                   pCli->ctree_cb.modeStack
                      [++pCli->ctree_cb.modeStackPtr] = pCli->ctree_cb.currNode;
                   
                   if(CLI_PASSWORD_MATCHED != io_param->o_status) {
                    if(strcmp(pCli->ctree_cb.trvMrkr->mode , "exec") != 0)
                    {               /*For suppressing passwd for enable cmd */
                      strcat(o_session->prompt_string, "-");
                      strcat(o_session->prompt_string, pCli->ctree_cb.trvMrkr->mode);
                      
                      /* Set Mode Information */
                      pCli->ctree_cb.trvMrkr->pData = pCli->subsys_cb.i_cef_mode;                      
                      pCli->subsys_cb.i_cef_mode = 0;
                    }
                   }
                } 
             }
             else {
                if(CLI_PASSWORD_MATCHED == io_param->o_status)                                        
                   io_param->o_status = CLI_INVALID_PASSWD;                   
             }             
          }
       }
       else if(CLI_NO_MATCH == io_param->o_status) {
          /* if error get error position */
          io_param->o_errpos = cli_find_errpos(io_param->i_cmdbuf, 
             cli_cmd_error.errstring, cli_cmd_error.errorpos);
       }
    }
    else if (CLI_TAB == io_param->i_cmdtype) {
       /* user has pressed 'TAB' need to complete the keyword  */
       cli_complete_cmd(pCli, &ctok, io_param, o_session);
    }
    else if (CLI_HELP == io_param->i_cmdtype) {
       /* User has requested help information */
       cli_get_helpstr(pCli, &ctok, io_param, o_session);
    }

clean_arg:
    /* Free the allocated strval in the arg structure*/
    clean_arg(&ctok);
    if(NCSCC_RC_SUCCESS != rc) return NCSCC_RC_FAILURE;    
    return status;
}

/*****************************************************************************

  PROCEDURE NAME:       CLI_GET_HELP_DESC

  DESCRIPTION:          This function finds the help description for
                        for the command and returns the command and
                        associated help string for the command.

  ARGUMENTS:
    CLI_CB *             Control block pointer
    NCSCLI_ARG_SET *        Pointer to NCSCLI_ARG_SET structure   
    CLI_EXECUTE_PARAM *  Poinetr to CLI_EXECUTE_PARAM structure   
    CLI_CMD_ERROR *      Poinetr to CLI_CMD_ERROR structure

  RETURNS:
    CLI_NO_MATCH          If help desc could not be found
    CLI_PARTIAL_MATCH     Help desc found
    CLI_SUCCESSFULL_MATCH Help desc found
    CLI_INVALID_PASSWD    Next param is password
    CLI_PASSWORD_MATCHED  Next param is password

*****************************************************************************/
uns32 
cli_get_help_desc(CLI_CB              *pCli, 
                  NCSCLI_ARG_SET         *i_cmd_tokens, 
                  CLI_EXECUTE_PARAM   *io_param, 
                  CLI_CMD_ERROR       *o_cli_cmd_error)
{
   uns32    ret = CLI_NO_MATCH,
            i = 0,
            helpcount = 0;
   NCS_BOOL  loopflag = FALSE;
   uns32    cmd_pos = i_cmd_tokens->i_pos_value, 
            optStack_ptr = 0;    /* copy token bit pos locally */
   CLI_CMD_ELEMENT *optnode_stack[CLI_DUMMY_STACK_SIZE];       
   
   /* Set Level and current node markers to first command element */   
   while(cmd_pos & 1) {
      /* check if token matches with command element in command tree */
      ret = cli_check_token(pCli, &i_cmd_tokens->i_arg_record[i], o_cli_cmd_error);
      
      if(CLI_NO_MATCH == ret || CLI_PARTIAL_MATCH == ret || CLI_SUCCESSFULL_MATCH == ret) {
         if(CLI_HELP_NEXT_LEVEL == io_param->i_cmdtype && 
            (NCSCLI_OPTIONAL != pCli->ctree_cb.cmdElement->tokType) && 
            (NCSCLI_GROUP != pCli->ctree_cb.cmdElement->tokType) &&
            (NCSCLI_CONTINOUS_EXP != pCli->ctree_cb.cmdElement->tokType)) {         
            if(ncscli_user_access_level_authenticate(pCli)) {
            cli_help_str_fill(pCli, &io_param->o_hotkey.hlpstr.helpargs[helpcount]);            
            helpcount++;
            io_param->o_hotkey.hlpstr.count++;
          }
         }
         
         if((pCli->ctree_cb.cmdElement->tokType == NCSCLI_CONTINOUS_EXP) && 
            (pCli->ctree_cb.cmdElement->node_ptr.pChild)) {
            pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pChild;                
            continue;
         }

         /* If partial or sucessfull match  then add to help struct */
         if(CLI_PARTIAL_MATCH == ret || CLI_SUCCESSFULL_MATCH == ret) {
            if(CLI_HELP_PARTIAL_MATCH == io_param->i_cmdtype) {
            if(ncscli_user_access_level_authenticate(pCli)) {
               cli_help_str_fill(pCli, &io_param->o_hotkey.hlpstr.helpargs[helpcount]);
               helpcount++;
               io_param->o_hotkey.hlpstr.count++;
           }
          }
         }
         
         /* match not found traverse further through pDataPtr, a pDataPtr indicates that
         this is a grp/opt node */
         if(pCli->ctree_cb.cmdElement->node_ptr.pDataPtr) {
            /* Put dummy node in a stack */
            optnode_stack[optStack_ptr] = pCli->ctree_cb.cmdElement;
            optStack_ptr++;            
            pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pDataPtr;
            
            /* Opt/Grp node, set command-matched to FALSE */
            continue;
         }
         
         /* siblingptr if exists indicates a OR condition */
         if(pCli->ctree_cb.cmdElement->node_ptr.pSibling) {
            pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pSibling;
            continue;
         }
         
         /* Reached end of a node, both data and sibling ptrs are NULL */
         loopflag = FALSE;
         
         /* check if there are any elements in opt stack, */
         while(optStack_ptr) {            
            /* Go to prev opt/grp node */
            optStack_ptr--;
            pCli->ctree_cb.cmdElement = optnode_stack[optStack_ptr];
            
            /* There was no match, so traverse thru sibling ptr */
            if(pCli->ctree_cb.cmdElement->node_ptr.pSibling) {
               pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pSibling;
               loopflag = TRUE;
               break;
            }                
            
            /* If the opt/grp node is not mandatory and has child pointers */
            if(!pCli->ctree_cb.cmdElement->isMand &&
               pCli->ctree_cb.cmdElement->node_ptr.pChild) {
               pCli->ctree_cb.cmdElement = pCli->ctree_cb.cmdElement->node_ptr.pChild;
               loopflag = TRUE;
               break;
            }
         }
         
         if(loopflag) continue;
         
         /* reached end of token exit */
         break;
      }
      else if (CLI_INVALID_PASSWD == ret || CLI_PASSWORD_MATCHED == ret) break;      
   }
   return ret;
}

/*****************************************************************************

  PROCEDURE NAME:        CLI_GET_HELPSTR

  DESCRIPTION:           This function is called to get the help information 
                         associated with the command. Users will type '?' to 
                         get list of all commands in the mode or command ? 
                         to get next level help information

                        The following possible combinations of help exists
                        Command ?           : List the command 1stparameter
                        Command keyword ?   : List keyword's next level 
                                              parameter
                        ?                   : Lists all commands in the command
                                              mode
                        Command-name?       : Display all commands starting with
                                              character string

  ARGUMENTS:
   CLI_CB *              Control Block pointer 
   NCSCLI_ARG_SET *         A param structure containing command tokens
   CLI_EXECUTE_PARAM *   Param struct containing buf, status and tab string
   CLI_SESSION_INFO *    Session Info structure

  RETURNS: 
    int8 *              helpstring  
*****************************************************************************/
void 
cli_get_helpstr(CLI_CB             *pCli,
                NCSCLI_ARG_SET      *io_cmd_tokens, 
                CLI_EXECUTE_PARAM  *io_param, 
                CLI_SESSION_INFO   *io_session)
{
   uns32           count = 0,len=0, str_len = 0, bit_pos;
   CLI_CMD_ERROR   cli_cmd_error;
   static int8     tmpbuf[CLI_BUFFER_SIZE], *token = 0;
   
   /* extract the position of the help string.
      if only '?' is entered then get the list of all commands at that node
      
      if user has entered command-name?, no space between command-name and '?'
      then display all commands starting with that character
        
      if user has entered command ?, i.e. space exists between token and '?' then
      get token's next level token. Extract '?' and pass the command string and
      check syntax. If syntax is partial-match or sucessfull match then
      get pointer to 'cmd-element' struct. Display the next level token.
   */   
   bit_pos = io_cmd_tokens->i_pos_value;
   memset(&io_param->o_hotkey.hlpstr, '\0', sizeof(io_param->o_hotkey.hlpstr));
   
   /* Get index of last token */
   m_CLI_GET_LAST_TOKEN(bit_pos);
   
   
   strcpy(tmpbuf, io_cmd_tokens->i_arg_record[count].cmd.strval);
   token = tmpbuf;
   str_len = strlen(tmpbuf);
   if(CLI_CONS_HELP == io_cmd_tokens->i_arg_record[count].cmd.strval[0]) {        
      io_param->i_cmdtype = CLI_HELP_NEXT_LEVEL;
   }
   else {
      io_param->i_cmdtype = CLI_HELP_PARTIAL_MATCH;
      
      /* Indicates that cmd? has been entered */
      while(CLI_CONS_HELP != *token && str_len) {
         token++;            
         str_len--;
      }
      *token = '\0';
   }

   /* Remove the last token containing ? from the arg struct. Check the syntax of the
      remaining portion of the command 
   */
   io_cmd_tokens->i_pos_value = io_cmd_tokens->i_pos_value >> 1;   
   io_param->o_status = cli_check_syntax(pCli, io_cmd_tokens, io_param, &cli_cmd_error);
   
   if(CLI_AMBIGUOUS_MATCH == io_param->o_status) return;
   
   if(CLI_SUCCESSFULL_MATCH == io_param->o_status) {
      strcpy(io_param->o_hotkey.hlpstr.helpargs
         [io_param->o_hotkey.hlpstr.count].cmdstring, CLI_CR);
      io_param->o_hotkey.hlpstr.count++;
      return;
   }
   
   if((io_cmd_tokens->i_pos_value != bit_pos) &&
      (CLI_NO_MATCH == io_param->o_status)) {
      /* Error occured in command */
      io_param->o_errpos = cli_find_errpos(io_param->i_cmdbuf, 
         cli_cmd_error.errstring, cli_cmd_error.errorpos);            
      return;
   }
   else io_param->o_status = CLI_SUCCESSFULL_MATCH;
   
   /* Now call the help_desc function with the last keyword */           
   io_cmd_tokens->i_pos_value = 1;
   
   if(CLI_HELP_PARTIAL_MATCH == io_param->i_cmdtype) {
      if(io_cmd_tokens->i_arg_record[0].cmd.strval)
         m_MMGR_FREE_CLI_DEFAULT_VAL(io_cmd_tokens->i_arg_record[0].cmd.strval);
      
      len = strlen(tmpbuf) + 1;
      io_cmd_tokens->i_arg_record[0].cmd.strval = m_MMGR_ALLOC_CLI_DEFAULT_VAL(len);
      if(!io_cmd_tokens->i_arg_record[0].cmd.strval) return;
      memset(io_cmd_tokens->i_arg_record[0].cmd.strval, '\0', len);
      strcpy(io_cmd_tokens->i_arg_record[0].cmd.strval, tmpbuf);
   }
   
   cli_get_help_desc(pCli, io_cmd_tokens, io_param, &cli_cmd_error);
   if(CLI_HELP_PARTIAL_MATCH == io_param->i_cmdtype) {
      /* It is a partial match and no matched found */
      if (0 == io_param->o_hotkey.hlpstr.count) {
         io_param->o_status = CLI_NO_MATCH;
      }
   }    
   return;
}


/*****************************************************************************

  PROCEDURE NAME:         CLI_COMPLETE_CMD

  DESCRIPTION:            The user can enter a part of command and
                          press 'TAB'to complete the command. Returns
                          the match status as part of arg and returns 
                          remaining string on success.

                          Status can be of following values

                          -If the search results in multiple matches 
                           then returns CLI_AMBIGUOUS_MATCH
                          -If the search results in no match then returns
                           CLI_NO_MATCH
                          -If the search results in a unique match returns
                           CLI_SUCCESSFULL_MATCH or CLI_PARTIAL_MATCH and 
                           returns the matched string.

  ARGUMENTS:
   CLI_CB *               Control Block pointer 
   NCSCLI_ARG_SET *          A param structure containing command tokens
   CLI_EXECUTE_PARAM *    Param struct containing buf, status and tab string
   CLI_SESSION_INFO *     Session Info structure

  RETURNS: 
   none

*****************************************************************************/
void cli_complete_cmd(CLI_CB            *pCli, 
                      NCSCLI_ARG_SET     *io_cmd_tokens, 
                      CLI_EXECUTE_PARAM *io_param, 
                      CLI_SESSION_INFO  *io_session)
{
   int32          i = 0, index;
   CLI_CMD_ERROR  cli_cmd_error;
   int32          cmdlen = 0;
   
   /* extract the last token from the string (user has pressed 'enter' so need to complete last token)
   after extracting check command syntax for the command user typed.
   if it is a partial or successfull match then get the pointer to the last matched element.
   extract command name and send back to user 
   */    

   index = io_cmd_tokens->i_pos_value;

   /* count number of tokens present */
   while(index & 1) {
      index = index >> 1;
      i++;
   }
   
   /* check if it is a keyword, else return as only keywords
   can be completed filled. A keyword is initially stored
   as string type 
   */
   if(NCSCLI_STRING != io_cmd_tokens->i_arg_record[i-1].i_arg_type) {
      io_param->o_status = CLI_NO_MATCH;
      return;
   }
   
   /* Store length of command, as user has pressed TAB needs to
   append remaining part of command 
   */
   cmdlen = strlen(io_cmd_tokens->i_arg_record[i-1].cmd.strval);
   io_param->o_status = cli_check_syntax(pCli, io_cmd_tokens, io_param, &cli_cmd_error);
   
   i = 0;
   index = io_cmd_tokens->i_pos_value;
   /* count number of tokens present */
   while(index & 1) {
      index = index >> 1;
      i++;
   }
   
   /* Now command syntax has been matched, modify the arg structure
   to send to subsystem 
   */
   cli_modify_cmdargs(io_cmd_tokens);
   
   /* check if command matched and if token type is keyword, 
   the above function will differentiate String & Keyword
   tokens 
   */
   if((CLI_SUCCESSFULL_MATCH == io_param->o_status ||
      CLI_PARTIAL_MATCH == io_param->o_status)) {
      if(io_cmd_tokens->i_arg_record[i-1].i_arg_type == NCSCLI_KEYWORD) {
      if(ncscli_user_access_level_authenticate(pCli)) {
         index = strlen(pCli->ctree_cb.cmdElement->tokName);
         strncpy(io_param->o_hotkey.tabstring,
            &io_cmd_tokens->i_arg_record[i-1].cmd.strval[cmdlen],
            index - cmdlen +1);
      }
      }
   }
   
   /* Check if TAB string is empty if so set status to NO_MATCH 
   so that a blank is not inserted 
   */
   if(io_param->o_hotkey.tabstring[0] == '\0') io_param->o_status = CLI_NO_MATCH;
}

static NCS_BOOL is_any_optional_node(CLI_CMD_ELEMENT **ptr)
{
   CLI_CMD_ELEMENT *optionalptr = *ptr;
   while(0 != optionalptr) {
      if(NCSCLI_OPTIONAL == optionalptr->tokType) {
         *ptr = optionalptr;
         return TRUE;
      }
      else optionalptr = optionalptr->node_ptr.pSibling;
   }
   return FALSE;
}

static NCS_BOOL is_any_wildcard_node(CLI_CMD_ELEMENT **ptr)
{
   CLI_CMD_ELEMENT *wildcardptr = *ptr;
   while(0 != wildcardptr) {
      if(NCSCLI_WILDCARD == wildcardptr->tokType) {
         *ptr = wildcardptr;
         return TRUE;
      }
      else wildcardptr = wildcardptr->node_ptr.pSibling;
   }
   return FALSE;
}

static void convert_to_macaddr(NCSCLI_ARG_VAL *arg_val, int8* str)
{
   int8  *delimiter = ":";
   int8  buffer[CLI_BUFFER_SIZE];
   int8  *token = 0, idx = 0;
   uns32 val = 0;
   
   memset(buffer, 0, sizeof(buffer));
   strcpy(buffer, str);
   token = sysf_strtok(buffer, delimiter);      
   
   m_MMGR_FREE_CLI_DEFAULT_VAL(str);
   sscanf(token, "%x", &val);
   arg_val->cmd.macaddr[idx] = (uns8)val;
   while(0 != token) {
      token = sysf_strtok(0, delimiter);
      if(token) {
         idx++;
         sscanf(token, "%x", &val);
         arg_val->cmd.macaddr[idx] = (uns8)val;
      }
   }
}

static void convert_to_community(NCSCLI_ARG_VAL *arg_val, int8* str)
{
   int8  *delimiter = ":";
   int8  buffer[CLI_BUFFER_SIZE];
   int8  ipbuff[16];
   int8  *token = 0;
   uns32 ret = CLI_NO_MATCH;
   
   memset(buffer, 0, sizeof(buffer));
   memset(ipbuff, 0, sizeof(ipbuff));
   
   strcpy(buffer, str);
   token = sysf_strtok(buffer, delimiter);
   if(token) {
      /* Get firt part of community */      
      strcpy(ipbuff, token);
      if(CLI_SUCCESSFULL_MATCH == (ret = cli_check_ipv4addr(ipbuff))) {
         m_MMGR_FREE_CLI_DEFAULT_VAL(str);
         arg_val->cmd.community.type = NCSCLI_COMM_TYPE_IPADDR; 
      }
      else {
         m_MMGR_FREE_CLI_DEFAULT_VAL(str);
         arg_val->cmd.community.type = NCSCLI_COMM_TYPE_ASN;
         arg_val->cmd.community.val1.asn = (uns32)atoi(token);
      }
   }
   
   while(0 != token) {
      token = sysf_strtok(0, delimiter);
      if(token) arg_val->cmd.community.num = (uns32)atoi(token);
   }
   
   if(NCSCLI_COMM_TYPE_IPADDR == arg_val->cmd.community.type)
      arg_val->cmd.community.val1.ip_addr = cli_ipv4_to_int(ipbuff);      
}

#if(NCS_IPV6 == 1)
static void cli_str_to_ipv6addr (NCSCLI_ARG_VAL *arg_val, int8* str)
{

   m_NCS_STR_TO_IPV6_ADDR(&arg_val->cmd.ipv6_pfx.addr, str);

   /* Free the string */
   m_MMGR_FREE_CLI_DEFAULT_VAL(str);
   return;

}

static void cli_str_to_ipv6pfx(NCSCLI_ARG_VAL *arg_val, int8* str)
{
   int8    *delimiter = "/";
   int8    *token = 0;
   uns32   val = 0;
   uns8    pfx_len = 0;

   token = sysf_strtok(str, delimiter);
   token = sysf_strtok(0, delimiter);

   /* Read the pfx_len value from token */
   if(token) sscanf(token, "%d", &val);
   pfx_len = (uns8) val;

   cli_str_to_ipv6addr (arg_val, str);

   /* Fill the prefix length in arg_val */
   arg_val->cmd.ipv6_pfx.len = pfx_len;
   return;
}
#endif /* NCS_IPV6 == 1 */

void cli_update_cmd_arg(NCSCLI_ARG_SET *ctok, CLI_EXECUTE_PARAM *io_param)
{
   int8  *delimiter = " ";   
   int8  *token = 0;
   uns32 pos = 0,i = 0, cnt = io_param->o_tokprocs, 
         len = 0, cmd_pos = ctok->i_pos_value;
   char  buffer[CLI_BUFFER_SIZE];
   char  wldcbuff[CLI_BUFFER_SIZE];
   
   memset(buffer, 0, sizeof(buffer));
   memset(wldcbuff, 0, sizeof(wldcbuff));
   
   if(!io_param->o_tokprocs) return;
   
   strcpy(buffer, io_param->i_cmdbuf); 
   
   /* Tok the process part of the input */
   token = sysf_strtok(buffer, delimiter);
   cnt--;
   pos += strlen(token)+1;
   
   while((0 != token) && cnt) {
      token = sysf_strtok(0, delimiter);
      cnt--;
      pos += strlen(token)+1;
   }   
   
   strcpy(wldcbuff, io_param->i_cmdbuf+pos);   
   while(cmd_pos & 1) {
      cmd_pos = cmd_pos >> 1;           
      
      /* Fill the remaining part of string till EOL */
      if(i == io_param->o_tokprocs) {         
         /* Free the previous allocated memory for the string */
         if(NCSCLI_NUMBER != ctok->i_arg_record[i].i_arg_type &&
            ctok->i_arg_record[i].cmd.strval) {
            m_MMGR_FREE_CLI_DEFAULT_VAL(ctok->i_arg_record[i].cmd.strval);
         }
         
         memset(&ctok->i_arg_record[i], 0, sizeof(NCSCLI_ARG_VAL));
         ctok->i_arg_record[i].i_arg_type = NCSCLI_STRING;
         
         /* Alloc new chunk for wildcard characters */
         len = strlen(wldcbuff) + 1;
         ctok->i_arg_record[i].cmd.strval = m_MMGR_ALLOC_CLI_DEFAULT_VAL(len);
         if(!ctok->i_arg_record[i].cmd.strval) return;
         memset(ctok->i_arg_record[i].cmd.strval, '\0', len);   
         strcpy(ctok->i_arg_record[i].cmd.strval, wldcbuff);
      }
      
      /* Reset the BIT position */
      if(i > io_param->o_tokprocs) {
         /* Set bit to 0 */
         ctok->i_pos_value = ctok->i_pos_value ^cli_to_power(2, i);
         if(NCSCLI_NUMBER != ctok->i_arg_record[i].i_arg_type && 
            ctok->i_arg_record[i].cmd.strval) {
            m_MMGR_FREE_CLI_DEFAULT_VAL(ctok->i_arg_record[i].cmd.strval);
         }
         memset(&ctok->i_arg_record[i], 0, sizeof(NCSCLI_ARG_VAL));
      }         
      ++i;
   }      
}

void clean_arg(NCSCLI_ARG_SET *arg)
{
   uns32 i=0;
   for(i=0; i<NCSCLI_ARG_COUNT; i++) {
      switch(arg->i_arg_record[i].i_arg_type) {
      case 0:
         break;
         
      case NCSCLI_KEYWORD:
      case NCSCLI_STRING:      
         if(arg->i_arg_record[i].cmd.strval)
            m_MMGR_FREE_CLI_DEFAULT_VAL(arg->i_arg_record[i].cmd.strval);     
         break;

      case NCSCLI_IPv4:    
         break;
         
      default:         
         if((FALSE == arg->i_arg_record[i].sym_valid) && 
            (NCSCLI_NUMBER != arg->i_arg_record[i].i_arg_type) &&
            (arg->i_arg_record[i].cmd.strval)) {
            m_MMGR_FREE_CLI_DEFAULT_VAL(arg->i_arg_record[i].cmd.strval);     
         }         
         break;
      }
   }
}

static void cli_help_str_fill(CLI_CB *pCli, CLI_CMD_HELP *buf)
{
   uns32 lvalue = 0, hvalue = 0;
   char  str[24];
   
   if((NCSCLI_NUMBER == pCli->ctree_cb.cmdElement->tokType) &&
      pCli->ctree_cb.cmdElement->range) {               
      lvalue = *(uns32 *)pCli->ctree_cb.cmdElement->range->lLimit;
      hvalue = *(uns32 *)pCli->ctree_cb.cmdElement->range->uLimit;
      if(!lvalue && !hvalue)
         strcpy(buf->cmdstring, pCli->ctree_cb.cmdElement->tokName);
      else {
         sysf_sprintf(str, "<%u-%u>", lvalue, hvalue);      
         strcpy(buf->cmdstring, str);
      }
   }
   else strcpy(buf->cmdstring, pCli->ctree_cb.cmdElement->tokName);

   if(pCli->ctree_cb.cmdElement->helpStr)
      strcpy(buf->helpstr, pCli->ctree_cb.cmdElement->helpStr);
}
#else
extern int dummy;
#endif /* (if NCS_CLI == 1) */
