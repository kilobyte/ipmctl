/*
 * Copyright (c) 2018, Intel Corporation.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <Debug.h>
#include <Library/BaseMemoryLib.h>
#include "CommandParser.h"
#include "Common.h"
#include "DeletePcdCommand.h"
#include <PcdCommon.h>

/**
  Command syntax definition
**/
struct Command DeletePcdCommand =
{
  DELETE_VERB,                                                                     //!< verb
  {                                                                                //!< options
    {VERBOSE_OPTION_SHORT, VERBOSE_OPTION, L"", L"",HELP_VERBOSE_DETAILS_TEXT, FALSE, ValueEmpty},
    { L"", PROTOCOL_OPTION_DDRT, L"", L"",HELP_DDRT_DETAILS_TEXT, FALSE, ValueEmpty},
    { L"", PROTOCOL_OPTION_SMBUS, L"", L"",HELP_SMBUS_DETAILS_TEXT, FALSE, ValueEmpty},
    { L"", LARGE_PAYLOAD_OPTION, L"", L"",HELP_LPAYLOAD_DETAILS_TEXT, FALSE, ValueEmpty},
    { L"", SMALL_PAYLOAD_OPTION,  L"", L"",HELP_SPAYLOAD_DETAILS_TEXT, FALSE, ValueEmpty},
    {FORCE_OPTION_SHORT, FORCE_OPTION, L"", L"",HELP_FORCE_DETAILS_TEXT, FALSE, ValueEmpty}
#ifdef OS_BUILD
    ,{ OUTPUT_OPTION_SHORT, OUTPUT_OPTION, L"", OUTPUT_OPTION_HELP,HELP_OPTIONS_DETAILS_TEXT, FALSE, ValueRequired }
#endif
  },
  {                                                                                //!< targets
    {DIMM_TARGET, L"", HELP_TEXT_DIMM_IDS, TRUE, ValueOptional},
    {PCD_TARGET, L"", PCD_CONFIG_TARGET_VALUE
#ifndef OS_BUILD
     L"|"
     PCD_LSA_TARGET_VALUE
#endif
    , TRUE, ValueOptional}
  },
  {{L"", L"", L"", FALSE, ValueOptional}},                                         //!< properties
  L"Clear the namespace LSA partition on one or more DIMMs",                       //!< help
  DeletePcdCmd,
  TRUE                                                                             //!< enable print control support
};


STATIC
EFI_STATUS
ValidatePcdTarget(
  IN     CHAR16 *pTargetValue,
  IN     CHAR16 *pExpectedTargetValue
  )
{
  EFI_STATUS ReturnCode = EFI_INVALID_PARAMETER;

  NVDIMM_ENTRY();

  if (pTargetValue == NULL || pExpectedTargetValue == NULL) {
    goto Finish;
  }

  if (StrICmp(pTargetValue, pExpectedTargetValue) != 0) {
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }

  ReturnCode = EFI_SUCCESS;

Finish:
  NVDIMM_EXIT_I64(ReturnCode);
  return ReturnCode;
}

/**
  Execute the Delete PCD command

  @param[in] pCmd command from CLI

  @retval EFI_SUCCESS success
  @retval EFI_INVALID_PARAMETER pCmd is NULL or invalid command line parameters
  @retval EFI_NOT_FOUND couldn't open Config Protocol
  @retval EFI_ABORTED internal
**/
EFI_STATUS
DeletePcdCmd(
  IN     struct Command *pCmd
  )
{
  EFI_STATUS ReturnCode = EFI_INVALID_PARAMETER;
  COMMAND_STATUS *pCommandStatus = NULL;
  EFI_DCPMM_CONFIG2_PROTOCOL *pNvmDimmConfigProtocol = NULL;
  UINT16 *pDimmIds = NULL;
  UINT32 DimmIdsCount = 0;
  DIMM_INFO *pDimms = NULL;
  UINT32 DimmCount = 0;
  CHAR16 DimmStr[MAX_DIMM_UID_LENGTH];
  CHAR16 *pTargetValue = NULL;
  UINT32 DimmIndex = 0;
  UINT32 DimmHandle = 0;
  UINT32 Index = 0;
  BOOLEAN Force = FALSE;
  BOOLEAN Confirmation = FALSE;
  CHAR16 *pCommandStatusMessage = NULL;
  UINT32 ConfigIdMask = 0;
  CHAR16 *pDisplayTargets = NULL;
  PRINT_CONTEXT *pPrinterCtx = NULL;

  NVDIMM_ENTRY();

  SetDisplayInfo(L"DeletePcd", ResultsView, NULL);

  ZeroMem(DimmStr, sizeof(DimmStr));

  if (pCmd == NULL) {
    PRINTER_SET_MSG(pPrinterCtx, ReturnCode, CLI_ERR_NO_COMMAND);
    goto Finish;
  }

  pPrinterCtx = pCmd->pPrintCtx;

  /** Get config protocol **/
  ReturnCode = OpenNvmDimmProtocol(gNvmDimmConfigProtocolGuid, (VOID **)&pNvmDimmConfigProtocol, NULL);
  if (EFI_ERROR(ReturnCode)) {
    ReturnCode = EFI_NOT_FOUND;
    PRINTER_SET_MSG(pPrinterCtx, ReturnCode, CLI_ERR_OPENING_CONFIG_PROTOCOL);
    goto Finish;
  }

  // Populate the list of DIMM_INFO structures with relevant information
  ReturnCode = GetDimmList(pNvmDimmConfigProtocol, pCmd, DIMM_INFO_CATEGORY_NONE, &pDimms, &DimmCount);
  if (EFI_ERROR(ReturnCode)) {
    if(ReturnCode == EFI_NOT_FOUND) {
        PRINTER_SET_MSG(pPrinterCtx, ReturnCode, CLI_INFO_NO_FUNCTIONAL_DIMMS);
    }
    goto Finish;
  }

  /** Check force option **/
  if (containsOption(pCmd, FORCE_OPTION) || containsOption(pCmd, FORCE_OPTION_SHORT)) {
    Force = TRUE;
  }

  ReturnCode = InitializeCommandStatus(&pCommandStatus);
  if (EFI_ERROR(ReturnCode)) {
    goto Finish;
  }

  pTargetValue = GetTargetValue(pCmd, DIMM_TARGET);
  ReturnCode = GetDimmIdsFromString(pCmd, pTargetValue, pDimms, DimmCount, &pDimmIds, &DimmIdsCount);
  if (EFI_ERROR(ReturnCode)) {
    goto Finish;
  }

  if (!AllDimmsInListAreManageable(pDimms, DimmCount, pDimmIds, DimmIdsCount)){
    PRINTER_SET_MSG(pPrinterCtx, ReturnCode, CLI_ERR_UNMANAGEABLE_DIMM);
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }

  pTargetValue = GetTargetValue(pCmd, PCD_TARGET);
  if (EFI_SUCCESS == ValidatePcdTarget(pTargetValue, PCD_LSA_TARGET_VALUE)) {
#ifdef OS_BUILD
    ReturnCode = EFI_INVALID_PARAMETER;
    PRINTER_SET_MSG(pPrinterCtx, ReturnCode, CLI_ERR_INCORRECT_VALUE_TARGET_PCD);
    goto Finish;
#endif
    ConfigIdMask |= DELETE_PCD_CONFIG_LSA_MASK;
  }

  if (EFI_SUCCESS == ValidatePcdTarget(pTargetValue, PCD_CONFIG_TARGET_VALUE)) {
    ConfigIdMask |= DELETE_PCD_CONFIG_CIN_MASK | DELETE_PCD_CONFIG_COUT_MASK | DELETE_PCD_CONFIG_CCUR_MASK;
  }

  if (0 == ConfigIdMask) {
#ifdef OS_BUILD
    ConfigIdMask |= DELETE_PCD_CONFIG_CIN_MASK | DELETE_PCD_CONFIG_COUT_MASK | DELETE_PCD_CONFIG_CCUR_MASK;
#else
    ConfigIdMask |= DELETE_PCD_CONFIG_ALL_MASK;
#endif
  }

  /* If no dimms specified then use all dimms */
  if (DimmIdsCount == 0) {
    FREE_POOL_SAFE(pDimmIds);
    pDimmIds = AllocateZeroPool(sizeof(*pDimmIds) * DimmCount);

    if (pDimmIds == NULL) {
      ReturnCode = EFI_OUT_OF_RESOURCES;
      PRINTER_SET_MSG(pPrinterCtx, ReturnCode, CLI_ERR_OUT_OF_MEMORY);
      goto Finish;
    }

    for (Index =0; Index < DimmCount; Index++) {
      pDimmIds[Index] = pDimms[Index].DimmID;
    }
    DimmIdsCount = DimmCount;
  }

  if (ConfigIdMask & (DELETE_PCD_CONFIG_CIN_MASK | DELETE_PCD_CONFIG_COUT_MASK | DELETE_PCD_CONFIG_CCUR_MASK)) {
    pDisplayTargets = CatSPrint(pDisplayTargets, L"Config ");
    if (NULL == pDisplayTargets) {
      ReturnCode = EFI_OUT_OF_RESOURCES;
      PRINTER_SET_MSG(pPrinterCtx, ReturnCode, CLI_ERR_OUT_OF_MEMORY);
      goto Finish;
    }
  }

  if (ConfigIdMask & (DELETE_PCD_CONFIG_LSA_MASK)) {
    if (pDisplayTargets) {
      pDisplayTargets = CatSPrint(pDisplayTargets, L"& ");
      if (NULL == pDisplayTargets) {
        ReturnCode = EFI_OUT_OF_RESOURCES;
        PRINTER_SET_MSG(pPrinterCtx, ReturnCode, CLI_ERR_OUT_OF_MEMORY);
        goto Finish;
      }
    }
    pDisplayTargets = CatSPrint(pDisplayTargets, L"LSA ");
    if (NULL == pDisplayTargets) {
      ReturnCode = EFI_OUT_OF_RESOURCES;
      PRINTER_SET_MSG(pPrinterCtx, ReturnCode, CLI_ERR_OUT_OF_MEMORY);
      goto Finish;
    }
  }

  pCommandStatusMessage = CatSPrint(NULL, L"Clear " FORMAT_STR L"partition(s)", pDisplayTargets);

  if (!Force) {
    PRINTER_PROMPT_MSG(pPrinterCtx, ReturnCode, L"WARNING: Modifying the Platform Configuration Data can result in loss of data!");
    ReturnCode = PromptYesNo(&Confirmation);
    if (EFI_ERROR(ReturnCode) || !Confirmation) {
      ReturnCode = EFI_NOT_STARTED;
      goto Finish;
    }
    PRINTER_PROMPT_MSG(pPrinterCtx, ReturnCode, L"\n");

    for (Index = 0; Index < DimmIdsCount; Index++) {
      ResetCmdStatus(pCommandStatus, NVM_ERR_OPERATION_NOT_STARTED);

      ReturnCode = GetDimmHandleByPid(pDimmIds[Index], pDimms, DimmCount, &DimmHandle, &DimmIndex);
      if (EFI_ERROR(ReturnCode)) {
        goto Finish;
      }
      ReturnCode = GetPreferredDimmIdAsString(DimmHandle, pDimms[DimmIndex].DimmUid,
          DimmStr, MAX_DIMM_UID_LENGTH);
      if (EFI_ERROR(ReturnCode)) {
        goto Finish;
      }
      PRINTER_PROMPT_MSG(pPrinterCtx, ReturnCode, L"Clear " FORMAT_STR L"partition(s) on DIMM " FORMAT_STR L".", pDisplayTargets, DimmStr);
      ReturnCode = PromptYesNo(&Confirmation);
      if (EFI_ERROR(ReturnCode) || !Confirmation) {
        ReturnCode = EFI_NOT_STARTED;
        goto Finish;
      }
    }
  }

  PRINTER_PROMPT_MSG(pPrinterCtx, ReturnCode, L"\n");

  ReturnCode = pNvmDimmConfigProtocol->ModifyPcdConfig(pNvmDimmConfigProtocol, pDimmIds, DimmIdsCount, ConfigIdMask, pCommandStatus);

  if (EFI_ERROR(ReturnCode)) {
    ReturnCode = MatchCliReturnCode(pCommandStatus->GeneralStatus);
    PRINTER_SET_COMMAND_STATUS(pPrinterCtx, ReturnCode, L"Clear partition(s)", L" on", pCommandStatus);
    goto Finish;
  }

  PRINTER_SET_COMMAND_STATUS(pPrinterCtx, ReturnCode, pCommandStatusMessage, L" on", pCommandStatus);

Finish:
  PRINTER_PROCESS_SET_BUFFER(pPrinterCtx);
  FreeCommandStatus(&pCommandStatus);
  FREE_POOL_SAFE(pCommandStatusMessage);
  FREE_POOL_SAFE(pDimmIds);
  FREE_POOL_SAFE(pDimms);
  FREE_POOL_SAFE(pDisplayTargets);
  NVDIMM_EXIT_I64(ReturnCode);
  return ReturnCode;
}

/**
  Register the Delete PCD command

  @retval EFI_SUCCESS success
  @retval EFI_ABORTED registering failure
  @retval EFI_OUT_OF_RESOURCES memory allocation failure
**/
EFI_STATUS
RegisterDeletePcdCommand(
  )
{
  EFI_STATUS ReturnCode = EFI_SUCCESS;

  NVDIMM_ENTRY();

  ReturnCode = RegisterCommand(&DeletePcdCommand);

  NVDIMM_EXIT_I64(ReturnCode);
  return ReturnCode;
}
