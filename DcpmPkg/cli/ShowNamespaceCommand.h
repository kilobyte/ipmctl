/*
 * Copyright (c) 2018, Intel Corporation.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SHOW_NAMESPACE_COMMAND_H_
#define _SHOW_NAMESPACE_COMMAND_H_

#include <Uefi.h>

#include <Debug.h>
#include <Types.h>
#include <Utility.h>
#include <Convert.h>
#include <NvmInterface.h>
#include <NvmTypes.h>
#include "Common.h"
#include <NvmHealth.h>

#define NAMESPACE_ID_STR            L"NamespaceId"
#define NAME_STR                    L"Name"
#define CAPACITY_STR                L"Capacity"
#define HEALTH_STATE_STR            L"HealthState"
#define REGION_ID_STR               L"RegionID"
#define BLOCK_SIZE_STR              L"BlockSize"
#define MODE_STR                    L"Mode"
#define LABEL_VERSION_STR           L"LabelVersion"
#define NAMESPACE_GUID_STR          L"NamespaceGuid"
#define REGION_ID_STR               L"RegionID"


/**
  Register the show namespace command

  @retval EFI_SUCCESS success
  @retval EFI_ABORTED registering failure
  @retval EFI_OUT_OF_RESOURCES memory allocation failure
**/
EFI_STATUS
RegisterShowNamespaceCommand(
  );

/**
  Execute the show namespace command

  @param[in] pCmd command from CLI

  @retval EFI_SUCCESS on success
  @retval EFI_INVALID_PARAMETER pCmd is NULL or invalid command line parameters
  @retval EFI_OUT_OF_RESOURCES memory allocation failure
  @retval EFI_ABORTED invoking CONFIG_PROTOCOL function failure
**/
EFI_STATUS
ShowNamespace(
  IN     struct Command *pCmd
  );

#endif /* _SHOW_NAMESPACE_COMMAND_H_ */
