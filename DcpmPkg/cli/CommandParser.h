/*
 * Copyright (c) 2018, Intel Corporation.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _COMMAND_PARSER_H_
#define _COMMAND_PARSER_H_

#include <Uefi.h>
#include <Debug.h>
#include <Types.h>
#include <Printer.h>
#include <Utility.h>

#define DISP_NAME_LEN             32    //!< Display string length (used when formatting output in alternative formats)
#define DISP_DELIMS_LEN           10    //!< Deliminter string length (used when formatting output in alternative formats)
#define VERB_LEN                  16    //!< Verb string length
#define TARGET_LEN                32    //!< Target name string length
#define TARGET_VALUE_LEN          4096  //!< Target value string length for maximum-possible DIMM IDs
#define OPTION_LEN                16    //!< Option name string length
#define OPTION_VALUE_LEN          1024  //!< Option value string length
#define PARSER_OPTION_VALUE_LEN   2048  //!< Option value string length for command parser
#define PROPERTY_KEY_LEN          128   //!< Property name string length
#define PROPERTY_VALUE_LEN        128   //!< Property value string length
#define MAX_TARGETS               8     //!< Maximum number of targets in a single command
#define MAX_OPTIONS               12     //!< Maximum number of options in a single command
#define MAX_PROPERTIES            20    //!< Maximum number of properties in a single command
#define MAX_TOKENS                50    //!< Maximum number of tokens per line

/** command keywords **/
#define LOAD_VERB     L"load"
#define HELP_VERB     L"help"
#define VERSION_VERB  L"version"
#define SHOW_VERB     L"show"
#define SET_VERB      L"set"
#define DELETE_VERB   L"delete"
#define CREATE_VERB   L"create"
#define DUMP_VERB     L"dump"
#define START_VERB    L"start"
#define STOP_VERB     L"stop"

/** command options **/
#define ALL_OPTION                      L"-all"                                //!< 'all' option name
#define ALL_OPTION_SHORT                L"-a"                                  //!< 'all' option short form
#define ALL_OPTION_HELP                 L"Show all attributes"                 //!< 'all' option help text
#define DISPLAY_OPTION                  L"-display"                            //!< 'display' option name
#define DISPLAY_OPTION_SHORT            L"-d"                                  //!< 'display' option short form
#define DISPLAY_OPTION_HELP             L"Show specific attributes"            //!< 'display' option help text
#define HELP_OPTION                     L"-help"                               //!< 'help' option name
#define HELP_OPTION_SHORT               L"-h"                                  //!< 'help' option short form
#define SOURCE_OPTION                   L"-source"                             //!< 'source' option name
#define SOURCE_OPTION_HELP              L"path"                                //!< 'source' option help text
#define DESTINATION_OPTION              L"-destination"
#define DESTINATION_OPTION_HELP         L"file"
#define DESTINATION_PREFIX_OPTION       L"-destination"
#define DESTINATION_PREFIX_OPTION_HELP  L"file_prefix (prefix for output files)"
#define DICTIONARY_OPTION               L"-dict"
#define DICTIONARY_OPTION_HELP          L"file"
#define EXAMINE_OPTION                  L"-examine"                            //!< 'examine' option name
#define EXAMINE_OPTION_SHORT            L"-x"                                  //!< 'examine' option short form
#define EXAMINE_OPTION_DETAILS_TEXT     L"Test the provided firmware image for compatibility"  //!< 'examine' option detailed help text
#define FORCE_OPTION                    L"-force"                              //!< 'force' option name
#define FORCE_OPTION_SHORT              L"-f"                                  //!< 'force' option short form
#define FORCE_OPTION_DETAILS_TEXT       L"Suppress confirmations"              //!< 'force' option help text detail
#define RECOVER_OPTION                  L"-recover"                            //!< 'recover' option name
#define RECOVER_OPTION_FLASH_SPI        L"FlashSPI"                            //!< 'recover' option value to FlashSpi during update
#define RECOVER_OPTION_DETAILS_TEXT     L"Run update on non-functional " PMEM_MODULES_STR L" only (deprecated) or run SPI Flash recovery"     //!< 'recover' option value to FlashSpi during update
#define UNITS_OPTION                    L"-units"                              //!< 'units' option name
#define UNITS_OPTION_B                  L"B"                                   //!< 'units' option value for B
#define UNITS_OPTION_MB                 L"MB"                                  //!< 'units' option value for MB
#define UNITS_OPTION_MIB                L"MiB"                                 //!< 'units' option value for MiB
#define UNITS_OPTION_GB                 L"GB"                                  //!< 'units' option value for GB
#define UNITS_OPTION_GIB                L"GiB"                                 //!< 'units' option value for GiB
#define UNITS_OPTION_TB                 L"TB"                                  //!< 'units' option value for TB
#define UNITS_OPTION_TIB                L"TiB"                                 //!< 'units' option value for TiB
#define UNITS_OPTION_HELP               L"B|MB|MiB|GB|GiB|TB|TiB"              //!< 'units' option help text
#define UNITS_OPTION_SHORT              L"-u"                                  //!< 'units' option short form
#define PROPERTY_VALUE_0_1_HELP         L"0|1"                                 //!< Property 0 or 1 value
#define PROPERTY_VALUE_NO_YES_IGN_HELP  L"No|Yes|Ignore"                       //!< Property: No, Yes or Ignore
#define OUTPUT_OPTION_SHORT             L"-o"                                  //!< 'output' option name short form
#define OUTPUT_OPTION                   L"-output"                             //!< 'output' option
#define OUTPUT_OPTION_TEXT              L"text"                                //!< 'output' option value for text
#define OUTPUT_OPTION_NVMXML            L"nvmxml"                              //!< 'output' option value for nvmxml
#define OUTPUT_OPTION_ESX_XML           L"esx"                                 //!< 'output' option value for esx xml
#define OUTPUT_OPTION_ESX_TABLE_XML     L"esxtable"                            //!< 'output' option value for esx xml
#define OUTPUT_OPTION_HELP              L"text|nvmxml"                         //!< 'output' option help text
#define VERBOSE_OPTION_SHORT            L"-v"                                  //!< 'verbose' option short form
#define VERBOSE_OPTION                  L"-verbose"                            //!< 'verbose' option name
#define MASTER_OPTION                   L"-master"                             //!< 'master' option name
#define DEFAULT_OPTION                  L"-default"                            //!< 'default' option name
#define PBR_MODE_OPTION                 L"-mode"                               //!< 'mode' option name
#define PROTOCOL_OPTION_DDRT            L"-ddrt"                               //!< 'ddrt' option name
#define PROTOCOL_OPTION_SMBUS           L"-smbus"                              //!< 'smbus' option name
#define LARGE_PAYLOAD_OPTION            L"-lpmb"                               //!< 'large payload mailbox' option name
#define SMALL_PAYLOAD_OPTION            L"-spmb"                               //!< 'small payload mailbox' option name
#define NFIT_OPTION                     L"-nfit"                               //!< 'nfit' option name

/** command targets **/
#define DIMM_TARGET                          L"-dimm"                    //!< 'dimm' target name
#define REGION_TARGET                        L"-region"                  //!< 'region' target name
#define MEMORY_RESOURCES_TARGET              L"-memoryresources"         //!< 'memoryresources' target name
#define SYSTEM_TARGET                        L"-system"                  //!< 'system' target name
#define CAPABILITIES_TARGET                  L"-capabilities"            //!< 'capabilities' target name
#define SOCKET_TARGET                        L"-socket"                  //!< 'socket' target name
#define GOAL_TARGET                          L"-goal"                    //!< 'goal' target name
#define CAP_TARGET                           L"-cap"                     //!< 'cap' target name
#define NAMESPACE_TARGET                     L"-namespace"               //!< 'namespace' target name
#define HOST_TARGET                          L"-host"                    //!< 'host' target name
#define TOPOLOGY_TARGET                      L"-topology"                //!< 'topology' target name
#define CONFIG_TARGET                        L"-config"                  //!< 'config' target name
#define SENSOR_TARGET                        L"-sensor"                  //!< 'sensor' target name
#define ERROR_TARGET                         L"-error"                   //!< 'error' target name
#define CEL_TARGET                         L"-cel"                   //!< 'cel' target name
#define DEBUG_TARGET                         L"-debug"                   //!< 'debug' target name
#define REGISTER_TARGET                      L"-register"                //!< 'register' target name
#define FIRMWARE_TARGET                      L"-firmware"                //!< 'firmware' target name
#define PCD_TARGET                           L"-pcd"                     //!< 'pcd' target name
#define SMBIOS_TARGET                        L"-smbios"                  ///< 'smbios' target name
#define SUPPORT_TARGET                       L"-support"                 //!< 'support' target name
#define CONTROLLER_TEMPERATURE_TARGET_VALUE  L"ControllerTemperature"    //!< 'sensor' target value
#define MEDIA_TEMPERATURE_TARGET_VALUE       L"MediaTemperature"         //!< 'sensor' target value
#define SPARE_CAPACITY_TARGET_VALUE          L"PercentageRemaining"      //!< 'sensor' target value
#define SENSOR_TARGETS \
  L"MediaTemperature|ControllerTemperature|PercentageRemaining"          //!< the sensors combined for the target message
#define DIAGNOSTIC_TARGET                    L"-diagnostic"              //!< 'diagnostic' target name
#define ALL_TEST_TARGET_VALUE                L"All"                      //!< 'diagnostic' target value
#define QUICK_TEST_TARGET_VALUE              L"Quick"                    //!< 'diagnostic' target value
#define CONFIG_TEST_TARGET_VALUE             L"Config"                   //!< 'diagnostic' target value
#define SECURITY_TEST_TARGET_VALUE           L"Security"                 //!< 'diagnostic' target value
#define FW_TEST_TARGET_VALUE                 L"FW"                       //!< 'diagnostic' target value
#define ERROR_TARGET_THERMAL_VALUE           L"Thermal"                  //!< 'error' target value
#define ERROR_TARGET_MEDIA_VALUE             L"Media"                    //!< 'error' target value
#define ALL_DIAGNOSTICS_TARGETS              L"Quick|Config|Security|FW" //!< diagnostics targets combined
#define PCD_CONFIG_TARGET_VALUE              L"Config"
#define PCD_LSA_TARGET_VALUE                 L"LSA"
#define NFIT_TARGET_VALUE                    L"NFIT"                     //!< 'system' target value
#define PCAT_TARGET_VALUE                    L"PCAT"                     //!< 'system' target value
#define PMTT_TARGET_VALUE                    L"PMTT"                     //!< 'system' target value
#define SYSTEM_ACPI_TARGETS \
  L"NFIT|PCAT|PMTT"                                                           //!< the system acpi combined
#define SMBIOS_TARGET_VALUES                 L"17"                    ///< 'smbios' target values
#define FORMAT_TARGET                        L"-format"                  //!< 'format' target value
#define PREFERENCES_TARGET                   L"-preferences"             //!< 'preferences' target value
#define PERFORMANCE_TARGET                   L"-performance"             //!< 'performance' target value
#define SESSION_TARGET                       L"-session"                 //!< 'session' target value
#define PBR_MODE_TARGET                      L"-mode"                    //!< 'mode' target value
#define PBR_RECORD_MODE_VAL                  L"record"                   //!< 'mode' target value
#define PBR_PLAYBACK_MODE_VAL                L"playback"                 //!< 'mode' target value
#define PBR_PLAYBACK_MANUAL_MODE_VAL         L"playback_manual"          //!< 'mode' target value
#define PBR_MODE_TAG                         L"-tag"                     //!< 'tag' target value
/** Persistent memory type **/
#define PERSISTENT_MEM_TYPE_AD_STR        L"AppDirect"
#define PERSISTENT_MEM_TYPE_AD_NI_STR     L"AppDirectNotInterleaved"

/** command properties **/
#define TYPE_PROPERTY                     L"Type"                     //!< 'Type' property name
#define TYPE_VALUE_FW                     L"Fw"                       //!< 'Type' property FW value
#define TYPE_VALUE_TRAINGING              L"Training"                 //!< 'Type' property Training value
#define UPDATE_PROPERTY                   L"Update"                   //!< 'Update' property name
#define EXEC_PROPERTY                     L"Execute"                  //!< 'Exec' property name
#define TEMPERATURE_INJ_PROPERTY          L"Temperature"              //!<  Inject error 'Temperature' property name
#define POISON_INJ_PROPERTY               L"Poison"                   //!< Inject error 'Poison' property
#define POISON_TYPE_INJ_PROPERTY          L"PoisonType"               //!< Inject error 'PoisonType' property
#define CLEAR_ERROR_INJ_PROPERTY          L"Clear"                    //!< Clear error injection property
#define PACKAGE_SPARING_INJ_PROPERTY      L"PackageSparing"           //!< PackageSparing error injection property
#define PERCENTAGE_REAMAINING_INJ_PROPERTY       L"PercentageRemaining"      //!< PercentageRemaining error injection property
#define FATAL_MEDIA_ERROR_INJ_PROPERTY    L"FatalMediaError"          //!< FatalMediaError error injection property
#define DIRTY_SHUTDOWN_ERROR_INJ_PROPERTY L"DirtyShutdown"            //!< DirtyShutdown error injection property
#define LOCKSTATE_PROPERTY                L"LockState"                //!< 'LockState' property name
#define LOCKSTATE_VALUE_ENABLED           L"Enabled"                  //!< 'LockState' property Enabled value
#define LOCKSTATE_VALUE_DISABLED          L"Disabled"                 //!< 'LockState' property Disabled value
#define LOCKSTATE_VALUE_UNLOCKED          L"Unlocked"                 //!< 'LockState' property Unlocked value
#define LOCKSTATE_VALUE_FROZEN            L"Frozen"                   //!< 'LockState' property Frozen value
#define CONFIG_STATUS_VALUE_VALID               L"Valid"                        //!< 'ConfigStatus' property Valid value
#define CONFIG_STATUS_VALUE_NOT_CONFIG          L"Not configured"               //!< 'ConfigStatus' property Not Configured value
#define CONFIG_STATUS_VALUE_BAD_CONFIG          L"Failed - Bad configuration"   //!< 'ConfigStatus' property Bad Configuration value
#define CONFIG_STATUS_VALUE_BROKEN_INTERLEAVE   L"Failed - Broken interleave"   //!< 'ConfigStatus' property Broken Interleave value
#define CONFIG_STATUS_VALUE_REVERTED            L"Failed - Reverted"            //!< 'ConfigStatus' property Reverted value
#define CONFIG_STATUS_VALUE_UNSUPPORTED         L"Failed - Unsupported"         //!< 'ConfigStatus' property Unsupported value
#define CONFIG_STATUS_VALUE_PARTIALLY_SUPPORTED L"Failed - Partially supported" //!< 'ConfigStatus' property Partially Supported value
#define PASSPHRASE_PROPERTY               L"Passphrase"               //!< 'Passphrase' property name
#define NEWPASSPHRASE_PROPERTY            L"NewPassphrase"            //!< 'NewPassphrase' property name
#define CONFIRMPASSPHRASE_PROPERTY        L"ConfirmPassphrase"        //!< 'ConfirmPassphrase' property name
#define ALARM_THRESHOLD_PROPERTY          L"AlarmThreshold"           //!< 'AlarmThreshold' property
#define ALARM_ENABLED_PROPERTY            L"AlarmEnabled"             //!< 'AlarmEnabled' property
#define MEMORY_MODE_PROPERTY              L"MemoryMode"               //!< 'MemoryMode' property name
#define PERSISTENT_MEM_TYPE_PROPERTY      L"PersistentMemoryType"     //!< 'PersistentMemoryType' property name
#define MEMORY_SIZE_PROPERTY              L"MemorySize"               //!< 'MemorySize' property name
#define RESERVED_PROPERTY                 L"Reserved"                 //!< 'Reserved' property name
#define APPDIRECT_SIZE_PROPERTY           L"AppDirectSize"            //!< 'AppDirectSize' property name
#define APPDIRECT_INDEX_PROPERTY          L"AppDirectIndex"           //!< 'AppDirectIndex ' property name
#define APPDIRECT_1_SIZE_PROPERTY         L"AppDirect1Size"           //!< 'AppDirect1Size' property name
#define APPDIRECT_1_SETTINGS_PROPERTY     L"AppDirect1Settings"       //!< 'AppDirect1Setting' property name
#define APPDIRECT_1_INDEX_PROPERTY        L"AppDirect1Index"          //!< 'AppDirect1Index' property name
#define APPDIRECT_2_SIZE_PROPERTY         L"AppDirect2Size"           //!< 'AppDirect2Size' property name
#define APPDIRECT_2_SETTINGS_PROPERTY     L"AppDirect2Settings"       //!< 'AppDirect2Setting' property name
#define APPDIRECT_2_INDEX_PROPERTY        L"AppDirect2Index"          //!< 'AppDirect2Index' property name
#define MEM_INFO_PAGE_PROPERTY            L"Page"                     //!< 'MemoryInfo page' property name
#define LOG_PROPERTY                      L"Log"                      //!< 'Log' property name
#define PROPERTY_VALUE_0                  L"0"                        //!< Property 0 value
#define PROPERTY_VALUE_1                  L"1"                        //!< Property 1 value
#define PROPERTY_VALUE_IGNORE             L"Ignore"                   //!< Property 'Ignore' value
#define PROPERTY_VALUE_NO                 L"No"                       //!< Property 'No' value
#define PROPERTY_VALUE_YES                L"Yes"                      //!< Property 'Yes' value
#define PROPERTY_VALUE_ENABLED            L"Enabled"                  //!< Property enabled value
#define PROPERTY_VALUE_DISABLED           L"Disabled"                 //!< Property disabled value
#define SEQUENCE_NUM_PROPERTY             L"SequenceNumber"           //!< 'error' property name
#define COUNT_PROPERTY                    L"Count"                    //!< 'error' property name
#define LEVEL_PROPERTY                    L"Level"                    //!< 'error' property name
#define LEVEL_HIGH_PROPERTY_VALUE         L"High"                     //!< 'error' property 'Level' value
#define LEVEL_LOW_PROPERTY_VALUE          L"Low"                      //!< 'error' property 'Level' value
#define NAMESPACE_ID_PROPERTY             L"NamespaceId"
#define NAMESPACE_GUID_PROPERTY           L"NamespaceGuid"
#define CAPACITY_PROPERTY                 L"Capacity"
#define NAME_PROPERTY                     L"Name"
#define HEALTH_PROPERTY                   L"HealthState"
#define REGION_ID_PROPERTY                L"RegionID"
#define BLOCK_SIZE_PROPERTY               L"BlockSize"
#define BLOCK_COUNT_PROPERTY              L"BlockCount"
#define MODE_PROPERTY                     L"Mode"
#define PROPERTY_VALUE_NONE               L"None"
#define PROPERTY_VALUE_SECTOR             L"Sector"
#define AVG_PWR_REPORTING_TIME_CONSTANT_MULT_PROPERTY L"AveragePowerReportingTimeConstantMultiplier"
#define AVG_PWR_REPORTING_TIME_CONSTANT   L"AveragePowerReportingTimeConstant"
#define ACCESS_TYPE_PROPERTY              L"AccessType"
#define ERASE_CAPABLE_PROPERTY            L"EraseCapable"
#define ENCRYPTION_PROPERTY               L"Encryption"
#define CLI_DEFAULT_DIMM_ID_PROPERTY      L"CLI_DEFAULT_DIMM_ID"
#define CLI_DEFAULT_SIZE_PROPERTY         L"CLI_DEFAULT_SIZE"
#define APP_DIRECT_SETTINGS_PROPERTY      L"APPDIRECT_SETTINGS"
#define LABEL_VERSION_PROPERTY            L"LabelVersion"
#define NS_LABEL_VERSION_PROPERTY         L"NamespaceLabelVersion"
#define SEVERITY_PROPERTY                 L"Severity"
#define PROPERTY_VALUE_UID                L"UID"
#define PROPERTY_VALUE_HANDLE             L"HANDLE"
#define PROPERTY_VALUE_AUTO               L"AUTO"
#define PROPERTY_VALUE_AUTO10             L"AUTO_10"
#define PROPERTY_VALUE_RECOMMENDED        L"RECOMMENDED"
#define CATEGORY_PROPERTY                 L"Category"
#define DBG_LOG_LEVEL                     L"DBG_LOG_LEVEL"
#define CREATE_SUPP_NAME                  L"Name"
#define PROPERTY_ERROR_UNKNOWN                      L"Reason for failure unknown"
#define PROPERTY_ERROR_DEFAULT_DIMM_NOT_PROVIDED    L"Default DimmID Type not provided"
#define PROPERTY_ERROR_INCORRECT_DEFAULT_DIMM_TYPE  L"Incorrect default DimmID type"
#define PROPERTY_ERROR_DISPLAY_DEFAULT_NOT_PROVIDED L"Display default size type not provided"
#define PROPERTY_ERROR_DEFAULT_INCORRECT_SIZE_TYPE  L"Incorrect default size type"
#define PROPERTY_ERROR_INTERLEAVE_TYPE_NOT_PROVIDED L"AppDirect interleave setting type not provided"
#define PROPERTY_ERROR_APPDIR_INTERLEAVE_TYPE       L"Incorrect AppDirect interleave setting type"
#define PROPERTY_ERROR_GRANULARITY_NOT_PROVIDED     L"AppDirect Granularity setting type not provided"
#define PROPERTY_ERROR_INVALID_GRANULARITY          L"Invalid granularity"
#define PROPERTY_ERROR_INVALID_OUT_OF_RANGE         L"Setting is invalid or out of range"
#define PROPERTY_ERROR_SET_FAILED_UNKNOWN           L"Set operation failed"

/** Performance Metric Messages **/
#define DCPMM_PERFORMANCE_MEDIA_READS             L"MediaReads"
#define DCPMM_PERFORMANCE_MEDIA_WRITES            L"MediaWrites"
#define DCPMM_PERFORMANCE_READ_REQUESTS           L"ReadRequests"
#define DCPMM_PERFORMANCE_WRITE_REQUESTS          L"WriteRequests"
#define DCPMM_PERFORMANCE_TOTAL_MEDIA_READS       L"TotalMediaReads"
#define DCPMM_PERFORMANCE_TOTAL_MEDIA_WRITES      L"TotalMediaWrites"
#define DCPMM_PERFORMANCE_TOTAL_READ_REQUESTS     L"TotalReadRequests"
#define DCPMM_PERFORMANCE_TOTAL_WRITE_REQUESTS    L"TotalWriteRequests"

/** Sensor Detail Messages **/
#define DIMM_HEALTH_STR_DETAIL                       L"Health - The current " PMEM_MODULE_STR L" health as reported in the SMART log"
#define MEDIA_TEMPERATURE_STR_DETAIL                 L"MediaTemperature - The current " PMEM_MODULE_STR L" media temperature in Celsius"
#define CONTROLLER_TEMPERATURE_STR_DETAIL            L"ControllerTemperature - The current " PMEM_MODULE_STR " controller temperature in Celsius"
#define SPARE_CAPACITY_STR_DETAIL                    L"PercentageRemaining - Remaining " PMEM_MODULES_STR L" life as a percentage value of factory expected\
 life spa"
#define LATCHED_DIRTY_SHUTDOWN_COUNT_STR_DETAIL      L"LatchedDirtyShutdownCount - The number of shutdowns without notification over the lifetime of\
 the " PMEM_MODULE_STR
#define UNLATCHED_DIRTY_SHUTDOWN_COUNT_STR_DETAIL    L"UnlatchedDirtyShutdownCount - The number of shutdowns without notification over the lifetime of\
 the " PMEM_MODULE_STR L"."
#define POWER_ON_TIME_STR_DETAIL                     L"PowerOnTime - The total power-on time over the lifetime of the " PMEM_MODULE_STR
#define UPTIME_STR_DETAIL                            L"UpTime - The total power-on time since the last power cycle of the " PMEM_MODULE_STR
#define POWER_CYCLES_STR_DETAIL                      L"PowerCycles - The number of power cycles over the lifetime of the " PMEM_MODULE_STR
#define FW_ERROR_COUNT_STR_DETAIL                    L"FwErrorCount - The total number of firmware error log entries"


/** common help messages **/
#define HELP_OPTIONS_DETAILS_TEXT       L"Changes the output format."
#define HELP_VERBOSE_DETAILS_TEXT       L"Change the Debug Level Message Display"
#define HELP_ALL_DETAILS_TEXT           L"Shows all attributes."
#define HELP_DISPLAY_DETAILS_TEXT       L"Shows attributes specified in a comma-separated list"
#define HELP_FORCE_DETAILS_TEXT         L"Suppresses the confirmation from the User to use this operation"
#define HELP_UNIT_DETAILS_TEXT          L"Desired Unit for display"
#define HELP_DDRT_DETAILS_TEXT          L"Used to specify DDRT as the desired transport protocol"
#define HELP_SMBUS_DETAILS_TEXT         L"Used to specify SMBUS as the desired transport protocol"
#define HELP_LPAYLOAD_DETAILS_TEXT      L"Used to specify large transport payload size"
#define HELP_SPAYLOAD_DETAILS_TEXT      L"Used to specify small transport payload size"
#define HELP_TEXT_DIMM_IDS              L"DimmIDs"
#define HELP_TEXT_DIMM_ID               L"DimmID"
#define HELP_TEXT_ATTRIBUTES            L"Attributes"
#define HELP_TEXT_REGION_ID             L"RegionID"
#define HELP_TEXT_SOCKET_IDS            L"SocketIDs"
#define HELP_TEXT_SENSORS               L"List of Sensors"
#define HELP_TEXT_VALUE                 L"value"
#define HELP_TEXT_COUNT                 L"count"
#define HELP_TEXT_GiB                   L"GiB"
#define HELP_TEXT_GB                    L"GB"
#define HELP_TEXT_STRING                L"string"
#define HELP_TEXT_ERROR_LOG             L"Thermal|Media"
#define HELP_TEXT_PERCENT               L"0|%%"
#define HELP_TEXT_APPDIRECT_SETTINGS    PROPERTY_VALUE_RECOMMENDED L"|" L"(IMCSize)_(ChannelSize)"
#define HELP_TEXT_NO_MIRROR_APPDIRECT_SETTINGS L"ByOne|(iMCSize)_(ChannelSize)"
#define HELP_TEXT_NS_LABEL_VERSION      L"1.1|1.2"
#define HELP_NFIT_DETAILS_TEXT         L"Used to specify NFIT as the source"
#define HELP_TEXT_DEFAULT_SIZE          PROPERTY_VALUE_AUTO   L"|" \
                                        PROPERTY_VALUE_AUTO10 L"|" \
                                        UNITS_OPTION_B        L"|" \
                                        UNITS_OPTION_MB       L"|" \
                                        UNITS_OPTION_MIB      L"|" \
                                        UNITS_OPTION_GB       L"|" \
                                        UNITS_OPTION_GIB      L"|" \
                                        UNITS_OPTION_TB       L"|" \
                                        UNITS_OPTION_TIB
#define HELP_TEXT_PERSISTENT_MEM_TYPE   L"AppDirect|AppDirectNotInterleaved"
#define HELP_TEXT_FLASH_SPI             L"FlashSPI"
#define HELP_DBG_LOG_LEVEL              L"log level"
#define HELP_TEXT_PERFORMANCE_CAT       L"Performance Metrics"

#define HELP_TEXT_AVG_PWR_REPORTING_TIME_CONSTANT_MULT_PROPERTY     L"<0, 32>"
#define HELP_TEXT_AVG_PWR_REPORTING_TIME_CONSTANT_PROPERTY          L"<100, 12000>"

#define HELP_TEXT_PERFORMANCE_CAT_DETAILS  L"\n    "DCPMM_PERFORMANCE_MEDIA_READS \
                                           L"\n    "DCPMM_PERFORMANCE_MEDIA_WRITES \
                                           L"\n    "DCPMM_PERFORMANCE_READ_REQUESTS \
                                           L"\n    "DCPMM_PERFORMANCE_WRITE_REQUESTS\
                                           L"\n    "DCPMM_PERFORMANCE_TOTAL_MEDIA_READS \
                                           L"\n    "DCPMM_PERFORMANCE_TOTAL_MEDIA_WRITES\
                                           L"\n    "DCPMM_PERFORMANCE_TOTAL_READ_REQUESTS\
                                           L"\n    "DCPMM_PERFORMANCE_TOTAL_WRITE_REQUESTS

#define HELP_TEXT_SENSORS_SHORT  L"\n    "MEDIA_TEMPERATURE_STR_DETAIL \
                                 L"\n    "CONTROLLER_TEMPERATURE_STR_DETAIL \
                                 L"\n    "SPARE_CAPACITY_STR_DETAIL

#define HELP_TEXT_SENSORS_ALL    L"\n    "DIMM_HEALTH_STR_DETAIL \
                                 L"\n    "MEDIA_TEMPERATURE_STR_DETAIL \
                                 L"\n    "CONTROLLER_TEMPERATURE_STR_DETAIL \
                                 L"\n    "SPARE_CAPACITY_STR_DETAIL \
                                 L"\n    "LATCHED_DIRTY_SHUTDOWN_COUNT_STR_DETAIL \
                                 L"\n    "UNLATCHED_DIRTY_SHUTDOWN_COUNT_STR_DETAIL \
                                 L"\n    "POWER_ON_TIME_STR_DETAIL \
                                 L"\n    "UPTIME_STR_DETAIL \
                                 L"\n    "POWER_CYCLES_STR_DETAIL \
                                 L"\n    "FW_ERROR_COUNT_STR_DETAIL
enum ValueRequirementType
{
  ValueEmpty = 1,
  ValueOptional = 2,
  ValueRequired = 3
};

/**
  Defines a single option of a CLI command
**/
struct option
{
  CHAR16 OptionNameShort[OPTION_LEN];
  CHAR16 OptionName[OPTION_LEN];
  CHAR16 *pOptionValueStr;
  CONST CHAR16 *pHelp;
  CONST CHAR16 *pHelpDetails;
  BOOLEAN Required;
  UINT8 ValueRequirement;
};

/**
  Defines a single target of a CLI command
**/
struct target
{
  CHAR16 TargetName[TARGET_LEN];
  CHAR16 *pTargetValueStr;
  CONST CHAR16 *pHelp;
  BOOLEAN Required;
  UINT8 ValueRequirement;
};

/**
  Defines a single property of a CLI command
**/
struct property
{
  CHAR16 PropertyName[PROPERTY_KEY_LEN];
  CHAR16 PropertyValue[PROPERTY_VALUE_LEN];
  CONST CHAR16 *pHelp;
  BOOLEAN Required;
  UINT8 ValueRequirement;
};

enum DisplayType {
  ResultsView   = 0,
  ListView      = 1,
  ListView2L    = 2,
  TableView     = 3,
  TableTabView  = 4,
  ErrorView     = 5,
  HelpView      = 6,
  DiagView      = 7
};


/**
  Defines the parts of a CLI command
**/
typedef
struct Command
{
  CHAR16 verb[VERB_LEN];
  struct option options[MAX_OPTIONS];
  struct target targets[MAX_TARGETS];
  struct property properties[MAX_PROPERTIES];
  CONST CHAR16 *pHelp;
  EFI_STATUS (*run)(struct Command *pCmd); //!< Execute the command
  BOOLEAN PrinterCtrlSupported;
  BOOLEAN ExcludeDriverBinding;
  BOOLEAN Hidden; //!< Never print
  BOOLEAN ShowHelp;
  BOOLEAN SyntaxErrorHelpNeeded;
  BOOLEAN VerbMatch;
  UINT8 CommandId;
  UINT8 DispType;
  CHAR16 DispName[DISP_NAME_LEN];
  PRINT_CONTEXT *pPrintCtx;
} COMMAND;

typedef
struct CommandInput
{
  UINT32 TokenCount;
  CHAR16 **ppTokens;
} COMMAND_INPUT;

typedef
struct _DispInfo
{
   CHAR16 Name[DISP_NAME_LEN];
   UINT8 Type;
   CHAR16 Delims[DISP_DELIMS_LEN];
}DispInfo;

extern DispInfo gDisplayInfo;
/**
  Add the specified command to the list of supported commands
**/
EFI_STATUS RegisterCommand(struct Command *pCommand);

/**
  Free the allocated memory for target values
  in  the CLI command structure.

  @param[in out] pCommand pointer to the command structure
**/
VOID
FreeCommandStructure(
  IN OUT COMMAND *pCommand
  );

/**
  The function parse the input string and split it to the tokens

  The caller function is responsible for deallocation of pCmdInput. The FreeCommandInput function should be
  used to deallocate memory.

  @param[in]  pCommand  The input string
  @param[out] pCmdInput
**/
VOID
FillCommandInput(
  IN     CHAR16 *pCommand,
     OUT struct CommandInput *pCmdInput
  );

/**
  Clean up the resources associated with the command list
**/
void FreeCommands();

/**
  Clean up the resources associated with the input
**/
void FreeCommandInput(struct CommandInput *pCommandInput);

/**
  Parse the given the command line arguments to
  identify the correct command.
  It is the responsibility of the caller function to free the allocated
  memory for target values in the Command structure.

  @param[in] the command input
  @param[in,out] p_command
  @return
  The results of the command execution or a syntax error
**/
EFI_STATUS Parse(struct CommandInput *pInput, struct Command *pCommand);

/**
  If parsing fails, retrieve a more useful syntax error
**/
CHAR16 *getSyntaxError();

/**
 If parsing fails, set syntax error, but first free old one
**/
VOID SetSyntaxError(
  IN      CHAR16 *pSyntaxError
);

/**
  Get the help for a command read from the user.

  @param[in] pCommand a pointer to the parsed struct Command.
  @param[in] SingleCommand a BOOLEAN flag indicating if we are
    trying to match to a single command help or to all commands
    with the same verb.

  @retval NULL if the command verb could not be matched to any
    of the registered commands. Or the pointer to the help message.

  NOTE: If the return pointer is not NULL, the caller is responsible
  to free the memory using FreePool.
**/
CHAR16
*getCommandHelp(
  IN     struct Command *pCommand,
  BOOLEAN SingleCommand
  );

CHAR16
*getOverallCommandHelp();

/**
  Checks if the Unicode string contains the given character.

    @param[in] Character is the 16-bit character that we are searching for.
    @param[in] pInputString is the Unicode (16-bit) string that we want to check.

    @retval TRUE if the input string contains the character we are searching for.
    @retval FALSE if the character is not present in the string.
**/
BOOLEAN
ContainsCharacter(
  IN     CHAR16 Character,
  IN     CONST CHAR16* pInputString
  );

/**
  Get the number of properties
    @param[in] pCmd is a pointer to the struct Command that contains the user input.
    @param[out] pPropertyCount represents the number of properties defined on the command line

    @retval EFI_INVALID_PARAMETER if any of the parameters is a NULL.
    @retval EFI_SUCCESS
**/
EFI_STATUS
GetPropertyCount(
  IN     CONST struct Command *pCmd,
  IN     UINT16 *pPropertyCount
);

/**
  Check if a specific property is found
    @param[in] pCmd is a pointer to the struct Command that contains the user input.
    @param[in] pProperty is a CHAR16 string that represents the property we want to find.

    @retval EFI_SUCCESS if we've found the property.
    @retval EFI_NOT_FOUND if no such property exists for the given pCmd.
    @retval EFI_INVALID_PARAMETER if any of the parameters is a NULL.
**/
EFI_STATUS
ContainsProperty(
  IN     CONST struct Command *pCmd,
  IN     CONST CHAR16 *pProperty
  );

/**
  Get a specific property value
    @param[in] pCmd is a pointer to the struct Command that contains the user input.
    @param[in] pProperty is a CHAR16 string that represents the property we want to find.
    @param[out] ppReturnValue is a pointer to a pointer to the 16-bit character string
        that will contain the return property value.

    @retval EFI_SUCCESS if we've found the property and the value is set.
    @retval EFI_NOT_FOUND if no such property exists for the given pCmd.
    @retval EFI_INVALID_PARAMETER if any of the parameters is a NULL.
**/
EFI_STATUS
GetPropertyValue(
  IN     CONST struct Command *pCmd,
  IN     CONST CHAR16 *pProperty,
     OUT CHAR16 **ppReturnValue
  );

/**
  Check if a specific option is found
**/
BOOLEAN containsOption(CONST struct Command *pCmd, CONST CHAR16 *optionName);

/**
  Check if a specific target is found in the command

  @param[in] pCmd
  @param[in] pTarget

  @retval TRUE if the target has been found
  @retval FALSE if the target has not been found
**/
BOOLEAN
ContainTarget(
  IN CONST struct Command *pCmd,
  IN CONST CHAR16 *pTarget
  );

/**
  Get the value of a specific option
  NOTE:  Returned value needs to be freed by the caller
**/
CHAR16 *getOptionValue(CONST struct Command *pCmd,
    CONST CHAR16 *optionName);

/**
  Get the value of a specific target

  @param[in] pCmd
  @param[in] pTarget

  @retval the target value if the target has been found
  @retval NULL otherwise
**/
CHAR16*
GetTargetValue(
  IN struct Command *pCmd,
  IN CONST CHAR16 *pTarget
  );

/**
  Determine if the specified value is in the specified comma
  separated display list.
**/
BOOLEAN ContainsValue(CONST CHAR16 *displayList,
    CONST CHAR16 *value);

/**
  Get the value of the units option

  @param[in] pCmd The input command structure
  @param[out] pUnitsToDisplay Units to display based on input units option

  @retval EFI_INVALID_PARAMETER if input parameter is NULL, else EFI_SUCCESS
**/
EFI_STATUS
GetUnitsOption(
  IN     CONST struct Command *pCmd,
     OUT UINT16 *pUnitsToDisplay
  );

/**
Sets a display information needed when outputting alternative formats like XML.
@param[in] pName is a CHAR16 string that represents the output message.
@param[in] Type represents the type of output being displayed.
@param[in] pDelims is a CHAR16 string that represents deliminters to use when parsing text output
@retval EFI_SUCCESS if the name was copied correctly.
@retval EFI_INVALID_PARAMETER if any of the parameters is a NULL.
**/
EFI_STATUS
SetDisplayInfo(
  IN     CONST CHAR16 *pName,
  IN     CONST UINT8 Type,
  IN     CONST CHAR16 *pDelims
);

/**
Get display information needed when outputting alternative formats like XML.
@param[out] pName is a CHAR16 string that represents the output message.
@param[int] NameSize is the size of pName in bytes
@param[out] pType represents the type of output being displayed.
@param[out] pDelims represents the deliminters to use when parsing text output.
@param[int] DelimsSize is the size of pDelims in bytes
@retval EFI_SUCCESS if the name was copied correctly.
@retval EFI_INVALID_PARAMETER if any of the parameters is a NULL.
**/
EFI_STATUS
GetDisplayInfo(
  OUT    CHAR16 *pName,
  IN     CONST UINT32 NameSize,
  OUT    UINT8 *pType,
  OUT    CHAR16 *pDelims,
  IN     CONST UINT32 DelimnsSize
);

/**
Execute UpdateCmdCtx (if defined), run, and RunCleanup (if defined).
@param[in] pCommand pointer to the command structure
@retval EFI_SUCCESS if the name was copied correctly.
@retval EFI_INVALID_PARAMETER if any of the parameters is a NULL.
**/
EFI_STATUS
ExecuteCmd(COMMAND *pCommand);

#endif /** _COMMAND_PARSER_H_**/
