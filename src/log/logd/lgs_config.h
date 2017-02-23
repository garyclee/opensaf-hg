/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2015 The OpenSAF Foundation
 * Copyright Ericsson AB [2015, 2017] - All Rights Reserved
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

/***
 * This handler contains handling of Log service configuration including:
 *  - Configuration object
 *  - Environment variables
 *  - Default values
 *  - Verification of attribute values
 *  - Check-pointing
 *  - Presenting of configuration related information using runtime object
 *
 * All configuration data that can be found in the
 * 'logConfig=1,safApp=safLogService' configuration object is stored locally on
 * both active and standby nodes. The nodes are synchronized using message based
 * check-pointing. Local data is synchronized with changes in the configuration
 * object in the OI.
 *
 * Note1:
 * The primary interfaces are used for updating e.g. in OI, verifying values
 * e.g. in OI and getting values.
 * There are also some help interfaces used for event handling of the 
 * OpenSafLogCurrentConfig information runtime object
 *
 */

#ifndef LOG_LOGD_LGS_CONFIG_H_
#define LOG_LOGD_LGS_CONFIG_H_

#include <stdbool.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "imm/saf/saImmOi.h"
#include "amf/saf/saAmf.h"

#define LGS_CFG_RUNTIME_OBJECT "logConfig=currentConfig,safApp=safLogService"

#define LOG_ROOT_DIRECTORY "logRootDirectory"
#define LOG_DATA_GROUPNAME "logDataGroupname"
#define LOG_MAX_LOGRECSIZE "logMaxLogrecsize"
#define LOG_STREAM_FILE_FORMAT "logStreamFileFormat"
#define LOG_STREAM_SYSTEM_HIGH_LIMIT "logStreamSystemHighLimit"
#define LOG_STREAM_SYSTEM_LOW_LIMIT "logStreamSystemLowLimit"
#define LOG_STREAM_APP_HIGH_LIMIT "logStreamAppHighLimit"
#define LOG_STREAM_APP_LOW_LIMIT "logStreamAppLowLimit"
#define LOG_MAX_APPLICATION_STREAMS "logMaxApplicationStreams"
#define LOG_FILE_IO_TIMEOUT "logFileIoTimeout"
#define LOG_FILE_SYS_CONFIG "logFileSysConfig"
#define LOG_RECORD_DESTINATION_CONFIGURATION "logRecordDestinationConfiguration"
#define LOG_RECORD_DESTINATION_STATUS "logRecordDestinationStatus"

typedef enum {
  LGS_IMM_LOG_ROOT_DIRECTORY,
  LGS_IMM_DATA_GROUPNAME,
  LGS_IMM_LOG_MAX_LOGRECSIZE,
  LGS_IMM_LOG_STREAM_FILE_FORMAT,
  LGS_IMM_LOG_STREAM_SYSTEM_HIGH_LIMIT,
  LGS_IMM_LOG_STREAM_SYSTEM_LOW_LIMIT,
  LGS_IMM_LOG_STREAM_APP_HIGH_LIMIT,
  LGS_IMM_LOG_STREAM_APP_LOW_LIMIT,
  LGS_IMM_LOG_MAX_APPLICATION_STREAMS,
  LGS_IMM_FILE_IO_TIMEOUT,
  LGS_IMM_LOG_FILE_SYS_CONFIG,
  LGS_IMM_LOG_RECORD_DESTINATION_CONFIGURATION,
  LGS_IMM_LOG_RECORD_DESTINATION_STATUS,

  LGS_IMM_LOG_NUMBER_OF_PARAMS,
  LGS_IMM_LOG_OPENSAFLOGCONFIG_CLASS_EXIST,

  LGS_IMM_LOG_NUMEND
} lgs_logconfGet_t;

static inline lgs_logconfGet_t param_name_to_id(std::string param_name) {
  if (param_name == LOG_ROOT_DIRECTORY) {
    return LGS_IMM_LOG_ROOT_DIRECTORY;
  } else if (param_name == LOG_DATA_GROUPNAME) {
    return LGS_IMM_DATA_GROUPNAME;
  } else if (param_name == LOG_MAX_LOGRECSIZE) {
    return LGS_IMM_LOG_MAX_LOGRECSIZE;
  } else if (param_name == LOG_STREAM_FILE_FORMAT) {
    return LGS_IMM_LOG_STREAM_FILE_FORMAT;
  } else if (param_name == LOG_STREAM_SYSTEM_HIGH_LIMIT) {
    return LGS_IMM_LOG_STREAM_SYSTEM_HIGH_LIMIT;
  } else if (param_name == LOG_STREAM_SYSTEM_LOW_LIMIT) {
    return LGS_IMM_LOG_STREAM_SYSTEM_LOW_LIMIT;
  } else if (param_name == LOG_STREAM_APP_HIGH_LIMIT) {
    return LGS_IMM_LOG_STREAM_APP_HIGH_LIMIT;
  } else if (param_name == LOG_STREAM_APP_LOW_LIMIT) {
    return LGS_IMM_LOG_STREAM_APP_LOW_LIMIT;
  } else if (param_name == LOG_MAX_APPLICATION_STREAMS) {
    return LGS_IMM_LOG_MAX_APPLICATION_STREAMS;
  } else if (param_name == LOG_FILE_IO_TIMEOUT) {
    return LGS_IMM_FILE_IO_TIMEOUT;
  } else if (param_name == LOG_FILE_SYS_CONFIG) {
    return LGS_IMM_LOG_FILE_SYS_CONFIG;
  } else if (param_name == LOG_RECORD_DESTINATION_CONFIGURATION) {
    return LGS_IMM_LOG_RECORD_DESTINATION_CONFIGURATION;
  } else if (param_name == LOG_RECORD_DESTINATION_STATUS) {
    return LGS_IMM_LOG_RECORD_DESTINATION_STATUS;
  } else {
    return LGS_IMM_LOG_NUMEND; // Error
  }
}

/**
 * Structure for log server configuration changing data
 */
typedef struct config_chkpt {
  char *ckpt_buffer_ptr;          /* Buffer for config data */
  uint64_t ckpt_buffer_size;      /* Total size of the buffer */
}lgs_config_chg_t;

/* ----------------------------
 * Primary interface functions
 */

/**
 * Read the log service configuration data verify and update configuration
 * data structure
 */
void lgs_cfg_init(SaImmOiHandleT immOiHandle, SaAmfHAStateT ha_state);

/**
 * Get value of log service configuration parameter from the configuration data 
 * struct. The scope of configuration data is restricted to this file.
 * This function gives read access to the configuration data
 * Note: The type of the returned value is void. This means that the reader
 * must know the actual type and convert to correct type.
 *
 * Note1: Multivalue is returned as a void * to a std::vector<type>
 * Example:
 * LGS_IMM_LOG_RECORD_DESTINATION_CONFIGURATION is returned as a void * to
 * std::vector<std::string>
 *
 * @param lgs_logconfGet_t param[in]
 *     Defines what configuration parameter to return
 *
 * @return void *
 *     Pointer to the parameter. See struct lgs_conf
 *
 */
const void *lgs_cfg_get(lgs_logconfGet_t param);

/**
 * Create configuration data buffer
 * Creates a buffer containing all configuration data to be updated.
 * Each time the function is called a configuration parameter is added.
 *
 * The buffer structure can be sent as a check-point message and the updating
 * is done on the Standby. It can also be used in the OI to create an update
 * buffer in the apply callback. Use lgs_cfg_update() for updating the
 * configuration.
 *
 * NOTE1: Parameter name and value is not validated
 * NOTE2: This function allocates memory pointed to by config_data in the
 *        lgs_config_chg_t structure. This memory has to bee freed after usage
 *        Example: free(config_data.ckpt_buffer_ptr);
 * NOTE3: A multi-value is added by iterating the multivalue vector and enter
 *        each value separately using the same attribute name (name_str).
 *        Help functions for handling multi-value can be used.
 *        See lgs_cfgupd_multival_xxx() functions.
 *
 * @param name_str[in]  String containing the parameter name
 * @param value_str[in] Parmeter value as a string
 * @param config_data[out] Filled in config data structure
 *                         NOTE! Must be initiated before first call {NULL, 0}
 *
 */
void lgs_cfgupd_list_create(const char *name_str, char *value_str,
                            lgs_config_chg_t *config_data);

/*******************************************************************************
 * Help functions for handling multi value attributes with the cfgupd list.
 * Multi values can be handled in three ways Add, Delete and Replace
 * In the configuration handler multi values will always be replaced. These
 * help functions will always create a complete list of values and add them
 * to the cfgupd list. Multiple values are added to the list by adding
 * the same attribute multiple times with different values.
 * When reading a list using lgs_cfgupd_list_read() a multi value will be given
 * by returning the same attribute multiple times with different values
 *
 * Note1: The NO_DUPLICATES flag shall be used with multi value attributes
 *
 * Note2: These help functions use lgs_cfgupd_list_create() so information and
 *        notes for lgs_cfgupd_list_create() regarding config_data also applies
 *        here.
 *
 * All functions takes the following parameters:
 *
 * @param attribute_name:
 * The name of the multi value attribute
 *
 * @param value_list:
 * A list of strings where each string represents a value
 *
 * @param config_data
 * See lgs_cfgupd_list_create()
 */

/**
 * Add a list of new values to the existing values in the attribute
 *
 */
void lgs_cfgupd_multival_add(const std::string& attribute_name,
                             const std::vector<std::string>& value_list,
                             lgs_config_chg_t *config_data);

/**
 * Delete the values given in the list from the multi value attribute
 * Note: The attribute shall have the NO_DUPLICATES flag
 *
 */
void lgs_cfgupd_multival_delete(const std::string attribute_name,
                                const std::vector<std::string> value_list,
                                lgs_config_chg_t *config_data);

/**
 * Replace all existing values in the multi value attribute with the values in
 * the list
 */
void lgs_cfgupd_mutival_replace(const std::string attribute_name,
                                const std::vector<std::string> value_list,
                                lgs_config_chg_t *config_data);

/**
 * Read a config update buffer and get parameter name and value.
 * The first time the function is called next_param_ptr shall be
 * a pointer to the buffer containing the config data.
 * See ckpt_buffer_ptr in lgs_config_chg_t.
 * To get the next parameter the next_param_ptr[in] shall be set to the
 * return value from the previous call. NULL is returned when the last parameter
 * is read.
 * The last parameter cfgupd_ptr shall be a pointer to the buffer structure to
 * read
 *
 * NOTE1: The string pointed to by cfgupd_ptr->ckpt_buffer_ptr is changed in
 *        this function! See strtok_r()
 *
 * NOTE2: See lgs_cfgupd_list_create() for info about multi-value attribute
 *
 * @param name_str[out]
 * @param value_str[out]
 * @param next_param_ptr[in]
 * @param cfgupd_ptr[in]
 *
 * @return next_param_ptr
 */
char *lgs_cfgupd_list_read(char **name_str, char **value_str,
                           char *next_param_ptr, lgs_config_chg_t *cfgupd_ptr);

/**
 * Parse a configuration data buffer and update the configuration structure.
 * Used e.g. on Standby when receiving configuration check-point data and by OI
 * to update configuration at modify apply.
 *
 * Comment: Check-pointed configuration data is always sent when the
 *          configuration object is updated or when applying changes in IO.
 *          This means that the corresponding cnfflag is set to LGS_CNF_OBJ
 *
 * NOTE1: Multi-value is always replaced. For information on how to handle
 *        Add, delete and replace see lgs_cfgupd_list_create() and the
 *        multi-value handling help functions
 *        This configuration is stored as a C++ vector of C++ strings
 *        A completely new list is always created if this configuration is
 *        updated. This attribute may have multiple instances in the config_data
 *        list. Each instance will add a value to the vector. If the config_data
 *        list has an empty string as value it means that the multivalue is
 *        empty and shall have an empty vector as value
 *
 * @param config_data[in] Pointer to structure containing configuration
 *        data buffer
 *
 * @return -1 if validation error
 */
int lgs_cfg_update(const lgs_config_chg_t *config_data);

/**
 * Parameter value validation functions. Validates parameters.
 * For more information e.g. validation rules see lgs_conf.cc file
 */
bool lgs_path_is_writeable_dir_h(const std::string &pathname);
int lgs_cfg_verify_root_dir(const std::string &root_str_in);
int lgs_cfg_verify_log_data_groupname(char *group_name);
int lgs_cfg_verify_log_file_format(const char* log_file_format);
int lgs_cfg_verify_max_logrecsize(uint32_t max_logrecsize_in);
int lgs_cfg_verify_mbox_limit(uint32_t high, uint32_t low);
int lgs_cfg_verify_max_application_streams(uint32_t max_app_streams);
int lgs_cfg_verify_file_io_timeout(uint32_t log_file_io_timeout);
int lgs_cfg_verify_log_record_destination_configuration(
    std::vector<std::string>& vdest,
    SaImmAttrModificationTypeT type);
/*
 * Functions for updating some parameters. Used to support check-point before
 * version 5
 */
void lgs_rootpathconf_set(const std::string &root_path_str);
void lgs_groupnameconf_set(const char *data_groupname_str);

/*
 * Handle runtime object for showing actual configuration and configuration
 * related information
 */
void conf_runtime_obj_create(SaImmOiHandleT immOiHandle);
void conf_runtime_obj_hdl(SaImmOiHandleT immOiHandle, const SaImmAttrNameT *attributeNames);

/*
 *  Trace functions
 */
void lgs_trace_config();
void lgs_cfg_read_trace();

#endif  // LOG_LOGD_LGS_CONFIG_H_

