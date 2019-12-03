/*
 * Copyright (c) 2018, Intel Corporation.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _NAMESPACE_H_
#define _NAMESPACE_H_

#include <Debug.h>
#include <Types.h>
#include "Dimm.h"
#include "Region.h"
#include <NvmDimmBlockIo.h>
#include "Btt.h"
#include <LbaCommon.h>
#include "Pfn.h"

/**
  BTT Arena Info defines
**/
#define BTT_ALIGNMENT    4096 //!< Alignment of all BTT structures
#define BTTINFO_SIG_LEN  16

#pragma pack(push)
#pragma pack(1)

#define NAMESPACE_SIGNATURE                SIGNATURE_64('N', 'A', 'M', 'E', 'S', 'P', 'C', 'E')
#define NAMESPACE_FROM_NODE(a, FieldName)  CR(a, NAMESPACE, FieldName, NAMESPACE_SIGNATURE)

#define BYTE_MASK 0xFF
#define BYTE_SHIFT 8
#define CREATE_NAMESPACE_ID(RegionId, NamespaceIndex) (((UINT8) (RegionId & BYTE_MASK)) << BYTE_SHIFT) + (UINT8)(NamespaceIndex & BYTE_MASK)

typedef struct {
  DIMM *pDimm;
  UINT64 Dpa;
  UINT64 Size;
} NAMESPACE_REGION;

typedef struct _NAMESPACE {
  UINT64 Signature;
  UINT64 SpaNamespaceBase;
  LIST_ENTRY NamespaceNode;
  LIST_ENTRY IsNode;
  LIST_ENTRY DimmNode;
  EFI_BLOCK_IO_MEDIA Media;
  EFI_HANDLE BlockIoHandle;
  LABEL_FLAGS Flags;
  UINT16 NamespaceId;
  UINT8 NamespaceGuid[NSGUID_LEN];
  UINT16 HealthState;
  UINT64 BlockSize;
  UINT64 BlockCount;
  BOOLEAN Enabled;
  UINT8 NamespaceType;
  DIMM *pParentDimm;
  NVM_IS *pParentIS;
  UINT32 RangesCount;
  UINT16 Major;
  UINT16 Minor;
  NAMESPACE_REGION Range[MAX_NAMESPACE_RANGES];
  BOOLEAN ProtocolsInstalled;
  EFI_BLOCK_IO_PROTOCOL BlockIoInstance;
  EFI_DEVICE_PATH *pBlockDevicePath;
  EFI_UNICODE_STRING_TABLE *pNamespaceName;
  BOOLEAN IsBttEnabled;
  BTT *pBtt;
  UINT8 Name[NLABEL_NAME_LEN_WITH_TERMINATOR];
  UINT64 InterleaveSetCookie;
  BOOLEAN IsPfnEnabled;
  PFN *pPfn;
  BOOLEAN IsRawNamespace;
  UINT64 UsableSize;
} NAMESPACE;

typedef struct _NVM_COOKIE_DATA {
  UINT64 RegionSpaOffset;
  UINT32 SerialNum;
  UINT16 VendorId;
  UINT16 ManufacturingDate;
  UINT8 ManufacturingLocation;
  UINT8 Reserved[31];
} NVM_COOKIE_DATA;

typedef struct _NVM_COOKIE_DATA_1_1 {
  UINT64 RegionSpaOffset;
  UINT32 SerialNum;
  UINT32 Reserved;
} NVM_COOKIE_DATA_1_1;

#pragma pack(pop)

/**
  Definitions for flags mask for BttArenaInfo structure above.
**/
#define BTTINFO_FLAG_ERROR  0x00000001 //!< Error state (read-only)

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4200 )
#endif

typedef struct {
  UINT32 PreMapLBA[0];
} BTT_MAP;

#if defined(_MSC_VER)
#pragma warning( pop )
#endif

EFI_STATUS
IsNamespaceLocked(
  IN     NAMESPACE *pNamespace,
     OUT BOOLEAN *pIsLocked
  );

/**
  Clean the namespaces list

  @param[in,out] pList: the list to clean
**/
VOID
CleanNamespacesList(
  IN OUT LIST_ENTRY *pList
  );

/**
  Iterates over all of the existing namespaces and installs block io protocols

  This function will install two protocols: block and device path on the namespace handle.
  Also it will allocate memory for the block device (child) device path instance so it can
  be attached to the Controller (provided by the parameter).
  At last, the function will open the Controllers device path instance as a child device.

  Before unloading the driver, CleanNamespaces should be run to close, uninstall those protocols
  and to free the memory.

  @retval EFI_SUCCESS the iteration and installation were successful.
  Other errors from InstallMultipleProtocolInterfaces and OpenProtocol functions.

  Warning! This function does not exit at the first error. It will be logged and then it continues
  to next namespace, trying to create as much working block devices as it is possible.
**/
EFI_STATUS
EFIAPI
InstallProtocolsOnNamespaces(
  );

/**
  This function closes all protocols opened by the block devices handles,
  it will also uninstall the block and device path protocols. At the end
  it deallocated the memory taken for the device path protocol instance.

  @retval EFI_SUCCESS the cleanup completed successfully.
  Other errors returned from UninstallMultipleProtocolInterfaces function.

  Warning! In case of an error, the cleanup does not break, it tries to
  clean the rest of the DIMMs.
**/
EFI_STATUS
EFIAPI
CleanNamespaces(
  );

/**
  Attaches the block device to the DIMM and installs the Block IO Protocol to the Namespace.

  @param[in,out] pNamespace - is the pointer to the NAMESPACE structure that is supposed to be
    exposed in the system.

  @retval EFI_INVALID_PARAMETER if the pNamespace equals NULL.
  @retval EFI_ABORTED if the parent DIMM block window was not properly initialized or the DIMM security state
    could not be determined.
  @retval EFI_NOT_READY if the Namespace was not Enabled.
  @retval EFI_ACCESS_DENIED if the DIMM is locked
  @retval EFI_SUCCESS if the operation completed successfully. Or the Namespace is not Block.
  Other return values from OpenProtocol and InstallMultipleProtocolInterfaces function.
**/
EFI_STATUS
InstallNamespaceProtocols(
  IN OUT NAMESPACE *pNamespace
  );

/**
  Detaches the block device from the DIMM and uninstalls the Block IO Protocol from the Namespace.
  If the Namespace is not enabled, no actions are taken.

  @param[in,out] pNamespace - is the pointer to the NAMESPACE structure that is supposed to be
    removed from the system.

  @retval EFI_INVALID_PARAMETER if the pNamespace equals NULL.
  @retval EFI_ABORTED if the Block IO instance handle equals NULL and no actions can be performed.
  @retval EFI_SUCCESS if the operation completed successfully. Or the Namespace is not Block.
  Other return values from UninstallMultipleProtocolInterfaces function.
**/
EFI_STATUS
UninstallNamespaceProtocols(
  IN OUT NAMESPACE *pNamespace
  );

/**
  Performs a block read or write to the Namespace.
  The function calculates the proper DPA DIMM offset and issues
  the proper read or write operation on the destination DIMM.

  @param[in] pNamespace The Intel NVM Dimm Namespace to perform the IO Block operation.
  @param[in] Lba the Logica Block Addressing block offset to perform the IO on.
  @param[out] pBuffer the destination/source buffer where or from the data will be copied.
  @param[in] BlockLength the length of the buffer - should equal to the namespace block size.
  @param[in] ReadOperation boolean value indicating what type of IO is requested.
    TRUE means a read operation, and FALSE results in write operation.

  @retval EFI_SUCCESS if the IO operation was performed without errors.
  @retval Other return codes from functions:
    DimmRead, DimmWrite, AppDirectIo
**/
EFI_STATUS
IoNamespaceBlock(
  IN     NAMESPACE *pNamespace,
  IN     CONST UINT64 Lba,
     OUT CHAR8 *pBuffer,
  IN     CONST UINT32 BlockLength,
  IN     CONST BOOLEAN ReadOperation
  );

/**
  Performs a read or write to the AppDirect Namespace.
  The data is read/written from/to Interlave Set mapped in system memory.

  @param[in] pNamespace Intel NVM Dimm Namespace to perform the IO operation.
  @param[in] Offset Offset of AppDirect Namespace
  @param[in, out] pBuffer Destination/source buffer where or from the data will be copied.
  @param[in] Nbytes Number of bytes to read/write
  @param[in] ReadOperation boolean value indicating what type of IO is requested.

  @retval EFI_SUCCESS If the IO operation was performed without errors.
  @retval EFI_INVALID_PARAMETER Input parameter is NULL
**/
EFI_STATUS
AppDirectIo(
  IN     NAMESPACE *pNamespace,
  IN     UINT64 Offset,
  IN OUT CHAR8 *pBuffer,
  IN     UINT64 Nbytes,
  IN     BOOLEAN ReadOperation
  );

/**
  Performs a buffer read or write to the namespace.
  The function calculates the proper DPA DIMM offset and issues
  the proper read or write operation on the destination DIMM.

  @param[in] pNamespace The Intel NVM Dimm Namespace to perform the IO buffer operation.
  @param[in] Offset - byte offset on the Namespace to perform the IO on - should be a multiple of cache line size.
  @param[out] pBuffer the destination/source buffer where or from the data will be copied.
  @param[in] BufferLength the length of the buffer - should be a multiple of cache line size.
  @param[in] ReadOperation boolean value indicating what type of IO is requested.
    TRUE means a read operation, and FALSE results in write operation.

  @retval EFI_SUCCESS if the IO operation was performed without errors.
  @retval EFI_INVALID_PARAMETER if pNamespace and/or pBuffer equals NULL.
  @retval EFI_BAD_BUFFER_SIZE if Offset and/or BufferLength are not aligned to the cache line size.
  @retval Other return codes from functions:
    DimmRead, DimmWrite, AppDirectIo
**/
EFI_STATUS
IoNamespaceBytes(
  IN     NAMESPACE *pNamespace,
  IN     UINT64 Offset,
     OUT CHAR8 *pBuffer,
  IN     UINT32 BufferLength,
  IN     BOOLEAN ReadOperation
  );

/**
  Copies the Raw LSA Index data into proper data structures

  @param[in] pRawData Raw data from the PCD
  @param[in] PcdLsaPartitionSize for checking Index size
  @param[out] pLsa The Label Storage Area structure

  @retval EFI_INVALID_PARAMETER Invalid parameter passed
  @retval EFI_SUCCESS Data was copied successfully
**/
EFI_STATUS
RawDataToLabelIndexArea(
   IN     UINT8 *pRawData,
   IN     UINT32 PcdLsaPartitionSize,
   OUT LABEL_STORAGE_AREA *pLsa
);
/**
  Reads Label Storage Area of a specified DIMM.

  Function reads Platform Config Data partition 3. of a DIMM and invokes
  validation subroutine to check for data consistency. Required memory
  will be allocated, it's caller responsibility to free it after it's
  no longer needed.

  @param[in] DimmPid Dimm ID of DIMM from which to read the data
  @param[out] ppLsa Pointer with address at which memory
    LSA data will be stored.

  @retval EFI_INVALID_PARAMETER NULL pointer provided as a parameter
  @retval EFI_DEVICE_ERROR Unable to retrieve data from DIMM or retrieved
    data is not valid
  @retval EFI_SUCCESS Valid LSA retrieved
**/
EFI_STATUS
ReadLabelStorageArea(
  IN     UINT16 DimmPid,
     OUT LABEL_STORAGE_AREA **ppLsa
  );

/**
  Writes Label Storage Area to a specified DIMM.

  Function invokes validation subroutine to check provided data consistency
  and then stores LSA on Platform Config Data partition 3. of a DIMM

  @param[in] DimmPid Dimm ID of DIMM on which to write LSA
  @param[in] pLsa Pointer with LSA structure

  @retval EFI_INVALID_PARAMETER No data provided or the data is invalid
  @retval EFI_DEVICE_ERROR Unable to store data on a DIMM
  @retval EFI_SUCCESS LSA written correctly
**/
EFI_STATUS
WriteLabelStorageArea(
  IN     UINT16 DimmPid,
  IN     LABEL_STORAGE_AREA *pLsa
  );

/**
  Zero the Label Storage Area on the specified DIMM.

  @param[in] DimmPid Dimm ID of DIMM on which to write LSA

  @retval EFI_INVALID_PARAMETER No data provided or the data is invalid
  @retval EFI_OUT_RESOURCES Unable to allocate resources
  @retval EFI_SUCCESS LSA written correctly
**/
EFI_STATUS
ZeroLabelStorageArea(
  IN     UINT16 DimmPid
  );

/**
  Initialize a random seed using current time.

  Get current time first. Then initialize a random seed based on some basic
  mathematics operation on the hour, day, minute, second, nanosecond and year
  of the current time.

  @return The random seed initialized with current time.
  @return 0 if there was an error while getting the current time.
**/
UINT32
EFIAPI
GenerateCurrentTimeSeed(
  VOID
  );

/**
  Generate random numbers in a buffer.

  @param[in, out]  Rand       The buffer to contain random numbers.
  @param[in]       RandLength The length of the Rand buffer.
**/
VOID
RandomizeBuffer(
  IN OUT UINT8  *Rand,
  IN     UINT64  RandLength
  );

/**
  Generate a NamespaceId value

  @retval The generated ID
**/
UINT16
EFIAPI
GenerateNamespaceId(
  IN UINT16 RegionId
  );

/**
  Generate a random (version 4) GUID

  @retval The generated GUID
  @retval Zeroed buffer, if there was a problem while getting the time seed
**/
VOID
EFIAPI
GenerateRandomGuid(
     OUT GUID *pResultGuid
);

/**
  GetNamespaceByName
  Looks through namespaces list searching for a namespace with a particular name.

  @param[in] pName Target Name of the Namespace that the function is searching for.

  @retval NAMESPACE structure pointer if Namespace has been found
  @retval NULL pointer if not found
**/
NAMESPACE*
GetNamespaceByName(
  IN     CHAR8* pName
  );

/**
  Looks through namespaces list searching for a namespace with a particular id.

  @param[in] NamespaceId Target ID of the Namespace that the function is searching for.

  @retval NAMESPACE structure pointer if Namespace has been found
  @retval NULL pointer if not found
**/
NAMESPACE*
GetNamespaceById(
 IN     UINT16 NamespaceId
  );

/**
  Read data from an Intel NVM Dimm Namespace.
  Transform LBA into RDPA and call the Intel NVM Dimm read function.
  The function reads the block size, so the input buffer size needs
  to be at least the namespace block size.

  @param[in] pNamespace The Intel NVM Dimm Namespace to read data from
  @param[in] Lba LBA of the start of the data region to read
  @param[out] pBuffer Buffer to place read data into

  @retval Return values from IoNamespaceBlock function
**/
EFI_STATUS
ReadNamespaceBlock(
  IN     NAMESPACE *pNamespace,
  IN     UINT64 Lba,
     OUT CHAR8 *pBuffer
  );

/**
  Write data to an Intel NVM Dimm Namespace.
  Transform LBA into RDPA and call the Intel NVM Dimm DIMM write function.
  The function writes the block size, so the input buffer size needs
  to be at least the namespace block size.

  @param[in] pNamespace The Intel NVM Dimm Namespace to write data to
  @param[in] Lba LBA of the start of the data region to write
  @param[in] pBuffer Buffer with data to write

  @retval EFI_SUCCESS on a successful write
  @retval Error return values from IoNamespaceBlock function
**/
EFI_STATUS
WriteNamespaceBlock(
  IN     NAMESPACE *pNamespace,
  IN     UINT64 Lba,
  IN     CHAR8 *pBuffer
  );

/**
  Read block from the namespace
  The function reads only one block from the device, the buffer length needs to
  be at least the Namespace block size.

  @param[in] pNamespace Namespace that data will be read from
  @param[in] Lba LBA to retrieve read from
  @param[out] pBuffer pointer to the memory where the result should be stored

  @retval EFI_SUCCESS on a successful read
  @retval Error return values from BttRead or ReadNamespaceBlock function
**/
EFI_STATUS
ReadBlockDevice(
  IN     NAMESPACE *pNamespace,
  IN     UINT64 Lba,
     OUT CHAR8 *pBuffer
  );

/**
  Write to the namespace through the BTT mechanism.
  The function writes only one block from the device, the buffer length needs to
  be at least the Namespace block size.

  @param[in] pNamespace Namespace containing the BTT
  @param[in] Lba LBA to store the write to
  @param[out] pBuffer pointer to the memory where the destination data resides

  @retval EFI_SUCCESS on a successful write
  @retval Error return values from BttWrite or WriteNamespaceBlock function
**/
EFI_STATUS
WriteBlockDevice(
  IN     NAMESPACE *pNamespace,
  IN     UINT64 Lba,
     OUT CHAR8 *pBuffer
  );
/**
Checks whether NamespaceType is AppDirect

Function reads the label version and NameSpaceLabel and
determines whether App Direct Type or not

@retval TRUE if AppDirect Type
@retval FALSE if not AppDirect Type
**/
BOOLEAN
IsNameSpaceTypeAppDirect(IN NAMESPACE_LABEL *pNamespaceLabel, IN BOOLEAN Is_Namespace1_1
);
/*
  Checks if Lsa status of Dimms is not initalized
  for all manageable dimms

  @retval TRUE - if all manageable dimms have
                 lsaStatus set to LSA_NOT_INIT
*/
BOOLEAN IsLSANotInitializedOnDimms();
/**
  Initializes Namespaces inventory

  Function reads LSA data from all DIMMs, then scans for Namespaces
  data in it. All found Namespaces are stored in a list in global
  PMEMDev structure.

  @retval EFI_DEVICE_ERROR Reading LSA data failed
  @retval EFI_ABORTED Reading Namespaces data from LSA failed
  @retval EFI_SUCCESS Namespaces inventory correctly initialized
**/
EFI_STATUS
InitializeNamespaces(
  );

/**
  Aligns the size of the Label Index area and calculates the number of
  free blocks, padding the driver can support.

  @param[in] PcdSize Size of the PCD Partition area
  @param[in] UseLatestLabelVersion Use latest version of Labels
  @param[out] pFreeBlocks The size of free array
  @param[out] pPadding The padding required to complete the index area

  @retval EFI_INVALID_PARAMETER null parameter provided
  @retval EFI_SUCCESS Alignment went well
**/
EFI_STATUS
AlignLabelStorageArea(
  IN     UINT32 PcdSize,
  IN     BOOLEAN UseLatestLabelVersion,
     OUT UINT32 *pFreeBlocks,
     OUT UINT32 *pPadding
  );

/**
  Gets the Label Index Data in a contiguous memory

  @param[in] pLsa The Label Storage Area
  @param[in] LabelIndex The index of the LSA
  @param[out] pRawData Pointer to the contigous memory region

  @retval EFI_INVALID_PARAMETER NULL pointer provided
  @retval EFI_OUT_OF_RESOURCES could not allocate memory
  @retval EFI_SUCCESS Successfully transferred memory
**/
EFI_STATUS
LabelIndexAreaToRawData(
  IN     LABEL_STORAGE_AREA *pLsa,
  IN     UINT32  LabelIndex,
     OUT UINT8 **ppRawData
  );

/**
  Check if LSA of a specified DIMM is initialized.
  If empty LSA is detected then it is initialized.
  If non empty LSA is detected then it's validated
  for data correctness.

  @param[in] pDimm Target DIMM
  @param[in] LabelVersionMajor Major version of label to init
  @param[in] LabelVersionMinor Minor version of label to init

  @retval EFI_INVALID_PARAMETER NULL pointer provided
  @retval EFI_VOLUME_CORRUPTED LSA data is broken
  @retval EFI_SUCCESS Valid LSA detected or initialized correctly
**/
EFI_STATUS
InitializeLabelStorageArea(
  IN     DIMM *pDimm,
  IN     UINT16 LabelVersionMajor,
  IN     UINT16 LabelVersionMinor
  );

/**
  Initialize all label storage areas

  @param[in] ppDimms Array of DIMMs
  @param[in] DimmsNum Number of DIMMs
  @param[in] LabelVersionMajor Major version of label to init
  @param[in] LabelVersionMinor Minor version of label to init
  @param[out] pCommandStatus Pointer to command status structure

  @retval EFI_SUCCESS success
  @retval EFI_INVALID_PARAMETER pDimmList is NULL
  @retval return codes from SendConfigInputToDimm
**/

EFI_STATUS
InitializeAllLabelStorageAreas(
  IN     DIMM **ppDimms,
  IN     UINT32 DimmsNum,
  IN     UINT16 LabelVersionMajor,
  IN     UINT16 LabelVersionMinor,
     OUT COMMAND_STATUS *pCommandStatus
  );

/**
  Find the Memory map range with the lowest DPA that contains a given region size in a given IS.

  IS that is passed in is of the type the user wishes to create NS on
  Size has already been aligned for NS requirements

  If size is MAX_UINT64 ignore lowest DPA and instead find the largest free memory region for that exists on
  all dimms in the interleave set

  @param[in] pIS pointer to Interleave Set that the memory range is contained in.
  @param[in] Size Minimum size the memory map range must be, if MAX_UINT64 then find the largest possible.
  @param[out] pFoundRange pointer memory range structure range will be copied into

  @retval EFI_SUCCESS everything went fine
  @retval EFI_OUT_OF_RESOURCES when memory allocation fails
  @retval EFI_INVALID_PARAMETER if any of the parameters is a NULL.
  @retval EFI_NOT_FOUND could not find a region of given size in IS
  Other return codes from functions:
    GetDimmFreemap
    GetListSize
**/

EFI_STATUS
FindADMemmapRangeInIS(
  IN      NVM_IS *pIS,
  IN      UINT64 Size,
      OUT MEMMAP_RANGE *pFoundRange
  );
/**
 Namespace Capacity Alignment

 Aligning the Namespace Capacity Helper Function
 @param [in] NamespaceCapacity Namespace Capacity provided
 @param [in] DimmCount
 @param [out] *pAlignedNamespaceCapacity  Total Namespace Capacity after alignment

 @retval EFI_INVALID_PARAMETER Invalid set of parameters provided
 @retval EFI_SUCCESS Namespace capacity aligned
**/
EFI_STATUS
AlignNamespaceCapacity(
  IN  UINT64 NamespaceCapacity,
  IN  UINT64 DimmCount,
  OUT UINT64* pAlignedNamespaceCapacity
);
/**
  Provision Namespace capacity on a DIMM or Interleave Set.

  Only one of target pointers must be provided, other one must be NULL.
  Function analyses possibility of allocation of NamespaceCapacity bytes
  on a DIMM or on an Interleave Set. If there is enough free space
  function assigns appropriate Namespace regions.

  @param[in] pDimm DIMM structure pointer in case of Block Mode Namespaces
  @param[in] pIS Interleave Set structure pointer in case of Persistent Memory Namespaces
  @param[in out] NamespaceCapacity Required capacity in bytes
  @param[out] pNamespace Namespace which regions will be updated if allocation
    is successful

  @retval EFI_INVALID_PARAMETER Invalid set of parameters provided
  @retval EFI_OUT_OF_RESOURCES Not enough free space on target
  @retval EFI_SUCCESS Namespace capacity allocated
**/
EFI_STATUS
AllocateNamespaceCapacity(
  IN     DIMM *pDimm, OPTIONAL
  IN     NVM_IS *pIS, OPTIONAL
  IN OUT UINT64 *pNamespaceCapacity,
     OUT NAMESPACE *pNamespace
  );

/**
  This function tries to match the given EFI_HANDLE in the list of existing
  block namespace handles.

  @param[in] Handle - the EFI_HANDLE that the caller needs the NAMESPACE structure pointer to.

  @retval NULL - if the handle did not match any installed handles.
  @retval pointer to the result NAMESPACE structure.
**/
NAMESPACE *
HandleToNamespace(
  IN     EFI_HANDLE Handle
  );

/**
  Function updates sequence number and checksum of a next Index Block
  of the specified Label Storage Area.

  @param[in] pLsa Target Label Storage Area structure

  @retval EFI_INVALID_PARAMETER Invalid set of parameters provided
  @retval EFI_SUCCESS Operation successful
**/
EFI_STATUS
UpdateLsaIndex(
  IN     LABEL_STORAGE_AREA *pLsa
  );

/**
  Check if the label version on the DIMM is new

  @param[in] pDimm The target DIMM
  @param[out] pHasLatestIndexVersion Pointer to boolean if the index is v1.2

  @retval EFI_INVALID_PARAMETER Null parameter is passed
  @retval EFI_NOT_FOUND If no index blocks found on the DIMM
  @retval EFI_SUCCESS If found the label version successfully
**/
EFI_STATUS
CheckDimmNsLabelVersion(
  IN     DIMM *pDimm,
     OUT BOOLEAN *pHasLatestIndexVersion
  );

/**
  Check if we can use the latest index block version

  @param[in] pNamespace The target namespace
  @param[in out] pUseLatestVersion Returns if the LSA uses new version
                 TRUE if all DIMMs in the Namespace have same Major/Minor
                      version of index blocks. No index blocks is treated
                      as version 1.2
                 FALSE if the DIMMs in the Namespace have a mismatch of the
                       Major/Minor versions in the index block
  @retval EFI_INVALID_PARAMETER Null parameter is passed
  @retval EFI_DEVICE_ERROR If the versions don't match on the DIMMs
  @retval EFI_OUT_OF_RESOURCES Memory allocation failed
  @retval EFI_SUCCESS Retrieved versions successfully
**/
EFI_STATUS
UseLatestNsLabelVersion(
  IN     NVM_IS *pIS,
  IN     DIMM *pParentDimm,
     OUT BOOLEAN *pUseLatestVersion
  );

/**
  Creates the namespace labels based on the version of the labels

  @param[in] pNamespace The target namespace
  @param[in] Index The index of the label in the namespace
  @param[in] UseLatestVersion Use latest version of labels
  @param[in out] pLabel The output label

  @retval EFI_INVALID_PARAMETER Null parameter passed
  @retval EFI_SUCCESS Created new label successfully
**/
EFI_STATUS
CreateNamespaceLabels(
  IN     NAMESPACE *pNamespace,
  IN     UINT16 Index,
  IN     BOOLEAN UseLatestVersion,
  IN OUT NAMESPACE_LABEL *pLabel
  );

/**
  Insert Namespace labels into a Label Storage Area of a DIMM.
  Function updates all required fields, index bitmap, sequence
  numbers, etc. Finally Label Storage Area is written to the DIMM.

  @param[in] pDimm DIMM structure pointer to which insert the labels
  @param[in] ppLabel Array of label structures to be inserted
  @param[in] LabelCount Number of labels in the array
  @param[in] LabelVersionMajor Major version of label to init
  @param[in] LabelVersionMinor Minor version of label to init

  @retval EFI_INVALID_PARAMETER Invalid set of parameters provided
  @retval EFI_OUT_OF_RESOURCES No more label free slots in LSA
  @retval EFI_SUCCESS Operation successful
**/
EFI_STATUS
InsertNamespaceLabels(
  IN     DIMM *pDimm,
  IN     NAMESPACE_LABEL **ppLabel,
  IN     UINT16 LabelCount,
  IN     UINT16 LabelVersionMajor,
  IN     UINT16 LabelVersionMinor
  );

/**
  Modify attributes of all Namespace labels identified by UUID on a DIMM.
  Function updates all required fields, index bitmap, sequence
  numbers, etc. Finally Label Storage Area is written to the DIMM.

  @param[in] pDimm DIMM structure pointer to which insert the labels
  @param[in] pUuid Target Namespace identifier
  @param[in] pFlags New value of Flags field
  @param[in] pName New value of name
  @param[in] LabelsNum the new value for the total labels in the block namespace labels set

  @retval EFI_INVALID_PARAMETER Invalid set of parameters provided
  @retval EFI_ABORTED Failed to modify the labels
  @retval EFI_SUCCESS Operation successful
**/
EFI_STATUS
ModifyNamespaceLabels(
  IN     DIMM *pDimm,
  IN     GUID *pUuid,
  IN     UINT32 *pFlags, OPTIONAL
  IN     CHAR8 *pName, OPTIONAL
  IN     UINT16 LabelsNum OPTIONAL
  );

/**
  Remove all Namespace labels with specified UUID from a DIMM.
  Function updates all required fields, index bitmap, sequence
  numbers, etc. Finally Label Storage Area is written to the DIMM.

  @param[in] pDimm DIMM structure pointer to which insert the labels
  @param[in] pUuid Target Namespace identifier
  @param[in] Dpa Limit removal to only one label corresponding to this DPA

  @retval EFI_INVALID_PARAMETER Invalid set of parameters provided
  @retval EFI_SUCCESS Operation successful
**/
EFI_STATUS
RemoveNamespaceLabels(
  IN     DIMM *pDimm,
  IN     GUID *pUuid,
  IN     UINT64 Dpa OPTIONAL
  );

/**
  This function calculates the real block and raw size that needs to be allocated on the DIMM in
  order to allow the caller of his desired block size and raw size.

  The function checks the Raw size to block size alignment and the block size to cache line size alignment,
  doing appropriate increasing or decreasing of the Raw Size.

  At least one of the optional parameters is required to be specified.

  One of: BlockCount or DesiredRawSize must be specified, the other needs to equal 0.

  @param[in] DesiredRawSize the raw size of the device (should be multiply of DesiredBlockSize, but if not,
    the function will decrease it accordingly).
  @param[in] DesiredBlockSize the required block size that will be aligned to the cache line size.
  @param[in] BlockCount the input block amount - specified by the caller.
  @param[out] pRealDimmRawSize pointer to where the real DIMM raw size needed will be stored.
  @param[out] pRealDimmBlockSize pointer to where the real DIMM block size will be stored.
  @param[out] pBlockCount pointer to where the calculated block count will be stored.

  @retval EFI_SUCCESS the parameters are OK and the values were calculated properly.
  @retval EFI_INVALID_PARAMETER the provided parameters are not passed accordingly to the function header.
**/
EFI_STATUS
GetRealRawSizeAndRealBlockSize(
  IN     UINT64 DesiredRawSize OPTIONAL,
  IN     UINT32 DesiredBlockSize,
  IN     UINT64 BlockCount OPTIONAL,
     OUT UINT64 *pRealDimmRawSize OPTIONAL,
     OUT UINT32 *pRealDimmBlockSize OPTIONAL,
     OUT UINT64 *pBlockCount OPTIONAL
  );

/**
  Read data from an Intel NVM Dimm Namespace.
  Transform buffer offset into RDPA.
  Call the Intel NVM Dimm read function.
  The buffer length and the offset need to be aligned to the cache line size.

  @param[in] pNamespace The Intel NVM Dimm Namespace to read data from
  @param[in] Offset bytes offset of the start of the data region to read
  @param[in] Length the length of the buffer to read
  @param[out] pBuffer Buffer to place read data into

  @retval EFI_SUCCESS on a successful read
  @retval Error return values from IoNamespaceBlock function
**/
EFI_STATUS
ReadNamespaceBytes(
  IN     NAMESPACE *pNamespace,
  IN     CONST UINT64 Offset,
     OUT VOID *pBuffer,
  IN     CONST UINT64 Length
  );

/**
  Write data to an Intel NVM Dimm Namespace.
  Transform buffer offset into RDPA.
  Call the Intel NVM Dimm write function.
  The buffer length and the offset need to be aligned to the cache line size.

  @param[in] pNamespace The Intel NVM Dimm Namespace to write data to
  @param[in] Offset bytes offset of the start of the data region to write
  @param[in] Length the length of the buffer to write
  @param[in] pBuffer Buffer with data to write

  @retval EFI_SUCCESS on a successful write
  @retval Error return values from IoNamespaceBlock function
**/
EFI_STATUS
WriteNamespaceBytes(
  IN     NAMESPACE *pNamespace,
  IN     CONST UINT64 Offset,
  IN     VOID *pBuffer,
  IN     CONST UINT64 Length
  );

/**
  Calculate checksum (called Cookie) from Namespace ranges

  @param[in]     pFitHead Fully populated NVM Firmware Interface Table
  @param[in]     pIS  InterleaveSet that interleave set cookie will be calculated for

  @retval EFI_SUCCESS           Success
  @retval EFI_LOAD_ERROR        When calculated cookie didn't match
  @retval EFI_INVALID_PARAMETER Input parameter is NULL
**/
EFI_STATUS
CalculateISetCookie(
  IN     ParsedFitHeader *pFitHead,
  IN OUT NVM_IS *pIS
  );

/**
  Calculate checksum (called Cookie) from Namespace ranges for Namespace Labels V1.1

  @param[in]     pFitHead Fully populated NVM Firmware Interface Table
  @param[in]     pIS  InterleaveSet that interleave set cookie will be calculated for

  @retval EFI_SUCCESS           Success
  @retval EFI_LOAD_ERROR        When calculated cookie didn't match
  @retval EFI_INVALID_PARAMETER Input parameter is NULL
**/
EFI_STATUS
CalculateISetCookieVer1_1(
  IN     ParsedFitHeader *pFitHead,
  IN OUT NVM_IS *pIS
  );

/**
  Find and assign parent Interleave Set for AppDirect Namespace

  @param[in,out] pNamespace AppDirect Namespace that Interleave Set will be found for

  @retval EFI_SUCCESS           Success
  @retval EFI_INVALID_PARAMETER Input parameter is NULL
  @retval EFI_NOT_FOUND         Interleave Set has not been found
**/
EFI_STATUS
FindAndAssignISForNamespace(
  IN OUT NAMESPACE *pNamespace
  );

/**
  Calculate actual Namespace size on Dimm

  @param[in] BlockSize the size of each of the block in the device.
  @param[in] Capacity usable capacity
  @param[in] Mode -  boolean value to decide when the block namespace should have the BTT arena included
  @param[out] pActualBlockCount actual block count that needs to be occupied on Dimm
  @param[out] pActualCapacity actual capacity that needs to be occupied on Dimm
  @param[out] pCommandStatus Structure containing detailed NVM error codes

  @retval EFI_SUCCESS           Success
  @retval EFI_INVALID_PARAMETER Input parameter is NULL
**/
EFI_STATUS
ConvertUsableSizeToActualSize(
  IN     UINT32 BlockSize,
  IN     UINT64 Capacity,
  IN     BOOLEAN Mode,
     OUT UINT64 *pActualBlockCount,
     OUT UINT64 *pActualCapacity,
     OUT COMMAND_STATUS *pCommandStatus
  );

/**
  Check if there is at least one Namespace on specified Dimms.

  @param[in] pDimms Array of Dimm pointers
  @param[in] DimmIdsCount Number of items in array
  @param[out] pFound Output variable saying if there is Namespace or not

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER one or more parameters are NULL
**/
EFI_STATUS
IsNamespaceOnDimms(
  IN     DIMM *pDimms[],
  IN     UINT32 DimmsNum,
     OUT BOOLEAN *pFound
  );

/**
  Retrieve mapped AppDirect memory from NFIT.

  @param[in] pFitHead Fully populated NVM Firmware Interface Table
  @param[in, out] pIS Interleave Set to retrieve AppDirect I/O structures for

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER one or more parameters are NULL
  @retval EFI_NOT_FOUND SPA table has not been found
**/
EFI_STATUS
RetrieveAppDirectMappingFromNfit(
  IN     ParsedFitHeader *pFitHead,
  IN OUT NVM_IS *pIS
  );

/**
  Get block size of block device for Namesapce

  @param[in] pNamespace Namespace that block size of block device will be retrieved for

  @retval Block size
**/
UINT64
GetBlockDeviceBlockSize(
  IN     NAMESPACE *pNamespace
  );

/**
  Get persistent memory type for Namespace

  @param[in] pNamespace Namespace that persistent memory type will be determined for
  @param[out] pPersistentMemType Persistent memory type

  @retval EFI_SUCCESS Success
  @retval EFI_INVALID_PARAMETER One or more parameters are NULL
**/
EFI_STATUS
GetPersistentMemoryType(
  IN     NAMESPACE *pNamespace,
     OUT UINT8 *pPersistentMemType
  );

/**
  Get accessible capacity in bytes

  @param[in] pNamespace Namespace that accessible capacity will be calculated for

  @retval Accessible capacity
**/
UINT64
GetAccessibleCapacity(
  IN     NAMESPACE *pNamespace
  );

/**
  Get raw capacity in bytes

  @param[in] pNamespace Namespace that raw capacity will be calculated for

  @retval Raw capacity
**/
UINT64
GetRawCapacity(
  IN     NAMESPACE *pNamespace
  );

#ifndef OS_BUILD
/**
  Checks to see if a given address block collides with one or more of the addresses BIOS has marked as bad

  @param[in] Address The base address
  @param[in] Length The address range size

  @retval EFI_SUCCESS If the range does not collide with an ARS address
  @retval EFI_DEVICE_ERROR If the range is found to collide with one or more addresses from BIOS
**/
EFI_STATUS
IsAddressRangeInArsList(
  IN     UINT64  Address,
  IN     UINT64  Length
);
#endif

#endif /** _NAMESPACE_H_ **/
