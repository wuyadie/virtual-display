/*++

Copyright (c) 1990-2000 Microsoft Corporation All Rights Reserved

Module Name:

    Toaster.h

Abstract:

    Header file for the toaster driver modules.

Environment:

    Kernel mode

--*/


#if !defined(_LJB_VMON_PRIVATE_H_)
#define _LJB_VMON_PRIVATE_H_

#include <ntddk.h>
#include <wdf.h>

#include "wmilib.h"
#include "driver.h"
#include "public.h"
#include "lci_usbav_ioctl.h"
#include "lci_display_internal_ioctl.h"

#define LJB_VMON_POOL_TAG (ULONG) 'nomV'

#define MOFRESOURCENAME L"VMON_FUNC_Wmi"

__checkReturn
PVOID
FORCEINLINE
LJB_VMON_GetPoolZero(
    __in SIZE_T NumberOfBytes
    )
    {
    PVOID   pBuf;

    pBuf = ExAllocatePoolWithTag(
        NonPagedPool,
        NumberOfBytes,
        LJB_VMON_POOL_TAG
        );
    if (pBuf != NULL)
        RtlZeroMemory(pBuf, NumberOfBytes);
    return pBuf;
    }

#define LJB_VMON_FreePool(p) ExFreePoolWithTag(p, LJB_VMON_POOL_TAG)

/*
 * Debug Print macro.
 * To enable debugging message, set DEFAULT registry value
 * to 1 under "Debug Print Filter", and set DebugMask registry
 * value under driver's software key.
 */
#define DBGLVL_ERROR        (1 << 0)
#define DBGLVL_PNP          (1 << 1)
#define DBGLVL_POWER        (1 << 2)
#define DBGLVL_FLOW         (1 << 3)
#define DBGLVL_INFO         (1 << 4)
#define DBGLVL_FNENTRY      (1 << 5)

#define DBGLVL_DEFAULT      (DBGLVL_ERROR | DBGLVL_PNP | DBGLVL_POWER)

#if DBG
#define LJB_VMON_Printf(pDevCtx, Mask, _x_)      \
    if (pDevCtx->DebugLevel & Mask)                 \
        {                                           \
        DbgPrint(" LJB_VMON:");                     \
        DbgPrint _x_;                               \
        }
#else
#define LJB_VMON_Printf(pDevCtx, Mask, _x_)
#endif

#define LJB_VMON_PrintfAlways(pDevCtx, Mask, _x_)      \
        DbgPrint(" LJB_VMON:");                     \
        DbgPrint _x_;



#define MAX_POINTER_SIZE        (256*256*4)   /* width(256)/height(256)/4Byte */
typedef struct _LCI_USBAV_PRIMARY_SURFACE
    {
    LIST_ENTRY                  ListEntry;
    HANDLE                      hPrimarySurface;
    PVOID                       pRemoteBuffer;

    PVOID                       pBuffer;
    SIZE_T                      BufferSize;

    UINT                        Width;
    UINT                        Height;
    UINT                        Pitch;
    UINT                        BytesPerPixel;

    MDL *                       pMdl;
    PVOID                       pUserBuffer;
    LONG                        ReferenceCount;
    BOOLEAN                     bIsAcquiredByUserMode;
    WDFFILEOBJECT               UserFileObject;

	UCHAR * 					pOrigSurfPosStart;
	UINT 						EffCurWidth;
	UINT 						EffCurHeight;
	BOOLEAN						bIsCursorDrew;
	UCHAR                       SurfBitmap[MAX_POINTER_SIZE];

	ULONG						FrameId;
	BOOLEAN						bTransferDone;
	BOOLEAN						bBusyBltting;
    } LCI_USBAV_PRIMARY_SURFACE;

typedef struct _LCI_POINTER_INFO
    {
    /*
     updated by DxgiDdiSetPointerPosition
     */
    INT                             X;
    INT                             Y;
    BOOLEAN                         Visible;

    /*
     Updated by DxgkDdiSetPointerShape
     */
    DXGK_POINTERFLAGS               Flags;
    UINT                            Width;
    UINT                            Height;
    UINT                            Pitch;
    UCHAR                           Bitmap[MAX_POINTER_SIZE];
    UINT                            XHot;
    UINT                            YHot;
    } LCI_POINTER_INFO;

typedef struct _LCI_WAIT_UPDATE_REQUEST
    {
    LIST_ENTRY                      ListEntry;
    WDFREQUEST                      Request;
    LCI_WAIT_FOR_UPDATE *           pInputWaitUpdateData;
    LCI_WAIT_FOR_UPDATE *           pOutputWaitUpdateData;
    ULONG                           IoctlCode;
    } LCI_WAIT_UPDATE_REQUEST;

typedef struct _LJB_VMON_CTX
    {
    WDFWMIINSTANCE                  WmiDeviceArrivalEvent;
    BOOLEAN                         WmiPowerDeviceEnableRegistered;
    TOASTER_INTERFACE_STANDARD      BusInterface;
    ULONG                           DebugLevel;

    LCI_GENERIC_INTERFACE           TargetGenericInterface;
    LONG                            InterfaceReferenceCount;

    KSPIN_LOCK                      PrimarySurfaceListLock;
    LIST_ENTRY                      PrimarySurfaceListHead;

    KSPIN_LOCK                      WaitRequestListLock;
    LIST_ENTRY                      WaitRequestListHead;

    KSPIN_LOCK                      PendingIoctlListLock;

//    volatile ULONG                  LatestFrameId;
    PVOID                           hLatestPrimarySurface;

	/*
	 Store pointer shape and position
	*/
	LCI_POINTER_INFO				PointerInfo;
    BOOLEAN                         DeviceDead;

    ULONG                           MediaPlayerState;
    BOOLEAN                         bCursorUpdatePending;
    ULONG			                FrameIdSent;
	LONG 		                    AcquirelistCount;
	ULONG							LatestFrameId;
    } LJB_VMON_CTX, *PLJB_VMON_CTX;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(LJB_VMON_CTX, LJB_VMON_GetVMonCtx)

//
// Connector Types
//

#define TOASTER_WMI_STD_I8042 0
#define TOASTER_WMI_STD_SERIAL 1
#define TOASTER_WMI_STD_PARALEL 2
#define TOASTER_WMI_STD_USB 3

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD       LJB_VMON_EvtDeviceAdd;

EVT_WDF_DEVICE_CONTEXT_CLEANUP  LJB_VMON_EvtDeviceContextCleanup;
EVT_WDF_DEVICE_D0_ENTRY         LJB_VMON_EvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT          LJB_VMON_EvtDeviceD0Exit;
EVT_WDF_DEVICE_PREPARE_HARDWARE LJB_VMON_EvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE LJB_VMON_EvtDeviceReleaseHardware;
EVT_WDF_DEVICE_SURPRISE_REMOVAL	LJB_VMON_EvtDeviceSurpriseRemoval;

EVT_WDF_DEVICE_SELF_MANAGED_IO_INIT LJB_VMON_EvtDeviceSelfManagedIoInit;

//
// Io events callbacks.
//
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL
                                    LJB_VMON_InternalDeviceIoControl;
EVT_WDF_IO_IN_CALLER_CONTEXT        LJB_VMON_IoInCallerContext;
EVT_WDF_IO_QUEUE_IO_READ            LJB_VMON_EvtIoRead;
EVT_WDF_IO_QUEUE_IO_STOP            LJB_VMON_EvtIoStop;

EVT_WDF_IO_QUEUE_IO_WRITE           LJB_VMON_EvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL  LJB_VMON_EvtIoDeviceControl;
EVT_WDF_DEVICE_FILE_CREATE          LJB_VMON_EvtDeviceFileCreate;
EVT_WDF_FILE_CLOSE                  LJB_VMON_EvtFileClose;

NTSTATUS
LJB_VMON_GenericIoctl(
    __in PVOID          ProviderContext,
    __in ULONG          IoctlCode,
    __in_opt PVOID      InputBuffer,
    __in SIZE_T         InputBufferSize,
    __out_opt PVOID     OutputBuffer,
    __in SIZE_T         OutputBufferSize,
    __out ULONG *       BytesReturned
    );

NTSTATUS
VMON_WmiRegistration(
    __in WDFDEVICE Device
    );

//
// Power events callbacks
//
EVT_WDF_DEVICE_ARM_WAKE_FROM_S0         LJB_VMON_EvtDeviceArmWakeFromS0;
EVT_WDF_DEVICE_ARM_WAKE_FROM_SX         LJB_VMON_EvtDeviceArmWakeFromSx;
EVT_WDF_DEVICE_DISARM_WAKE_FROM_S0      LJB_VMON_EvtDeviceDisarmWakeFromS0;
EVT_WDF_DEVICE_DISARM_WAKE_FROM_SX      LJB_VMON_EvtDeviceDisarmWakeFromSx;
EVT_WDF_DEVICE_WAKE_FROM_S0_TRIGGERED   LJB_VMON_EvtDeviceWakeFromS0Triggered;
EVT_WDF_DEVICE_WAKE_FROM_SX_TRIGGERED   LJB_VMON_EvtDeviceWakeFromSxTriggered;

PCHAR
DbgDevicePowerString(
    IN WDF_POWER_DEVICE_STATE Type
    );

//
// WMI event callbacks
//
EVT_WDF_WMI_INSTANCE_QUERY_INSTANCE     EvtWmiInstanceStdDeviceDataQueryInstance;
EVT_WDF_WMI_INSTANCE_QUERY_INSTANCE     EvtWmiInstanceToasterControlQueryInstance;
EVT_WDF_WMI_INSTANCE_SET_INSTANCE       EvtWmiInstanceStdDeviceDataSetInstance;
EVT_WDF_WMI_INSTANCE_SET_INSTANCE       EvtWmiInstanceToasterControlSetInstance;
EVT_WDF_WMI_INSTANCE_SET_ITEM           EvtWmiInstanceToasterControlSetItem;
EVT_WDF_WMI_INSTANCE_SET_ITEM           EvtWmiInstanceStdDeviceDataSetItem;
EVT_WDF_WMI_INSTANCE_EXECUTE_METHOD     EvtWmiInstanceToasterControlExecuteMethod;

NTSTATUS
LJB_VMON_FireArrivalEvent(
    __in WDFDEVICE  Device
    );

#endif  // _TOASTER_H_

