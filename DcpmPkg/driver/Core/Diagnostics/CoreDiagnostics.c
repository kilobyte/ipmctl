/*
 * Copyright (c) 2018, Intel Corporation.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "CoreDiagnostics.h"
#include "QuickDiagnostic.h"
#include "ConfigDiagnostic.h"
#include "SecurityDiagnostic.h"
#include "FwDiagnostic.h"

extern NVMDIMMDRIVER_DATA *gNvmDimmData;

/**
  Append to the results string for a paricular diagnostic test, and modify
  the test state as per the message being appended.

  @param[in] pStrToAppend Pointer to the message string to be appended
  @param[in] DiagStateMask State corresonding to the string that is being appended
  @param[in out] ppResult Pointer to the result string of the particular test-suite's messages
  @param[in out] pDiagState Pointer to the particular test state

  @retval EFI_SUCCESS Success
  @retval EFI_INVALID_PARAMETER if any of the parameters is a NULL
**/
EFI_STATUS
AppendToDiagnosticsResult (
  IN     DIMM *pDimm OPTIONAL,
  IN     UINT32 Code OPTIONAL,
  IN     CHAR16 *pStrToAppend,
  IN     UINT8 DiagStateMask,
  IN OUT CHAR16 **ppResultStr,
  IN OUT UINT8 *pDiagState
  )
{
  EFI_STATUS ReturnCode = EFI_SUCCESS;

  NVDIMM_ENTRY();

  if (pStrToAppend == NULL || ppResultStr == NULL || pDiagState == NULL) {
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }

  *ppResultStr = CatSPrintClean(*ppResultStr, FORMAT_STR_NL, pStrToAppend);

  *pDiagState |= DiagStateMask;

Finish:
  FREE_POOL_SAFE(pStrToAppend);
  NVDIMM_EXIT_I64(ReturnCode);
  return ReturnCode;
}

#ifdef OS_BUILD
#include "event.h"
/**
Converts the event code to the event category
**/
enum system_event_category EventCodeToEventCategory(
  IN UINT32 Code)
{
  if (DEVICE_CONFIG_BASE_CODE == (Code / EVENT_CODE_BASE_VALUE)) {
    return SYSTEM_EVENT_CAT_PM; //!< PM metadata diagnostic test events
  }
  else if (DEVICE_HEALTH_BASE_CODE == (Code / EVENT_CODE_BASE_VALUE)) {
    return SYSTEM_EVENT_CAT_HEALTH; //!< Device health events
  }
  else if (CONFIG_CHANGE_BASE_CODE == (Code / EVENT_CODE_BASE_VALUE)) {
    return SYSTEM_EVENT_CAT_MGMT; //!< Management software generated events
  }
  else if (QUICK_HEALTH_BASE_CODE == (Code / EVENT_CODE_BASE_VALUE)) {
    return SYSTEM_EVENT_CAT_QUICK; //!< Quick diagnostic test events
  }
  else if (PLATFORM_CONFIG_BASE_CODE == (Code / EVENT_CODE_BASE_VALUE)) {
    return SYSTEM_EVENT_CAT_CONFIG; //!< Platform config diagnostic test events
  }
  else if (SECURITY_CHECK_BASE_CODE == (Code / EVENT_CODE_BASE_VALUE)) {
    return SYSTEM_EVENT_CAT_SECURITY; //!< Secuirty diagnostic test events
  }
  else if (FW_CONSISTENCY_BASE_CODE == (Code / EVENT_CODE_BASE_VALUE)) {
    return SYSTEM_EVENT_CAT_FW;    //!< FW consistency diagnostic test events
  }

  return SYSTEM_EVENT_CAT_DIAG;  //!< Diagnostic test events
}

/**
Converts the event code to the action required state
**/
BOOLEAN EventCodeToActionRequiredState(
  IN UINT32 Code)
{
  if (
    (EVENT_CODE_507 == Code) ||
    (EVENT_CODE_513 == Code) ||
    (EVENT_CODE_514 == Code) ||
    (EVENT_CODE_515 == Code) ||
    (EVENT_CODE_519 == Code) ||
    (EVENT_CODE_520 == Code) ||
    (EVENT_CODE_521 == Code) ||
    (EVENT_CODE_522 == Code) ||
    (EVENT_CODE_523 == Code) ||
    (EVENT_CODE_533 == Code) ||
    (EVENT_CODE_534 == Code) ||
    (EVENT_CODE_535 == Code) ||
    (EVENT_CODE_536 == Code) ||
    (EVENT_CODE_537 == Code) ||
    (EVENT_CODE_609 == Code) ||
    (EVENT_CODE_624 == Code) ||
    (EVENT_CODE_625 == Code) ||
    (EVENT_CODE_626 == Code) ||
    (EVENT_CODE_627 == Code) ||
    (EVENT_CODE_628 == Code) ||
    (EVENT_CODE_629 == Code)) {
    return TRUE;
  }
  return FALSE;
}

/**
Append to the results string for a paricular diagnostic test, modify
the test state as per the message being appended and send the event
to the event log.

@param[in] pDimm Pointer to the DIMM structure
@param[in] Code The event's code (for details see the event CSPEC)
@param[in] pStrToAppend Pointer to the message string to be appended
@param[in] DiagStateMask State corresonding to the string that is being appended
@param[in out] ppResult Pointer to the result string of the particular test-suite's messages
@param[in out] pDiagState Pointer to the particular test state

@retval EFI_SUCCESS Success
@retval EFI_INVALID_PARAMETER if any of the parameters is a NULL
**/
EFI_STATUS
SendTheEventAndAppendToDiagnosticsResult(
    IN     DIMM *pDimm,
    IN     UINT32 Code,
    IN     CHAR16 *pStrToAppend,
    IN     UINT8 DiagStateMask,
    IN OUT CHAR16 **ppResultStr,
    IN OUT UINT8 *pDiagState
)
{
    EFI_STATUS ReturnCode = EFI_SUCCESS;
    CHAR16 DimmUid[MAX_DIMM_UID_LENGTH] = { 0 };
    CHAR8 AsciiDimmUid[MAX_DIMM_UID_LENGTH] = { 0 };
	  enum system_event_type EventSeverity = SYSTEM_EVENT_TYPE_INFO;
    CHAR8 * pAsciiStrToAppend = NULL;
    BOOLEAN StoreInSystemLog = FALSE;
    BOOLEAN ActionReqState = EventCodeToActionRequiredState(Code);
    enum system_event_category Category = EventCodeToEventCategory(Code);

    // Prepare the string
    pAsciiStrToAppend = (CHAR8*)AllocateZeroPool(StrLen(pStrToAppend) + 1); // +1 makes room for string termination
    if (NULL == pAsciiStrToAppend) {
      FREE_POOL_SAFE(pStrToAppend);
      return EFI_OUT_OF_RESOURCES;
    }
    UnicodeStrToAsciiStrS(pStrToAppend, pAsciiStrToAppend, StrLen(pStrToAppend) + 1);
    // Get the message severity
    if ((DiagStateMask & DIAG_STATE_MASK_ABORTED) || (DiagStateMask & DIAG_STATE_MASK_FAILED)) {
        EventSeverity = SYSTEM_EVENT_TYPE_ERROR;
        StoreInSystemLog = TRUE;
    }
    else if (DiagStateMask & DIAG_STATE_MASK_WARNING) {
        EventSeverity = SYSTEM_EVENT_TYPE_WARNING;
    }
    else if (DiagStateMask & DIAG_STATE_MASK_OK) {
        EventSeverity = SYSTEM_EVENT_TYPE_INFO;
    }
    // Store the log
    if (pDimm != NULL)
    {
        // Get the current dimm uid
        ReturnCode = GetDimmUid(pDimm, DimmUid, MAX_DIMM_UID_LENGTH);
        if (EFI_ERROR(ReturnCode)) {
            NVDIMM_DBG("ERROR: GetDimmUid\n");
        }
        else
        {
            // Prepare DIMM UId
            UnicodeStrToAsciiStrS(DimmUid, AsciiDimmUid, MAX_DIMM_UID_LENGTH);
            nvm_store_system_entry(NVM_SYSLOG_SOURCE,
                SYSTEM_EVENT_CREATE_EVENT_TYPE(Category, EventSeverity, Code, FALSE, StoreInSystemLog, TRUE, TRUE, ActionReqState),
                AsciiDimmUid, pAsciiStrToAppend, DO_NOT_PARSE_ARGS);
        }
    }
    else
    {
        nvm_store_system_entry(NVM_SYSLOG_SOURCE,
            SYSTEM_EVENT_CREATE_EVENT_TYPE(Category, EventSeverity, Code, FALSE, StoreInSystemLog, TRUE, TRUE, ActionReqState),
            NULL, pAsciiStrToAppend, DO_NOT_PARSE_ARGS);
    }

    FREE_POOL_SAFE(pAsciiStrToAppend);
    return AppendToDiagnosticsResult(pDimm, Code, pStrToAppend, DiagStateMask, ppResultStr, pDiagState);
}
#endif // OS_BUILD

/**
  Helper function to convert the diagnostic test's result-state to its corresponding string form

  @param[in] DiagState The result-state of the test

  @retval NULL if the passed reuslt-state was invalid
  @return String form of the diagnostic test's result-state
**/
STATIC
CHAR16 *
GetDiagnosticState(
  IN     UINT8 DiagState
  )
{
  NVDIMM_ENTRY();
  CHAR16 *pDiagStateStr = NULL;

  if (DiagState & DIAG_STATE_MASK_ABORTED) {
    pDiagStateStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_STATE_ABORTED), NULL);
  } else if (DiagState & DIAG_STATE_MASK_FAILED) {
    pDiagStateStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_STATE_FAILED), NULL);
  } else if (DiagState & DIAG_STATE_MASK_WARNING) {
    pDiagStateStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_STATE_WARNING), NULL);
  } else if (DiagState & DIAG_STATE_MASK_OK) {
    pDiagStateStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_STATE_OK), NULL);
  } else if ((DiagState & DIAG_STATE_MASK_ALL) <= DIAG_STATE_MASK_OK) {
 pDiagStateStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_STATE_OK), NULL);
  }

  NVDIMM_EXIT();
  return pDiagStateStr;
}

/**
  Helper function to retrieve the diagnostics test-name from the test index

  @param[in] DiagnosticTestIndex Diagnostic test index

  @retval NULL if the passed test index was invalid
  @return String form of the diagnostic test name
**/
STATIC
CHAR16 *
GetDiagnosticTestName(
  IN    UINT8 DiagnosticTestIndex
  )
{
  CHAR16 *pDiagTestStr = NULL;

  NVDIMM_ENTRY();

  switch (DiagnosticTestIndex) {
    case  QuickDiagnosticIndex:
      pDiagTestStr =  HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_QUICK_NAME), NULL);
      break;
    case ConfigDiagnosticIndex:
      pDiagTestStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_CONFIG_NAME), NULL);
      break;
    case SecurityDiagnosticIndex:
      pDiagTestStr =  HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_SECURITY_NAME), NULL);
      break;
    case FwDiagnosticIndex:
      pDiagTestStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_FW_NAME), NULL);
      break;
    default:
      NVDIMM_DBG("invalid diagnostic test");
      break;
  }

  NVDIMM_EXIT();
  return pDiagTestStr;
}

/**
  Add headers to the message results from all the tests that were run,
  and then append those messages into one single Diagnostics result string

  @param[in] pBuffer Array of the result messages for all tests
  @param[in] DiagState Array of the result state for all tests
  @param[out] ppResult Pointer to the final result string for all tests that were run

  @retval EFI_SUCCESS Success
  @retval EFI_INVALID_PARAMETER if any of the parameters is a NULL
**/
EFI_STATUS
CombineDiagnosticsTestResults(
  IN     CHAR16 *pBuffer[],
  IN     UINT8 DiagState[],
     OUT CHAR16 **ppResult
  )
{
  EFI_STATUS ReturnCode = EFI_SUCCESS;
  UINTN Index = 0;
  CHAR16 *pColonMarkStr = NULL;
  CHAR16 *pTestNameValueStr = NULL;
  CHAR16 *pDiagStateValueStr = NULL;
  CHAR16 *pTempHeaderStr = NULL;

  NVDIMM_ENTRY();

  if (pBuffer == NULL || ppResult == NULL) {
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }

  pColonMarkStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DCPMM_COLON_MARK), NULL);

  for (Index = 0; Index < DIAGNOSTIC_TEST_COUNT; Index++) {
    if (pBuffer[Index] != NULL) {

      //creating test name string
      pTestNameValueStr = GetDiagnosticTestName((UINT8)Index);
      if (pTestNameValueStr == NULL) {
        NVDIMM_DBG("Retrieval of the test name failed");
        //log as warning state and a message
        continue;
      }

      //creating state string
      pDiagStateValueStr = GetDiagnosticState(DiagState[Index]);
      if (pDiagStateValueStr == NULL) {
        NVDIMM_DBG("Retrieval of the test state failed");
        FREE_POOL_SAFE(pTestNameValueStr);
        //log as warning state and a message
        continue;
      }

      pTempHeaderStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_TEST_NAME_HEADER), NULL);
      *ppResult = CatSPrintClean(*ppResult, FORMAT_STR FORMAT_STR L" " FORMAT_STR_NL, pTempHeaderStr, pColonMarkStr, pTestNameValueStr);
      FREE_POOL_SAFE(pTempHeaderStr);
      FREE_POOL_SAFE(pTestNameValueStr);

      pTempHeaderStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_STATE_HEADER), NULL);
      *ppResult = CatSPrintClean(*ppResult, FORMAT_STR FORMAT_STR L" " FORMAT_STR_NL, pTempHeaderStr, pColonMarkStr, pDiagStateValueStr);
      FREE_POOL_SAFE(pTempHeaderStr);
      FREE_POOL_SAFE(pDiagStateValueStr);

      pTempHeaderStr = HiiGetString(gNvmDimmData->HiiHandle, STRING_TOKEN(STR_DIAGNOSTIC_MESSAGE_HEADER), NULL);
      *ppResult = CatSPrintClean(*ppResult, FORMAT_STR  FORMAT_STR L"\n" FORMAT_STR_NL, pTempHeaderStr, pColonMarkStr, pBuffer[Index]);
      FREE_POOL_SAFE(pTempHeaderStr);

      FREE_POOL_SAFE(pTestNameValueStr);
      FREE_POOL_SAFE(pDiagStateValueStr);
    }
  }

Finish:
  FREE_POOL_SAFE(pColonMarkStr);
  NVDIMM_EXIT_I64(ReturnCode);
  return ReturnCode;
}

//  [ATTENTION] : Do not use this function for implementing diagnostic tests. This is kept maintain the backward compatibility.
/**
  The fundamental core diagnostics function that is used by both
  the NvmDimmConfig protocol and the DriverDiagnostic protoocls.

  It runs the specified diagnotsics tests on the list of specified dimms,
  and returns a single combined test result message

  @param[in] ppDimms The platform DIMM pointers list
  @param[in] DimmsNum Platform DIMMs count
  @param[in] pDimmIds Pointer to an array of user-specified DIMM IDs
  @param[in] DimmIdsCount Number of items in the array of user-specified DIMM IDs
  @param[in] DiagnosticsTest The selected tests bitmask
  @param[in] DimmIdPreference Preference for Dimm ID display (UID/Handle)
  @param[out] ppResult Pointer to the combined result string

  @retval EFI_SUCCESS Test executed correctly
  @retval EFI_OUT_OF_RESOURCES memory allocation failure
  @retval EFI_INVALID_PARAMETER if any of the parameters is a NULL.
**/
EFI_STATUS
CoreStartDiagnostics(
  IN     DIMM **ppDimms,
  IN     UINT32 DimmsNum,
  IN     UINT16 *pDimmIds OPTIONAL,
  IN     UINT32 DimmIdsCount,
  IN     UINT8 DiagnosticsTest,
  IN     UINT8 DimmIdPreference,
     OUT CHAR16 **ppResult
  )
{
  EFI_STATUS ReturnCode = EFI_SUCCESS;
  EFI_STATUS TempReturnCode = EFI_SUCCESS;
  DIMM **ppManageableDimms = NULL;
  UINT16 ManageableDimmsNum = 0;
  DIMM **ppSpecifiedDimms = NULL;
  UINT16 SpecifiedDimmsNum = 0;
  LIST_ENTRY *pDimmList = NULL;
  UINT32 PlatformDimmsCount = 0;
  DIMM *pCurrentDimm = NULL;
  UINTN Index = 0;

  CHAR16 *pBuffer[DIAGNOSTIC_TEST_COUNT];
  UINT8 DiagState[DIAGNOSTIC_TEST_COUNT];

  NVDIMM_ENTRY();

  ZeroMem(pBuffer, sizeof(pBuffer));
  ZeroMem(DiagState, sizeof(DiagState));

  if (ppDimms == NULL || ppResult == NULL) {
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }

  ppManageableDimms = AllocateZeroPool(DimmsNum * sizeof(DIMM *));
  if (ppManageableDimms == NULL) {
    ReturnCode = EFI_OUT_OF_RESOURCES;
    goto Finish;
  }

  // Populate the manageable dimms
  for (Index = 0; Index < DimmsNum; Index++) {
    if (ppDimms[Index] == NULL) {
      ReturnCode = EFI_INVALID_PARAMETER;
      goto Finish;
    }

    if (IsDimmManageable(ppDimms[Index])) {
      ppManageableDimms[ManageableDimmsNum] = ppDimms[Index];
      ManageableDimmsNum++;
    }
  }

  if ((DimmIdPreference != DISPLAY_DIMM_ID_HANDLE) &&
      (DimmIdPreference != DISPLAY_DIMM_ID_UID)) {
    NVDIMM_DBG("Invalid value for Dimm Id preference");
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }

  if ((DiagnosticsTest & DIAGNOSTIC_TEST_ALL) == 0) {
    NVDIMM_DBG("Invalid diagnostics test");
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }

  // Populate the specified dimms for quick diagnostics
  if ((DiagnosticsTest & DIAGNOSTIC_TEST_QUICK) && (DimmIdsCount > 0)) {
    if (pDimmIds == NULL) {
      ReturnCode = EFI_INVALID_PARAMETER;
      goto Finish;
    }

    pDimmList = &gNvmDimmData->PMEMDev.Dimms;
    ReturnCode = GetListSize(pDimmList, &PlatformDimmsCount);
    if (EFI_ERROR(ReturnCode)) {
      NVDIMM_DBG("Failed on DimmListSize");
      goto Finish;
    }

    if (DimmIdsCount > PlatformDimmsCount) {
      NVDIMM_DBG("User specified Dimm count exceeds the platform Dimm count.");
      ReturnCode = EFI_INVALID_PARAMETER;
      goto Finish;
    }

    ppSpecifiedDimms = AllocateZeroPool(DimmIdsCount * sizeof(DIMM *));
    if (ppSpecifiedDimms == NULL) {
      ReturnCode = EFI_OUT_OF_RESOURCES;
      goto Finish;
    }

    for (Index = 0; Index < DimmIdsCount; Index++) {
      pCurrentDimm = GetDimmByPid(pDimmIds[Index], pDimmList);
      if (pCurrentDimm == NULL) {
        NVDIMM_DBG("Failed on GetDimmByPid. Does DIMM 0x%04x exist?", pDimmIds[Index]);
        ReturnCode = EFI_INVALID_PARAMETER;
        goto Finish;
      }

      ppSpecifiedDimms[SpecifiedDimmsNum] = pCurrentDimm;
      SpecifiedDimmsNum++;
    }
  }

  if (DiagnosticsTest & DIAGNOSTIC_TEST_QUICK) {
    if (SpecifiedDimmsNum > 0) {
      TempReturnCode = RunQuickDiagnostics(ppSpecifiedDimms, (UINT16)SpecifiedDimmsNum, DimmIdPreference,
        &(pBuffer[QuickDiagnosticIndex]), &(DiagState[QuickDiagnosticIndex]));
    } else {
      TempReturnCode = RunQuickDiagnostics(ppDimms, (UINT16)DimmsNum, DimmIdPreference, &(pBuffer[QuickDiagnosticIndex]), &(DiagState[QuickDiagnosticIndex]));
    }
    if (EFI_ERROR(TempReturnCode)) {
      KEEP_ERROR(ReturnCode, TempReturnCode);
      NVDIMM_DBG("Quick diagnostics failed. (" FORMAT_EFI_STATUS ")", TempReturnCode);
    }
  }
  if (DiagnosticsTest & DIAGNOSTIC_TEST_CONFIG) {
    TempReturnCode = RunConfigDiagnostics(ppManageableDimms, (UINT16)ManageableDimmsNum, DimmIdPreference,
        &(pBuffer[ConfigDiagnosticIndex]), &(DiagState[ConfigDiagnosticIndex]));
    if (EFI_ERROR(TempReturnCode)) {
      KEEP_ERROR(ReturnCode, TempReturnCode);
      NVDIMM_DBG("Platform configuration diagnostics failed. (" FORMAT_EFI_STATUS ")", TempReturnCode);
    }
  }
  if (DiagnosticsTest & DIAGNOSTIC_TEST_SECURITY) {
    TempReturnCode = RunSecurityDiagnostics(ppManageableDimms, (UINT16)ManageableDimmsNum, DimmIdPreference, &(pBuffer[SecurityDiagnosticIndex]), &(DiagState[SecurityDiagnosticIndex]));
    if (EFI_ERROR(TempReturnCode)) {
      KEEP_ERROR(ReturnCode, TempReturnCode);
      NVDIMM_DBG("Security diagnostics failed. (" FORMAT_EFI_STATUS ")", TempReturnCode);
    }
  }
  if (DiagnosticsTest & DIAGNOSTIC_TEST_FW) {
    TempReturnCode = RunFwDiagnostics(ppManageableDimms, (UINT16)ManageableDimmsNum, DimmIdPreference, &(pBuffer[FwDiagnosticIndex]), &(DiagState[FwDiagnosticIndex]));
    if (EFI_ERROR(TempReturnCode)) {
      KEEP_ERROR(ReturnCode, TempReturnCode);
      NVDIMM_DBG("Firmware and consistency settings diagnostics failed. (" FORMAT_EFI_STATUS ")", TempReturnCode);
    }
  }

  TempReturnCode = CombineDiagnosticsTestResults(pBuffer, DiagState, ppResult);
  if (EFI_ERROR(TempReturnCode)) {
    KEEP_ERROR(ReturnCode, TempReturnCode);
    goto Finish;
  }

Finish:
  for (Index = 0; Index < DIAGNOSTIC_TEST_COUNT; Index++) {
    FREE_POOL_SAFE(pBuffer[Index]);
  }
  FREE_POOL_SAFE(ppManageableDimms);
  FREE_POOL_SAFE(ppSpecifiedDimms);
  NVDIMM_EXIT_I64(ReturnCode);
  return ReturnCode;
}


/**
  The fundamental core diagnostics function that is used by both
  the NvmDimmConfig protocol and the DriverDiagnostic protoocls.

  It runs the specified diagnotsics tests on the list of specified dimms,
  and returns a single combined test result message

  @param[in] ppDimms The platform DIMM pointers list
  @param[in] DimmsNum Platform DIMMs count
  @param[in] pDimmIds Pointer to an array of user-specified DIMM IDs
  @param[in] DimmIdsCount Number of items in the array of user-specified DIMM IDs
  @param[in] DiagnosticsTest The selected tests bitmask
  @param[in] DimmIdPreference Preference for Dimm ID display (UID/Handle)
  @param[out] ppResult Pointer to the structure with information about test

  @retval EFI_SUCCESS Test executed correctly
  @retval EFI_OUT_OF_RESOURCES memory allocation failure
  @retval EFI_INVALID_PARAMETER if any of the parameters is a NULL.
**/
EFI_STATUS
CoreStartDiagnosticsDetail(
  IN     DIMM **ppDimms,
  IN     UINT32 DimmsNum,
  IN     UINT16 *pDimmIds OPTIONAL,
  IN     UINT32 DimmIdsCount,
  IN     UINT8 DiagnosticsTest,
  IN     UINT8 DimmIdPreference,
  OUT    DIAG_INFO **ppResult
)
{
  EFI_STATUS ReturnCode = EFI_SUCCESS;
  EFI_STATUS TempReturnCode = EFI_SUCCESS;
  DIMM **ppManageableDimms = NULL;
  UINT16 ManageableDimmsNum = 0;
  DIMM **ppSpecifiedDimms = NULL;
  UINT16 SpecifiedDimmsNum = 0;
  LIST_ENTRY *pDimmList = NULL;
  UINT32 PlatformDimmsCount = 0;
  DIMM *pCurrentDimm = NULL;
  UINTN Index = 0;
  UINTN Id = 0;

  DIAG_INFO *pBuffer = NULL;
  pBuffer = AllocateZeroPool(sizeof(*pBuffer));

  NVDIMM_ENTRY();

  if (ppDimms == NULL || ppResult == NULL) {
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }

  if (pBuffer == NULL) {
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }
  *ppResult = pBuffer;

  ppManageableDimms = AllocateZeroPool(DimmsNum * sizeof(DIMM *));
  if (ppManageableDimms == NULL) {
    ReturnCode = EFI_OUT_OF_RESOURCES;
    goto Finish;
  }

  // Populate the manageable dimms
  for (Index = 0; Index < DimmsNum; Index++) {
    if (ppDimms[Index] == NULL) {
      ReturnCode = EFI_INVALID_PARAMETER;
      goto Finish;
    }

    if (IsDimmManageable(ppDimms[Index])) {
      ppManageableDimms[ManageableDimmsNum] = ppDimms[Index];
      ManageableDimmsNum++;
    }
  }

  if ((DimmIdPreference != DISPLAY_DIMM_ID_HANDLE) &&
    (DimmIdPreference != DISPLAY_DIMM_ID_UID)) {
    NVDIMM_DBG("Invalid value for Dimm Id preference");
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }

  if ((DiagnosticsTest & DIAGNOSTIC_TEST_ALL) == 0) {
    NVDIMM_DBG("Invalid diagnostics test");
    ReturnCode = EFI_INVALID_PARAMETER;
    goto Finish;
  }

  // Populate the specified dimms for quick diagnostics
  if ((DiagnosticsTest & DIAGNOSTIC_TEST_QUICK) && (DimmIdsCount > 0)) {
    if (pDimmIds == NULL) {
      ReturnCode = EFI_INVALID_PARAMETER;
      goto Finish;
    }

    pDimmList = &gNvmDimmData->PMEMDev.Dimms;
    ReturnCode = GetListSize(pDimmList, &PlatformDimmsCount);
    if (EFI_ERROR(ReturnCode)) {
      NVDIMM_DBG("Failed on DimmListSize");
      goto Finish;
    }

    if (DimmIdsCount > PlatformDimmsCount) {
      NVDIMM_DBG("User specified Dimm count exceeds the platform Dimm count.");
      ReturnCode = EFI_INVALID_PARAMETER;
      goto Finish;
    }

    ppSpecifiedDimms = AllocateZeroPool(DimmIdsCount * sizeof(DIMM *));
    if (ppSpecifiedDimms == NULL) {
      ReturnCode = EFI_OUT_OF_RESOURCES;
      goto Finish;
    }

    for (Index = 0; Index < DimmIdsCount; Index++) {
      pCurrentDimm = GetDimmByPid(pDimmIds[Index], pDimmList);
      if (pCurrentDimm == NULL) {
        NVDIMM_DBG("Failed on GetDimmByPid. Does DIMM 0x%04x exist?", pDimmIds[Index]);
        ReturnCode = EFI_INVALID_PARAMETER;
        goto Finish;
      }

      ppSpecifiedDimms[SpecifiedDimmsNum] = pCurrentDimm;
      SpecifiedDimmsNum++;
    }
  }

  if (DiagnosticsTest & DIAGNOSTIC_TEST_QUICK) {
    pBuffer->TestName = GetDiagnosticTestName(QuickDiagnosticIndex);
    if (SpecifiedDimmsNum > 0) {
      TempReturnCode = RunQuickDiagnosticsDetail(ppSpecifiedDimms, (UINT16)SpecifiedDimmsNum, DimmIdPreference, pBuffer);
    }
    else {
      TempReturnCode = RunQuickDiagnosticsDetail(ppDimms, (UINT16)DimmsNum, DimmIdPreference, pBuffer);
    }
    if (EFI_ERROR(TempReturnCode)) {
      KEEP_ERROR(ReturnCode, TempReturnCode);
      NVDIMM_DBG("Quick diagnostics failed. (" FORMAT_EFI_STATUS ")", TempReturnCode);
    }
    for (Id = 0; Id < MAX_NO_OF_DIAGNOSTIC_SUBTESTS; Id++) {
      if (pBuffer->SubTestName[Id] != NULL)
      {
        pBuffer->state[Id] = GetDiagnosticState(pBuffer->SubTestStateVal[Id]);
      }
    }
  }
  else if (DiagnosticsTest & DIAGNOSTIC_TEST_CONFIG) {
    pBuffer->TestName = GetDiagnosticTestName(ConfigDiagnosticIndex);
    TempReturnCode = RunConfigDiagnosticsDetail(ppManageableDimms, (UINT16)ManageableDimmsNum, DimmIdPreference, pBuffer);
    if (EFI_ERROR(TempReturnCode)) {
      KEEP_ERROR(ReturnCode, TempReturnCode);
      NVDIMM_DBG("Platform configuration diagnostics failed. (" FORMAT_EFI_STATUS ")", TempReturnCode);
    }
    for (Id = 0; Id < MAX_NO_OF_DIAGNOSTIC_SUBTESTS; Id++) {
      if (pBuffer->SubTestName[Id] != NULL)
      {
        pBuffer->state[Id] = GetDiagnosticState(pBuffer->SubTestStateVal[Id]);
      }
    }
  }
  else if (DiagnosticsTest & DIAGNOSTIC_TEST_SECURITY) {
    pBuffer->TestName = GetDiagnosticTestName(SecurityDiagnosticIndex);
    TempReturnCode = RunSecurityDiagnosticsDetail(ppManageableDimms, (UINT16)ManageableDimmsNum, DimmIdPreference, pBuffer);
    if (EFI_ERROR(TempReturnCode)) {
      KEEP_ERROR(ReturnCode, TempReturnCode);
      NVDIMM_DBG("Security diagnostics failed. (" FORMAT_EFI_STATUS ")", TempReturnCode);
    }
    for (Id = 0; Id < MAX_NO_OF_DIAGNOSTIC_SUBTESTS; Id++) {
      if (pBuffer->SubTestName[Id] != NULL)
      {
        pBuffer->state[Id] = GetDiagnosticState(pBuffer->SubTestStateVal[Id]);
      }
    }
  }
  else if (DiagnosticsTest & DIAGNOSTIC_TEST_FW) {
    pBuffer->TestName = GetDiagnosticTestName(FwDiagnosticIndex);
    TempReturnCode = RunFwDiagnosticsDetail(ppManageableDimms, (UINT16)ManageableDimmsNum, DimmIdPreference, pBuffer);
    if (EFI_ERROR(TempReturnCode)) {
      KEEP_ERROR(ReturnCode, TempReturnCode);
      NVDIMM_DBG("Firmware and consistency settings diagnostics failed. (" FORMAT_EFI_STATUS ")", TempReturnCode);
    }
    for (Id = 0; Id < MAX_NO_OF_DIAGNOSTIC_SUBTESTS; Id++) {
      if (pBuffer->SubTestName[Id] != NULL)
      {
        pBuffer->state[Id] = GetDiagnosticState(pBuffer->SubTestStateVal[Id]);
      }
    }
  }
  else {
    NVDIMM_DBG("Invalid Diagnostic Test Id");
    ReturnCode = EFI_INVALID_PARAMETER;
  }


Finish:
  FREE_POOL_SAFE(ppManageableDimms);
  FREE_POOL_SAFE(ppSpecifiedDimms);

  NVDIMM_EXIT_I64(ReturnCode);
  return ReturnCode;
}
