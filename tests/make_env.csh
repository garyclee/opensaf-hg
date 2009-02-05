#!/bin/csh

if ($#argv < 3) then 

   echo "+=================================================================+";
   echo "| USAGE: make_env.csh TARGET_OS FUNCTIONALITY_NAME SERVICE_NAME    ";
   echo "+=================================================================+";
   exit 1

endif

setenv TARGET_OS		"$1"
setenv FUNCTIONALITY            "$2"
setenv SERVICE_NAME             "$3"

setenv OPENSAF_INC               "$OPENSAF_HOME/include"

setenv TET_OPENSAF_HOME             $PWD

switch ($TARGET_OS)
   case linux:
      setenv TARGET_LIB_PATH  "$TARGET_LIB_PATH/lib"
      breaksw

   case linux-64:
      setenv TARGET_LIB_PATH "$TARGET_LIB_PATH/lib64"
      breaksw

   default:
      echo "Wrong option for OS_TYPE - $TARGET_OS"
      exit 1
endsw


setenv TARGET_ARCH "$TARGET_HOST"
setenv TET_ROOT "/usr/local/tet"
set MAKE_FILE_PATH=$TET_OPENSAF_HOME/makefile


# Legacy defines and common stuff 
setenv OS_HJ_CPP_DEFINES "-DHJ_LINUX=1 -DUSE_READLINE -DENABLE_LEAP_DBG=0 -DHJ_USE_LINUXATM"
setenv COMMON_CPP_DEFINES "-DNCS_MMGR_DEBUG=1 -DNDEBUG=1 -DNCS_SAF=1"

foreach type ($FUNCTIONALITY)

   setenv OPENSAF_SVC_DEFINES "$COMMON_CPP_DEFINES $OS_HJ_CPP_DEFINES"
   setenv OPENSAF_TGT_LIBS "-lncs_core"

   switch ($SERVICE_NAME)

      case ifsv:
         setenv MAKE_DIR "$TET_OPENSAF_HOME/ifsv/src"
         setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lifsv_common -ledsv_common -lsaf_common"

         switch ($type)

            case DRIVER:
               setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES -DNCS_IFND=1 -DNCS_IFSV_LOG=1 -DNCS_IFSV_IPXS=1"
               setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lifa -lSaEvt -lmaa "
               setenv TET_SVC_DEFINES "-DTET_DRIVER=1 -DTET_IFSV=1 -DTET_RED=0"
               setenv TARGET "$TET_OPENSAF_HOME/ifsv/ifsv_driver_$TARGET_ARCH.exe"
               breaksw

            case A:
               setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES -DNCS_IFA=1 -DNCS_IFSV_IPXS=1"
               setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lifa -lSaEvt -lmaa "
               setenv TET_SVC_DEFINES "-DTET_A=1 -DTET_IFSV=1 -DTET_APP1=1 -DTET_RED=0"
               setenv TARGET "$TET_OPENSAF_HOME/ifsv/ifsv_a_$TARGET_ARCH.exe"
               breaksw 

            case VIP:
               setenv MAKE_DIR "$TET_OPENSAF_HOME/ifsv/vip"

               setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES -DNCS_IFA=1 -DNCS_IFSV_IPXS=1"
               setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lifa -lifsv_common"
               setenv TET_SVC_DEFINES "-DTET_A=1 -DTET_IFSV=1 -DTET_APP1=1 -DNCS_VIP=1 -DVIP_HALS_SUPPORT=1"
               setenv TARGET "$TET_OPENSAF_HOME/ifsv/ifsv_vip_$TARGET_ARCH.exe"
               breaksw

         default:
            echo " Unknown Option $FUNCTIONALITY  "
            exit 1
         endsw
   breaksw
# -- end of ifsv stuff

   case glsv:

      setenv MAKE_DIR "$TET_OPENSAF_HOME/glsv/src"

      switch ($type) 
       
         case A:
            setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES -DNCS_GLA=1"
            setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lSaLck -lmaa "
            setenv TET_SVC_DEFINES "-DTET_GLSV=1 -DTET_A=1 -DTET_RED_GLA=0"
            setenv TARGET "$TET_OPENSAF_HOME/glsv/glsv_a_$TARGET_ARCH.exe"
            breaksw

         default:
            echo " Unknown Option $FUNCTIONALITY  "
            exit 1
      endsw
    breaksw
# -- end of glsv stuff

    case mqsv:

       setenv MAKE_DIR "$TET_OPENSAF_HOME/mqsv/src"

       switch ($type) 
            case A:
               setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES"
               setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lSaMsg -lmaa  -lmqsv_common -lsaf_common"
               setenv TET_SVC_DEFINES "-DTET_MQSV=1 -DTET_A=1 -DTET_RED_MQA=0"
               setenv TARGET "$TET_OPENSAF_HOME/mqsv/mqsv_a_$TARGET_ARCH.exe"
               breaksw

            default:
               echo " Unknown Option $FUNCTIONALITY  "
               exit 1
         endsw
      breaksw
# -- end of mqsv stuff

      case avsv:

         setenv TET_SVC_DEFINES "-DTET_AVSV=1"
         setenv MAKE_DIR "$TET_OPENSAF_HOME/avsv/src"

         switch ($type)

            case ND:
               setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES -DNCS_AVND=1 -DNCS_AVSV=1"
               setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lavnd -lSaAmf -lsrma"
               setenv TET_SVC_DEFINES "$TET_SVC_DEFINES -DTET_ND=1"
               setenv TARGET "$TET_OPENSAF_HOME/avsv/avsv_nd_$TARGET_ARCH.exe"
               breaksw

            case A:
               setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES"
               setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lSaAmf -lSaClm -lavsv_common -lsaf_common -lreadline"
               setenv TET_SVC_DEFINES "$TET_SVC_DEFINES -DTET_A=1 -DAMFT_PROXY_COMP -DAMFT_NO_LOGGING"
               setenv TARGET "$TET_OPENSAF_HOME/avsv/avsv_a_$TARGET_ARCH.exe"
               breaksw

            case CLM:
               setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES"
               setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lSaAmf -lSaClm -lavsv_common -lsaf_common"
               setenv TET_SVC_DEFINES "$TET_SVC_DEFINES -DTET_A=1 -DTET_CLM=1"
               setenv TARGET "$TET_OPENSAF_HOME/avsv/avsv_clm_$TARGET_ARCH.exe"
               breaksw

            default:
               echo " Unknown Option $FUNCTIONALITY  "
               exit 1
      endsw
   breaksw
# -- end of avsv stuff

   case dlsv:
      setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES -DNCS_FLA=1"
      setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS "
      setenv TET_SVC_DEFINES "-DTET_DLSV=1"
      setenv MAKE_DIR "$TET_OPENSAF_HOME/dlsv/src"

      switch($type)

         case A:
            setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES -DNCS_DLFLA=1"
            setenv TET_SVC_DEFINES "$TET_SVC_DEFINES -DTET_DLSV_A=1 _DTET_DLSV_SVC=1"
            setenv TARGET "$TET_OPENSAF_HOME/dlsv/dlsv_a_$TARGET_ARCH.exe"
	    breaksw

        default:
               echo " Unknown Option $FUNCTIONALITY  "
               exit 1
           breaksw

      endsw
     breaksw
# -- end of dlsv stuff

   case cpsv:

      setenv  MAKE_DIR "$TET_OPENSAF_HOME/cpsv/src"
      setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS"
      setenv TET_SVC_DEFINES "-DTET_CPSV=1"

      switch ($type) 

         case A:
            setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES"
            setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lSaCkpt -lmaa -lcpsv_common -lsaf_common"
            setenv TET_SVC_DEFINES "$TET_SVC_DEFINES -DTET_A=1 -DTET_APP=1 -DTET_RED_FLAG=0"
            setenv TARGET "$TET_OPENSAF_HOME/cpsv/cpsv_app_$TARGET_ARCH.exe"
            breaksw

         default:
            echo " Unknown Option $FUNCTIONALITY  "
            exit 1
      endsw
   breaksw

   case edsv:

      setenv  MAKE_DIR "$TET_OPENSAF_HOME/edsv/src"
      setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS"

      switch ($type)

         case A:
              setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lSaEvt -lmaa  -ledsv_common -lsaf_common"
              setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES"
              setenv TET_SVC_DEFINES "-DTET_EDSV=1 -DTET_RED=0"
              setenv TARGET "$TET_OPENSAF_HOME/edsv/eda_ut_$TARGET_ARCH.exe"
              breaksw

         default:
            echo " Unknown Option $FUNCTIONALITY  "
            exit 1
        endsw
   breaksw

   case mbcsv:

      setenv  MAKE_DIR "$TET_OPENSAF_HOME/mbcsv/src"

      switch ($type)

         case A:
	    setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lmbca"
      	    setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES"
      	    setenv TET_SVC_DEFINES "-DTET_MBCSV=1"
            setenv TARGET "$TET_OPENSAF_HOME/mbcsv/mbcsv_$TARGET_ARCH.exe"
            breaksw

         default:
            echo " Unknown Option $FUNCTIONALITY  "
            exit 1
         endsw
      breaksw

    case mds:

      setenv MAKE_DIR "$TET_OPENSAF_HOME/mds/src"
      setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES"
      setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lSaAmf -lavsv_common -lsaf_common"
      setenv TET_SVC_DEFINES "-DTET_MDS_TDS=1"

      switch ($type)
         case SNDR:
            setenv TARGET "$TET_OPENSAF_HOME/mds/mds_sender_$TARGET_ARCH.exe"
            breaksw
        case RCVR:
            setenv TARGET "$TET_OPENSAF_HOME/mds/mds_receiver_$TARGET_ARCH.exe"
            breaksw
         default:
            echo "Unkonown option $FUNCTIONALITY  "
            exit 1
      endsw
   breaksw


   case srmsv:
      setenv  MAKE_DIR "$TET_OPENSAF_HOME/srmsv/src"

      switch ($type)
         case A:
            setenv OPENSAF_SVC_DEFINES "$OPENSAF_SVC_DEFINES -DNCS_SRMA=1"
	    setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lsrma"
            setenv TET_SVC_DEFINES "-DTET_A=1 -DTET_SRMSV=1"
            setenv TARGET "$TET_OPENSAF_HOME/srmsv/srmsv_$TARGET_ARCH.exe"
            breaksw

         default:
            echo " Unknown Option $FUNCTIONALITY  "
            exit 1
         endsw
      breaksw

   case clean:

      switch ($type)
         case clean:
            setenv OPENSAF_TGT_LIBS "$OPENSAF_TGT_LIBS -lSaAmf -lSaMsg -lSaEvt -lSaEvt -lSaLck -lSaCkpt -lSaClm"
            setenv TET_SVC_DEFINES "-DTET_A=1"
            setenv TARGET ""
            setenv HJCPP_DEFINES "$OPENSAF_SVC_DEFINES $TET_SVC_DEFINES "
            setenv OPENSAF_SHARED_LIBRARIES "$OPENSAF_TGT_LIBS "
            make -f $MAKE_FILE_PATH clean
            exit
        breaksw
        endsw

    breaksw
               

endsw

## set appropriate old env variable to minimise changes in makefile

  setenv HJCPP_DEFINES "$OPENSAF_SVC_DEFINES $TET_SVC_DEFINES "
  setenv OPENSAF_SHARED_LIBRARIES "$OPENSAF_TGT_LIBS "
  echo $MAKE_FILE_PATH
  make -f $MAKE_FILE_PATH 

