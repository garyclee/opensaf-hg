#if ( (NCS_SRMA == 1)&& (TET_A==1) )

#include "tet_startup.h"
#include "tet_srmsv.h"
#include "tet_srmsv_conf.h"

#define m_TET_SRMSV_PRINTF printf

const char *srmsv_notif[] = {
    "SRMSV_CBK_NOTIF_RSRC_THRESHOLD",
    "SRMSV_CBK_NOTIF_WATERMARK_VAL",
    "SRMSV_CBK_NOTIF_WM_CFG_ALREADY_EXIST",
    "SRMSV_CBK_NOTIF_RSRC_MON_EXPIRED",
    "SRMSV_CBK_NOTIF_PROCESS_EXPIRED",
};

const char *srmsv_scale_type[] = {
    "NCS UNKNOWN VALUE",
    "NCS_SRMSV_STAT_SCALE_BYTE",
    "NCS_SRMSV_STAT_SCALE_KBYTE",
    "NCS_SRMSV_STAT_SCALE_MBYTE",
    "NCS_SRMSV_STAT_SCALE_GBYTE",
    "NCS_SRMSV_STAT_SCALE_TBYTE",
    "NCS_SRMSV_STAT_SCALE_PERCENT",
};
                                                                                                                             
const char *srmsv_val_type[] =
{
   "NCS UNKNOWN VALUE",
   "NCS_SRMSV_VAL_TYPE_FLOAT",
   "NCS_SRMSV_VAL_TYPE_INT32",
   "NCS_SRMSV_VAL_TYPE_UNS32",
   "NCS_SRMSV_VAL_TYPE_INT64",
   "NCS_SRMSV_VAL_TYPE_UNS64",
};
                                                                                                                             
const char *srmsv_rsrc_type[] =
{
   "NCS UNKNOWN VALUE",
   "NCS_SRMSV_RSRC_CPU_UTIL",
   "NCS_SRMSV_RSRC_CPU_KERNEL",
   "NCS_SRMSV_RSRC_CPU_USER",
   "NCS_SRMSV_RSRC_CPU_UTIL_ONE",
   "NCS_SRMSV_RSRC_CPU_UTIL_FIVE",
   "NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN",
   "NCS_SRMSV_RSRC_MEM_PHYSICAL_USED",
   "NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE",
   "NCS_SRMSV_RSRC_SWAP_SPACE_USED",
   "NCS_SRMSV_RSRC_SWAP_SPACE_FREE",
   "NCS_SRMSV_RSRC_USED_BUFFER_MEM",
   "NCS_SRMSV_RSRC_USED_CACHED_MEM",
   "NCS_SRMSV_RSRC_PROC_EXIT",
   "NCS_SRMSV_RSRC_PROC_MEM",
   "NCS_SRMSV_RSRC_PROC_CPU",
};

const char *srmsv_wmk_type[] =
{
   "NCS UNKNOWN VALUE",
   "NCS_SRMSV_WATERMARK_HIGH",
   "NCS_SRMSV_WATERMARK_LOW",
   "NCS_SRMSV_WATERMARK_DUAL",
};

const char *srmsv_condition[] =
{
   "NCS UNKNOWN VALUE",
   "NCS_SRMSV_THRESHOLD_VAL_IS_AT",
   "NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT",
   "NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE",
   "NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE",
   "NCS_SRMSV_THRESHOLD_VAL_IS_BELOW",
   "NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW",
};


int convert_scaletype_to_scaletype(NCS_SRMSV_MON_INFO mon_info)
{
  FILE *fp;
  char buf[100];
  int total_mem=-1;
  if(mon_info.monitor_data.mon_cfg.threshold.threshold_val.scale_type ==   \
                      mon_info.monitor_data.mon_cfg.threshold.threshold_val.scale_type)
     return total_mem;

  sprintf(buf, "cat /proc/meminfo | grep MemTotal | cut -d: -f2|tr -d [A-z]|tr -d [' ']");
  fp = popen(buf, "r");
  if (fp == NULL)
  {
     fprintf(stderr, "Dieing");
     exit(-1);
  }
  memset(buf,'\0',100);
  fread(buf,1,10,fp);
                                                                                                                             
  total_mem = atoi(buf);
  pclose(fp);
  m_TET_SRMSV_PRINTF(" Total Physical Memory %d \n",total_mem);

  return total_mem;
}

int check_f_val(double clbk_val,double exp_val,double tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition)
{
  int result = TET_FAIL;
  double min_val,max_val;

  min_val = exp_val - tolerable_val;
  max_val = exp_val + tolerable_val;

  switch(condition)
  {
     case NCS_SRMSV_THRESHOLD_VAL_IS_AT:  
                                            if((clbk_val >= min_val) && (clbk_val <= max_val))
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT:  
                                            if((clbk_val < min_val) && (clbk_val > max_val))
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE:  
                                            if(clbk_val > exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE:  
                                            if(clbk_val >= exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_BELOW:  
                                            if(clbk_val < exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW:  
                                            if(clbk_val <= exp_val)
                                                result = TET_PASS;
                                            break;

     default:
              m_TET_SRMSV_PRINTF("\n UNKNOWN CONDITION \n");
  }

  if(result == TET_FAIL)
  {
    m_TET_SRMSV_PRINTF("\n Condition check failed \n");
    tet_printf("\n Condition check failed \n");
  }

  return result;
}

int check_i_val32(int32 clbk_val,int32 exp_val,int32 tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition)
{
  int result = TET_FAIL;
  int32 min_val,max_val;

  min_val = exp_val - tolerable_val;
  max_val = exp_val + tolerable_val;

  switch(condition)
  {
     case NCS_SRMSV_THRESHOLD_VAL_IS_AT:  
                                            if((clbk_val >= min_val) && (clbk_val <= max_val))
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT:  
                                            if((clbk_val < min_val) && (clbk_val > max_val))
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE:  
                                            if(clbk_val > exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE:  
                                            if(clbk_val >= exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_BELOW:  
                                            if(clbk_val < exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW:  
                                            if(clbk_val <= exp_val)
                                                result = TET_PASS;
                                            break;

     default:
              m_TET_SRMSV_PRINTF("\n UNKNOWN CONDITION \n");
  }

  if(result == TET_FAIL)
  {
    m_TET_SRMSV_PRINTF("\n Condition check failed \n");
    tet_printf("\n Condition check failed \n");
  }

  return result;
}

int check_u_val32(uns32 clbk_val,uns32 exp_val,uns32 tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition)
{
  int result = TET_FAIL;
  uns32 min_val,max_val;

  min_val = exp_val - tolerable_val;
  max_val = exp_val + tolerable_val;

  switch(condition)
  {
     case NCS_SRMSV_THRESHOLD_VAL_IS_AT:  
                                            if((clbk_val >= min_val) && (clbk_val <= max_val))
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT:  
                                            if((clbk_val < min_val) && (clbk_val > max_val))
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE:  
                                            if(clbk_val > exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE:  
                                            if(clbk_val >= exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_BELOW:  
                                            if(clbk_val < exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW:  
                                            if(clbk_val <= exp_val)
                                                result = TET_PASS;
                                            break;

     default:
              m_TET_SRMSV_PRINTF("\n UNKNOWN CONDITION \n");
  }

  if(result == TET_FAIL)
  {
    m_TET_SRMSV_PRINTF("\n Condition check failed \n");
    tet_printf("\n Condition check failed \n");
  }

  return result;
}

int check_i_val64(int64 clbk_val,int64 exp_val,int64 tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition)
{
  int result = TET_FAIL;
  int64 min_val,max_val;

  min_val = exp_val - tolerable_val;
  max_val = exp_val + tolerable_val;

  switch(condition)
  {
     case NCS_SRMSV_THRESHOLD_VAL_IS_AT:  
                                            if((clbk_val >= min_val) && (clbk_val <= max_val))
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT:  
                                            if((clbk_val < min_val) && (clbk_val > max_val))
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE:  
                                            if(clbk_val > exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE:  
                                            if(clbk_val >= exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_BELOW:  
                                            if(clbk_val < exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW:  
                                            if(clbk_val <= exp_val)
                                                result = TET_PASS;
                                            break;

     default:
              m_TET_SRMSV_PRINTF("\n UNKNOWN CONDITION \n");
  }

  if(result == TET_FAIL)
  {
    m_TET_SRMSV_PRINTF("\n Condition check failed \n");
    tet_printf("\n Condition check failed \n");
  }

  return result;
}

int check_u_val64(uns64 clbk_val,uns64 exp_val,uns64 tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition)
{
  int result = TET_FAIL;
  uns64 min_val,max_val;

  min_val = exp_val - tolerable_val;
  max_val = exp_val + tolerable_val;

  switch(condition)
  {
     case NCS_SRMSV_THRESHOLD_VAL_IS_AT:  
                                            if((clbk_val >= min_val) && (clbk_val <= max_val))
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT:  
                                            if((clbk_val < min_val) && (clbk_val > max_val))
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE:  
                                            if(clbk_val > exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE:  
                                            if(clbk_val >= exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_BELOW:  
                                            if(clbk_val < exp_val)
                                                result = TET_PASS;
                                            break;

     case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW:  
                                            if(clbk_val <= exp_val)
                                                result = TET_PASS;
                                            break;

     default:
              m_TET_SRMSV_PRINTF("\n UNKNOWN CONDITION \n");
  }

  if(result == TET_FAIL)
  {
    m_TET_SRMSV_PRINTF("\n Condition check failed \n");
    tet_printf("\n Condition check failed \n");
  }

  return result;
}


int check_condition(NCS_SRMSV_VALUE clbk_val,NCS_SRMSV_VALUE exp_val,
                    NCS_SRMSV_VALUE tolerable_val,NCS_SRMSV_THRESHOLD_TEST_CONDITION condition)
{
  m_TET_SRMSV_PRINTF(" Checking with the condition  : %s\n",srmsv_condition[condition]); 
  tet_printf(" Checking with the condition  : %s\n",srmsv_condition[condition]); 

  int result = TET_FAIL;
  switch(clbk_val.val_type)
  {
    case NCS_SRMSV_VAL_TYPE_FLOAT :
             result = check_f_val(clbk_val.val.f_val,exp_val.val.f_val,tolerable_val.val.f_val,condition);
             break;

    case NCS_SRMSV_VAL_TYPE_INT32 :
             result = check_i_val32(clbk_val.val.i_val32,exp_val.val.i_val32,tolerable_val.val.i_val32,condition);
             break;

    case NCS_SRMSV_VAL_TYPE_UNS32 :
             result = check_u_val32(clbk_val.val.u_val32,exp_val.val.u_val32,tolerable_val.val.u_val32,condition);
             break;

    case NCS_SRMSV_VAL_TYPE_INT64 :
             result = check_i_val64(clbk_val.val.i_val64,exp_val.val.i_val64,tolerable_val.val.i_val64,condition);
             break;

    case NCS_SRMSV_VAL_TYPE_UNS64 :
             result = check_u_val64(clbk_val.val.u_val64,exp_val.val.u_val64,tolerable_val.val.u_val64,condition);
             break;

    default:
             m_TET_SRMSV_PRINTF(" Unknown val type in check condition\n");
             tet_printf(" Unknown val type in check condition\n");
             break;
  }
  return result;
}

void fill_exp_value(NCS_SRMSV_VALUE threshold_value,NCS_SRMSV_VALUE tolerable_value)
{
  switch(threshold_value.val_type)
  {
    case NCS_SRMSV_VAL_TYPE_FLOAT :
             tcd.exp_val.val.f_val = threshold_value.val.f_val;
             break;

    case NCS_SRMSV_VAL_TYPE_INT32 :
             tcd.exp_val.val.i_val32 = threshold_value.val.i_val32;
             break;

    case NCS_SRMSV_VAL_TYPE_UNS32 :
             tcd.exp_val.val.u_val32 = threshold_value.val.u_val32;
             break;

    case NCS_SRMSV_VAL_TYPE_INT64 :
             tcd.exp_val.val.i_val64 = threshold_value.val.i_val64;
             break;

    case NCS_SRMSV_VAL_TYPE_UNS64 :
             tcd.exp_val.val.u_val64 = threshold_value.val.u_val64;
             break;

    default:
             m_TET_SRMSV_PRINTF(" Unknown threshold val type in fill_exp_value condition\n");
             tet_printf(" Unknown threshold val type in fill_exp_value condition\n");
             break;
  }

  switch(tolerable_value.val_type)
  {
    case NCS_SRMSV_VAL_TYPE_FLOAT :
             tcd.tolerable_val.val.f_val = tolerable_value.val.f_val;
             break;

    case NCS_SRMSV_VAL_TYPE_INT32 :
             tcd.tolerable_val.val.i_val32 = tolerable_value.val.i_val32;
             break;

    case NCS_SRMSV_VAL_TYPE_UNS32 :
             tcd.tolerable_val.val.u_val32 = tolerable_value.val.u_val32;
             break;

    case NCS_SRMSV_VAL_TYPE_INT64 :
             tcd.tolerable_val.val.i_val64 = tolerable_value.val.i_val64;
             break;

    case NCS_SRMSV_VAL_TYPE_UNS64 :
             tcd.tolerable_val.val.u_val64 = tolerable_value.val.u_val64;
             break;

    default:
             m_TET_SRMSV_PRINTF(" Unknown tolerable val type in fill_exp_value condition\n");
             tet_printf(" Unknown tolerable val type in fill_exp_value condition\n");
             break;
  }
}

void print_threshold_clbk_values(NCS_SRMSV_VALUE *rsrc_value)
{
  m_TET_SRMSV_PRINTF(" Notify rsrc_value value type :%s \n",srmsv_val_type[rsrc_value->val_type]) ;    
  tet_printf(" Notify rsrc_value value type :%s \n",srmsv_val_type[rsrc_value->val_type]) ;    
  tcd.clbk_data.threshold_val.val_type = rsrc_value->val_type;
  
  switch(rsrc_value->val_type)
  {
    case NCS_SRMSV_VAL_TYPE_FLOAT :
             tcd.clbk_data.threshold_val.val.f_val = rsrc_value->val.f_val;
             m_TET_SRMSV_PRINTF(" Notify rsrc_value f_val      :%lf \n",rsrc_value->val.f_val);
             tet_printf(" Notify rsrc_value f_val      :%lf \n",rsrc_value->val.f_val);
             break;

    case NCS_SRMSV_VAL_TYPE_INT32 :
             tcd.clbk_data.threshold_val.val.i_val32 = rsrc_value->val.i_val32;
             m_TET_SRMSV_PRINTF(" Notify rsrc_value ival32     :%d \n",rsrc_value->val.i_val32);
             tet_printf(" Notify rsrc_value ival32     :%ld \n",rsrc_value->val.i_val32);
             break;

    case NCS_SRMSV_VAL_TYPE_UNS32 :
             tcd.clbk_data.threshold_val.val.u_val32 = rsrc_value->val.u_val32;
             m_TET_SRMSV_PRINTF(" Notify rsrc_value uval32     :%u \n",rsrc_value->val.u_val32);
             tet_printf(" Notify rsrc_value uval32     :%u \n",rsrc_value->val.u_val32);
             break;

    case NCS_SRMSV_VAL_TYPE_INT64 :
             tcd.clbk_data.threshold_val.val.i_val64 = rsrc_value->val.i_val64;
             m_TET_SRMSV_PRINTF(" Notify rsrc_value ival64     :%llu \n",rsrc_value->val.i_val64);
             tet_printf(" Notify rsrc_value ival64     :%llu \n",rsrc_value->val.i_val64);
             break;

    case NCS_SRMSV_VAL_TYPE_UNS64 :
             tcd.clbk_data.threshold_val.val.u_val64 = rsrc_value->val.u_val64;
             m_TET_SRMSV_PRINTF(" Notify rsrc_value uval64     :%lld \n",rsrc_value->val.u_val64);
             tet_printf(" Notify rsrc_value uval64     :%lld \n",rsrc_value->val.u_val64);
             break;

    default:
             m_TET_SRMSV_PRINTF(" Unknown val type \n");
             tet_printf(" Unknown val type \n");
             break;
  }
}

void print_wmk_low_val_values(NCS_SRMSV_VALUE *low_val)
{
  m_TET_SRMSV_PRINTF(" Watermark Low Value type     :%s \n",srmsv_val_type[low_val->val_type]) ;    
  m_TET_SRMSV_PRINTF(" Watermark Low scale type     :%s \n",srmsv_scale_type[low_val->scale_type]) ;    
  tet_printf(" Watermark Low Value type     :%s \n",srmsv_val_type[low_val->val_type]) ;    
  tet_printf(" Watermark Low scale type     :%s \n",srmsv_scale_type[low_val->scale_type]) ;    
  tcd.clbk_data.low_val.val_type = low_val->val_type;
  tcd.clbk_data.low_val.scale_type = low_val->scale_type;

  switch(low_val->val_type)
  {
     case NCS_SRMSV_VAL_TYPE_FLOAT :
              tcd.clbk_data.low_val.val.f_val = low_val->val.f_val;
              m_TET_SRMSV_PRINTF(" Notify wmk low value fval32  :%lf \n",low_val->val.f_val);
              tet_printf(" Notify wmk low value fval32  :%lf \n",low_val->val.f_val);
              break;

     case NCS_SRMSV_VAL_TYPE_INT32 :
              tcd.clbk_data.low_val.val.i_val32 = low_val->val.i_val32;
              m_TET_SRMSV_PRINTF(" Notify wmk low value ival32  :%d \n",low_val->val.i_val32);
              tet_printf(" Notify wmk low value ival32  :%ld \n",low_val->val.i_val32);
              break;

     case NCS_SRMSV_VAL_TYPE_UNS32 :
              tcd.clbk_data.low_val.val.u_val32 = low_val->val.u_val32;
              m_TET_SRMSV_PRINTF(" Notify wmk low value uval32  :%u \n",low_val->val.u_val32);
              tet_printf(" Notify wmk low value uval32  :%u \n",low_val->val.u_val32);
              break;

     case NCS_SRMSV_VAL_TYPE_INT64 :
              tcd.clbk_data.low_val.val.i_val64 = low_val->val.i_val64;
              m_TET_SRMSV_PRINTF(" Notify wmk low value ival64  :%llu \n",low_val->val.i_val64);
              tet_printf(" Notify wmk low value ival64  :%llu \n",low_val->val.i_val64);
              break;

     case NCS_SRMSV_VAL_TYPE_UNS64 :
              tcd.clbk_data.low_val.val.u_val64 = low_val->val.u_val64;
              m_TET_SRMSV_PRINTF(" Notify wmk low value uval64  :%lld \n",low_val->val.u_val64);
              tet_printf(" Notify wmk low value uval64  :%lld \n",low_val->val.u_val64);
              break;

     default:
              m_TET_SRMSV_PRINTF(" Unknown low val type \n");
              tet_printf(" Unknown low val type \n");
              break;
  }
}

void print_wmk_high_val_values(NCS_SRMSV_VALUE *high_val)
{
  m_TET_SRMSV_PRINTF(" Watermark High value type    :%s \n",srmsv_val_type[high_val->val_type]) ;    
  m_TET_SRMSV_PRINTF(" Watermark High scale type    :%s \n",srmsv_scale_type[high_val->scale_type]) ;    
  tet_printf(" Watermark High value type    :%s \n",srmsv_val_type[high_val->val_type]) ;    
  tet_printf(" Watermark High scale type    :%s \n",srmsv_scale_type[high_val->scale_type]) ;    
  tcd.clbk_data.high_val.val_type = high_val->val_type;
  tcd.clbk_data.high_val.scale_type = high_val->scale_type;

  switch(high_val->val_type)
  {
     case NCS_SRMSV_VAL_TYPE_FLOAT :
              tcd.clbk_data.high_val.val.f_val = high_val->val.f_val;
              m_TET_SRMSV_PRINTF(" Notify wmk high value fval32 :%lf \n",high_val->val.f_val);
              tet_printf(" Notify wmk high value fval32 :%lf \n",high_val->val.f_val);
              break;

     case NCS_SRMSV_VAL_TYPE_INT32 :
              tcd.clbk_data.high_val.val.i_val32 = high_val->val.i_val32;
              m_TET_SRMSV_PRINTF(" Notify wmk high value ival32 :%d \n",high_val->val.i_val32);
              tet_printf(" Notify wmk high value ival32 :%ld \n",high_val->val.i_val32);
              break;

     case NCS_SRMSV_VAL_TYPE_UNS32 :
              tcd.clbk_data.high_val.val.u_val32 = high_val->val.u_val32;
              m_TET_SRMSV_PRINTF(" Notify wmk high value uval32 :%u \n",high_val->val.u_val32);
              tet_printf(" Notify wmk high value uval32 :%u \n",high_val->val.u_val32);
              break;

     case NCS_SRMSV_VAL_TYPE_INT64 :
              tcd.clbk_data.high_val.val.i_val64 = high_val->val.i_val64;
              m_TET_SRMSV_PRINTF(" Notify wmk high value ival64 :%llu \n",high_val->val.i_val64);
              tet_printf(" Notify wmk high value ival64 :%llu \n",high_val->val.i_val64);
              break;

     case NCS_SRMSV_VAL_TYPE_UNS64 :
              tcd.clbk_data.high_val.val.u_val64 = high_val->val.u_val64;
              m_TET_SRMSV_PRINTF(" Notify wmk high value uval64 :%lld \n",high_val->val.u_val64);
              tet_printf(" Notify wmk high value uval64 :%lld \n",high_val->val.u_val64);
              break;

     default:
              m_TET_SRMSV_PRINTF(" Unknown High val type \n");
              tet_printf(" Unknown High val type \n");
              break;
  }
}

void print_wmk_clbk_values(SRMSV_WATERMARK_VAL *watermarks)
{
  m_TET_SRMSV_PRINTF(" Water mark type              :%s \n",srmsv_wmk_type[watermarks->wm_type]) ;    
  tet_printf(" Water mark type              :%s \n",srmsv_wmk_type[watermarks->wm_type]) ;    
  tcd.clbk_data.wm_type = watermarks->wm_type;

  switch(watermarks->wm_type)
  {
     case NCS_SRMSV_WATERMARK_HIGH:
              if(watermarks->wm_exist)
                 print_wmk_high_val_values(&(watermarks->high_val));
              break;

     case NCS_SRMSV_WATERMARK_LOW:
              if(watermarks->wm_exist)
                 print_wmk_low_val_values(&(watermarks->low_val));
              break;

     case NCS_SRMSV_WATERMARK_DUAL:
              if(watermarks->wm_exist)
              {
                 print_wmk_low_val_values(&(watermarks->low_val));
                 print_wmk_high_val_values(&(watermarks->high_val));
              }
              break;

     default:
              m_TET_SRMSV_PRINTF(" Unknown watermark type \n");
              tet_printf(" Unknown watermark type \n");
              break;
  }
}

void app_srmsv_callback(NCS_SRMSV_RSRC_CBK_INFO *rsrc_info)
{
    tcd.clbk_count++;

    m_TET_SRMSV_PRINTF("\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ");
    m_TET_SRMSV_PRINTF("\n                CALLBACK EVENT ARRIVED                 ");
    m_TET_SRMSV_PRINTF("\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ \n");
    tet_printf("\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ");
    tet_printf("\n                 CALLBACK EVENT ARRIVED                ");
    tet_printf("\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ \n");

    tcd.clbk_data.not_type = rsrc_info->notif_type;
    m_TET_SRMSV_PRINTF(" Notification type            :%s \n",srmsv_notif[rsrc_info->notif_type]);
    tet_printf(" Notification type            :%s \n",srmsv_notif[rsrc_info->notif_type]);

    switch(rsrc_info->notif_type)
    {
        case SRMSV_CBK_NOTIF_RSRC_THRESHOLD :
             m_TET_SRMSV_PRINTF(" Monitor Handle               :%u \n",rsrc_info->rsrc_mon_hdl);
             m_TET_SRMSV_PRINTF(" Node Id                      :%d \n",rsrc_info->node_num);
             m_TET_SRMSV_PRINTF(" Notify rsrc_value scale type :%s \n",
                                  srmsv_scale_type[rsrc_info->notify.rsrc_value.scale_type]);
             tet_printf(" Monitor Handle               :%u \n",rsrc_info->rsrc_mon_hdl);
             tet_printf(" Node Id                      :%d \n",rsrc_info->node_num);
             tet_printf(" Notify rsrc_value scale type :%s \n",srmsv_scale_type[rsrc_info->notify.rsrc_value.scale_type]);
             tcd.clbk_data.rsrc_mon_hdl = rsrc_info->rsrc_mon_hdl;
             tcd.clbk_data.nodeId = rsrc_info->node_num;
             tcd.clbk_data.threshold_val.scale_type = rsrc_info->notify.rsrc_value.scale_type;
             print_threshold_clbk_values(&(rsrc_info->notify.rsrc_value));
             break;

        case SRMSV_CBK_NOTIF_RSRC_MON_EXPIRED :
             m_TET_SRMSV_PRINTF(" Monitor Handle               :%u \n",rsrc_info->rsrc_mon_hdl);
             m_TET_SRMSV_PRINTF(" Node Id                      :%d \n",rsrc_info->node_num);
             tet_printf(" Monitor Handle               :%u \n",rsrc_info->rsrc_mon_hdl);
             tet_printf(" Node Id                      :%d \n",rsrc_info->node_num);
             tcd.clbk_data.rsrc_mon_hdl = rsrc_info->rsrc_mon_hdl;
             break;

        case SRMSV_CBK_NOTIF_PROCESS_EXPIRED :
             m_TET_SRMSV_PRINTF(" Monitor Handle               :%u \n",rsrc_info->rsrc_mon_hdl);
             m_TET_SRMSV_PRINTF(" Node Id                      :%d \n",rsrc_info->node_num);
             m_TET_SRMSV_PRINTF(" Notify pid                   :%d \n",rsrc_info->notify.pid);    
             tet_printf(" Monitor Handle               :%u \n",rsrc_info->rsrc_mon_hdl);
             tet_printf(" Node Id                      :%d \n",rsrc_info->node_num);
             tet_printf(" Notify pid                   :%d \n",rsrc_info->notify.pid);    
             tcd.clbk_data.rsrc_mon_hdl = rsrc_info->rsrc_mon_hdl;
             tcd.clbk_data.nodeId = rsrc_info->node_num;
             tcd.clbk_data.pid = rsrc_info->notify.pid;
             break;

        case SRMSV_CBK_NOTIF_WATERMARK_VAL :
             m_TET_SRMSV_PRINTF(" Node Id                      :%d \n",rsrc_info->node_num);
             m_TET_SRMSV_PRINTF(" Water mark rsrc type         :%s \n",
                                                       srmsv_rsrc_type[rsrc_info->notify.watermarks.rsrc_type]);    
             m_TET_SRMSV_PRINTF(" Water mark value exist       :%d \n",rsrc_info->notify.watermarks.wm_exist);    
             tet_printf(" Node Id                      :%d \n",rsrc_info->node_num);
             tet_printf(" Water mark rsrc type         :%s \n",srmsv_rsrc_type[rsrc_info->notify.watermarks.rsrc_type]);    
             tet_printf(" Water mark value exist       :%d \n",rsrc_info->notify.watermarks.wm_exist);    
             tcd.clbk_data.rsrc_mon_hdl = rsrc_info->rsrc_mon_hdl;
             tcd.clbk_data.nodeId = rsrc_info->node_num;
             tcd.clbk_data.rsrc_type = rsrc_info->notify.watermarks.rsrc_type;
             tcd.clbk_data.wm_exist = rsrc_info->notify.watermarks.wm_exist;
             tcd.clbk_data.pid = rsrc_info->notify.watermarks.pid;
             if((rsrc_info->notify.watermarks.rsrc_type == NCS_SRMSV_RSRC_PROC_MEM) || 
                (rsrc_info->notify.watermarks.rsrc_type == NCS_SRMSV_RSRC_PROC_CPU))
             {
                m_TET_SRMSV_PRINTF(" Notify watermark pid  :%d \n",rsrc_info->notify.watermarks.pid);    
                tet_printf(" Notify watermark pid         :%d \n",rsrc_info->notify.watermarks.pid);    
             }
             print_wmk_clbk_values(&(rsrc_info->notify.watermarks));
             break;

        case SRMSV_CBK_NOTIF_WM_CFG_ALREADY_EXIST :
             m_TET_SRMSV_PRINTF(" Monitor Handle               :%u \n",rsrc_info->rsrc_mon_hdl);
             m_TET_SRMSV_PRINTF(" Node Id                 :%d \n",rsrc_info->node_num);
             m_TET_SRMSV_PRINTF(" Water mark rsrc type    :%s \n",srmsv_rsrc_type[rsrc_info->notify.rsrc_type]);    
             m_TET_SRMSV_PRINTF(" Water mark type         :%s \n",srmsv_wmk_type[rsrc_info->notify.watermarks.wm_type]);    
             m_TET_SRMSV_PRINTF(" Water mark values exist :%d \n",rsrc_info->notify.watermarks.wm_exist);    
             tet_printf(" Monitor Handle               :%u \n",rsrc_info->rsrc_mon_hdl);
             tet_printf(" Node Id                      :%d \n",rsrc_info->node_num);
             tet_printf(" Water mark rsrc type         :%s \n",srmsv_rsrc_type[rsrc_info->notify.rsrc_type]);    
             tet_printf(" Water mark type              :%s \n",srmsv_wmk_type[rsrc_info->notify.watermarks.wm_type]);    
             tet_printf(" Water mark values exist      :%d \n",rsrc_info->notify.watermarks.wm_exist);    
             tcd.clbk_data.rsrc_mon_hdl = rsrc_info->rsrc_mon_hdl;
             tcd.clbk_data.nodeId = rsrc_info->node_num;
             tcd.clbk_data.rsrc_type = rsrc_info->notify.rsrc_type;
             break;

        default : 
             m_TET_SRMSV_PRINTF("\n UNKNOWN CALLBACK TYPE \n"); 
             tet_printf("\n UNKNOWN CALLBACK TYPE \n"); 
    }

    tcd.disp_flag=1;
    m_TET_SRMSV_PRINTF("\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ");
    m_TET_SRMSV_PRINTF("\n                  END OF CALLBACK EVENT                 ");
    m_TET_SRMSV_PRINTF("\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ \n");
    tet_printf("\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ");
    tet_printf("\n                  END OF CALLBACK EVENT                 ");
    tet_printf("\n ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ \n");

} 

int scale_type_float_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, double clbk_val)
{
   int result = TET_PASS;
   switch(scale_type)
   {
       case NCS_SRMSV_STAT_SCALE_BYTE: 
       case NCS_SRMSV_STAT_SCALE_KBYTE:   
       case NCS_SRMSV_STAT_SCALE_MBYTE:  
       case NCS_SRMSV_STAT_SCALE_GBYTE: 
       case NCS_SRMSV_STAT_SCALE_TBYTE:
             if(clbk_val < 0)
             {
                 m_TET_SRMSV_PRINTF(" Callback value is less than zero, undefined value \n");
                 tet_printf(" Callback value is less than zero, undefined value \n");
                 result = TET_FAIL;
             } 
             break;

       case NCS_SRMSV_STAT_SCALE_PERCENT:
             if(clbk_val < 0)
             {
                 m_TET_SRMSV_PRINTF(" Callback value is less than zero, undefined value \n");
                 tet_printf(" Callback value is less than zero, undefined value \n");
                 result = TET_FAIL;
             } 
             if(clbk_val > 100)
             {
                 m_TET_SRMSV_PRINTF(" Percentages max value is 100, check failed \n");
                 tet_printf(" Percentages max value is 100, check failed \n");
                 result = TET_FAIL;
             } 
             break;

       default:
             m_TET_SRMSV_PRINTF(" UNDEFINED SCALE TYPE \n");
             tet_printf(" UNDEFINED SCALE TYPE \n");
   }

   return result;
}

int scale_type_ival32_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, int32 clbk_val)
{
   int result = TET_PASS;
   switch(scale_type)
   {
       case NCS_SRMSV_STAT_SCALE_BYTE: 
       case NCS_SRMSV_STAT_SCALE_KBYTE:   
       case NCS_SRMSV_STAT_SCALE_MBYTE:  
       case NCS_SRMSV_STAT_SCALE_GBYTE: 
       case NCS_SRMSV_STAT_SCALE_TBYTE:
             if(clbk_val < 0)
             {
                 m_TET_SRMSV_PRINTF(" Callback value is less than zero, undefined value \n");
                 tet_printf(" Callback value is less than zero, undefined value \n");
                 result = TET_FAIL;
             } 
             break;

       case NCS_SRMSV_STAT_SCALE_PERCENT:
             if(clbk_val < 0)
             {
                 m_TET_SRMSV_PRINTF(" Callback value is less than zero, undefined value \n");
                 tet_printf(" Callback value is less than zero, undefined value \n");
                 result = TET_FAIL;
             } 
             if(clbk_val > 100)
             {
                 m_TET_SRMSV_PRINTF(" Percentages max value is 100, check failed \n");
                 tet_printf(" Percentages max value is 100, check failed \n");
                 result = TET_FAIL;
             } 
             break;

       default:
             m_TET_SRMSV_PRINTF(" UNDEFINED SCALE TYPE \n");
             tet_printf(" UNDEFINED SCALE TYPE \n");
   }

   return result;
}

int scale_type_uval32_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, uns32 clbk_val)
{
   int result = TET_PASS;
   switch(scale_type)
   {
       case NCS_SRMSV_STAT_SCALE_BYTE: 
       case NCS_SRMSV_STAT_SCALE_KBYTE:   
       case NCS_SRMSV_STAT_SCALE_MBYTE:  
       case NCS_SRMSV_STAT_SCALE_GBYTE: 
       case NCS_SRMSV_STAT_SCALE_TBYTE:
             if(clbk_val < 0)
             {
                 m_TET_SRMSV_PRINTF(" Callback value is less than zero, undefined value \n");
                 tet_printf(" Callback value is less than zero, undefined value \n");
                 result = TET_FAIL;
             } 
             break;

       case NCS_SRMSV_STAT_SCALE_PERCENT:
             if(clbk_val < 0)
             {
                 m_TET_SRMSV_PRINTF(" Callback value is less than zero, undefined value \n");
                 tet_printf(" Callback value is less than zero, undefined value \n");
                 result = TET_FAIL;
             } 
             if(clbk_val > 100)
             {
                 m_TET_SRMSV_PRINTF(" Percentages max value is 100, check failed \n");
                 tet_printf(" Percentages max value is 100, check failed \n");
                 result = TET_FAIL;
             } 
             break;

       default:
             m_TET_SRMSV_PRINTF(" UNDEFINED SCALE TYPE \n");
             tet_printf(" UNDEFINED SCALE TYPE \n");
   }

   return result;
}

int scale_type_ival64_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, int64 clbk_val)
{
   int result = TET_PASS;
   switch(scale_type)
   {
       case NCS_SRMSV_STAT_SCALE_BYTE: 
       case NCS_SRMSV_STAT_SCALE_KBYTE:   
       case NCS_SRMSV_STAT_SCALE_MBYTE:  
       case NCS_SRMSV_STAT_SCALE_GBYTE: 
       case NCS_SRMSV_STAT_SCALE_TBYTE:
             if(clbk_val < 0)
             {
                 m_TET_SRMSV_PRINTF(" Callback value is less than zero, undefined value \n");
                 tet_printf(" Callback value is less than zero, undefined value \n");
                 result = TET_FAIL;
             } 
             break;

       case NCS_SRMSV_STAT_SCALE_PERCENT:
             if(clbk_val < 0)
             {
                 m_TET_SRMSV_PRINTF(" Callback value is less than zero, undefined value \n");
                 tet_printf(" Callback value is less than zero, undefined value \n");
                 result = TET_FAIL;
             } 
             if(clbk_val > 100)
             {
                 m_TET_SRMSV_PRINTF(" Percentages max value is 100, check failed \n");
                 tet_printf(" Percentages max value is 100, check failed \n");
                 result = TET_FAIL;
             } 
             break;

       default:
             m_TET_SRMSV_PRINTF(" UNDEFINED SCALE TYPE \n");
             tet_printf(" UNDEFINED SCALE TYPE \n");
   }

   return result;
}

int scale_type_uval64_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, uns64 clbk_val)
{
   int result = TET_PASS;
   switch(scale_type)
   {
       case NCS_SRMSV_STAT_SCALE_BYTE: 
       case NCS_SRMSV_STAT_SCALE_KBYTE:   
       case NCS_SRMSV_STAT_SCALE_MBYTE:  
       case NCS_SRMSV_STAT_SCALE_GBYTE: 
       case NCS_SRMSV_STAT_SCALE_TBYTE:
             if(clbk_val < 0)
             {
                 m_TET_SRMSV_PRINTF(" Callback value is less than zero, undefined value \n");
                 tet_printf(" Callback value is less than zero, undefined value \n");
                 result = TET_FAIL;
             } 
             break;

       case NCS_SRMSV_STAT_SCALE_PERCENT:
             if(clbk_val < 0)
             {
                 m_TET_SRMSV_PRINTF(" Callback value is less than zero, undefined value \n");
                 tet_printf(" Callback value is less than zero, undefined value \n");
                 result = TET_FAIL;
             } 
             if(clbk_val > 100)
             {
                 m_TET_SRMSV_PRINTF(" Percentages max value is 100, check failed \n");
                 tet_printf(" Percentages max value is 100, check failed \n");
                 result = TET_FAIL;
             } 
             break;

       default:
             m_TET_SRMSV_PRINTF(" UNDEFINED SCALE TYPE \n");
             tet_printf(" UNDEFINED SCALE TYPE \n");
   }

   return result;
}

int scale_type_check(NCS_SRMSV_STAT_SCALE_TYPE scale_type, NCS_SRMSV_VALUE clbk_val)
{
  int result = TET_FAIL;

  if(scale_type != clbk_val.scale_type)
  {
    m_TET_SRMSV_PRINTF(" Scale Type not expected \n");
    tet_printf(" Scale Type not expected \n");
    return result; 
  }

  switch(clbk_val.val_type)
  {
    case NCS_SRMSV_VAL_TYPE_FLOAT :
             result = scale_type_float_check(scale_type,clbk_val.val.f_val);
             break;

    case NCS_SRMSV_VAL_TYPE_INT32 :
             result = scale_type_ival32_check(scale_type,clbk_val.val.i_val32);
             break;

    case NCS_SRMSV_VAL_TYPE_UNS32 :
             result = scale_type_uval32_check(scale_type,clbk_val.val.u_val32);
             break;

    case NCS_SRMSV_VAL_TYPE_INT64 :
             result = scale_type_ival64_check(scale_type,clbk_val.val.i_val64);
             break;

    case NCS_SRMSV_VAL_TYPE_UNS64 :
             result = scale_type_uval64_check(scale_type,clbk_val.val.u_val64);
             break;

    default:
             m_TET_SRMSV_PRINTF(" Unknown val type in scale_type check\n");
             tet_printf(" Unknown val type in scale_type check\n");
             break;
  }

  return result;
}


int check_threshold_clbk(SRMSV_CALLBACK_DATA clbk_data,uns32 node_num,NCS_SRMSV_STAT_SCALE_TYPE scale_type,
                         NCS_SRMSV_THRESHOLD_TEST_CONDITION condition,NCS_SRMSV_VALUE exp_val,
                         NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl)
{
   m_TET_SRMSV_PRINTF("\n Checking callback return values ... \n");
   tet_printf("\n Checking callback return values ... \n");

   int result = TET_FAIL;
   if(clbk_data.not_type == SRMSV_CBK_NOTIF_RSRC_THRESHOLD)
   {
      if(clbk_data.nodeId == node_num)
      {
         if(clbk_data.rsrc_mon_hdl == rsrc_mon_hdl)
         {
            result = scale_type_check(scale_type,clbk_data.threshold_val);
            if(result != TET_PASS)
               goto final;

            result = check_condition(clbk_data.threshold_val,exp_val,tcd.tolerable_val,condition);
         }
         else
         {
            m_TET_SRMSV_PRINTF(" Handle doesnot match \n");
            tet_printf(" Handle doesnot match \n");
         }
      }
      else
      {
         m_TET_SRMSV_PRINTF(" Node num check failed \n");
         tet_printf(" Node num check failed \n");
      } 
   }
   else
   {
      m_TET_SRMSV_PRINTF(" Not a threshold callback . This should never be seen, please verify testcase\n");
      tet_printf(" Not a threshold callback . This should never be seen, please verify testcase\n");
   } 

final:
  return result;
};

int check_threshold_allclbk(SRMSV_CALLBACK_DATA clbk_data,NCS_SRMSV_STAT_SCALE_TYPE scale_type,
                         NCS_SRMSV_THRESHOLD_TEST_CONDITION condition,NCS_SRMSV_VALUE exp_val,
                         NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl)
{
   m_TET_SRMSV_PRINTF("\n Checking callback return values ... \n");
   tet_printf("\n Checking callback return values ... \n");

   int result = TET_FAIL;
   if(clbk_data.not_type == SRMSV_CBK_NOTIF_RSRC_THRESHOLD)
   {
      if(clbk_data.rsrc_mon_hdl == rsrc_mon_hdl)
      {
         result = scale_type_check(scale_type,clbk_data.threshold_val);
         if(result != TET_PASS)
            goto final;

         result = check_condition(clbk_data.threshold_val,exp_val,tcd.tolerable_val,condition);
      }
      else
      {
         m_TET_SRMSV_PRINTF(" Handle doesnot match \n");
         tet_printf(" Handle doesnot match \n");
      }
   }
   else
   {
      m_TET_SRMSV_PRINTF(" Not a threshold callback . This should never be seen, please verify testcase\n");
      tet_printf(" Not a threshold callback . This should never be seen, please verify testcase\n");
   } 

final:
  return result;
};


int wmk_value_check(NCS_SRMSV_VALUE clbk_value)
{
    int result = TET_FAIL;

    switch(clbk_value.val_type)
    {
       case NCS_SRMSV_VAL_TYPE_FLOAT :
                result = scale_type_float_check(clbk_value.scale_type,clbk_value.val.f_val);
                break;

       case NCS_SRMSV_VAL_TYPE_INT32 :
                result = scale_type_ival32_check(clbk_value.scale_type,clbk_value.val.i_val32);
                break;

       case NCS_SRMSV_VAL_TYPE_UNS32 :
                result = scale_type_uval32_check(clbk_value.scale_type,clbk_value.val.u_val32);
                break;

       case NCS_SRMSV_VAL_TYPE_INT64 :
                result = scale_type_ival64_check(clbk_value.scale_type,clbk_value.val.i_val64);
                break;

       case NCS_SRMSV_VAL_TYPE_UNS64 :
                result = scale_type_uval64_check(clbk_value.scale_type,clbk_value.val.u_val64);
                break;

       default:
                m_TET_SRMSV_PRINTF(" Unknown val type in wmk_value check\n");
                tet_printf(" Unknown val type in wmk_value check\n");
                result = TET_FAIL;
                break;
    }

    return result;
}

int check_wmk_clbk(SRMSV_CALLBACK_DATA clbk_data,uns32 node_num,NCS_SRMSV_RSRC_TYPE rsrc_type)
{
   m_TET_SRMSV_PRINTF("\n Checking callback return values ... \n");
   tet_printf("\n Checking callback return values ... \n");

   int result = TET_FAIL;
 
   if(clbk_data.not_type == SRMSV_CBK_NOTIF_WATERMARK_VAL)
   {
      if(clbk_data.nodeId == node_num)
      {
         if(clbk_data.wm_exist == 1)
         {
            if(clbk_data.wm_type != tcd.clbk_data.exp_wm_type)
            {
               m_TET_SRMSV_PRINTF(" Not expected Water mark type \n");
               tet_printf(" Not expected Water mark type \n");
               goto final;
            }

            switch(clbk_data.wm_type)
            {
               case NCS_SRMSV_WATERMARK_HIGH:

                    if(clbk_data.rsrc_type == rsrc_type)
                       result = wmk_value_check(clbk_data.high_val);
                    else
                    {
                       m_TET_SRMSV_PRINTF(" check_wmk_clbk :: Rsrc type not expected, test failed \n");
                       tet_printf(" check_wmk_clbk :: Rsrc type not expected, test failed \n");
                    } 
                    break;

               case NCS_SRMSV_WATERMARK_LOW:
                    if(clbk_data.rsrc_type == rsrc_type)
                        result = wmk_value_check(clbk_data.low_val);
                    else
                    {
                       m_TET_SRMSV_PRINTF(" check_wmk_clbk :: Rsrc type not expected, test failed \n");
                       tet_printf(" check_wmk_clbk :: Rsrc type not expected, test failed \n");
                    } 
                    break;

               case NCS_SRMSV_WATERMARK_DUAL:
                    if(clbk_data.rsrc_type == rsrc_type)
                    {
                        m_TET_SRMSV_PRINTF(" LOW_VAL check ... \n");
                        tet_printf(" LOW_VAL check ... \n");
                        result = wmk_value_check(clbk_data.low_val);
                        if(result == TET_PASS)
                        {
                           m_TET_SRMSV_PRINTF(" HIGH_VAL check ... \n");
                           tet_printf(" HIGH_VAL check ... \n");
                           result = wmk_value_check(clbk_data.high_val);
                        }
                    }
                    else
                    {
                       m_TET_SRMSV_PRINTF(" check_wmk_clbk :: Rsrc type not expected, test failed \n");
                       tet_printf(" check_wmk_clbk :: Rsrc type not expected, test failed \n");
                    } 
                    break;

               default:
                    m_TET_SRMSV_PRINTF(" check_wmk_clbk :: wmk_type not expected, test failed \n");
                    tet_printf(" check_wmk_clbk :: wmk_type not expected, test failed \n");
            }
         }
         else
         {
            m_TET_SRMSV_PRINTF(" WM_EXIST flag not set, test fail \n");
            tet_printf(" WM_EXIST flag not set, test fail \n");
         }
      }
      else
      {
         m_TET_SRMSV_PRINTF(" Node num check failed \n");
         tet_printf(" Node num check failed \n");
      } 
   }
   else
   {
      m_TET_SRMSV_PRINTF(" Not a watermark val callback . This should never be seen, please verify testcase\n");
      tet_printf(" Not a watermark val callback . This should never be seen, please verify testcase\n");
   } 

final:
  return result;
};


int check_wmk_allclbk(SRMSV_CALLBACK_DATA clbk_data,NCS_SRMSV_RSRC_TYPE rsrc_type)
{
   m_TET_SRMSV_PRINTF("\n Checking callback return values ... \n");
   tet_printf("\n Checking callback return values ... \n");

   int result = TET_FAIL;
 
   if(clbk_data.not_type == SRMSV_CBK_NOTIF_WATERMARK_VAL)
   {
      if(clbk_data.wm_exist == 1)
      {
         if(clbk_data.wm_type != tcd.clbk_data.exp_wm_type)
         {
            m_TET_SRMSV_PRINTF(" Not expected Water mark type \n");
            tet_printf(" Not expected Water mark type \n");
            goto final;
         }

         switch(clbk_data.wm_type)
         {
            case NCS_SRMSV_WATERMARK_HIGH:
                 if(clbk_data.rsrc_type == rsrc_type)
                    result = wmk_value_check(clbk_data.high_val);
                 else
                 {
                    m_TET_SRMSV_PRINTF(" check_wmk_allclbk :: Rsrc type not expected, test failed \n");
                    tet_printf(" check_wmk_allclbk :: Rsrc type not expected, test failed \n");
                 } 
                 break;

            case NCS_SRMSV_WATERMARK_LOW:
                 if(clbk_data.rsrc_type == rsrc_type)
                     result = wmk_value_check(clbk_data.low_val);
                 else
                 {
                    m_TET_SRMSV_PRINTF(" check_wmk_allclbk :: Rsrc type not expected, test failed \n");
                    tet_printf(" check_wmk_allclbk :: Rsrc type not expected, test failed \n");
                 } 
                 break;

            case NCS_SRMSV_WATERMARK_DUAL:
                 if(clbk_data.rsrc_type == rsrc_type)
                 {
                     m_TET_SRMSV_PRINTF(" LOW_VAL check ... \n");
                     tet_printf(" LOW_VAL check ... \n");
                     result = wmk_value_check(clbk_data.low_val);
                     if(result == TET_PASS)
                     {
                        m_TET_SRMSV_PRINTF(" HIGH_VAL check ... \n");
                        tet_printf(" HIGH_VAL check ... \n");
                        result = wmk_value_check(clbk_data.high_val);
                     }
                 }
                 else
                 {
                    m_TET_SRMSV_PRINTF(" check_wmk_allclbk :: Rsrc type not expected, test failed \n");
                    tet_printf(" check_wmk_allclbk :: Rsrc type not expected, test failed \n");
                 } 
                 break;

            default:
                 m_TET_SRMSV_PRINTF(" check_wmk_allclbk :: wmk_type not expected, test failed \n");
                 tet_printf(" check_wmk_allclbk :: wmk_type not expected, test failed \n");
         }
      }
      else
      {
         m_TET_SRMSV_PRINTF(" WM_EXIST flag not set, test fail \n");
         tet_printf(" WM_EXIST flag not set, test fail \n");
      }
   }
   else
   {
      m_TET_SRMSV_PRINTF(" Not a watermark val callback . This should never be seen, please verify testcase\n");
      tet_printf(" Not a watermark val callback . This should never be seen, please verify testcase\n");
   } 

final:
  return result;
};


#endif
