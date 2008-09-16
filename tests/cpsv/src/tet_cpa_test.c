#if ( TET_A==1 )

#include "tet_startup.h"
#include "tet_cpsv.h"
#include "tet_cpsv_conf.h"
#include "ncs_main_papi.h"

#define m_TET_CPSV_PRINTF printf

extern int gl_prev_act;

void tet_readFile()
{
  FILE *fp1,*fp2,*fp3;
  int x=0,rv;
  char *tmp_ptr=NULL;
  char *base_dir;

  tmp_ptr = (char *) getenv("TET_BASE_DIR");
  if(tmp_ptr)
  {
     base_dir = (char *)calloc(sizeof(char),strlen(tmp_ptr)+strlen("/cpsv/src/tet_cpa.c")+1);
     strcpy(base_dir,tmp_ptr);
     strcat(base_dir,"/cpsv/src/tet_cpa.c");

     if ((fp1 = fopen(base_dir,"r"))==NULL)
     {
        perror("\nERROR OPENING FILE");
        m_NCS_CONS_PRINTF("Cannot Open tet_cpa.c\n");
        exit(0);
     }

     if ((fp2 = fopen(base_dir,"r"))==NULL)
     {
        perror("\nERROR OPENING FILE");
        m_NCS_CONS_PRINTF("Cannot Open tet_cpa.c\n");
        exit(0);
     }

     if ((fp3 = fopen(base_dir,"r"))==NULL)
     {
        perror("\nERROR OPENING FILE");
        m_NCS_CONS_PRINTF("Cannot Open tet_cpa.c\n");
        exit(0);
     }

     while(!feof(fp1))
     {
        if(x<500)
        {
          tcd.filedata1[x]=getc(fp1);
          x++;
        }
        else break;
     }

     x=0;
     while(!feof(fp2))
     {
       if(x<500)
       {
          tcd.filedata2[x]=getc(fp2);
          x++;
       }
        else break;
     }

     x=0;
     while(!feof(fp3))
     {
        if(x<500)
        {
          tcd.filedata3[x]=getc(fp3);
          x++;
        }
        else break;
     }
     rv = fclose(fp1);
     rv = fclose(fp2);
     rv = fclose(fp3);
     free(base_dir);
  }
}

void cpsv_clean_clbk_params()
{
   tcd.open_clbk_hdl = 0;
   tcd.open_clbk_err = 0;
   tcd.open_clbk_invo = 0;
   tcd.sync_clbk_invo = 0;
   tcd.sync_clbk_err = 0;
   tcd.arr_clbk_flag = 0;
   tcd.arr_clbk_err = 0;
}

void printHead(char *str)
{
  m_TET_CPSV_PRINTF("\n************************************************************* \n");
  m_TET_CPSV_PRINTF("%s",str);
  m_TET_CPSV_PRINTF("\n************************************************************* \n");

  tet_printf("\n************************************************************* \n");
  tet_printf("%s",str);
  tet_printf("\n************************************************************* \n");
}

void printResult(int result)
{
  gl_sync_pointnum=1;
  if (result == TET_PASS)
  {
     m_TET_CPSV_PRINTF("\n ##### TEST CASE SUCCEEDED #####\n");
     tet_printf("\n ##### TEST CASE SUCCEEDED #####\n");
  }
  else if(result == TET_FAIL)
  {
     m_TET_CPSV_PRINTF("\n ##### TEST CASE FAILED #####\n");
     tet_printf("\n ##### TEST CASE FAILED #####\n");
  }
  else if(result == TET_UNRESOLVED)
  {
     m_TET_CPSV_PRINTF("\n ##### TEST CASE UNRESOLVED #####\n");
     tet_printf("\n ##### TEST CASE UNRESOLVED #####\n");
  }

  tet_result(result);
}

void handleAssigner(SaInvocationT invocation, SaCkptCheckpointHandleT checkpointHandle)
{
   if (invocation == 1014)
    tcd.all_collocated_Writehdl=checkpointHandle;
   else if (invocation == 1015)
    tcd.all_collocated_Readhdl=checkpointHandle;
   else if (invocation == 1016)
    tcd.weak_collocated_Writehdl=checkpointHandle;
   else if (invocation == 1017)
    tcd.weak_collocated_Readhdl=checkpointHandle;
   else if (invocation == 1018)
    tcd.async_all_replicas_hdl = checkpointHandle;
}

void fill_ckpt_version(SaVersionT *version,SaUint8T rel_code,SaUint8T mjr_ver,SaUint8T mnr_ver)
{
   version->releaseCode = rel_code;
   version->majorVersion = mjr_ver;
   version->minorVersion = mnr_ver;
}
                                                                                                                                                                      
void fill_ckpt_callbacks(SaCkptCallbacksT *clbks,SaCkptCheckpointOpenCallbackT open_clbk,SaCkptCheckpointSynchronizeCallbackT sync_clbk)
{
   clbks->saCkptCheckpointOpenCallback = open_clbk;
   clbks->saCkptCheckpointSynchronizeCallback = sync_clbk;
}
                                                                                                                                                                      
void fill_ckpt_attri(SaCkptCheckpointCreationAttributesT *cr_attr,SaCkptCheckpointCreationFlagsT cr_flags,SaSizeT ckptSize, SaTimeT ret,
                                                         SaUint32T max_sec, SaSizeT max_sec_size, SaSizeT max_sec_id_size)
{
   cr_attr->creationFlags = cr_flags;
   cr_attr->checkpointSize = ckptSize;
   cr_attr->retentionDuration = ret;
   cr_attr->maxSections = max_sec;
   cr_attr->maxSectionSize = max_sec_size;
   cr_attr->maxSectionIdSize = max_sec_id_size;
}
                                                                                                                                                                      
void fill_ckpt_name(SaNameT *name,char *string)
{
   strcpy(name->value,string);
   name->length = strlen(name->value);
}

void fill_sec_attri(SaCkptSectionCreationAttributesT *sec_cr_attr,SaCkptSectionIdT *sec,SaTimeT exp_time)
{
   sec_cr_attr->sectionId = sec;
   sec_cr_attr->expirationTime = exp_time;
}
                                                                                                                                                                      
void fill_write_iovec(SaCkptIOVectorElementT *iovec,SaCkptSectionIdT sec,char *buf,SaSizeT size,SaOffsetT offset,SaSizeT read_size)
{
   iovec->sectionId = sec;
   iovec->dataBuffer = buf;
   iovec->dataSize = size;
   iovec->dataOffset = offset;
   iovec->readSize = read_size;
}
                                                                                                                                                                      
void fill_read_iovec(SaCkptIOVectorElementT *iovec,SaCkptSectionIdT sec,char *buf,SaSizeT size,SaOffsetT offset,SaSizeT read_size)
{
   iovec->sectionId = sec;
   iovec->dataBuffer = buf;
   iovec->dataSize = size;
   iovec->dataOffset = offset;
   iovec->readSize = read_size;
}
                                                                                                                                                                      
void AppCkptArrivalCallback(const SaCkptCheckpointHandleT ckptArrivalHandle, SaCkptIOVectorElementT *ioVector, SaUint32T num_of_elem)
{
  int x;
  int result;

  tcd.arr_clbk_flag = 1;
  for (x=0;x<num_of_elem;x++)
  {
     m_TET_CPSV_PRINTF(" Section Id is %s\n",(char *)(ioVector[x].sectionId.id)); 
     m_TET_CPSV_PRINTF(" Section IdLen is %d\n",ioVector[x].sectionId.idLen);
     m_TET_CPSV_PRINTF("DataSize:%llu\n",ioVector[x].dataSize);
     m_TET_CPSV_PRINTF("ReadSize:%llu\n",ioVector[x].readSize);
     if(ioVector[x].dataBuffer != NULL)
     {
        result = TET_FAIL;
        tcd.arr_clbk_err = 1;
        break;
     }
     else
        result = TET_PASS;
  }
}

void AppCkptOpenCallback(SaInvocationT invocation, SaCkptCheckpointHandleT checkpointHandle, SaAisErrorT error)
{
  tcd.open_clbk_invo = invocation;
  tcd.open_clbk_err = error;

  if (error != SA_AIS_OK)
  {
    if ((invocation == 1006) && (error == SA_AIS_ERR_NOT_EXIST) )
    {
      m_TET_CPSV_PRINTF(" Checkpoint Open Async callback success with invocation %llu\n",invocation);
      return;
    }
    else if ((invocation == 1019) && (error == SA_AIS_ERR_EXIST) )
    {
      m_TET_CPSV_PRINTF(" Checkpoint Open Async callback success with invocation %llu and error %s\n",invocation,saf_error_string[error]);
      return;
    }
    else
    {
      m_TET_CPSV_PRINTF("%s Checkpoint Open Async callback unsuccessful with invocation %llu\n",saf_error_string[error],invocation);
      return;
    }
  }
  else
  {
    tcd.open_clbk_hdl = checkpointHandle;
    handleAssigner(invocation,checkpointHandle);
    m_TET_CPSV_PRINTF(" Checkpoint Open Async callback success with invocation %llu\n",invocation);
    return;
  }

}

void AppCkptSyncCallback(SaInvocationT invocation, SaAisErrorT error)
{
  tcd.sync_clbk_invo = invocation;
  tcd.sync_clbk_err = error;

  if (error != SA_AIS_OK)
  {
    if ((invocation == 2112) && (error == SA_AIS_ERR_NOT_EXIST) )
    {
      m_TET_CPSV_PRINTF(" Checkpoint Sync Callback success with invocation %llu\n",invocation);
      return;
    }
    else
    {
      m_TET_CPSV_PRINTF(" %s Checkpoint Sync Callback unsuccessful with invocation %llu\n",saf_error_string[error],invocation);
      return;
    }
  }
  else
  {
    m_TET_CPSV_PRINTF(" Checkpoint Sync Callback success with invocation %llu\n",invocation);
    return;
  }
}

struct cpsv_testcase_data tcd= {.section3=SA_CKPT_DEFAULT_SECTION_ID,.section4=SA_CKPT_GENERATED_SECTION_ID,.section6=SA_CKPT_GENERATED_SECTION_ID,.gen_sec_del=SA_CKPT_GENERATED_SECTION_ID,.gen_sec=SA_CKPT_GENERATED_SECTION_ID,.section7=SA_CKPT_GENERATED_SECTION_ID};

void fill_testcase_data()
{
   /* Variables for initialization */
   fill_ckpt_version(&tcd.version_err,'A',0x01,0x01);
   fill_ckpt_version(&tcd.version_supported,'B',0x02,0x02);
   fill_ckpt_version(&tcd.version_err2,'C',0x01,0x01);

   fill_ckpt_callbacks(&tcd.general_callbks,AppCkptOpenCallback,AppCkptSyncCallback);
   fill_ckpt_callbacks(&tcd.open_null_callbk,NULL,AppCkptSyncCallback);
   fill_ckpt_callbacks(&tcd.synchronize_null_callbk,AppCkptOpenCallback,NULL);
   fill_ckpt_callbacks(&tcd.null_callbks,NULL,NULL);

   /* Variables for ckpt open */
   fill_ckpt_attri(&tcd.all_replicas,SA_CKPT_WR_ALL_REPLICAS,169,100000,2,85,3);
   fill_ckpt_attri(&tcd.smoke_test_replica,SA_CKPT_CHECKPOINT_COLLOCATED |SA_CKPT_WR_ACTIVE_REPLICA,169,100,2,85,3);
   fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,100,2,85,3);
   fill_ckpt_attri(&tcd.replica_weak,SA_CKPT_WR_ACTIVE_REPLICA_WEAK,85,100,1,85,3);
   fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
   fill_ckpt_attri(&tcd.ckpt_all_collocated_replica,SA_CKPT_WR_ALL_REPLICAS|SA_CKPT_CHECKPOINT_COLLOCATED,169,10,2,85,3);
   fill_ckpt_attri(&tcd.ckpt_weak_collocated_replica,SA_CKPT_WR_ACTIVE_REPLICA_WEAK|SA_CKPT_CHECKPOINT_COLLOCATED,1069,10,2,850,3);
   fill_ckpt_attri(&tcd.multi_io_replica,SA_CKPT_WR_ALL_REPLICAS,4000,50,10,400,3);
   fill_ckpt_attri(&tcd.invalid,SA_CKPT_WR_ACTIVE_REPLICA,300,100,3,85,3);
   fill_ckpt_attri(&tcd.invalid2,SA_CKPT_WR_ALL_REPLICAS|SA_CKPT_WR_ACTIVE_REPLICA,255,100,3,85,3);
   fill_ckpt_attri(&tcd.invalid3,SA_CKPT_WR_ALL_REPLICAS|SA_CKPT_WR_ACTIVE_REPLICA_WEAK,255,100,3,85,3);
   fill_ckpt_attri(&tcd.invalid4,SA_CKPT_WR_ACTIVE_REPLICA|SA_CKPT_WR_ACTIVE_REPLICA_WEAK,255,100,3,85,3);
   fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,140,100,2,85,3);
   fill_ckpt_attri(&tcd.my_app,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ALL_REPLICAS,140,SA_TIME_END,2,85,3);

   fill_ckpt_name(&tcd.all_replicas_ckpt,"all replicas ckpt");
   fill_ckpt_name(&tcd.active_replica_ckpt,"active replica ckpt");
   fill_ckpt_name(&tcd.weak_replica_ckpt,"weak replica ckpt");
   fill_ckpt_name(&tcd.collocated_ckpt,"collocated ckpt");
   fill_ckpt_name(&tcd.async_all_replicas_ckpt,"async all ckpt");
   fill_ckpt_name(&tcd.async_active_replica_ckpt,"async active ckpt");
   fill_ckpt_name(&tcd.smoketest_ckpt,"active ckpt test");
   fill_ckpt_name(&tcd.all_collocated_ckpt,"all collocated ckpt");
   fill_ckpt_name(&tcd.weak_collocated_ckpt,"weak collocated ckpt");
   fill_ckpt_name(&tcd.non_existing_ckpt,"nonexisting ckpt");
   fill_ckpt_name(&tcd.multi_vector_ckpt,"multi vector ckpt");
   fill_ckpt_name(&tcd.all_replicas_ckpt_large,"all replicas large ckpt");
   fill_ckpt_name(&tcd.active_replica_ckpt_large,"active replica large ckpt");
   fill_ckpt_name(&tcd.weak_replica_ckpt_large,"weak replica large ckpt");
   fill_ckpt_name(&tcd.collocated_ckpt_large,"collocated large ckpt");

   /* Variables for sec create */
   tcd.sec_id1 = 11;
   tcd.section1.idLen = 2;
   tcd.section1.id = &tcd.sec_id1;

   tcd.sec_id2 = 21;
   tcd.section2.idLen = 2;
   tcd.section2.id = &tcd.sec_id2;

   tcd.sec_id3 = 31;
   tcd.section5.idLen = 2;
   tcd.section5.id = &tcd.sec_id3;

   tcd.invalid_sec.idLen = 8;
   tcd.invalid_sec.id = &tcd.sec_id2;

   tcd.inv_sec = 222;
   tcd.invalidsection.idLen = 3;
   tcd.invalidsection.id = &tcd.inv_sec;
   
   tcd.sec_id4 = 123;
   tcd.invalidSection.idLen = 3;
   tcd.invalidSection.id = &tcd.sec_id4;

#if 0
   fill_sec(&tcd.section1,2,"11");
   fill_sec(&tcd.section2,2,"21");
   fill_sec(&tcd.section5,2,"31");
   fill_sec(&tcd.invalid_sec,8,"21");
   fill_sec(&tcd.invalidsection,3,"222");
   fill_sec(&tcd.invalidSection,3,"123");
   tcd.section3=SA_CKPT_DEFAULT_SECTION_ID;
   tcd.section4=SA_CKPT_GENERATED_SECTION_ID;
   tcd.section6=SA_CKPT_GENERATED_SECTION_ID;
   fill_ckpt_name(&tcd.invalidName,NULL);
#endif
   fill_sec_attri(&tcd.general_attr,&tcd.section1,SA_TIME_ONE_DAY);
   fill_sec_attri(&tcd.expiration_attr,&tcd.section2,SA_TIME_END);
   fill_sec_attri(&tcd.section_attr,&tcd.section3,SA_TIME_ONE_DAY);
   fill_sec_attri(&tcd.special_attr,&tcd.section4,SA_TIME_END);
   fill_sec_attri(&tcd.multi_attr,&tcd.section5,SA_TIME_END);
   fill_sec_attri(&tcd.special_attr2,&tcd.section6,SA_TIME_END);
   fill_sec_attri(&tcd.special_attr3,&tcd.section7,SA_TIME_END);
   fill_sec_attri(&tcd.invalid_attr,&tcd.invalid_sec,SA_TIME_END);

   strcpy(tcd.data1,"This is data1");
   strcpy(tcd.data2,"This is data2");
   strcpy(tcd.data3,"This is data3");
#if 0 
   tcd.data1 = "This is data1";
   tcd.data2 = "This is data2";
   tcd.data3 = "This is data3";
#endif
   tcd.size = strlen("This is datax");
   tcd.size_zero=0;

   fill_write_iovec(&tcd.generate_write,tcd.section4,"Data in generated section",strlen("Data in generated section"),30,0);
   fill_write_iovec(&tcd.generate_write_large,tcd.section1,tcd.filedata2,350,0,0);
   fill_write_iovec(&tcd.general_write,tcd.section1,"Data in general east or west india is the best",strlen("Data in general east or west india is the best"),0,0);
   fill_write_iovec(&tcd.general_write2,tcd.section1,"Data in general east or west india is the best",
                                                      strlen("Data in general east or west india is the best"),590,0);
   fill_write_iovec(&tcd.generate_write1,tcd.section6,"Data in second generated section",strlen("Data in second generated section"),30,0);
   fill_write_iovec(&tcd.multi_vector[0],tcd.section1,"Data in general east or west india is the best",50,0,0);
   fill_write_iovec(&tcd.multi_vector[1],tcd.section2,(char *)tcd.filedata2,300,0,0);
   fill_write_iovec(&tcd.multi_vector[2],tcd.section5,(char *)tcd.filedata3,300,0,0);
   fill_write_iovec(&tcd.invalid_write,tcd.invalidsection,"invalid",7,0,0);
   fill_write_iovec(&tcd.default_write,tcd.section3,"default data",strlen("default data"),0,0);
   fill_write_iovec(&tcd.invalid_write2,tcd.section1,"Data in general east or west india is the best",1024,0,0);
   tcd.nOfE=1;
   tcd.nOfE2=3;

   fill_read_iovec(&tcd.generate_read,tcd.section1,tcd.buffer,75,0,0);
   fill_read_iovec(&tcd.generate_read_large,tcd.section1,tcd.buffer_large,325,0,0);
   fill_read_iovec(&tcd.general_read,tcd.section1,(char *)tcd.buffer,strlen("Data in general east or west india is the best"),0,0);
   fill_read_iovec(&tcd.default_read,tcd.section3,(char *)tcd.buffer,strlen("default data"),0,0);
   fill_read_iovec(&tcd.multi_read_vector[0],tcd.section1,(char *)tcd.buffer1,200,0,0);
   fill_read_iovec(&tcd.multi_read_vector[1],tcd.section2,(char *)tcd.buffer2,200,0,0);
   fill_read_iovec(&tcd.multi_read_vector[2],tcd.section5,(char *)tcd.buffer3,200,0,0);
   fill_read_iovec(&tcd.invalid_read,tcd.invalidsection,(char *)tcd.buffer,13,0,0);
   fill_read_iovec(&tcd.invalid_read2,tcd.section1,(char *)tcd.buffer,1024,0,0);
   fill_read_iovec(&tcd.invalid_read3,tcd.section1,(char *)tcd.buffer,10,10240,0);
   fill_read_iovec(&tcd.buffer_null,tcd.section3,NULL,13,0,0);
   strcpy(tcd.overdata,"overwritten data into the section");
   tcd.size1 = sizeof("Overwritten data");
   tcd.size2 = 1024;

   tcd.sec_forever = SA_CKPT_SECTIONS_FOREVER;
   tcd.sec_any = SA_CKPT_SECTIONS_ANY;
   tcd.exp_leq = SA_CKPT_SECTIONS_LEQ_EXPIRATION_TIME;
   tcd.exp_geq = SA_CKPT_SECTIONS_GEQ_EXPIRATION_TIME;
   tcd.sec_corrupt = SA_CKPT_SECTIONS_CORRUPTED;
   tcd.sec_invalid = 6;

   fill_ckpt_name(&tcd.invalidName2,"none");
}

void tet_cpsv_cleanup(CPSV_CLEANUP_TC_TYPE tc)
{
  int error;

  switch(tc)
  {
     case CPSV_CLEAN_INIT_SUCCESS_T:
          error = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;

     case CPSV_CLEAN_INIT_NULL_CBKS_T:
          error = tet_test_ckptFinalize(CKPT_FIN_SUCCESS6_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;

     case CPSV_CLEAN_INIT_OPEN_NULL_CBK_T:
          error = tet_test_ckptFinalize(CKPT_FIN_SUCCESS3_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;
  
     case CPSV_CLEAN_INIT_SYNC_NULL_CBK_T:
          error = tet_test_ckptFinalize(CKPT_FIN_SUCCESS4_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;
  
     case CPSV_CLEAN_INIT_NULL_CBKS2_T:
          error = tet_test_ckptFinalize(CKPT_FIN_SUCCESS5_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;
  
     case CPSV_CLEAN_INIT_SUCCESS_HDL2_T:
          error = tet_test_ckptFinalize(CKPT_FIN_SUCCESS2_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;

  }

  if(error != TET_PASS)
    m_TET_CPSV_PRINTF("\n Handle not cleanedup\n");
}

void tet_ckpt_cleanup(CPSV_CLEANUP_CKPT_TC_TYPE tc)
{
  int error;
  switch(tc)
  {
     case CPSV_CLEAN_ASYNC_ALL_MODES_SUCCESS_T:
     case CPSV_CLEAN_ASYNC_ALL_REPLICAS_CKPT:
          error = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS4_T,TEST_CONFIG_MODE);
          break;

     case CPSV_CLEAN_ALL_REPLICAS_CKPT:
          error = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS2_T,TEST_CONFIG_MODE);
          break;

     case CPSV_CLEAN_ACTIVE_REPLICAS_CKPT:
          error = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS3_T,TEST_CONFIG_MODE);
          break;

     case CPSV_CLEAN_WEAK_REPLICAS_CKPT:
          error = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS10_T,TEST_CONFIG_MODE);
          break;

     case CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT:
          error = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS5_T,TEST_CONFIG_MODE);
          break;

     case CPSV_CLEAN_ALL_COLLOCATED_REPLICAS_CKPT:
          error = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS6_T,TEST_CONFIG_MODE);
          break;

     case CPSV_CLEAN_WEAK_COLLOCATED_REPLICAS_CKPT:
          error = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS7_T,TEST_CONFIG_MODE);
          break;
     
     case CPSV_CLEAN_ASYNC_ACTIVE_REPLICAS_CKPT:
          error = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS8_T,TEST_CONFIG_MODE);
          break;
    
     case CPSV_CLEAN_MULTI_VECTOR_CKPT:
          error = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS9_T,TEST_CONFIG_MODE);
          break;

  }

  if(error != TET_PASS)
    m_TET_CPSV_PRINTF("\n Unlink failed ckpt not cleanedup\n");
}

void tet_red_cpsv_cleanup(CPSV_CLEANUP_TC_TYPE tc)
{
  int error;
                                                                                                                             
  switch(tc)
  {
     case CPSV_CLEAN_INIT_SUCCESS_T:
          error = tet_test_red_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;
                                                                                                                             
     case CPSV_CLEAN_INIT_NULL_CBKS_T:
          error = tet_test_red_ckptFinalize(CKPT_FIN_SUCCESS6_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;
                                                                                                                             
     case CPSV_CLEAN_INIT_OPEN_NULL_CBK_T:
          error = tet_test_red_ckptFinalize(CKPT_FIN_SUCCESS3_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;
                                                                                                                             
     case CPSV_CLEAN_INIT_SYNC_NULL_CBK_T:
          error = tet_test_red_ckptFinalize(CKPT_FIN_SUCCESS4_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;
                                                                                                                             
     case CPSV_CLEAN_INIT_NULL_CBKS2_T:
          error = tet_test_red_ckptFinalize(CKPT_FIN_SUCCESS5_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;
                                                                                                                             
     case CPSV_CLEAN_INIT_SUCCESS_HDL2_T:
          error = tet_test_red_ckptFinalize(CKPT_FIN_SUCCESS2_T,TEST_CONFIG_MODE);
          cpsv_clean_clbk_params();
          break;
  }
                                                                                                                             
  if(error != TET_PASS)
    m_TET_CPSV_PRINTF("\n Handle not cleanedup\n");
}
                                                                                                                             
void tet_red_ckpt_cleanup(CPSV_CLEANUP_CKPT_TC_TYPE tc)
{
  int error;
  switch(tc)
  {
     case CPSV_CLEAN_ASYNC_ALL_MODES_SUCCESS_T:
     case CPSV_CLEAN_ASYNC_ALL_REPLICAS_CKPT:
          error = tet_test_red_ckptUnlink(CKPT_UNLINK_SUCCESS4_T,TEST_CONFIG_MODE);
          break;
                                                                                                                             
     case CPSV_CLEAN_ALL_REPLICAS_CKPT:
          error = tet_test_red_ckptUnlink(CKPT_UNLINK_SUCCESS2_T,TEST_CONFIG_MODE);
          break;
                                                                                                                             
     case CPSV_CLEAN_ACTIVE_REPLICAS_CKPT:
          error = tet_test_red_ckptUnlink(CKPT_UNLINK_SUCCESS3_T,TEST_CONFIG_MODE);
          break;
                                                                                                                             
     case CPSV_CLEAN_WEAK_REPLICAS_CKPT:
          error = tet_test_red_ckptUnlink(CKPT_UNLINK_SUCCESS10_T,TEST_CONFIG_MODE);
          break;
                                                                                                                             
     case CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT:
          error = tet_test_red_ckptUnlink(CKPT_UNLINK_SUCCESS5_T,TEST_CONFIG_MODE);
          break;
                                                                                                                             
     case CPSV_CLEAN_ALL_COLLOCATED_REPLICAS_CKPT:
          error = tet_test_red_ckptUnlink(CKPT_UNLINK_SUCCESS6_T,TEST_CONFIG_MODE);
          break;
                                                                                                                             
     case CPSV_CLEAN_WEAK_COLLOCATED_REPLICAS_CKPT:
          error = tet_test_red_ckptUnlink(CKPT_UNLINK_SUCCESS7_T,TEST_CONFIG_MODE);
          break;
                                                                                                                             
     case CPSV_CLEAN_ASYNC_ACTIVE_REPLICAS_CKPT:
          error = tet_test_red_ckptUnlink(CKPT_UNLINK_SUCCESS8_T,TEST_CONFIG_MODE);
          break;
                                                                                                                             
     case CPSV_CLEAN_MULTI_VECTOR_CKPT:
          error = tet_test_red_ckptUnlink(CKPT_UNLINK_SUCCESS9_T,TEST_CONFIG_MODE);
          break;
  }
                                                                                                                             
  if(error != TET_PASS)
    m_TET_CPSV_PRINTF("\n Unlink failed ckpt not cleanedup\n");
}
                                                                                                                             


/********* saCkptInitialize TestCases ********/


void cpsv_it_init_01()
{
  int result;
  printHead("To verify that saCkptInitialize initializes checkpoint service");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
  printResult(result);
}

void cpsv_it_init_02()
{
  int result;
  printHead("To verify initialization when NULL pointer is fed as clbks");

  result = tet_test_ckptInitialize(CKPT_INIT_NULL_CBKS_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_NULL_CBKS_T);
  printResult(result);
}

void cpsv_it_init_03()
{
  int result;
  printHead("To verify initialization with NULL version pointer");

  result = tet_test_ckptInitialize(CKPT_INIT_NULL_VERSION_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_init_04()
{
  int result;
  printHead("To verify initialization with NULL as paramaters for clbks and version");

  result = tet_test_ckptInitialize(CKPT_INIT_VER_CBKS_NULL_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_init_05()
{
  int result;
  printHead("To verify initialization with error version relCode < supported release code");

  fill_ckpt_version(&tcd.version_err,'A',0x01,0x01);
  result = tet_test_ckptInitialize(CKPT_INIT_ERR_VERSION_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_init_06()
{
  int result;
  printHead("To verify initialization with error version relCode > supported");

  fill_ckpt_version(&tcd.version_err2,'C',0x01,0x01);
  result = tet_test_ckptInitialize(CKPT_INIT_ERR_VERSION2_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_init_07()
{
  int result;
  printHead("To verify initialization with error version majorVersion > supported");
  fill_ckpt_version(&tcd.version_err2,'B',0x03,0x01);

  result = tet_test_ckptInitialize(CKPT_INIT_ERR_VERSION2_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_init_08()
{
  int result;
  printHead("To verify init whether correct version is returned when wrong version is passed");
  fill_ckpt_version(&tcd.version_err2,'B',0x03,0x01);

  result = tet_test_ckptInitialize(CKPT_INIT_ERR_VERSION2_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS)
  {
     if(tcd.version_err2.releaseCode == 'B' &&
        tcd.version_err2.majorVersion == 2 &&
        tcd.version_err2.minorVersion == 2)
        result = TET_PASS;
      else
        result = TET_FAIL;
  }
  printResult(result);
}

void cpsv_it_init_09()
{
  int result;
  printHead("To verify saCkptInitialize with NULL handle");

  result = tet_test_ckptInitialize(CKPT_INIT_NULL_HDL_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_init_10()
{
  int result;
  printHead("To verify saCkptInitialize with one NULL clbk");

  result = tet_test_ckptInitialize(CKPT_INIT_OPEN_NULL_CBK_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_OPEN_NULL_CBK_T);
  printResult(result);
}

/****** saCkptSelectionObjectGet *****/


void cpsv_it_sel_01()
{
  int result;
  printHead("To verify saCkptSelectionObjectGet returns operating system handle");

  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
      goto final;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_sel_02()
{
  int result;
  printHead("To verify SelObj api with uninitialized handle");

  result = tet_test_ckptSelectionObject(CKPT_SEL_UNINIT_HDL_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_sel_03()
{
  int result;
  printHead("To verify SelObj api with finalized handle");

  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptSelectionObject(CKPT_SEL_FIN_HANDLE_T,TEST_NONCONFIG_MODE);

final:
  printResult(result);
}

void cpsv_it_sel_04()
{
  int result;
  printHead("To verify SelObj api with NULL handle");

  result = tet_test_ckptSelectionObject(CKPT_SEL_HDL_NULL_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_sel_05()
{
  int result;
  printHead("To verify SelObj api with NULL selObj");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptSelectionObject(CKPT_SEL_NULL_OBJ_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_sel_06()
{
  int result;
  printHead("To verify SelObj api with both NULL handle and selObj");

  result = tet_test_ckptSelectionObject(CKPT_SEL_HDL_OBJ_NULL_T,TEST_NONCONFIG_MODE);
  printResult(result);
}


/******* saCkptDispatch ********/

void cpsv_it_disp_01()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify that callback is invoked when dispatch is called with DISPATCH_ONE");

  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ALL_MODES_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1018 && tcd.open_clbk_err == SA_AIS_OK)
      result = TET_PASS;
   else
      result = TET_FAIL;

   tet_ckpt_cleanup(CPSV_CLEAN_ASYNC_ALL_MODES_SUCCESS_T);
      
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_disp_02()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify that callback is invoked when dispatch is called with DISPATCH_ALL");

  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
#if 0
  result = tet_test_ckptOpen(CKPT_OPEN_ALL_MODES_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
#endif

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ALL_MODES_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 90;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, NULL);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ALL_T,TEST_CONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1018 && tcd.open_clbk_err == SA_AIS_OK)
      result = TET_PASS;
  else
  {
      result = TET_FAIL;
      goto final2;
  }

  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ALL_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.sync_clbk_err == SA_AIS_OK && tcd.sync_clbk_invo == 2117)
      result = TET_PASS;
   else
      result = TET_FAIL;

final3:
     tet_ckpt_cleanup(CPSV_CLEAN_ASYNC_ALL_MODES_SUCCESS_T);
final2:
     tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_disp_03()
{
  int result;
  printHead("To verify that dispatch blocks for callback when dispatchFlag is DISPATCH_BLOCKING");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  cpsv_createthread(&tcd.ckptHandle);

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ALL_MODES_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  sleep(5);
  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  sleep(5);
  if(tcd.open_clbk_invo == 1018 && tcd.sync_clbk_invo == 2117 && tcd.open_clbk_err == SA_AIS_OK
                                && tcd.sync_clbk_err == SA_AIS_OK)
      result = TET_PASS;
   else
      result = TET_FAIL;

final3:
     tet_ckpt_cleanup(CPSV_CLEAN_ASYNC_ALL_MODES_SUCCESS_T);
final2:
     tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_disp_04()
{
  int result;
  printHead("To verify dispatch with invalid dispatchFlag");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptDispatch(CKPT_DISPATCH_INVALID_PARAM_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_disp_05()
{
  int result;
  printHead("To verify dispatch with NULL ckptHandle and DISPATCH_ONE");
  result = tet_test_ckptDispatch(CKPT_DISPATCH_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_disp_06()
{
  int result;
  printHead("To verify dispatch with NULL ckptHandle and DISPATCH_ALL");
  result = tet_test_ckptDispatch(CKPT_DISPATCH_BAD_HANDLE2_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_disp_07()
{
  int result;
  printHead("To verify dispatch with NULL ckptHandle and DISPATCH_BLOCKING");
  result = tet_test_ckptDispatch(CKPT_DISPATCH_BAD_HANDLE3_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_disp_08()
{
  int result;
  printHead("To verify dispatch with invalid ckptHandle and DISPATCH_ONE");
  result = tet_test_ckptDispatch(CKPT_DISPATCH_BAD_HANDLE4_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_disp_09()
{
  int result;
  printHead("To verify dispatch after finalizing ckpt service");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptDispatch(CKPT_DISPATCH_BAD_HANDLE5_T,TEST_NONCONFIG_MODE);

final:
  printResult(result);
}


/******* saCkptFinalize ********/


void cpsv_it_fin_01()
{
  int result;
  printHead("To verify that finalize closes association between service and process");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_BAD_HANDLE2_T,TEST_NONCONFIG_MODE);

final:
  printResult(result);
}

void cpsv_it_fin_02()
{
  int result;
  printHead("To verify finalize when service is not initialized");
  result = tet_test_ckptFinalize(CKPT_FIN_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_fin_03()
{
  int result;
  fd_set read_fd;
  struct timeval tv;

  printHead("To verify that after finalize selobj gets invalid");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 5;
  tv.tv_usec = 0;
                                                                                                                                                                      
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  result = select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
  if(result == -1)
     result = TET_PASS;
  else
     result = TET_FAIL;
  goto final1;

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_fin_04()
{
  int result;
  printHead("To verify that after finalize ckpts are closed");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptClose(CKPT_CLOSE_BAD_HANDLE2_T,TEST_NONCONFIG_MODE);

final:
  printResult(result);
}


/********* saCkptCheckpointOpen ************/


void cpsv_it_open_01()
{
  int result;
  printHead("To verify opening a ckpt with synchronous update option");

  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
 
final:
  printResult(result);
}

void cpsv_it_open_02()
{
  int result;
  printHead("To verify opening a ckpt with asynchronous update option");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_03()
{
  int result;
  printHead("To verify opening an existing ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}

void cpsv_it_open_04()
{
  int result;
  printHead("To verify opening an existing ckpt when its creation attributes match");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS2_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}

void cpsv_it_open_05()
{
  printHead("To verify opening an nonexisting ckpt");
  int result;
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_BAD_NAME_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_open_06()
{
  int result;
  printHead("To verify opening an ckpt when SA_CKPT_CHECKPOINT_CREATE flag not set and CR_ATTR is NOT NULL");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_INVALID_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_open_07()
{
  int result;
  printHead("To verify opening an ckpt when SA_CKPT_CHECKPOINT_CREATE flag set and CR_ATTR is NULL");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_NULL_ATTR_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_open_08()
{
  int result;
  printHead("To verify opening an ckpt with ckptSize > maxSec * maxSectionSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_ATTR_INVALID_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}


void cpsv_it_open_10()
{
  int result;
  printHead("To verify opening an ckpt when ALL_REPLICAS|ACTIVE_REPLICA specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_ATTR_INVALID2_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_open_11()
{
  int result;
  printHead("To verify opening an ckpt when ALL_REPLICAS|ACTIVE_REPLICA_WEAK specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_ATTR_INVALID3_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_open_12()
{
  int result;
  printHead("To verify opening an ckpt when ACTIVE_REPLICA|ACTIVE_REPLICA_WEAK specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_ATTR_INVALID4_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_open_13()
{
  int result;
  printHead("To verify opening an ckpt when NULL name is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_INVALID_PARAM_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_open_14()
{
  int result;
  printHead("To verify opening an ckpt for reading/writing which is not created");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_INVALID_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_open_15()
{
  int result;
  printHead("To verify opening an ckpt when NULL is passed as checkpointHandle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_INVALID_PARAM2_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_open_16()
{
  int result;
  printHead("To verify opening an ckpt when ALL_REPLICAS|COLLOCATED is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_COLLOC_CREATE_SUCCESS_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_COLLOCATED_REPLICAS_CKPT);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_17()
{
  int result;
  printHead("To verify opening an ckpt when REPLICA_WEAK|COLLOCATED is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_COLLOC_CREATE_SUCCESS_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_COLLOCATED_REPLICAS_CKPT);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_18()
{
  int result;
  printHead("To verify opening an ckpt when ACTIVE_REPLICA|COLLOCATED is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_19()
{
  int result;
  printHead("To verify opening an ckpt when invalid openFlags is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_BAD_FLAGS_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final:
  printResult(result);
}

void cpsv_it_open_20()
{
  int result;
  printHead("To verify creating a ckpt when already exists but with different creation attributes");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ERR_EXIST_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}

void cpsv_it_open_21()
{
  int result;
  printHead("To verify creating a ckpt when already exists but with different ckpt size");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1100,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ERR_EXIST2_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}

void cpsv_it_open_22()
{
  int result;
  printHead("To verify creating a ckpt when already exists but with different retention duration");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,1000000,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_SUCCESS_EXIST2_T,TEST_CONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}

void cpsv_it_open_23()
{
  int result;
  printHead("To verify creating a ckpt when already exists but with different maxSections");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,3,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ERR_EXIST2_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}

void cpsv_it_open_24()
{
  int result;
  printHead("To verify creating a ckpt when already exists but with different maxSectionSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,800,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ERR_EXIST2_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}

void cpsv_it_open_25()
{
  int result;
  printHead("To verify creating a ckpt when already exists but with different maxSectionIdSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,5);
  result = tet_test_ckptOpen(CKPT_OPEN_ERR_EXIST2_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}

void cpsv_it_open_26()
{
  int result;
  printHead("To verify creating a ckpt when ckpt service has not been initialized");
  result = tet_test_ckptOpen(CKPT_OPEN_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_open_27()
{
  int result;
  printHead("To verify creating a ckpt when NULL open clbk is provided during initialization");
  result = tet_test_ckptInitialize(CKPT_INIT_OPEN_NULL_CBK_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ERR_INIT_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_OPEN_NULL_CBK_T);
final:
  printResult(result);
}

void cpsv_it_open_28()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync with synchronous update option");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ALL_CREATE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1002 && tcd.open_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
  {
     result = TET_FAIL;
     goto final2;
  }

   tet_ckpt_cleanup(CPSV_CLEAN_ASYNC_ALL_REPLICAS_CKPT);
      
final2:
     tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_29()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync with asynchronous update option");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ACTIVE_CREATE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1003 && tcd.open_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
  {
     result = TET_FAIL;
     goto final2;
  }

  tet_ckpt_cleanup(CPSV_CLEAN_ASYNC_ACTIVE_REPLICAS_CKPT);
      
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_30()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync to open an existing ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_COLLOC_CREATE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_WEAK_COLLOC_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1016 && tcd.open_clbk_err == SA_AIS_OK)
      result = TET_PASS;
   else
      result = TET_FAIL;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_31()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync to open an existing ckpt if its creation attributes match");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_COLLOCATE_SUCCESS2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1005 && tcd.open_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
     result = TET_FAIL;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_32()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync to open an nonexisting ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_BAD_NAME_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1006 && tcd.open_clbk_err == SA_AIS_ERR_NOT_EXIST)
     result = TET_PASS;
  else
     result = TET_FAIL;

final2:
   tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_33()
{
  int result;
  printHead("To verify openAsync with SA_CKPT_CHECKPOINT_CREATE flag not set and CR_ATTR NOT NULL");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_INVALID_PARAM_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_34()
{
  int result;
  printHead("To verify openAsync with SA_CKPT_CHECKPOINT_CREATE flag set and NULL CR_ATTR");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_INVALID_PARAM2_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_35()
{
  int result;
  printHead("To verify openAsync with ckptSize > maxSec * maxSectionSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  fill_ckpt_attri(&tcd.invalid,SA_CKPT_WR_ACTIVE_REPLICA,300,100,3,85,3);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_INVALID_PARAM3_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_36()
{
  int result;
  printHead("To verify openAsync when ALL_REPLICAS|ACTIVE_REPLICA specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  fill_ckpt_attri(&tcd.invalid2,SA_CKPT_WR_ALL_REPLICAS|SA_CKPT_WR_ACTIVE_REPLICA,255,100,3,85,3);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_INVALID_PARAM4_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_37()
{
  int result;
  printHead("To verify openAsync when ALL_REPLICAS|ACTIVE_REPLICA_WEAK specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  fill_ckpt_attri(&tcd.invalid3,SA_CKPT_WR_ALL_REPLICAS|SA_CKPT_WR_ACTIVE_REPLICA_WEAK,255,100,3,85,3);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_INVALID_PARAM5_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_38()
{
  int result;
  printHead("To verify openAsync when ACTIVE_REPLICA|ACTIVE_REPLICA_WEAK specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  fill_ckpt_attri(&tcd.invalid4,SA_CKPT_WR_ACTIVE_REPLICA|SA_CKPT_WR_ACTIVE_REPLICA_WEAK,255,100,3,85,3);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_INVALID_PARAM6_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_39()
{
  int result;
  printHead("To verify openAsync when NULL name is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_INVALID_PARAM7_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_40()
{
  int result;
  printHead("To verify openAsync for reading/writing which is not created");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_INVALID_PARAM2_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_41()
{
  int result;
  printHead("To verify openAsync when NULL invocation is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_NULL_INVOCATION,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_42()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync when ALL_REPLICAS|COLLOCATED is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_COLLOC_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ALL_COLLOC_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1014 && tcd.open_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
     result = TET_FAIL;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_43()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync when REPLICA_WEAK|COLLOCATED is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_COLLOC_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_WEAK_COLLOC_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1016 && tcd.open_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
     result = TET_FAIL;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_44()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync when ACTIVE_REPLICA|COLLOCATED is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_COLLOCATE_SUCCESS2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1005 && tcd.open_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
  {
     result = TET_FAIL;
     goto final2;
  }

  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_45()
{
  int result;
  printHead("To verify openAsync when invalid openFlags is specified");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_BAD_FLAGS_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}

void cpsv_it_open_46()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync when already exists but with different creation attributes");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ERR_EXIST_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1019 && tcd.open_clbk_err == SA_AIS_ERR_EXIST)
     result = TET_PASS;
  else
  {
     result = TET_FAIL;
     goto final2;
  }

  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_47()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync when already exists but with different checkpointSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1100,100,2,600,3);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ERR_EXIST_T,TEST_NONCONFIG_MODE);

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1019 && tcd.open_clbk_err == SA_AIS_ERR_EXIST)
     result = TET_PASS;
  else
     result = TET_FAIL;

  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_48()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync when already exists but with different retentionDuration");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,1000000,2,600,3);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ERR_EXIST_T,TEST_NONCONFIG_MODE);

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1019 && tcd.open_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
     result = TET_FAIL;

  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_49()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync when already exists but with different maxSections");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,3,600,3);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ERR_EXIST_T,TEST_NONCONFIG_MODE);

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1019 && tcd.open_clbk_err == SA_AIS_ERR_EXIST)
     result = TET_PASS;
  else
     result = TET_FAIL;

  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_50()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync when already exists but with different maxSectionSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,800,3);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ERR_EXIST_T,TEST_NONCONFIG_MODE);

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1019 && tcd.open_clbk_err == SA_AIS_ERR_EXIST)
     result = TET_PASS;
  else
     result = TET_FAIL;

  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_51()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openAsync when already exists but with different maxSectionIdSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,5);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ERR_EXIST_T,TEST_NONCONFIG_MODE);

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1019 && tcd.open_clbk_err == SA_AIS_ERR_EXIST)
     result = TET_PASS;
  else
     result = TET_FAIL;

  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_open_52()
{
  int result;
  printHead("To verify creating a ckpt with invalid creation flags");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_CHECKPOINT_COLLOCATED,1069,1000000,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_INVALID_CR_FLAGS_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}

void cpsv_it_open_53()
{
  int result;
  printHead("To verify creating a ckpt with invalid creation flags");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.invalid_collocated,0,1069,1000000,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_INVALID_CR_FLAGS_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}

void cpsv_it_open_54()
{
  int result;
  printHead("To verify creating a ckpt with invalid creation flags");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.invalid_collocated,16,1069,1000000,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_INVALID_CR_FLAGS_T,TEST_NONCONFIG_MODE);

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final1:
  printResult(result);
}


/****** saCkptCheckpointClose *******/

void cpsv_it_close_01() 
{
  int result;
  printHead("To verify Closing of the checkpoint designated by checkpointHandle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptUnlink(CKPT_UNLINK_NOT_EXIST2_T,TEST_NONCONFIG_MODE);
  goto final2;
 
final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_close_02() 
{
  int result;
  printHead("To verify that after close checkpointHandle is no longer valid");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptClose(CKPT_CLOSE_BAD_HANDLE2_T,TEST_NONCONFIG_MODE);
  goto final2;
 
final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_close_03()
{
  int result;
  printHead("To test Closing of the checkpoint after unlink");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
 
  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptUnlink(CKPT_UNLINK_NOT_EXIST2_T,TEST_NONCONFIG_MODE);
 
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_close_04() 
{
  int result;
  printHead("To test Closing of the checkpoint before unlink");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.all_replicas,SA_CKPT_WR_ALL_REPLICAS,169,1000000000000000ULL,2,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS3_T,TEST_CONFIG_MODE);

  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
 
  result = tet_test_ckptUnlink(CKPT_UNLINK_NOT_EXIST2_T,TEST_NONCONFIG_MODE);
 
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_close_05() 
{
  int result;
  printHead("To test close api when calling process is not registered with checkpoint service");
  result = tet_test_ckptClose(CKPT_CLOSE_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_close_06() 
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify that close cancels all pending callbacks");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                            
  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                            
  cpsv_clean_clbk_params();
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ALL_MODES_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                            
  tv.tv_sec = 30;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  result = select(tcd.selobj + 1, &read_fd, NULL, NULL, NULL);
  printf("\n Select returned %d \n",result);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ALL_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1018 && tcd.open_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
  {
     result = TET_FAIL;
     goto final2;
  }
                                                                                                                            
  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
                                                                                                                            
  tv.tv_sec = 30;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  result = select(tcd.selobj + 1, &read_fd, NULL, NULL, NULL);
  printf("\n Select ckptSynchronizeAsync returned  %d \n",result);

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS7_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
                                                                                                                            
  tv.tv_sec = 120;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  result = select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     printf("\n Select ckptClose returned  %d \n",result);
     result = tet_test_ckptDispatch(CKPT_DISPATCH_ALL_T,TEST_NONCONFIG_MODE);
     if(tcd.sync_clbk_invo == 2117 &&  tcd.sync_clbk_err == SA_AIS_ERR_BAD_HANDLE)
        result = TET_PASS;
     else
        result = TET_FAIL;
  }
  goto final2;
                                                                                                                            
final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ASYNC_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_close_07()
{
  int result;
  printHead("To test that the checkpoint can be opened after close and before checkpoint expiry");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,SA_TIME_ONE_HOUR,2,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS9_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  sleep(60);
  result = tet_test_red_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_red_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,100,2,85,3);
  printResult(result);
}

void cpsv_it_close_08()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  SaCkptCheckpointHandleT hdl1,hdl2;
  printHead("To verify that close cancels pending callbacks for that handle only");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  cpsv_clean_clbk_params();
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ALL_MODES_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 30;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  result = select(tcd.selobj + 1, &read_fd, NULL, NULL, NULL);
  printf("\n Select returned %d \n",result);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ALL_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1018 && tcd.open_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
  {
     result = TET_FAIL;
     goto final2;
  }

  hdl1 = tcd.async_all_replicas_hdl;

  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ALL_MODES_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 30;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  result = select(tcd.selobj + 1, &read_fd, NULL, NULL, NULL);
  printf("\n Select returned %d \n",result);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ALL_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1018 && tcd.open_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
  {
     result = TET_FAIL;
     goto final2;
  }

  hdl2 = tcd.async_all_replicas_hdl;

  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tv.tv_sec = 30;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  result = select(tcd.selobj + 1, &read_fd, NULL, NULL, NULL);
  printf("\n First Sync returned %d \n",result);

  tcd.async_all_replicas_hdl = hdl1;
  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  sleep(5);
  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS7_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tv.tv_sec = 120;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  result = select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
  if(result == 0)
     result = TET_PASS;
  else
  {
     printf("\n Select returned %d \n",result);
     result = tet_test_ckptDispatch(CKPT_DISPATCH_ALL_T,TEST_NONCONFIG_MODE);
     if(tcd.sync_clbk_invo == 2117)
        result = TET_PASS;
     else
        result = TET_FAIL;
  }
  goto final2;
                                                                                                                             
final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ASYNC_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}
                                                                                                                             


/****** saCkptCheckpointUnlink *******/


void cpsv_it_unlink_01() 
{
  int result;
  printHead("To test Unlink deletes the existing checkpoint from the cluster");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
 
  result = tet_test_ckptOpen(CKPT_OPEN_ERR_NOT_EXIST_T,TEST_NONCONFIG_MODE);
 
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_unlink_02() 
{
  int result;
  printHead("To test that name is no longer valid after unlink");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
 
  result = tet_test_ckptUnlink(CKPT_UNLINK_NOT_EXIST2_T,TEST_NONCONFIG_MODE);
 
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_unlink_03() 
{
  int result;
  printHead("To test that the ckpt with same name can be created after unlinking and yet not closed");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
 
  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS2_T,TEST_CONFIG_MODE);
 
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_unlink_04() 
{
  int result;
  printHead("To test that ckpt gets deleted immediately when no process has opened it");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.all_replicas,SA_CKPT_WR_ALL_REPLICAS,169,1000000000000000ULL,2,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS3_T,TEST_CONFIG_MODE);

  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
 
  result = tet_test_ckptUnlink(CKPT_UNLINK_NOT_EXIST2_T,TEST_NONCONFIG_MODE);
 
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_unlink_05() 
{
  int result;
  printHead("To test that the ckpt can be accessed after unlinking till it is closed");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_MULTI_IO_REPLICA_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS9_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
 
  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_MULTI_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
 
  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS8_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
 
  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_MULTI4_T,TEST_NONCONFIG_MODE);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_unlink_06()  
{
  int result;
  printHead("To test unlink after retention duration of ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,100,2,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS9_T,TEST_CONFIG_MODE);

  result = tet_test_ckptUnlink(CKPT_UNLINK_NOT_EXIST3_T,TEST_NONCONFIG_MODE);
 
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_unlink_07() /* To test unlink after close */
{
  cpsv_it_close_04(); /* To test Closing of the checkpoint before unlink*/
}

void cpsv_it_unlink_08() 
{
  int result;
  printHead("To test unlink with correct handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,100,2,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS3_T,TEST_NONCONFIG_MODE);
 
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_unlink_09() 
{
  int result;
  printHead("To test unlink with uninitialized handle");
  result = tet_test_ckptUnlink(CKPT_UNLINK_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_unlink_10() /* invoke unlink in child process */
{
  printHead("To test unlink in the child process - not supported");
/* fork not supported */
}

void cpsv_it_unlink_11() 
{
  int result;
  printHead("To test unlink with NULL ckpt name");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;

  result = tet_test_ckptUnlink(CKPT_UNLINK_INVALID_PARAM_T,TEST_NONCONFIG_MODE);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

final:
  printResult(result);
}


/******* saCkptRetentionDurationSet ******/


void cpsv_it_rdset_01() 
{
  int result;
  printHead("To test that invoking rdset changes the rd for the checkpoint");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,SA_TIME_ONE_HOUR,2,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptDurationSet(CKPT_SECTION_DUR_SET_ACTIVE_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS9_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  sleep(1);
  result = tet_test_ckptUnlink(CKPT_UNLINK_NOT_EXIST3_T,TEST_NONCONFIG_MODE);
  goto final2;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,100,2,85,3);
  printResult(result);
}

void cpsv_it_rdset_02()/* To test ckpt gets deleted if no process opened during ret. duration*/
{
  cpsv_it_rdset_01();
}

void cpsv_it_rdset_03() 
{
  int result;
  printHead("To test rdset api with invalid handle");
  result = tet_test_ckptDurationSet(CKPT_SECTION_DUR_SET_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_rdset_04() 
{
  int result;
  printHead("To test when rdset is invoked after unlink");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,SA_TIME_ONE_HOUR,2,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptDurationSet(CKPT_SECTION_DUR_SET_BAD_OPERATION_T,TEST_NONCONFIG_MODE);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,100,2,85,3);
  printResult(result);
}

void cpsv_it_rdset_05() 
{
  int result;
  printHead("To test when ret.duration to be set is zero");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,SA_TIME_ONE_HOUR,2,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptDurationSet(CKPT_SECTION_DUR_SET_ZERO_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS9_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptUnlink(CKPT_UNLINK_NOT_EXIST3_T,TEST_NONCONFIG_MODE);
  goto final2;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,100,2,85,3);
  printResult(result);
}

void cpsv_it_rdset_06() /* child process invokes rdset api */
{
  printHead("child process invokes rdset api");
}


/******* saCkptActiveReplicaSet ********/


void cpsv_it_repset_01()
{
  int result;
  printHead("To verify that local replica becomes active after invoking replicaset");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptInitialize(CKPT_INIT_SYNC_NULL_CBK_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_NOT_EXIST_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS7_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SYNC_NULL_CBK_T);
final1:
  printResult(result);
}


void cpsv_it_repset_02() 
{
  int result;
  printHead("To test that replica can be set for only for collocated ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,SA_TIME_ONE_HOUR,2,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_BAD_OPERATION2_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  fill_ckpt_attri(&tcd.active_replica,SA_CKPT_WR_ACTIVE_REPLICA,169,100,2,85,3);
  printResult(result);
}

void cpsv_it_repset_03()  
{
  int result;
  printHead("To test replica can be set active only for asynchronous ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_BAD_OPERATION_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_repset_04() 
{
  int result;
  printHead("To test replica set when handle with read option is passed to the api");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_ERR_ACCESS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_repset_05()
{
  int result;
  printHead("To verify replicaset when an invalid handle is passed");
  result = tet_test_ckptReplicaSet(CKPT_SET_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}



/******* saCkptCheckpointStatusGet *******/


void cpsv_it_status_01() 
{
  int result;
  printHead("To test that this api can be used to retrieve the checkpoint status");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.replica_weak,SA_CKPT_WR_ACTIVE_REPLICA_WEAK,85,100,1,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptStatusGet(CKPT_STATUS_SUCCESS2_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.status.numberOfSections == 1 &&
                           tcd.status.memoryUsed == 0 &&
                           tcd.status.checkpointCreationAttributes.creationFlags == SA_CKPT_WR_ACTIVE_REPLICA_WEAK &&
                           tcd.status.checkpointCreationAttributes.checkpointSize == 85 &&
                           tcd.status.checkpointCreationAttributes.retentionDuration == 100 &&
                           tcd.status.checkpointCreationAttributes.maxSections == 1 &&
                           tcd.status.checkpointCreationAttributes.maxSectionSize == 85) 
     result = TET_PASS;
  else
     result = TET_FAIL;

  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


void cpsv_it_status_02() 
{
  int result;
  printHead("To test when arbitrary handle is passed to status get api");
  result = tet_test_ckptStatusGet(CKPT_STATUS_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_status_03() 
{
  int result;
  printHead("To test status get when there is no active replica");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptStatusGet(CKPT_STATUS_ERR_NOT_EXIST_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


void cpsv_it_status_05() 
{
    cpsv_it_status_01();
}

void cpsv_it_status_06() 
{
  int result;
  printHead("To test status get when invalid param is passed");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptStatusGet(CKPT_STATUS_INVALID_PARAM_T,TEST_NONCONFIG_MODE);
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


/******* saCkptSectionCreate ********/


void cpsv_it_seccreate_01() 
{
  int result;
  printHead("To verify section create with arbitrary handle");
  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_seccreate_02() 
{
  int result;
  printHead("To verify section create with correct handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_seccreate_03() 
{
  int result;
  printHead("To verify creating sections greater than maxSections");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_NO_SPACE_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


void cpsv_it_seccreate_04() 
{
   cpsv_it_expset_02();
}

void cpsv_it_seccreate_05() 
{
    cpsv_it_seccreate_02(); /* can be extended for two node case open the same checkpoint, delete the first replica, u will still have the section in the ckpt*/ 
}


void cpsv_it_seccreate_07() 
{
  int result;
  printHead("To verify section create when there is no active replica");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptInitialize(CKPT_INIT_SYNC_NULL_CBK_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_NOT_EXIST_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SYNC_NULL_CBK_T);
final1:
  printResult(result);
}

void cpsv_it_seccreate_09() 
{
  int result;
  printHead("To verify section create when ckpt is not opened in write mode");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_ERR_ACCESS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_seccreate_10() 
{
  int result;
  printHead("To verify section create when maxSections is 1");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS2_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_seccreate_11() 
{
  int result;
  printHead("To verify section create with NULL section creation attributes");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_INVALID_PARAM_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_seccreate_12() 
{
  int result;
  printHead("To verify section create with NULL initial data and datasize > 0");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_NULL_DATA_SIZE_NOTZERO_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_seccreate_13() 
{
  int result;
  printHead("To verify section create with NULL initial data and datasize = 0");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_NULL_DATA_SIZE_ZERO_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_seccreate_14()
{
  int result;
  printHead("To verify section create with sectionId which is already present");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_ERR_EXIST_T,TEST_CONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_seccreate_15()
{
  int result;
  printHead("To verify section create with generated sectionId");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptInitialize(CKPT_INIT_SYNC_NULL_CBK_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_GEN_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_GEN_EXIST_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptStatusGet(CKPT_STATUS_SUCCESS4_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.status.numberOfSections == 1 &&
                           (tcd.status.checkpointCreationAttributes.creationFlags == (SA_CKPT_WR_ACTIVE_REPLICA|SA_CKPT_CHECKPOINT_COLLOCATED)) &&
                           tcd.status.checkpointCreationAttributes.checkpointSize == 1069 &&
                           tcd.status.checkpointCreationAttributes.retentionDuration == 100 &&
                           tcd.status.checkpointCreationAttributes.maxSections == 2 &&
                           tcd.status.checkpointCreationAttributes.maxSectionSize == 600)
     result = TET_PASS;
   else
     result = TET_FAIL;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SYNC_NULL_CBK_T);
final1:
  printResult(result);
}

 void cpsv_it_seccreate_18()
{
  int result;
  printHead("To verify free of section create with generated sectionId");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
  result = tet_test_ckptInitialize(CKPT_INIT_SYNC_NULL_CBK_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_GEN_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
#if 0
    result = tet_test_ckptSectionDelete(CKPT_DEL_GEN_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
 #endif 
   result = tet_test_saCkptSectionIdFree(CKPT_FREE_GEN_T,TEST_CONFIG_MODE);
   if(result != TET_PASS)
     goto final3;
final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SYNC_NULL_CBK_T);
final1:
  printResult(result);
}
void cpsv_it_seccreate_16() 
{
  int result;
  printHead("To verify section create with section idLen greater than maxsec id size");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_INVALID_PARAM2_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}
 
void cpsv_it_seccreate_17() 
{
  int result;
  printHead("To verify section create with section idSize zero");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tcd.invalid_sec.idLen = 0;
  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_INVALID_PARAM3_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  tcd.invalid_sec.idLen = 8;
  printResult(result);
}
 

/******* saCkptSectionDelete ******/


void cpsv_it_secdel_01() 
{
  int result;
  printHead("To verify section delete with arbitrary handle");
  result = tet_test_ckptSectionDelete(CKPT_DEL_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_secdel_02() 
{
  int result;
  printHead("To verify section delete with correct handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionDelete(CKPT_DEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionDelete(CKPT_DEL_NOT_EXIST3_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_secdel_03() 
{
  int result;
  printHead("To verify section delete when ckpt is not opened in write mode");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionDelete(CKPT_DEL_ERR_ACCESS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_secdel_04() 
{
  int result;
  printHead("To verify section delete with an invalid section id");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionDelete(CKPT_DEL_NOT_EXIST_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_secdel_08() 
{
  int result;
  printHead("To verify that when a section is deleted it is deleted atleast in active replica");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS7_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionDelete(CKPT_DEL_SUCCESS3_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_NOT_EXIST2_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_secdel_09() 
{
  int result;
  printHead("To verify section delete when maxSections is 1");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionDelete(CKPT_DEL_SUCCESS2_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


/******* saCkptSectionExpirationTimeSet *******/


void cpsv_it_expset_01() 
{
  int result;
  printHead("To verify section expset with arbitrary handle");
  result = tet_test_ckptExpirationSet(CKPT_EXP_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_expset_02() 
{
  int result;
  printHead("To verify section expset with correct handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptExpirationSet(CKPT_EXP_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  sleep(2);

  result = tet_test_ckptSectionDelete(CKPT_DEL_NOT_EXIST4_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_expset_03() 
{
  int result;
  printHead("To verify that section exp time of a default section cannot be changed");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptExpirationSet(CKPT_EXP_INVALID_PARAM_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_expset_04() /*To test that except for SA_TIME_END the remaining sections gets deleted after expiration time */
{
  printHead("To verify that if section exp time is SA_TIME_END it is not deleted - TO DO");

}

void cpsv_it_expset_05() /*To test that section gets deleted automatically after exp time */
{
   cpsv_it_expset_02();
}

void cpsv_it_expset_06() 
{
  int result;
  printHead("To verify section exp set when ckpt is opened in read mode");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptExpirationSet(CKPT_EXP_ERR_ACCESS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_expset_07() 
{
  int result;
  printHead("To verify section exp set with invalid sectionid");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptExpirationSet(CKPT_EXP_ERR_NOT_EXIST_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


/********* saCkptSectionIterationInitialize *****/


void cpsv_it_iterinit_01() 
{
  int result;
  printHead("To verify that iter init returns handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_ANY_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_NO_SECTIONS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_iterinit_02() 
{
  int result;
  printHead("To verify iter init with bad handle");
  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_iterinit_03() 
{
  int result;
  printHead("To verify iter init with finalized handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                             
  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_FINALIZE_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_iterinit_04() 
{
  int result;
  printHead("To verify sections with SA_TIME_END are returned when sectionsChosen is FOREVER");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_FOREVER2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_NO_SECTIONS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_iterinit_05() 
{
  int result;
  printHead("To verify iter init when sectionsChosen is LEQ_EXP_TIME");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS10_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_LEQ_EXP_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_NO_SECTIONS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_iterinit_06() 
{
  int result;
  printHead("To verify iter init when sectionsChosen is GEQ_EXP_TIME");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS10_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_GEQ_EXP_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_NO_SECTIONS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_iterinit_07() /* To verify when sectionsChosen is CORRUPTED */
{
    printHead("No fixed procedure known for generating corrupted sections");
}

void cpsv_it_iterinit_08() 
{
    cpsv_it_iterinit_01();
}

void cpsv_it_iterinit_09() /* api selects sections as specified in sectionsChosen*/
{
    cpsv_it_iterinit_05();
}

void cpsv_it_iterinit_10()  
{
  int result;
  printHead("To verify iter init with NULL section iter handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_NULL_HANDLE_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_iterinit_12() 
{
  int result;
  printHead("To verify iter init with invalid sectionsChosen value");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_INVALID_PARAM_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


/********* saCkptSectionIterationNext *********/

void cpsv_it_iternext_01() /* to test that iter next goes through sections chosen by iter init */
{
    cpsv_it_iterinit_06();
}

void cpsv_it_iternext_02()   
{
  int result;
  printHead("To verify iter next with uninitialized handle");
  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_iternext_03() /* To test api with correct handle*/
{
    cpsv_it_iterinit_04();
}

void cpsv_it_iternext_04() /*To test that section descriptor contains details about the section */
{
    cpsv_it_iterinit_05();
}

void cpsv_it_iternext_05() /*To test that the api returns ERR_NO_SEC when there are no more sections that match the criteria*/
{
    cpsv_it_iterinit_05();
}

void cpsv_it_iternext_06() 
{
  int result;
  printHead("To verify that sec created before iter init and not deleted before iterfinalize are returned");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_ANY_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionDelete(CKPT_DEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_NO_SECTIONS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_iternext_08() 
{
  int result;
  printHead("To verify iter next with NULL sec descriptor");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_GEQ_EXP_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_INVALID_PARAM_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_iternext_09() 
{
  int result;
  printHead("To verify iter next after Finalize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_GEQ_EXP_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_FIN_HANDLE_T,TEST_NONCONFIG_MODE);
  goto final1;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


/****** saCkptSectionIterationFinalize *******/


void cpsv_it_iterfin_01() 
{
  int result;
  printHead("To verify iter finalize with correct handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_GEQ_EXP_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationFin(CKPT_ITER_FIN_HANDLE2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_FIN_HANDLE_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_iterfin_02() 
{
  int result;
  printHead("To verify iter finalize with arbitrary handle");
  result = tet_test_ckptIterationFin(CKPT_ITER_FIN_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_iterfin_04() 
{
  int result;
  printHead("To verify iter finalize when ckpt has been closed and unlinked");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_GEQ_EXP_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptUnlink(CKPT_UNLINK_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS9_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS11_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptIterationFin(CKPT_ITER_FIN_HANDLE_T,TEST_NONCONFIG_MODE);
  goto final2;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


/******* saCkptCheckpointWrite *******/


void cpsv_it_write_01() 
{
  int result;
  printHead("To verify that write api writes into the ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.general_write.dataBuffer,tcd.general_read.dataBuffer,tcd.general_read.readSize) != 0 )
     result = TET_FAIL;
  else 
     result = TET_PASS;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_02() 
{
    cpsv_it_write_01();
}

void cpsv_it_write_03() 
{
  int result;
  printHead("To verify that write api writes into the ACTIVE_REPLICA ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS8_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_SUCCESS7_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.general_write.dataBuffer,tcd.general_read.dataBuffer,tcd.general_read.readSize) != 0)
     result = TET_FAIL;
  else 
     result = TET_PASS;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_04() 
{
  int result;
  printHead("To verify that write api writes into the ACTIVE_REPLICA_WEAK ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_WEAK_REP_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.default_write.dataBuffer,tcd.default_read.dataBuffer,tcd.default_read.readSize) != 0 )
     result = TET_FAIL;
  else 
     result = TET_PASS;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_05() 
{
  int result;
  printHead("To verify that write api writes into the COLLOCATED ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS7_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS3_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_SUCCESS5_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.general_write.dataBuffer,tcd.general_read.dataBuffer,tcd.general_read.readSize) != 0 )
     result = TET_FAIL;
  else 
     result = TET_PASS;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_06() 
{
  int result;
  printHead("To verify write api with an arbitrary handle");
  result = tet_test_ckptWrite(CKPT_WRITE_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_write_07() 
{
  int result;
  printHead("To verify write api after finalize is called");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS7_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_FIN_HANDLE_T,TEST_NONCONFIG_MODE);
  goto final1;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_08() 
{
  int result;
  printHead("To verify write when there is no such section in the ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_NOT_EXIST_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_09() 
{
  int result;
  printHead("To verify write when the ckpt is not opened for writing");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_ERR_ACCESS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_10() 
{
  int result;
  printHead("To verify write when NULL iovector is passed");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS7_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_NULL_VECTOR_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_11() 
{
  int result;
  printHead("To verify write when the data to be written exceeds the maxSectionSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_INVALID_PARAM_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_12() 
{
  int result;
  printHead("To verify write with NULL err index");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS7_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_NULL_INDEX_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_14() 
{
  int result;
  printHead("To verify write when the dataOffset+dataSize exceeds maxSectionSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  fill_write_iovec(&tcd.err_no_res,tcd.section1,"Data in general east or west india is the best",strlen("Data in general east or west india is the best"),80,0);
  result = tet_test_ckptWrite(CKPT_WRITE_ERR_NO_RESOURCES_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_write_16() 
{
  int result;
  printHead("To verify write when there is more than one section to be written and read from ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_MULTI_IO_REPLICA_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_MULTI_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_MULTI2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_MULTI3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_MULTI_IO_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_MULTI_IO_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if(strncmp(tcd.multi_vector[0].dataBuffer,tcd.multi_read_vector[0].dataBuffer,tcd.multi_read_vector[0].readSize) != 0 &&
     strncmp(tcd.multi_vector[1].dataBuffer,tcd.multi_read_vector[1].dataBuffer,tcd.multi_read_vector[1].readSize) != 0 &&
     strncmp(tcd.multi_vector[2].dataBuffer,tcd.multi_read_vector[2].dataBuffer,tcd.multi_read_vector[2].readSize) != 0)
     result = TET_FAIL;
  else 
     result = TET_PASS;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_MULTI_VECTOR_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

/******* saCkptCheckpointRead *********/

void cpsv_it_read_01() /* to test that the api read from the checkpoint */
{
    cpsv_it_write_01();
}

void cpsv_it_read_02() 
{
  int result;
  printHead("To verify that when data is read the data is copied");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.general_write.dataBuffer,tcd.general_read.dataBuffer,tcd.general_read.readSize) != 0 )
  {
     result = TET_FAIL;
     goto final3;
  }
  else 
     result = TET_PASS;

  result = tet_test_ckptRead(CKPT_READ_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.general_write.dataBuffer,tcd.general_read.dataBuffer,tcd.general_read.readSize) != 0 )
     result = TET_FAIL;
  else 
     result = TET_PASS;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_read_03() 
{
  int result;
  printHead("To verify read with arbitrary handle");
  result = tet_test_ckptRead(CKPT_READ_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_read_04() 
{
  int result;
  printHead("To verify read with already finalized handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  cpsv_clean_clbk_params();
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptRead(CKPT_READ_FIN_HANDLE_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_read_05() 
{
  int result;
  printHead("To verify read with sectionid that doesnot exist");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_NOT_EXIST_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


void cpsv_it_read_06() 
{
  int result;
  printHead("To verify read when the ckpt is not opened for reading");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_ERR_ACCESS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


void cpsv_it_read_07() 
{
  int result;
  printHead("To verify read when the dataSize > maxSectionSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_INVALID_PARAM_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_read_08() 
{
  int result;
  printHead("To verify read when the dataOffset > maxSectionSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_INVALID_PARAM2_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_read_10() 
{
  int result;
  printHead("To verify read with NULL err index");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_NULL_INDEX_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_read_11() /*To test that readSize is returned */
{
   cpsv_it_read_02();
}

void cpsv_it_read_13() /*To test when there are multiple sections to read */
{
   cpsv_it_write_16();
}


/********* saCkptCheckpointSynchronize **************/

void cpsv_it_sync_01() 
{
  int result;
  printHead("To verify sync api");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronize(CKPT_SYNC_SUCCESS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_sync_02() 
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify sync async api");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.sync_clbk_invo == 2116 && tcd.sync_clbk_err == SA_AIS_OK)
     result = TET_PASS;
  else
     result = TET_FAIL;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_sync_03() 
{
  int result;
  printHead("To verify sync api when the ckpt is opened in read mode");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronize(CKPT_SYNC_ERR_ACCESS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_sync_04() 
{
  int result;
  printHead("To verify sync api when the ckpt is synchronous");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronize(CKPT_SYNC_BAD_OPERATION_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_sync_05() 
{
  int result;
  printHead("To verify sync async api when the ckpt is synchronous");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_BAD_OPERATION_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_sync_06() 
{
  int result;
  printHead("To verify sync async api when the ckpt is opened in read mode");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_ERR_ACCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


void cpsv_it_sync_07() 
{
  int result;
  printHead("To verify sync async api when process is not registered to have sync callback");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptInitialize(CKPT_INIT_SYNC_NULL_CBK_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS7_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_ERR_INIT_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SYNC_NULL_CBK_T);
final1:
  printResult(result);
}


void cpsv_it_sync_08() 
{
  int result;
  printHead("To verify sync api when there is no active replica");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_NOT_EXIST_T,TEST_NONCONFIG_MODE);
#if 0
  if(result != TET_PASS)
     goto final3;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.sync_clbk_invo == 2112 && tcd.sync_clbk_err == SA_AIS_ERR_NOT_EXIST)
     result = TET_PASS;
  else
     result = TET_FAIL;
#endif

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_sync_09() 
{
  int result;
  printHead("To verify sync api when there are no sections in the ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronize(CKPT_SYNC_SUCCESS2_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_sync_10() 
{
  int result;
  printHead("To verify sync api when service is not initialized");
  result = tet_test_ckptSynchronize(CKPT_SYNC_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_sync_11() 
{
  int result;
  printHead("To verify sync async api when service is not initialized");
  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_sync_12() 
{
  int result;
  printHead("To verify sync api when service is finalized");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  cpsv_clean_clbk_params();
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronize(CKPT_SYNC_FIN_HANDLE_T,TEST_NONCONFIG_MODE);
  goto final1;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_sync_13() 
{
  int result;
  printHead("To verify sync async api when service is finalized");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  cpsv_clean_clbk_params();
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_FIN_HANDLE_T,TEST_NONCONFIG_MODE);
  goto final1;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_sync_14() 
{
  int result;
  printHead("To verify sync api with NULL handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronize(CKPT_SYNC_NULL_HANDLE_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


/********** saCkptSectionOverwrite **************/


void cpsv_it_overwrite_01() 
{
  int result;
  printHead("To verify that overwrite writes into a section other than default section");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.general_write.dataBuffer,tcd.general_read.dataBuffer,tcd.general_read.readSize) != 0 )
  {
     result = TET_FAIL;
     goto final3;
  }

  result = tet_test_ckptOverwrite(CKPT_OVERWRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.overdata,tcd.general_read.dataBuffer,tcd.general_read.readSize) != 0 )
     result = TET_FAIL;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_overwrite_02() 
{
  int result;
  printHead("To verify that overwrite writes into a default section");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_WEAK_REP_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.default_write.dataBuffer,tcd.default_read.dataBuffer,tcd.default_read.readSize) != 0 )
  {
     result = TET_FAIL;
     goto final3;
  }

  result = tet_test_ckptOverwrite(CKPT_OVERWRITE_SUCCESS2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_WEAK_REP_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.overdata,tcd.default_read.dataBuffer,tcd.default_read.readSize) != 0 )
     result = TET_FAIL;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


void cpsv_it_overwrite_03() 
{
  int result;  
  printHead("To verify overwrite with dataSize > maxSectionSize");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOverwrite(CKPT_OVERWRITE_INVALID_PARAM_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_overwrite_04() /*When ALL REPLICA  */
{
    cpsv_it_overwrite_01();
}

void cpsv_it_overwrite_05() 
{
  int result;
  printHead("To verify overwrite in ACTIVE_REPLICA ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOverwrite(CKPT_OVERWRITE_SUCCESS6_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_SUCCESS7_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.overdata,tcd.general_read.dataBuffer,tcd.general_read.readSize) != 0 )
     result = TET_FAIL;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_overwrite_06() /*When REPLICA WEAK */
{
    cpsv_it_overwrite_02();
}

void cpsv_it_overwrite_07() 
{
  int result;
  printHead("To verify overwrite with an arbitrary handle");
  result = tet_test_ckptOverwrite(CKPT_OVERWRITE_BAD_HANDLE_T,TEST_NONCONFIG_MODE);
  printResult(result);
}

void cpsv_it_overwrite_08() 
{
  int result;
  printHead("To verify overwrite with already finalized handle");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
  result = tet_test_ckptFinalize(CKPT_FIN_SUCCESS_T,TEST_CONFIG_MODE);
  cpsv_clean_clbk_params();

  result = tet_test_ckptOverwrite(CKPT_OVERWRITE_FIN_HANDLE_T,TEST_NONCONFIG_MODE);
  goto final1;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_overwrite_09() 
{
  int result;
  printHead("To verify overwrite when section doesnot exist");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOverwrite(CKPT_OVERWRITE_NOT_EXIST_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


void cpsv_it_overwrite_10() 
{
  int result;
  printHead("To verify overwrite when ckpt is not opened for writing");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ACTIVE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS4_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOverwrite(CKPT_OVERWRITE_ERR_ACCESS_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ACTIVE_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_overwrite_11() 
{
  int result;
  printHead("To verify overwrite when NULL dataBuffer is provided");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOverwrite(CKPT_OVERWRITE_NULL_BUF_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_overwrite_12() 
{
  int result;
  printHead("To verify overwrite when NULL sectionId is provided");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOverwrite(CKPT_OVERWRITE_NULL_SEC_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


/******** OpenCallback *******/


void cpsv_it_openclbk_01()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify opencallback when opening an nonexisting ckpt");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_BAD_NAME_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1006 && tcd.open_clbk_err == SA_AIS_ERR_NOT_EXIST)
     result = TET_PASS;
  else
     result = TET_FAIL;

final2:
   tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_openclbk_02()
{
  int result;
  fd_set read_fd;
  struct timeval tv;
  printHead("To verify openclbk when ckpt already exists but trying to open with different creation attributes");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  fill_ckpt_attri(&tcd.invalid_collocated,SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpenAsync(CKPT_OPEN_ASYNC_ERR_EXIST_T,TEST_NONCONFIG_MODE);

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);

  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.open_clbk_invo == 1019 && tcd.open_clbk_err == SA_AIS_ERR_EXIST)
     result = TET_PASS;
  else
     result = TET_FAIL;

  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);

final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}



/********* SynchronizeCallback ********/


void cpsv_it_syncclbk_01() 
{
  int result;
  printHead("To verify sync clbk when there is no active replica");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                             
  fill_ckpt_attri(&tcd.collocated,SA_CKPT_CHECKPOINT_COLLOCATED|SA_CKPT_WR_ACTIVE_REPLICA,1069,100,2,600,3);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSynchronizeAsync(CKPT_ASYNC_NOT_EXIST_T,TEST_NONCONFIG_MODE);
#if 0
  if(result != TET_PASS)
     goto final3;

  tv.tv_sec = 60;
  tv.tv_usec = 0;
  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  select(tcd.selobj + 1, &read_fd, NULL, NULL, &tv);
                                                                                                                             
  result = tet_test_ckptDispatch(CKPT_DISPATCH_ONE_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.sync_clbk_invo == 2112 && tcd.sync_clbk_err == SA_AIS_ERR_NOT_EXIST)
     result = TET_PASS;
  else
     result = TET_FAIL;
#endif

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


/******* Single node test cases *******/

void cpsv_it_onenode_01()
{
  int result;
  printHead("To verify that process can write into ckpt incrementally");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_ALL_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.general_write.dataBuffer,tcd.general_read.dataBuffer,tcd.general_read.readSize) != 0 )
     result = TET_FAIL;
  else 
     result = TET_PASS;

  fill_write_iovec(&tcd.general_write,tcd.section1,"Data written incrementally",strlen("Data written incrementally"),46,0);
  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  fill_read_iovec(&tcd.general_read,tcd.section1,(char *)tcd.buffer,strlen("Data written incrementally"),46,0);
  result = tet_test_ckptRead(CKPT_READ_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  if( strncmp(tcd.general_write.dataBuffer,tcd.general_read.dataBuffer,tcd.general_read.readSize) != 0 )
     result = TET_FAIL;
  else 
     result = TET_PASS;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
  fill_write_iovec(&tcd.general_write,tcd.section1,"Data in general east or west india is the best",strlen("Data in general east or west india is the best"),0,0);
fill_read_iovec(&tcd.general_read,tcd.section1,(char *)tcd.buffer,strlen("Data in general east or west india is the best"),0,0);
}

void cpsv_it_onenode_02()
{
    cpsv_it_onenode_01();
}

void cpsv_it_onenode_03()
{
   cpsv_it_expset_02();
}

void cpsv_it_onenode_04()
{
    cpsv_it_onenode_01();
}

void cpsv_it_onenode_05()
{
   cpsv_it_write_16();
}

void cpsv_it_onenode_07()
{
   cpsv_it_open_01();
}

void cpsv_it_onenode_08()
{
   cpsv_it_open_02();
}

void cpsv_it_onenode_10()
{
  printHead("No fixed procedure known");
}

void cpsv_it_onenode_11()
{
  printHead("No fixed procedure known for generating corrupted sections");
}

void cpsv_it_onenode_12()
{
  printHead("No fixed procedure known for generating corrupted sections");
}

void cpsv_it_onenode_13()
{
  cpsv_it_repset_01();
}

void cpsv_it_onenode_17()
{
  cpsv_it_repset_03();
}

void cpsv_it_onenode_18()
{
  int result;
  printHead("To verify status get after a section has been deleted");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                            
  fill_ckpt_attri(&tcd.all_replicas,SA_CKPT_WR_ALL_REPLICAS,169,100000,2,85,3);
  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                            
  result = tet_test_ckptOpen(CKPT_OPEN_ALL_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
                                                                                                                            
  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
                                                                                                                            
  result = tet_test_ckptStatusGet(CKPT_STATUS_SUCCESS6_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.status.numberOfSections == 1 &&
                           tcd.status.checkpointCreationAttributes.creationFlags == SA_CKPT_WR_ALL_REPLICAS &&
                           tcd.status.checkpointCreationAttributes.checkpointSize == 169 &&
                           tcd.status.checkpointCreationAttributes.retentionDuration == 100000 &&
                           tcd.status.checkpointCreationAttributes.maxSections == 2 &&
                           tcd.status.checkpointCreationAttributes.maxSectionSize == 85)
     result = TET_PASS;
  else
  {
     result = TET_FAIL;
     goto final3;
  }
                                                                                                                            
  result = tet_test_ckptSectionDelete(CKPT_DEL_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
                                                                                                                            
  result = tet_test_ckptStatusGet(CKPT_STATUS_SUCCESS6_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.status.numberOfSections == 0 &&
                           tcd.status.memoryUsed == 0 &&
                           tcd.status.checkpointCreationAttributes.creationFlags == SA_CKPT_WR_ALL_REPLICAS &&
                           tcd.status.checkpointCreationAttributes.checkpointSize == 169 &&
                           tcd.status.checkpointCreationAttributes.retentionDuration == 100000 &&
                           tcd.status.checkpointCreationAttributes.maxSections == 2 &&
                           tcd.status.checkpointCreationAttributes.maxSectionSize == 85)
     result = TET_PASS;
  else
     result = TET_FAIL;
                                                                                                                            
final3:
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}


void cpsv_it_onenode_19()
{ 
  int result;
  printHead("To verify write and read with generated sectionId");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptInitialize(CKPT_INIT_SYNC_NULL_CBK_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
    
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
    
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_WRITE_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptReplicaSet(CKPT_SET_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  tcd.section6 = tcd.section4;
  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS5_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_GEN_T,TEST_NONCONFIG_MODE); /*contains sec generated using CKPT_SECTION_CREATE_SUCCESS5_T */
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_SUCCESS2_T,TEST_NONCONFIG_MODE);/*contains sec gen using CREATE_SUCCESS5T */
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_GEN_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS_GEN2_T,TEST_NONCONFIG_MODE); /*contains sec generated using CKPT_SECTION_CREATE_GEN_T */
  if(result != TET_PASS)
     goto final3;

  /*tcd.generate_read = tcd.generate_write1; */
  result = tet_test_ckptRead(CKPT_READ_SUCCESS2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptStatusGet(CKPT_STATUS_SUCCESS4_T,TEST_NONCONFIG_MODE);
  if(result == TET_PASS && tcd.status.numberOfSections == 2 &&
                           (tcd.status.checkpointCreationAttributes.creationFlags == (SA_CKPT_WR_ACTIVE_REPLICA|SA_CKPT_CHECKPOINT_COLLOCATED)) &&
                           tcd.status.checkpointCreationAttributes.checkpointSize == 1069 &&
                           tcd.status.checkpointCreationAttributes.retentionDuration == 100 &&
                           tcd.status.checkpointCreationAttributes.maxSections == 2 &&
                           tcd.status.checkpointCreationAttributes.maxSectionSize == 600)
     result = TET_PASS;
   else
     result = TET_FAIL;

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_COLLOCATED_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SYNC_NULL_CBK_T);
final1:
  printResult(result);
}


/********** END OF TEST CASES ************/

void tet_cpsv_get_inputs(TET_CPSV_INST *inst)
{
   char *tmp_ptr=NULL;
   memset(inst,'\0',sizeof(TET_CPSV_INST));

   tmp_ptr = (char *) getenv("TET_CPSV_INST_NUM");
   if(tmp_ptr)
   {
      inst->inst_num = atoi(tmp_ptr);
      tmp_ptr = NULL;
      tcd.inst_num = inst->inst_num;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_TEST_CASE_NUM");
   if(tmp_ptr)
   {
      inst->test_case_num = atoi(tmp_ptr);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_TEST_LIST");
   if(tmp_ptr)
   {
      inst->test_list = atoi(tmp_ptr);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_NUM_ITER");
   if(tmp_ptr)
   {
      inst->num_of_iter = atoi(tmp_ptr);
      tmp_ptr = NULL;
   }
   
   inst->node_id = m_NCS_GET_NODE_ID;
   tcd.nodeId = inst->node_id;

   tmp_ptr = (char *) getenv("TET_CPSV_RED_FLAG");
   if(tmp_ptr)
   {
      inst->redFlag = atoi(tmp_ptr);
      tmp_ptr = NULL;
      tcd.redFlag = inst->redFlag;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_ALL_REPLICAS_CKPT");
   if(tmp_ptr)
   {
      strcpy(inst->all_rep_ckpt_name.value,tmp_ptr);
      inst->all_rep_ckpt_name.length = strlen(inst->all_rep_ckpt_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_ACTIVE_REPLICA_CKPT");
   if(tmp_ptr)
   {
      strcpy(inst->active_rep_ckpt_name.value,tmp_ptr);
      inst->active_rep_ckpt_name.length = strlen(inst->active_rep_ckpt_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_WEAK_REPLICA_CKPT");
   if(tmp_ptr)
   {
      strcpy(inst->weak_rep_ckpt_name.value,tmp_ptr);
      inst->weak_rep_ckpt_name.length = strlen(inst->weak_rep_ckpt_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_COLLOCATED_CKPT");
   if(tmp_ptr)
   {
      strcpy(inst->collocated_ckpt_name.value,tmp_ptr);
      inst->collocated_ckpt_name.length = strlen(inst->collocated_ckpt_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_ASYNC_ALL_REPLICAS_CKPT");
   if(tmp_ptr)
   {
      strcpy(inst->async_all_rep_name.value,tmp_ptr);
      inst->async_all_rep_name.length = strlen(inst->async_all_rep_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_ASYNC_ACTIVE_REPLICA_CKPT");
   if(tmp_ptr)
   {
      strcpy(inst->async_active_rep_name.value,tmp_ptr);
      inst->async_active_rep_name.length = strlen(inst->async_active_rep_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_ALL_COLLOCATED_CKPT");
   if(tmp_ptr)
   {
      strcpy(inst->all_colloc_name.value,tmp_ptr);
      inst->all_colloc_name.length = strlen(inst->all_colloc_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_WEAK_COLLOCATED_CKPT");
   if(tmp_ptr)
   {
      strcpy(inst->weak_colloc_name.value,tmp_ptr);
      inst->weak_colloc_name.length = strlen(inst->weak_colloc_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_MULTI_VECTOR_CKPT");
   if(tmp_ptr)
   {
      strcpy(inst->multi_vector_ckpt_name.value,tmp_ptr);
      inst->multi_vector_ckpt_name.length = strlen(inst->multi_vector_ckpt_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_ALL_REPLICAS_CKPT_L");
   if(tmp_ptr)
   {
      strcpy(inst->all_rep_lckpt_name.value,tmp_ptr);
      inst->all_rep_lckpt_name.length = strlen(inst->all_rep_lckpt_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_ACTIVE_REPLICA_CKPT_L");
   if(tmp_ptr)
   {
      strcpy(inst->active_rep_lckpt_name.value,tmp_ptr);
      inst->active_rep_lckpt_name.length = strlen(inst->active_rep_lckpt_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_WEAK_REPLICA_CKPT_L");
   if(tmp_ptr)
   {
      strcpy(inst->weak_rep_lckpt_name.value,tmp_ptr);
      inst->weak_rep_lckpt_name.length = strlen(inst->weak_rep_lckpt_name.value);
      tmp_ptr = NULL;
   }

   tmp_ptr = (char *) getenv("TET_CPSV_COLLOCATED_CKPT_L");
   if(tmp_ptr)
   {
      strcpy(inst->collocated_lckpt_name.value,tmp_ptr);
      inst->collocated_lckpt_name.length = strlen(inst->collocated_lckpt_name.value);
      tmp_ptr = NULL;
   }
}


void tet_initialize()
{
   char inst_num_char[10]={0};
   sprintf(inst_num_char,"%d%d",tcd.inst_num,tcd.nodeId);

   strcpy(tcd.all_replicas_ckpt.value,"all replicas ckpt");
   strcat(tcd.all_replicas_ckpt.value,inst_num_char);
   tcd.all_replicas_ckpt.length = strlen(tcd.all_replicas_ckpt.value)+1;

   strcpy(tcd.active_replica_ckpt.value,"active replica ckpt");
   strcat(tcd.active_replica_ckpt.value,inst_num_char);
   tcd.active_replica_ckpt.length = strlen(tcd.active_replica_ckpt.value)+1;

   strcpy(tcd.weak_replica_ckpt.value,"weak replica ckpt");
   strcat(tcd.weak_replica_ckpt.value,inst_num_char);
   tcd.weak_replica_ckpt.length = strlen(tcd.weak_replica_ckpt.value)+1;

   strcpy(tcd.collocated_ckpt.value,"collocated ckpt");
   strcat(tcd.collocated_ckpt.value,inst_num_char);
   tcd.collocated_ckpt.length = strlen(tcd.collocated_ckpt.value)+1;

   strcpy(tcd.async_all_replicas_ckpt.value,"async all ckpt");
   strcat(tcd.async_all_replicas_ckpt.value,inst_num_char);
   tcd.async_all_replicas_ckpt.length = strlen(tcd.async_all_replicas_ckpt.value)+1;

   strcpy(tcd.async_active_replica_ckpt.value,"async active ckpt");
   strcat(tcd.async_active_replica_ckpt.value,inst_num_char);
   tcd.async_active_replica_ckpt.length = strlen(tcd.async_active_replica_ckpt.value)+1;

   strcpy(tcd.all_collocated_ckpt.value,"all collocated ckpt");
   strcat(tcd.all_collocated_ckpt.value,inst_num_char);
   tcd.all_collocated_ckpt.length = strlen(tcd.all_collocated_ckpt.value)+1;

   strcpy(tcd.weak_collocated_ckpt.value,"weak collocated ckpt");
   strcat(tcd.weak_collocated_ckpt.value,inst_num_char);
   tcd.weak_collocated_ckpt.length = strlen(tcd.weak_collocated_ckpt.value)+1;

   strcpy(tcd.multi_vector_ckpt.value,"multi vector ckpt");
   strcat(tcd.multi_vector_ckpt.value,inst_num_char);
   tcd.multi_vector_ckpt.length = strlen(tcd.multi_vector_ckpt.value)+1;

   strcpy(tcd.all_replicas_ckpt_large.value,"all replicas large ckpt");
   strcat(tcd.all_replicas_ckpt_large.value,inst_num_char);
   tcd.all_replicas_ckpt_large.length = strlen(tcd.all_replicas_ckpt_large.value)+1;

   strcpy(tcd.active_replica_ckpt_large.value,"active replica large ckpt");
   strcat(tcd.active_replica_ckpt_large.value,inst_num_char);
   tcd.active_replica_ckpt_large.length = strlen(tcd.active_replica_ckpt_large.value)+1;

   strcpy(tcd.weak_replica_ckpt_large.value,"weak replica large ckpt");
   strcat(tcd.weak_replica_ckpt_large.value,inst_num_char);
   tcd.weak_replica_ckpt_large.length = strlen(tcd.weak_replica_ckpt_large.value)+1;

   strcpy(tcd.collocated_ckpt_large.value,"collocated large ckpt");
   strcat(tcd.collocated_ckpt_large.value,inst_num_char);
   tcd.collocated_ckpt_large.length = strlen(tcd.collocated_ckpt_large.value)+1;
}


void tet_cpsv_fill_inputs(TET_CPSV_INST *inst)
{
   if(inst->all_rep_ckpt_name.length)
   {
      memset(&tcd.all_replicas_ckpt,'\0',sizeof(SaNameT));
      strcpy(tcd.all_replicas_ckpt.value,inst->all_rep_ckpt_name.value);
      tcd.all_replicas_ckpt.length = inst->all_rep_ckpt_name.length;
   }

   if(inst->active_rep_ckpt_name.length)
   {
      memset(&tcd.active_replica_ckpt,'\0',sizeof(SaNameT));
      strcpy(tcd.active_replica_ckpt.value,inst->active_rep_ckpt_name.value);
      tcd.active_replica_ckpt.length = inst->active_rep_ckpt_name.length;
   }

   if(inst->weak_rep_ckpt_name.length)
   {
      memset(&tcd.weak_replica_ckpt,'\0',sizeof(SaNameT));
      strcpy(tcd.weak_replica_ckpt.value,inst->weak_rep_ckpt_name.value);
      tcd.weak_replica_ckpt.length = inst->weak_rep_ckpt_name.length;
   }

   if(inst->collocated_ckpt_name.length)
   {
      memset(&tcd.collocated_ckpt,'\0',sizeof(SaNameT));
      strcpy(tcd.collocated_ckpt.value,inst->collocated_ckpt_name.value);
      tcd.collocated_ckpt.length = inst->collocated_ckpt_name.length;
   }

   if(inst->async_all_rep_name.length)
   {
      memset(&tcd.async_all_replicas_ckpt,'\0',sizeof(SaNameT));
      strcpy(tcd.async_all_replicas_ckpt.value,inst->async_all_rep_name.value);
      tcd.async_all_replicas_ckpt.length = inst->async_all_rep_name.length;
   }

   if(inst->async_active_rep_name.length)
   {
      memset(&tcd.async_active_replica_ckpt,'\0',sizeof(SaNameT));
      strcpy(tcd.async_active_replica_ckpt.value,inst->async_active_rep_name.value);
      tcd.async_active_replica_ckpt.length = inst->async_active_rep_name.length;
   }

   if(inst->all_colloc_name.length)
   {
      memset(&tcd.all_collocated_ckpt,'\0',sizeof(SaNameT));
      strcpy(tcd.all_collocated_ckpt.value,inst->all_colloc_name.value);
      tcd.all_collocated_ckpt.length = inst->all_colloc_name.length;
   }

   if(inst->weak_colloc_name.length)
   {
      memset(&tcd.weak_collocated_ckpt,'\0',sizeof(SaNameT));
      strcpy(tcd.weak_collocated_ckpt.value,inst->weak_colloc_name.value);
      tcd.weak_collocated_ckpt.length = inst->weak_colloc_name.length;
   }

   if(inst->multi_vector_ckpt_name.length)
   {
      memset(&tcd.multi_vector_ckpt,'\0',sizeof(SaNameT));
      strcpy(tcd.multi_vector_ckpt.value,inst->multi_vector_ckpt_name.value);
      tcd.multi_vector_ckpt.length = inst->multi_vector_ckpt_name.length;
   }

   if(inst->all_rep_lckpt_name.length)
   {
      memset(&tcd.all_replicas_ckpt_large,'\0',sizeof(SaNameT));
      strcpy(tcd.all_replicas_ckpt_large.value,inst->all_rep_lckpt_name.value);
      tcd.all_replicas_ckpt_large.length = inst->all_rep_lckpt_name.length;
   }

   if(inst->active_rep_lckpt_name.length)
   {
      memset(&tcd.active_replica_ckpt_large,'\0',sizeof(SaNameT));
      strcpy(tcd.active_replica_ckpt_large.value,inst->active_rep_lckpt_name.value);
      tcd.active_replica_ckpt_large.length = inst->active_rep_lckpt_name.length;
   }

   if(inst->weak_rep_lckpt_name.length)
   {
      memset(&tcd.weak_replica_ckpt_large,'\0',sizeof(SaNameT));
      strcpy(tcd.weak_replica_ckpt_large.value,inst->weak_rep_lckpt_name.value);
      tcd.weak_replica_ckpt_large.length = inst->weak_rep_lckpt_name.length;
   }

   if(inst->collocated_lckpt_name.length)
   {
      memset(&tcd.collocated_ckpt_large,'\0',sizeof(SaNameT));
      strcpy(tcd.collocated_ckpt_large.value,inst->collocated_lckpt_name.value);
      tcd.collocated_ckpt_large.length = inst->collocated_lckpt_name.length;
   }
}


void app1()
{
  int result;
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result !=TET_PASS)
     goto final;

  tcd.smoke_test_replica = tcd.my_app;
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result !=TET_PASS)
     goto final;

  result = tet_test_ckptSectionCreate(CKPT_SECTION_CREATE_SUCCESS6_T,TEST_CONFIG_MODE);

  tcd.active_replica_Writehdl = tcd.allflags_set_hdl;
  result = tet_test_ckptClose(CKPT_CLOSE_SUCCESS11_T,TEST_CONFIG_MODE);
  
final:
  printResult(result);
  exit(0);
}

void app2()
{
  int result;
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result !=TET_PASS)
     goto final;

  tcd.smoke_test_replica = tcd.my_app;
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS3_T,TEST_CONFIG_MODE);
  result = tet_test_ckptOpen(CKPT_OPEN_COLLOCATE_SUCCESS3_T,TEST_CONFIG_MODE);
  if(result !=TET_PASS)
     goto final;

  result = tet_test_ckptIterationInit(CKPT_ITER_INIT_SUCCESS2_T,TEST_CONFIG_MODE);
  if(result !=TET_PASS)
     goto final;

  result = tet_test_ckptIterationNext(CKPT_ITER_NEXT_SUCCESS_T,TEST_CONFIG_MODE);
final:
  printResult(result);
  exit(0);
}

void cpsv_it_noncolloc_01()
{
  int result;
  printHead("To verify the non-collocated ckpt mgmt");
                                                                                                                            
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final;
                                                                                                                            
  result = tet_test_ckptOpen(CKPT_OPEN_ALL_CREATE_SUCCESS_T,TEST_NONCONFIG_MODE);
  m_TET_CPSV_PRINTF("\n checkpoint created please check the replicas\n");
  getchar();
  tet_ckpt_cleanup(CPSV_CLEAN_ALL_REPLICAS_CKPT);
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
                                                                                                                            
final:
  printResult(result);
}

void cpsv_it_unlinktest()
{
        SaAisErrorT     error;
        int i;
        SaCkptHandleT    ckptHandle;
        SaCkptCallbacksT ckptCallback = {
                .saCkptCheckpointOpenCallback        = AppCkptOpenCallback,
                .saCkptCheckpointSynchronizeCallback = AppCkptSyncCallback
        };

        SaVersionT      version; 
        fill_ckpt_version(&version,'B',0x02,0x02);
        int ret = TET_UNRESOLVED;
                                                                                                                            
        char name_open[] = "checkpoint";
        SaNameT ckpt_name;
        SaCkptCheckpointCreationAttributesT ckpt_create_attri;
        SaCkptCheckpointHandleT checkpoint_handle;
                                                                                                                            
    for(i=0;i<50;i++)
    {
        error = saCkptInitialize(&ckptHandle, &ckptCallback, &version);
        if (error != SA_AIS_OK){
                printf("  saCkptInitialize, Return value: %s, should be "
                                "SA_AIS_OK\n", saf_error_string[error]);
                ret = TET_UNRESOLVED;
                goto out;
        }
        printf("\n Initialized \n");
 
        ckpt_name.length = sizeof(name_open) ;
        memcpy (ckpt_name.value, name_open, ckpt_name.length);
 
        ckpt_create_attri.creationFlags = SA_CKPT_WR_ACTIVE_REPLICA
                                        |SA_CKPT_CHECKPOINT_COLLOCATED;
        ckpt_create_attri.retentionDuration = SA_TIME_END;
        ckpt_create_attri.checkpointSize = 1000;
        ckpt_create_attri.maxSectionSize = 100;
        ckpt_create_attri.maxSections = 10;
        ckpt_create_attri.maxSectionIdSize = 10;

        error = saCkptCheckpointOpen (  ckptHandle,
                                        &ckpt_name,
                                        &ckpt_create_attri ,
                                        SA_CKPT_CHECKPOINT_CREATE
                                        |SA_CKPT_CHECKPOINT_WRITE,
                                        SA_TIME_MAX,
                                        &checkpoint_handle);
        if ( error != SA_AIS_OK){
                printf("  can't open checkpoint, return %s \n",
                                       saf_error_string[error]);
                ret = TET_UNRESOLVED;
                goto final_2;
        }

        printf("\n Checkpoint opened \n");

        error = saCkptFinalize(ckptHandle);
        if ( error != SA_AIS_OK){
                printf("  can't finalize, return %s \n",
                                       saf_error_string[error]);
                ret = TET_UNRESOLVED;
                goto final_1;
        }

        printf("\n Finalized\n");
 
        error = saCkptActiveReplicaSet(checkpoint_handle);
        if ( error != SA_AIS_ERR_BAD_HANDLE){
                printf("  invoke saCkptActiveReplicaSet, return %s \n",
                                       saf_error_string[error]);
                ret = TET_FAIL;
                goto out;
        }
        printf("\n Active replica set bad handle \n");
        ret = TET_PASS;
 
        error = saCkptInitialize(&ckptHandle, &ckptCallback, &version);
        if (error != SA_AIS_OK){
                printf("  saCkptInitialize, Return value: %s, should be "
                                "SA_AIS_OK\n", saf_error_string[error]);
                ret = TET_UNRESOLVED;
                goto out;
        }
        printf("\nsecond Initialized \n");
 
        saCkptCheckpointUnlink(ckptHandle, &ckpt_name);
 
        printf("\n unlinked \n");
        goto out;
final_1:
        saCkptCheckpointClose(checkpoint_handle);
        saCkptCheckpointUnlink(ckptHandle, &ckpt_name);
final_2:

       saCkptFinalize(ckptHandle);

out:
if (1);

    }
}

void cpsv_it_arr_default_sec()
{
  int result;
  fd_set read_fd;
  tcd.arr_clbk_flag = 0;
  tcd.arr_clbk_err = 0;
  printHead("To verify the functionality of arrival callback");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;
                                                                                                                         
  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;
                                                                                                                         
  ncsCkptRegisterCkptArrivalCallback(tcd.ckptHandle,AppCkptArrivalCallback);
                                                                                                                         
  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
                                                                                                                         
  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
                                                                                                                         
  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  FD_ZERO(&read_fd);
  FD_SET(tcd.selobj, &read_fd);
  result = select(tcd.selobj + 1, &read_fd, NULL, NULL, NULL);
  if(result == 1)
  {
     result = tet_test_ckptDispatch(CKPT_DISPATCH_ALL_T,TEST_NONCONFIG_MODE);
     if(result != TET_PASS)
        goto final3;
  }
  else
  {
     result = TET_FAIL;
     goto final3;
  }

  if(!tcd.arr_clbk_flag)
  {
     tet_printf("\nCallback didnot arrive at the application\n");
     result = TET_FAIL;
     goto final3;
  }

  if(tcd.arr_clbk_err)
  {
     tet_printf("\nCallback values not proper\n");
     result = TET_FAIL;
  }

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}
                                                                                                                         
void cpsv_it_read_withoutwrite()
{
  int result;
  printHead("To verify read api when there is no data to be read");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptRead(CKPT_READ_WEAK_REP_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}

void cpsv_it_read_withnullbuf()
{
  int result;
  printHead("To verify read api with NULL data buffer");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_WRITE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_READ_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final3;

  result = tet_test_ckptWrite(CKPT_WRITE_SUCCESS2_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final3;
  result = tet_test_ckptRead(CKPT_READ_BUFFER_NULL_T,TEST_NONCONFIG_MODE);

final3:
  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}
                                                                                                                             
void cpsv_it_arr_invalid_param()
{
  int result;
  SaAisErrorT rc;
  printHead("To verify the arrival callback when NULL clbk is passed");
  result = tet_test_ckptInitialize(CKPT_INIT_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final1;

  result = tet_test_ckptSelectionObject(CKPT_SEL_SUCCESS_T,TEST_NONCONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  result = tet_test_ckptOpen(CKPT_OPEN_WEAK_CREATE_SUCCESS_T,TEST_CONFIG_MODE);
  if(result != TET_PASS)
     goto final2;

  rc = ncsCkptRegisterCkptArrivalCallback(tcd.ckptHandle,NULL);
  m_TET_CPSV_PRINTF(" Return Value    : %s\n\n",saf_error_string[rc]);
  tet_printf(" Return Value    : %s\n\n",saf_error_string[rc]);

  if(rc == SA_AIS_ERR_INVALID_PARAM)
     result = TET_PASS;
  else
     result = TET_FAIL;

  tet_ckpt_cleanup(CPSV_CLEAN_WEAK_REPLICAS_CKPT);
final2:
  tet_cpsv_cleanup(CPSV_CLEAN_INIT_SUCCESS_T);
final1:
  printResult(result);
}
                                                                                                                             

struct tet_testlist cpsv_single_node_testlist[]={
  {cpsv_it_init_01,1,0},
  {cpsv_it_init_02,2,0},
  {cpsv_it_init_03,3,0},
  {cpsv_it_init_04,4,0},
  {cpsv_it_init_05,5,0},
  {cpsv_it_init_06,6,0},
  {cpsv_it_init_07,7,0},
  {cpsv_it_init_08,8,0},
  {cpsv_it_init_09,9,0},
  {cpsv_it_init_10,10,0},

  {cpsv_it_disp_01,11,0},
  {cpsv_it_disp_02,12,0},
/*dispatch blocking */
  {cpsv_it_disp_03,13,0},
  {cpsv_it_disp_04,14,0},
  {cpsv_it_disp_05,15,0},
  {cpsv_it_disp_06,16,0},
  {cpsv_it_disp_07,17,0},
  {cpsv_it_disp_08,18,0},
  {cpsv_it_disp_09,19,0},

  {cpsv_it_fin_01,20,0},
  {cpsv_it_fin_02,21,0},
  {cpsv_it_fin_03,22,0},
  {cpsv_it_fin_04,23,0},

  {cpsv_it_sel_01,24,0},
  {cpsv_it_sel_02,25,0},
  {cpsv_it_sel_03,26,0},
  {cpsv_it_sel_04,27,0},
  {cpsv_it_sel_05,28,0},
  {cpsv_it_sel_06,29,0},

  {cpsv_it_open_01,30,0},
  {cpsv_it_open_02,31,0},
  {cpsv_it_open_03,32,0},
  {cpsv_it_open_04,33,0},
  {cpsv_it_open_05,34,0},
  {cpsv_it_open_06,35,0},
  {cpsv_it_open_07,36,0},
  {cpsv_it_open_08,37,0},
  {cpsv_it_open_10,38,0},
  {cpsv_it_open_11,39,0},
  {cpsv_it_open_12,40,0},
  {cpsv_it_open_13,41,0},
  {cpsv_it_open_14,42,0},
  {cpsv_it_open_15,43,0},
  {cpsv_it_open_16,44,0},
  {cpsv_it_open_17,45,0},
  {cpsv_it_open_18,46,0},
  {cpsv_it_open_19,47,0},
  {cpsv_it_open_20,48,0},
  {cpsv_it_open_21,49,0},
  {cpsv_it_open_22,50,0},
  {cpsv_it_open_23,51,0},
  {cpsv_it_open_24,52,0},
  {cpsv_it_open_25,53,0},
  {cpsv_it_open_26,54,0},
  {cpsv_it_open_27,55,0},
  {cpsv_it_open_28,56,0},
  {cpsv_it_open_29,57,0},
  {cpsv_it_open_30,58,0},
  {cpsv_it_open_31,59,0},
  {cpsv_it_open_32,60,0},
  {cpsv_it_open_33,61,0},
  {cpsv_it_open_34,62,0},
  {cpsv_it_open_35,63,0},
  {cpsv_it_open_36,64,0},
  {cpsv_it_open_37,65,0},
  {cpsv_it_open_38,66,0},
  {cpsv_it_open_39,67,0},
  {cpsv_it_open_40,68,0},
  {cpsv_it_open_41,69,0},
  {cpsv_it_open_42,70,0},
  {cpsv_it_open_43,71,0},
  {cpsv_it_open_44,72,0},
  {cpsv_it_open_45,73,0},
  {cpsv_it_open_46,74,0},
  {cpsv_it_open_47,75,0},
  {cpsv_it_open_48,76,0},
  {cpsv_it_open_49,77,0},
  {cpsv_it_open_50,78,0},
  {cpsv_it_open_51,79,0},

  {cpsv_it_close_01,80,0},
  {cpsv_it_close_02,81,0},
  {cpsv_it_close_03,82,0},
  {cpsv_it_close_04,83,0},
  {cpsv_it_close_05,84,0},
  {cpsv_it_close_06,85,0},

  {cpsv_it_unlink_01,86,0},
  {cpsv_it_unlink_02,87,0},
  {cpsv_it_unlink_03,88,0},
  {cpsv_it_unlink_04,89,0},
  {cpsv_it_unlink_05,90,0},
  {cpsv_it_unlink_06,91,0},
  {cpsv_it_unlink_07,92,0},
  {cpsv_it_unlink_08,93,0},
  {cpsv_it_unlink_09,94,0},
#if 0
  {cpsv_it_unlink_10,95,0},
#endif
  {cpsv_it_unlink_11,96,0},

  {cpsv_it_rdset_01,97,0},
  {cpsv_it_rdset_02,98,0},
  {cpsv_it_rdset_03,99,0},
  {cpsv_it_rdset_04,100,0},
  {cpsv_it_rdset_05,101,0},
#if 0
  {cpsv_it_rdset_06,102,0},
#endif

  {cpsv_it_repset_01,103,0},
  {cpsv_it_repset_02,104,0},
  {cpsv_it_repset_03,105,0},
  {cpsv_it_repset_04,106,0},
  {cpsv_it_repset_05,107,0},

  {cpsv_it_status_01,108,0},
  {cpsv_it_status_02,109,0},
  {cpsv_it_status_03,110,0},
  {cpsv_it_status_05,111,0},
  {cpsv_it_status_06,112,0},

  {cpsv_it_seccreate_01,113,0},
  {cpsv_it_seccreate_02,114,0},
  {cpsv_it_seccreate_03,115,0},
  {cpsv_it_seccreate_04,116,0},
  {cpsv_it_seccreate_05,117,0},
  {cpsv_it_seccreate_07,118,0},
  {cpsv_it_seccreate_09,119,0},
  {cpsv_it_seccreate_10,120,0},
  {cpsv_it_seccreate_11,121,0},
  {cpsv_it_seccreate_12,122,0},
  {cpsv_it_seccreate_13,123,0},
  {cpsv_it_seccreate_14,124,0},
  {cpsv_it_seccreate_15,125,0},

  {cpsv_it_secdel_01,126,0},
  {cpsv_it_secdel_02,127,0},
  {cpsv_it_secdel_03,128,0},
  {cpsv_it_secdel_04,129,0},
  {cpsv_it_secdel_08,130,0},
  {cpsv_it_secdel_09,131,0},

  {cpsv_it_expset_01,132,0},
  {cpsv_it_expset_02,133,0},
  {cpsv_it_expset_03,134,0},
#if 0
  {cpsv_it_expset_04,135,0},
#endif
  {cpsv_it_expset_05,136,0},
  {cpsv_it_expset_06,137,0},
  {cpsv_it_expset_07,138,0},

  {cpsv_it_iterinit_01,139,0},
  {cpsv_it_iterinit_02,140,0},
  {cpsv_it_iterinit_03,141,0},
  {cpsv_it_iterinit_04,142,0},
  {cpsv_it_iterinit_05,143,0},
  {cpsv_it_iterinit_06,144,0},
#if 0 
  {cpsv_it_iterinit_07,145,0},
#endif
  {cpsv_it_iterinit_08,146,0},
  {cpsv_it_iterinit_09,147,0},
  {cpsv_it_iterinit_10,148,0},
  {cpsv_it_iterinit_12,149,0},

  {cpsv_it_iternext_01,150,0},
  {cpsv_it_iternext_02,151,0},
  {cpsv_it_iternext_03,152,0},
  {cpsv_it_iternext_04,153,0},
  {cpsv_it_iternext_05,154,0},
  {cpsv_it_iternext_06,155,0},
  {cpsv_it_iternext_08,156,0},
  {cpsv_it_iternext_09,157,0},

  {cpsv_it_iterfin_01,158,0},
  {cpsv_it_iterfin_02,159,0},
  {cpsv_it_iterfin_04,160,0},

  {cpsv_it_write_01,161,0},
  {cpsv_it_write_02,162,0},
  {cpsv_it_write_03,163,0},
  {cpsv_it_write_04,164,0},
  {cpsv_it_write_05,165,0},
  {cpsv_it_write_06,166,0},
  {cpsv_it_write_07,167,0},
  {cpsv_it_write_08,168,0},
  {cpsv_it_write_09,169,0},
  {cpsv_it_write_10,170,0},
  {cpsv_it_write_11,171,0},
  {cpsv_it_write_12,172,0},
  {cpsv_it_write_14,173,0},
  {cpsv_it_write_16,174,0},

  {cpsv_it_read_01,175,0},
  {cpsv_it_read_02,176,0},
  {cpsv_it_read_03,177,0},
  {cpsv_it_read_04,178,0},
  {cpsv_it_read_05,179,0},
  {cpsv_it_read_06,180,0},
  {cpsv_it_read_07,181,0},
  {cpsv_it_read_08,182,0},
  {cpsv_it_read_10,183,0},
  {cpsv_it_read_11,184,0},
  {cpsv_it_read_13,185,0},

  {cpsv_it_sync_01,186,0},
  {cpsv_it_sync_02,187,0},
  {cpsv_it_sync_03,188,0},
  {cpsv_it_sync_04,189,0},
  {cpsv_it_sync_05,190,0},
  {cpsv_it_sync_06,191,0},
  {cpsv_it_sync_07,192,0},
  {cpsv_it_sync_08,193,0},
  {cpsv_it_sync_09,194,0},
  {cpsv_it_sync_10,195,0},
  {cpsv_it_sync_11,196,0},
  {cpsv_it_sync_12,197,0},
  {cpsv_it_sync_13,198,0},
  {cpsv_it_sync_14,199,0},

  {cpsv_it_overwrite_01,200,0},
  {cpsv_it_overwrite_02,201,0},
  {cpsv_it_overwrite_03,202,0},
  {cpsv_it_overwrite_04,203,0},
  {cpsv_it_overwrite_05,204,0},
  {cpsv_it_overwrite_06,205,0},
  {cpsv_it_overwrite_07,206,0},
  {cpsv_it_overwrite_08,207,0},
  {cpsv_it_overwrite_09,208,0},
  {cpsv_it_overwrite_10,209,0},
  {cpsv_it_overwrite_11,210,0},
  {cpsv_it_overwrite_12,211,0},

  {cpsv_it_openclbk_01,212,0},
  {cpsv_it_openclbk_02,213,0},
  {cpsv_it_syncclbk_01,214,0},

  {cpsv_it_onenode_01,215,0},
  {cpsv_it_onenode_02,216,0},
  {cpsv_it_onenode_03,217,0},
  {cpsv_it_onenode_04,218,0},
  {cpsv_it_onenode_05,219,0},
  {cpsv_it_onenode_07,220,0},
  {cpsv_it_onenode_08,221,0},
  {cpsv_it_onenode_13,222,0},
  {cpsv_it_onenode_17,223,0},
  {cpsv_it_onenode_18,224,0},
  {cpsv_it_onenode_19,225,0},
  {cpsv_it_open_52,226,0},
  {cpsv_it_open_53,227,0},
  {cpsv_it_open_54,228,0},
  {cpsv_it_close_07,229,0},
  {cpsv_it_arr_default_sec,230,0},
  {cpsv_it_read_withoutwrite,231,0},
  {cpsv_it_read_withnullbuf,232,0},
  {cpsv_it_close_08,233,0},
  {cpsv_it_arr_invalid_param,234,0},
  {cpsv_it_seccreate_16,235,0},
  {cpsv_it_seccreate_17,236,0},
  {cpsv_it_seccreate_18,237,0},
#if 0
  /* Test procedure unknown */
  {cpsv_it_onenode_10,222,0},
  {cpsv_it_onenode_11,223,0},
  {cpsv_it_onenode_12,224,0},
  {cpsv_it_noncolloc_01,235,0},
  {cpsv_it_unlinktest,236,0},
#endif
  {NULL,-1,0}
};

void tet_run_cpsv_instance(TET_CPSV_INST *inst)
{
  int iter;
  int test_case_num=-1; /* DEFAULT - all test cases run */
  struct tet_testlist *cpsv_testlist = cpsv_single_node_testlist; 

  if(inst->test_case_num)
      test_case_num = inst->test_case_num;

  if(inst->test_list ==1)
      cpsv_testlist = cpsv_single_node_testlist;


  for(iter=0; iter < (inst->num_of_iter); iter++)
      tet_test_start(test_case_num,cpsv_testlist);
}

void tet_run_cpsv_app()
{

  TET_CPSV_INST inst;
  tet_readFile();
  fill_testcase_data();
  tet_cpsv_get_inputs(&inst);
  if(inst.test_list == 1)
      tet_initialize();
  tet_cpsv_fill_inputs(&inst);

  tware_mem_ign();
  tet_run_cpsv_instance(&inst);
  tware_mem_dump();
  sleep(5);
  tet_test_cleanup();
}

#endif
