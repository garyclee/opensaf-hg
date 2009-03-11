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
*  MODULE NAME:  eds_debug.c                                                 *
*                                                                            *
*                                                                            *
*  DESCRIPTION:                                                              *
*  This module contains debugging routines used during the development and   *
*  support of the NCS Event Distribution Service Server (EDS).               *
*                                                                            *
*****************************************************************************/
#include "eds.h"


#define BUFF_SIZE_80    80
#define INT_WIDTH_8      8



/***************************************************************************
 *
 * eds_dump_event_patterns() - Debug routine to dump contents of
 *                             a SAF patternArray.
 *
 ***************************************************************************/
void
eds_dump_event_patterns(SaEvtEventPatternArrayT *patternArray)
{
   SaEvtEventPatternT  *pEventPattern;
   int32   x = 0;
   int8    buf[256];

   if (patternArray == NULL)
      return;
   if (patternArray->patterns == 0)
      return;

   pEventPattern = patternArray->patterns; /* Point to first pattern */
   for (x=0; x<(int32)patternArray->patternsNumber; x++) {
      memcpy(buf, pEventPattern->pattern, (uns32)pEventPattern->patternSize);
      buf[pEventPattern->patternSize] = '\0';
      m_NCS_CONS_PRINTF("     pattern[%ld] =    {%2u, \"%s\"}\n",
             x, (uns32)pEventPattern->patternSize, buf);
      pEventPattern++;
   }
}


/***************************************************************************
 *
 * eds_dump_pattern_filter() - Dumps the values of a patternArray and
 *                             filterArray and compares them for a match
 *                             based on the filterType.
 *
 * Example Output:
 *
 * pattern {     jp, 2} ,  filter {  SA_EVT_PREFIX_FILTER,       j, 1} [MATCH]
 * pattern {  abcde, 5} ,  filter {  SA_EVT_SUFFIX_FILTER,  abyyyy, 6} [no]
 * pattern { abcxyz, 6} ,  filter {  SA_EVT_SUFFIX_FILTER,     xyz, 3} [MATCH]
 * pattern {   fooy, 4} ,  filter {   SA_EVT_EXACT_FILTER,    FOOY, 4} [no]
 * pattern {  futzy, 5} ,  filter {SA_EVT_PASS_ALL_FILTER, cracker, 7} [MATCH]
 * Match? NO
 *
 ***************************************************************************/
void
eds_dump_pattern_filter(SaEvtEventPatternArrayT *patternArray,
                        SaEvtEventFilterArrayT  *filterArray)
{
   int32                x;
   int32                match = 0;
   uns8                *p = NULL;
   SaEvtEventFilterT   *filter;
   SaEvtEventPatternT  *pattern;
   SaEvtEventPatternT   emptyPattern = {0,0,NULL};
   int8    buf[256];


   if ((patternArray == NULL) || (filterArray == NULL))
      return;


   pattern = patternArray->patterns;
   filter  = filterArray->filters;


   m_NCS_CONS_PRINTF("          PATTERN            FILTER\n");


   for (x=1; x <= (int32)filterArray->filtersNumber; x++)
   {
      /* NULL terminate for printing */
      memcpy(buf, pattern->pattern,
                   (uns32)pattern->patternSize);
      buf[(uns32)pattern->patternSize] = '\0';
      m_NCS_CONS_PRINTF(" {%14s, %3u} ,  {", buf, (uns32)pattern->patternSize);

      match = 0;
      switch (filter->filterType)
      {
      case SA_EVT_PREFIX_FILTER:
         m_NCS_CONS_PRINTF("  SA_EVT_PREFIX_FILTER");
         if (m_NCS_MEMCMP(filter->filter.pattern, pattern->pattern,
                          (size_t)filter->filter.patternSize) == 0)
            match = 1;
         break;
      case SA_EVT_SUFFIX_FILTER:
         m_NCS_CONS_PRINTF("  SA_EVT_SUFFIX_FILTER");
         /* Pattern must be at least as long as filter */
         if (pattern->patternSize < filter->filter.patternSize)
            break; /* No match */

         /* Set p to offset into pattern */
         p = pattern->pattern +
            ((int)pattern->patternSize - (int)filter->filter.patternSize);
         if (m_NCS_MEMCMP(filter->filter.pattern, p,
                          (size_t)filter->filter.patternSize) == 0)
            match = 1;
         break;
      case SA_EVT_EXACT_FILTER:
         m_NCS_CONS_PRINTF("   SA_EVT_EXACT_FILTER");
         if (filter->filter.patternSize == pattern->patternSize)
            if (m_NCS_MEMCMP(filter->filter.pattern, pattern->pattern,
                             (size_t)filter->filter.patternSize) == 0)
               match = 1;
         break;
      case SA_EVT_PASS_ALL_FILTER:
         m_NCS_CONS_PRINTF("SA_EVT_PASS_ALL_FILTER");
         match = 1;
         break;
      default:
         m_NCS_CONS_PRINTF("ERROR: Unknown filterType(%d)\n", filter->filterType);
      }

      /* NULL terminate for printing */
      memcpy(buf, filter->filter.pattern,
                   (uns32)filter->filter.patternSize);
      buf[(uns32)filter->filter.patternSize] = '\0';
      m_NCS_CONS_PRINTF(", %14s, %3u}", buf, (uns32)filter->filter.patternSize);
      if (match) m_NCS_CONS_PRINTF(" [MATCH]\n");
      else m_NCS_CONS_PRINTF(" [no]\n");


      filter++;
      if (x < (int32)patternArray->patternsNumber)
         pattern++;
      else
         pattern = &emptyPattern;

   } /* End for */

}

/****************************************************************************
 *
 * eds_dump_reglist() - DEBUG routine to dump the contents of the regList.
 *
 * This routine prints out all registration IDs and their associated
 * subscription IDs contained in a EDA_REG_LIST registration list.
 *
 ***************************************************************************/
void
eds_dump_reglist()
{
   uns32            num_registrations = 0;
   uns32            num_subscriptions = 0;
   CHAN_OPEN_LIST  *co = NULL;
   SUBSC_LIST      *s = NULL;
   int8            *bp;
   EDA_REG_REC     *rl;
   int8             buff[60];
   EDS_CB          *cb = NULL;
   uns32 temp;
   EDA_DOWN_LIST *down_rec = NULL;

   m_NCS_CONS_PRINTF("\n-=-=-=-=- REGLIST SUBSCRIPTION IDs -=-=-=-=-\n");
   m_NCS_CONS_PRINTF("     regIDs         Subscription IDs\n");
   
   /* Get the cb from the global handle */
   if (NULL == (cb = (EDS_CB*) ncshm_take_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl)))
   {
      return;
   }
 
   rl = (EDA_REG_REC *)ncs_patricia_tree_getnext(&cb->eda_reg_list,(uns8 *)0);

   while(rl)  /* While there are registration entries... */
   {
      num_registrations++;
      m_NCS_CONS_PRINTF("%6u %-8s->", rl->reg_id, " ");
      buff[0] = '\0';
      bp = buff;

      co = rl->chan_open_list;
      while(co)  /* While there are channelOpen entries... */
      {
         s = co->subsc_list_head;
         while(s)   /* While there are subscription entries... */
         {
            num_subscriptions++;
            /* Loop, appending subscription IDs to string */
            sprintf(bp," %9u ", s->subsc_rec->subscript_id);
            bp = buff + (m_NCS_STRLEN(buff));

            /* Close to end of buffer? */
            if ((bp-buff >= sizeof(buff)-12) &&
                ((s->next) || (co->next)))     /* and something follows? */
            {
               /* Output this chunk and start a new one */
               m_NCS_CONS_PRINTF("%s\n", buff);
               bp = buff;

               /* Space over the continuation line */
               sprintf(bp, "%24s", " ");
               bp = buff + (m_NCS_STRLEN(buff));
            }                    
            s = s->next;
         }
         co = co->next;
      }
      m_NCS_CONS_PRINTF("%s\n", buff);

      rl = (EDA_REG_REC *)ncs_patricia_tree_getnext
               (&cb->eda_reg_list,(uns8 *)&rl->reg_id_Net);
   }
   temp = 0;
   down_rec = cb->eda_down_list_head;
   while(down_rec)
   {
       temp++;
       down_rec = down_rec->next;
    }

   m_NCS_CONS_PRINTF("\nTotal Registrations:%d   Total Subscriptions:%d\n",
          num_registrations, num_subscriptions);
   m_NCS_CONS_PRINTF("\nEDA DOWN recs : %d Async Update Count :%d \n",temp,cb->async_upd_cnt);
   
   /* Give back the handle */
   ncshm_give_hdl(gl_eds_hdl);
}

/****************************************************************************
 *
 * eds_dump_worklist() - DEBUG routine to dump the contents of the workList.
 *
 * This routine prints out all channel IDs and their associated
 * subscription IDs contained in a EDS_WORKLIST work list.
 *
 ***************************************************************************/
void
eds_dump_worklist()
{
   uns32           num_events = 0;
   uns32           num_subscriptions = 0;
   CHAN_OPEN_REC   *co = NULL;
   SUBSC_REC       *s = NULL;
   EDS_RETAINED_EVT_REC *retd_evt_rec=NULL;
   int8            *bp;
   int8            buff[BUFF_SIZE_80];
   SaUint8T        list_iter;
   EDS_CB          *eds_cb = NULL;
   EDS_WORKLIST    *wp = NULL;
   EDS_CNAME_REC *cn= NULL;  
   
   m_NCS_CONS_PRINTF("\n-=-=-=-=- WORKLIST SUBSCRIPTION IDs -=-=-=-=-\n");
   m_NCS_CONS_PRINTF("     Channel               Copenid:retained events\n");
  

   m_NCS_CONS_PRINTF("   OpenID:regID            Subscription IDs\n");
   
   /* Get the cb from the global handle. */
    if (NULL == (eds_cb = (EDS_CB*) ncshm_take_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl)))
   {
      return;
   } 
   
   /* Get the worklist */
   wp = eds_cb->eds_work_list ; 

   while(wp)  /* While there are channel entries... */
   {
      num_events++;
      if (wp->chan_attrib & CHANNEL_UNLINKED) /* Flag if channel is UnLinked */
         sprintf(buff, "u");
      else sprintf(buff, " ");
      m_NCS_CONS_PRINTF("\n%s%6u %s", buff, wp->chan_id, wp->cname);
      
      m_NCS_CONS_PRINTF("         ");
      for(list_iter = SA_EVT_HIGHEST_PRIORITY; list_iter <= SA_EVT_LOWEST_PRIORITY ; list_iter++)
      {
          retd_evt_rec = wp->ret_evt_list_head[list_iter];
   
          m_NCS_CONS_PRINTF("    P:%u", list_iter );
          while (retd_evt_rec)
          {
               m_NCS_CONS_PRINTF("  %d:%u",retd_evt_rec->retd_evt_chan_open_id,retd_evt_rec->event_id );
               retd_evt_rec = retd_evt_rec->next;
          }
       }
       
       m_NCS_CONS_PRINTF("\n");
       bp = buff;


      co = (CHAN_OPEN_REC *)ncs_patricia_tree_getnext(&wp->chan_open_rec,
                                                      (uns8*)0);
      while(co)  /* While there are channelOpen entries... */
      {
         sprintf(bp, "%10d:%-6d", co->chan_open_id, co->reg_id);
         bp = buff + (m_NCS_STRLEN(buff));
         s = co->subsc_rec_head;
         while(s)   /* While there are subscription entries... */
         {
            num_subscriptions++;
            /* Loop, appending subscription IDs to string */
            sprintf(bp," %12u", s->subscript_id);
            bp = buff + (m_NCS_STRLEN(buff));

            /* Close to end of buffer? */
            if ((bp-buff >= sizeof(buff)-(INT_WIDTH_8*2)) &&
                (s->next))     /* and something follows? */
            {
               /* Output this chunk and start a new one */
               m_NCS_CONS_PRINTF("%s\n", buff);
               bp = buff;

               /* Space over the continuation line */
               sprintf(bp, "%25s", " ");
               bp = buff + (m_NCS_STRLEN(buff));
            }                    
            s = s->next;
         }
         m_NCS_CONS_PRINTF("%s\n", buff);
         bp = buff;

         co = (CHAN_OPEN_REC *)ncs_patricia_tree_getnext(&wp->chan_open_rec,
                                                    (uns8*)&co->copen_id_Net);
      }
      wp = wp->next;
   }
   
   m_NCS_CONS_PRINTF("\nTotal Channel IDs:%d   Total Subscriptions:%d\n",
          num_events, num_subscriptions);
   
/* Code for MIB DUMP */
   m_NCS_CONS_PRINTF("\nMIB DATA BASE \n");
   cn =  (EDS_CNAME_REC *)ncs_patricia_tree_getnext(&eds_cb->eds_cname_list,
                                                        (uns8*)NULL);

   while (cn)
   {
      if(cn->wp_rec)
         m_NCS_CONS_PRINTF("\nMib : %s                 Actual: %s" , cn->chan_name.value,(cn->wp_rec)->cname);
      cn = (EDS_CNAME_REC *)ncs_patricia_tree_getnext
               (&eds_cb->eds_cname_list,(uns8 *)&cn->chan_name);
   }


  

      
   /* Give back the handle */
   ncshm_give_hdl(gl_eds_hdl);

}

/* End eds_debug.c */
