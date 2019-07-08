/*
 * Copyright (c) 2018, Intel Corporation.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _STOP_SESSION_COMMAND_
#define _STOP_SESSION_COMMAND_

#include <Uefi.h>
#include "CommandParser.h"

/**
  Register the PBR command

  @retval EFI_SUCCESS success
  @retval EFI_ABORTED registering failure
  @retval EFI_OUT_OF_RESOURCES memory allocation failure
**/
EFI_STATUS
RegisterStopSessionCommand();

/**
  Execute the PBR command

  @param[in] pCmd command from CLI
  @retval EFI_SUCCESS success
  @retval EFI_INVALID_PARAMETER pCmd is NULL or invalid command line parameters
  @retval EFI_NOT_READY Invalid device state to perform action
**/
EFI_STATUS
StopSession(
  IN     struct Command *pCmd
  );

#endif //_STOP_SESSION_COMMAND_
