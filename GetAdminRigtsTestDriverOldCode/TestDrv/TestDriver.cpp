#include "Precompile.h"
#include "dbg.h"
#include "TestDriver.h"


/*===========================================================================

  TestDriver.cpp
  
	DESCRIPTION
	This source file Is the main file defining the driver;
	
	  Copyright (c) 2006 by xxxxxx technologies. All Rights Reserved.
===========================================================================*/
/* Changes are documented in reverse order */
/*===========================================================================

  EDIT HISTORY FOR FILE
  
	when       who     what, where, why
	--------   ---     ----------------------------------------------------------
	5/10/06  ephraim    created
===========================================================================*/

NTSTATUS OSVersionInit();

#define SIGNATURE 'EFRA' 

NTSTATUS
DeviceCreate(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DeviceDestroy(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DeviceOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DeviceClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


TestDriver*  GDialerApiDriver = NULL;

DWORD AGetWindowsBuildNumber();
extern "C"
NTSTATUS
DriverEntry(
			IN PDRIVER_OBJECT DriverObject,
			IN PUNICODE_STRING RegistryPath
			)
{
	DebugInit(DriverObject,RegistryPath);
	
	PROC_ENTRY
	NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;
	GDialerApiDriver = new TestDriver();
	if (GDialerApiDriver)
	{
		Status = GDialerApiDriver->Test_Dr_DriverEntry(DriverObject,RegistryPath);
      if( ! NT_SUCCESS( Status )) 
		{
			DBG_TRACE(DBG_ERROR,("Error loading driver"));
			TestDriver::Test_Dr_DriverExit(DriverObject);
		}
	}
	DBG_TRACE(DBG_MESSAGE,("Windows version: 0x%X", AGetWindowsBuildNumber() ));
	return Status;
};


typedef struct tagGetSystemContextData
{
//   KEVENT  Event1;
//   KEVENT  Event2;
   HANDLE hContext;

}GetSystemContextData, *pGetSystemContextData;

GetSystemContextData SystemContextData;
NTSTATUS imp_CaptureSecurityContext(IN PETHREAD pThread, OUT PHANDLE phSecurity);
VOID imp_ReleaseSecurityContext(IN PHANDLE   phSecurity);

NTSTATUS
TestDriver::Test_Dr_DriverEntry(
			IN PDRIVER_OBJECT DriverObject,
			IN PUNICODE_STRING RegistryPath
			)
{
//	if (KD_DEBUGGER_NOT_PRESENT == TRUE) goto DBGGO;
	
	PROC_ENTRY
		
		DBG_TRACE(DBG_MESSAGE,(""ORIGINALFILENAME"" " Driver loaded Build Date: "__DATE__" Time: "__TIME__" Version %s\n",
		VER_FILEVERSION_STR));
#ifdef DBG_USE_WPP
		DBG_TRACE(DBG_MESSAGE,("Using the WPP Tracer \n"));
#endif
	NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;


	
	__try
	{
		DriverObject->DriverUnload = TestDriver::Test_Dr_DriverExit;
		m_RegistryPath.MaximumLength = RegistryPath->Length + sizeof(UNICODE_NULL);
		m_RegistryPath.Length = RegistryPath->Length;
		
		// allocate memory to hold this registry path,
		m_RegistryPath.Buffer = (USHORT*)ExAllocatePoolWithTag(NonPagedPool,m_RegistryPath.MaximumLength,'PPWF');
		
		if (!m_RegistryPath.Buffer)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES ;
			DBG_TRACE(DBG_ERROR,("ERROR!... Couldn't allocate memory(Size=%d) for registry path (Status=%X)",
				RegistryPath->MaximumLength, Status));
			__leave;
		}
		
		RtlCopyUnicodeString(&m_RegistryPath, RegistryPath);
		DBG_TRACE(DBG_MESSAGE,("Registry path=[%wZ]", &m_RegistryPath) );
		
      Status = DeviceCreate(DriverObject);
		DBG_TRACE(DBG_MESSAGE,("DeviceCreate returned 0x%X", Status) );
      if( ! NT_SUCCESS( Status )) 
      {
			__leave;
      }

      Status = OSVersionInit();
		DBG_TRACE(DBG_MESSAGE,("OSVersionInit returned 0x%X", Status) );
      if( ! NT_SUCCESS( Status )) 
      {
			__leave;
      }

      PETHREAD pThread = PsGetCurrentThread();
      Status = imp_CaptureSecurityContext(pThread, &SystemContextData.hContext);
	   DBG_TRACE(DBG_MESSAGE,("imp_CaptureSecurityContext returned 0x%X", Status) );
      if( ! NT_SUCCESS( Status )) 
      {
         SystemContextData.hContext = NULL;
			__leave;
      }


      Status = STATUS_SUCCESS;
	}
	__finally
	{
		
	}
	
	
return Status;
}


void TestDriver::Test_Dr_DriverExit(IN PDRIVER_OBJECT DriverObject)
{
	PROC_ENTRY
	if (GDialerApiDriver)
	{
      DeviceDestroy(DriverObject);
		if (GDialerApiDriver->m_RegistryPath.Buffer) ExFreePool(GDialerApiDriver->m_RegistryPath.Buffer);
      imp_ReleaseSecurityContext(&SystemContextData.hContext);
      delete GDialerApiDriver;
	}
	DebugStop();
};

TestDriver::TestDriver()
{
	PROC_ENTRY
};

TestDriver::~TestDriver()
{
	PROC_ENTRY
};

#define CLEAR_FLAGS(flags)          ((flags)=0)
#define SET_FLAGS(flags,bits)       ((flags) |= (bits))
#define RESET_FLAGS(flags,bits)     ((flags) &= ~(bits))
#define GET_FLAGS(flags,bits)       ((flags) & (bits))


// O.S version
#define AWIN_VER_UNKNOWN     0
#define AWIN_VER_95          1
#define AWIN_VER_95_B        2
#define AWIN_VER_98          3
#define AWIN_VER_98_SE       4
#define AWIN_VER_ME          5
#define AWIN_VER_NT4         6
#define AWIN_VER_2000        7
#define AWIN_VER_XP          8
#define AWIN_VER_SERVER_2003 9
#define AWIN_VER_VISTA       10

// Statics that holds the O.S version
typedef struct tagOSVERSION_DATA
{
   // Version information
   struct
   {
      UINT                          BaseVersion ;
      BOOLEAN                       bIsMultiprocessor ;  // 
      BOOLEAN                       bIsCheckedBuild ;    // Free/Chekced build
      RTL_OSVERSIONINFOEXW          OsVersionInfoEx ; 
   } WinVerInfo ;

} OSVERSION_DATA, *POSVERSION_DATA;

// global define
OSVERSION_DATA g_BaseData = {0} ;

PWCHAR g_wszSystemPath = NULL;

// Taken from Microsoft's KB837643
extern "C"
extern PULONG InitSafeBootMode ;
#define SAFEBOOT_MINIMAL  1
#define SAFEBOOT_NETWORK  2
#define SAFEBOOT_DSREPAIR 3

/*------------------------------------------------------------------------
OSVersionInit()
   Description:
      1. Gathers Windows version information
   Returns:
      NTSTATUS 
------------------------------------------------------------------------*/
NTSTATUS OSVersionInit()
{
	PROC_ENTRY
   NTSTATUS Status;
   // Get Windows version
   {      
      g_BaseData.WinVerInfo.OsVersionInfoEx.dwOSVersionInfoSize = sizeof(g_BaseData.WinVerInfo.OsVersionInfoEx) ;      

      g_BaseData.WinVerInfo.bIsCheckedBuild = PsGetVersion( &g_BaseData.WinVerInfo.OsVersionInfoEx.dwMajorVersion ,
                                                            &g_BaseData.WinVerInfo.OsVersionInfoEx.dwMinorVersion,
                                                            &g_BaseData.WinVerInfo.OsVersionInfoEx.dwBuildNumber , 
                                                            NULL ) ;// PUNICODE_STRING CSDVersion OPTIONAL

      
      Status = RtlGetVersion( (PRTL_OSVERSIONINFOW)&g_BaseData.WinVerInfo.OsVersionInfoEx ) ;
      if ( STATUS_SUCCESS != Status )
      {
         DBG_TRACE(DBG_MESSAGE,("RtlGetVersion Status=0x%X", Status));
      }

      switch( g_BaseData.WinVerInfo.OsVersionInfoEx.dwMajorVersion )
      {
         case 4:
            DBG_TRACE(DBG_MESSAGE,("Windows NT4 - we don't care what SP"));
            g_BaseData.WinVerInfo.BaseVersion = AWIN_VER_NT4 ;
            break ;
            
         case 5:
            // Check Minor version
            switch( g_BaseData.WinVerInfo.OsVersionInfoEx.dwMinorVersion )
            {
               case 0 :
                  g_BaseData.WinVerInfo.BaseVersion = AWIN_VER_2000 ;
                  DBG_TRACE(DBG_MESSAGE,("Windows 2000"));
                  break ;
            
               case 1 :
                  g_BaseData.WinVerInfo.BaseVersion = AWIN_VER_XP ;
                  DBG_TRACE(DBG_MESSAGE,("Windows XP"));
                  break ;

               case 2 :
                  g_BaseData.WinVerInfo.BaseVersion = AWIN_VER_SERVER_2003 ;
                  DBG_TRACE(DBG_MESSAGE,("Windows Server 2003"));
                  break ;

               default:
                  DBG_TRACE(DBG_MESSAGE,("Unknown Windows minor version"));
                  break ;
            }
            break;

		 case 6:
			 g_BaseData.WinVerInfo.BaseVersion = AWIN_VER_VISTA ;
			 DBG_TRACE(DBG_MESSAGE,("Windows Vista"));
			 break ;

         default:
            DBG_TRACE(DBG_MESSAGE,("Unknown Windows version"));
            break ;
      } // switch major version

   } // NT

   return STATUS_SUCCESS ;
}

/*------------------------------------------------------------------------
AGetWindowsVersion()
   Description:
      Returns the O.S version.
      
   Returns:
      AWIN_VER_xxxx value matching the current version
@
------------------------------------------------------------------------*/
UINT AGetWindowsVersion()
{
	PROC_ENTRY
   return g_BaseData.WinVerInfo.BaseVersion ;
}

/*------------------------------------------------------------------------
AGetWindowsServicePack()
   Description:
   *** NT only ***
      Returns OS service pack
      in case no service pack installed - returns 0 
      
   Returns:
      WORD
------------------------------------------------------------------------*/
WORD AGetWindowsServicePackMajor()
{
	PROC_ENTRY
   return g_BaseData.WinVerInfo.OsVersionInfoEx.wServicePackMajor ;
}

/*------------------------------------------------------------------------
AGetWindowsMajorVersion()
   Description:
      *** NT only ***
   Returns:
      DWORD
------------------------------------------------------------------------*/
DWORD AGetWindowsMajorVersion()
{
	PROC_ENTRY
   return g_BaseData.WinVerInfo.OsVersionInfoEx.dwMajorVersion ;
}

/*------------------------------------------------------------------------
AGetWindowsMinorVersion()
   Description:
      *** NT only ***
   Returns:
      WORD
------------------------------------------------------------------------*/
DWORD AGetWindowsMinorVersion()
{
	PROC_ENTRY
   return g_BaseData.WinVerInfo.OsVersionInfoEx.dwMinorVersion ;
}

/*------------------------------------------------------------------------
AGetWindowsBuildNumber()
   Description:
      *** NT only ***
   Returns:
      WORD
------------------------------------------------------------------------*/
DWORD AGetWindowsBuildNumber()
{
	PROC_ENTRY
   return g_BaseData.WinVerInfo.OsVersionInfoEx.dwBuildNumber ;
}

/*------------------------------------------------------------------------
AIsWindowsSafeBoot()
	Description:
   *** NT only ***
   Drivers should use this function in order to fail their loading in case of Safe boot
		
	Returns:
		ULONG - Zero if NOT SafeBoot
------------------------------------------------------------------------*/
ULONG AIsWindowsSafeBoot()
{
	PROC_ENTRY
   ULONG Res = 0 ;
   
   if( NULL != InitSafeBootMode )
   {
      switch( *InitSafeBootMode )
      {
      case 0:
      default:
         break ;

      case SAFEBOOT_MINIMAL:
      case SAFEBOOT_NETWORK:
      case SAFEBOOT_DSREPAIR:
         Res = TRUE ;
         break ;
      }
   }
   return Res ;
}

//Flags definition for security object.
#define  AFLAG_OBJECT_IS_VALID      0x01
#define  AFLAG_USE_SUBJECT_CONTEXT  0x02

typedef struct tagASECURITY_CONTEXT_OBJECT
{
   ULONG                      Flags;
   SECURITY_CLIENT_CONTEXT    ClientContext;

} ASECURITY_CONTEXT_OBJECT, *PASECURITY_CONTEXT_OBJECT;

typedef struct tagSecurityFlags
{
   ULONG32 TokenFlags;
   ULONG32 PrivilegesFlags;
   ULONG32 ImpersonationLevel;
   BOOLEAN   bIsTokenFlagsChanged;// Did we change the token flags
   BOOLEAN   bIsPrivilegesChanged;// Did we change the token flags
   BOOLEAN   bImpersonationLevelChanged;// Did we change the token flags
}SecurityFlags, *PSecurityFlags;


#ifndef TOKEN_HAS_IMPERSONATE_PRIVILEGE
#define TOKEN_HAS_IMPERSONATE_PRIVILEGE 0x0080 // from ntddk.h (wnet)
#endif


NTSTATUS imp_CaptureSecurityContext(IN PETHREAD pThread, OUT PHANDLE phSecurity)
{
	PROC_ENTRY
   NTSTATUS                      Status  = STATUS_SUCCESS ;
   SECURITY_QUALITY_OF_SERVICE   ClientSecurityQos = {0} ;
   PASECURITY_CONTEXT_OBJECT     pObject = NULL ;

   if (0 == phSecurity)
   {
      Status = STATUS_INVALID_PARAMETER;
      DBG_TRACE(DBG_MESSAGE,("imp_CaptureSecurityContext FAILURE 'phSecurity'. Status=0x%X", Status));
      return Status;
   }

   if (0 == pThread)
   {
      Status = STATUS_INVALID_PARAMETER;
      DBG_TRACE(DBG_MESSAGE,("imp_CaptureSecurityContext FAILURE. 'pThread' Status=0x%X", Status));
      return Status;
   }
   *phSecurity = NULL;


   pObject = (PASECURITY_CONTEXT_OBJECT)ExAllocatePoolWithTag(NonPagedPool,sizeof(ASECURITY_CONTEXT_OBJECT),'UCES');
   if (0 == pObject)
   {
      Status = STATUS_INSUFFICIENT_RESOURCES;
      DBG_TRACE(DBG_MESSAGE,("imp_CaptureSecurityContext FAILURE. Status=0x%X", Status));
      return Status;
   }
   RtlZeroMemory(pObject, sizeof(ASECURITY_CONTEXT_OBJECT));

   ClientSecurityQos.Length               =  sizeof(SECURITY_QUALITY_OF_SERVICE);
   ClientSecurityQos.ImpersonationLevel   =  SecurityImpersonation;
//   ClientSecurityQos.ImpersonationLevel   =  SecurityDelegation;//i think it should be
   ClientSecurityQos.EffectiveOnly        =  TRUE;
//   ClientSecurityQos.EffectiveOnly        =  FALSE;//i think it should be
//   ClientSecurityQos.ContextTrackingMode  =  SECURITY_STATIC_TRACKING;
   ClientSecurityQos.ContextTrackingMode  =  SECURITY_DYNAMIC_TRACKING;

   Status   =  SeCreateClientSecurity(pThread,
                                  &ClientSecurityQos,
                                  //TBD: is this correct?
                                  FALSE,  //ServerIsRemote,
                                  &(pObject->ClientContext));

   if (Status != STATUS_SUCCESS)
   {
      DBG_TRACE(DBG_MESSAGE,("imp_CaptureSecurityContext FAILURE. 'SeCreateClientSecurity' Status=0x%X", Status));
      ExFreePoolWithTag(pObject,'UCES');

      return Status;
   }

   //Set valid object flags ON.
   SET_FLAGS(pObject->Flags ,AFLAG_OBJECT_IS_VALID);

   *phSecurity  =  pObject;

   DBG_TRACE(DBG_MESSAGE,("imp_CaptureSecurityContext SUCCESS. pObject=%p", pObject));

   return STATUS_SUCCESS;
}

/*------------------------------------------------------------------------
imp_ReleaseSecurityContext():
   Description:
      
        Releases the Security Context previously captured by imp_CaptureSecurityContext.

        Irql == IRQL PASSIVE_LEVEL.

   Parameters:
      
   Returns:
      
------------------------------------------------------------------------*/
VOID imp_ReleaseSecurityContext(IN PHANDLE   phSecurity)
{
	PROC_ENTRY

   PASECURITY_CONTEXT_OBJECT  pObject;

   pObject = (PASECURITY_CONTEXT_OBJECT)*phSecurity;
   if (0 == pObject )
   {
      return;
   }

   if ( ! GET_FLAGS(pObject->Flags, AFLAG_OBJECT_IS_VALID))
   {
      DBG_TRACE(DBG_MESSAGE,("Security context was not initialized"));

      return;
   }

   SeDeleteClientSecurity(&(pObject->ClientContext));

   //Reset memory values before we deallocate it.
   RtlFillMemory(pObject, sizeof(ASECURITY_CONTEXT_OBJECT), 'V');

   ExFreePoolWithTag(pObject,'UCES');
   *phSecurity = NULL;

   DBG_TRACE(DBG_MESSAGE,("imp_ReleaseSecurityContext: pObject=%p, SUCCESS", pObject));
}

#define x86
//#undef x86

BOOLEAN imp_GetSetTokenFlags(IN PSecurityFlags TokenFlags, BOOLEAN SetGetFlags)
{
	PROC_ENTRY

   PACCESS_TOKEN              pToken                     = NULL ;

   PUINT32                 pPrivilegesPtr             = 0x0 ;
   PUINT32                 pTokenFlagsPtr             = 0x0 ;
   PUINT32                 pTokenImpersonationLevelPtr= 0x0 ;

   UINT32                      TokenFlagsOffset           = 0 ;   
   UINT32                      PrivilegesOffset           = 0 ;   
   UINT32                      ImpersonationLevelOffset   = 0 ;   
   
	int IsVersionOk = 0;
   // Get the access token object for current process
   pToken = PsReferencePrimaryToken( PsGetCurrentProcess() ) ;
      
   if (0 == pToken )
   {
      DBG_TRACE(DBG_ERROR,("PsReferencePrimaryToken failed"));
      return FALSE;
   }

   UINT Version = AGetWindowsVersion();
   do
   {
      switch( Version )
      {
#ifdef x86
      case AWIN_VER_XP:
         {
            switch( AGetWindowsServicePackMajor() )
            {
            case 2: // Windows XP SP2
               {
                  DBG_TRACE(DBG_MESSAGE,("Windows XP SP2"));
                  TokenFlagsOffset = 0x88 ;
               }
               break ;
            /*   
            case 0:// Windows XP
            case 1:// Windows XP SP1
               {
                  DBG_TRACE(DBG_MESSAGE,("Windows XP or Windows XP SP1"));
                  TokenFlagsOffset = 0x7c ;
               }
               break ;
			   */

			default:                  
               break ;
            } // switch SP
         } // XP
         break ;
         
/*
		case AWIN_VER_SERVER_2003:
         {
            switch( AGetWindowsServicePackMajor() )
            {
            case 0: // Windows Server 2003
            case 1: // Windows Server 2003 SP1
               {
                  DBG_TRACE(DBG_MESSAGE,("Windows Server 2003 or Windows Server 2003 SP1"));
                  TokenFlagsOffset = 0x88 ;
               }
               break ;
               
            default:
               break ;
            } // switch SP
         }
         break ; // Server 2003
         
      case AWIN_VER_2000:
         if( 4 == AGetWindowsServicePackMajor() )
         {
            DBG_TRACE(DBG_MESSAGE,("Windows Server 2000"));
            TokenFlagsOffset = 0x78 ;
         }
         break ;
*/
#endif
		case AWIN_VER_VISTA:// Vista (5321)
         switch( AGetWindowsBuildNumber() )
         {
/*			case 5321: // (0x14C9) Build from 11/2005
            DBG_TRACE(DBG_MESSAGE,("Windows Vista (5321) Build from 11/2005"));
            TokenFlagsOffset = 0x98 ;
            break ;
            
			case 5600: // (0x15E0) (RC1 9/2006)
			case 5728:
			case 5744: // (0x1670) (RC2 10/2006)
            DBG_TRACE(DBG_MESSAGE,("Windows Vista (RC1 9/2006) or Windows Vista (RC2 10/2006)"));
            PrivilegesOffset = 0x48;
            break ;  
*/            
			case 6000: // (0x1770) (Release version 11/2006)
				DBG_TRACE(DBG_MESSAGE,("Windows Vista (Release version 11/2006)"));
				IsVersionOk = 1;
				break;
			case 6001: // (0x1771) (Sp1 RC1 version 11/2007)
				DBG_TRACE(DBG_MESSAGE,("Windows Vista (Sp1 RC1 version 11/2007)"));
				IsVersionOk = 1;
				break;
			case 6002:
				DBG_TRACE(DBG_MESSAGE,("Windows Vista (Sp1 RC2)"));
				IsVersionOk = 1;
				break;
			case 7100:// win7 RC1
				DBG_TRACE(DBG_MESSAGE,("win7 RC1"));
				IsVersionOk = 1;
				break;
			case 7600:// win7 RTM
				DBG_TRACE(DBG_MESSAGE,("win7 RTM"));
				IsVersionOk = 1;
				break;
			case 7601:// win7 RTM
				DBG_TRACE(DBG_MESSAGE,("win7 Final"));
				IsVersionOk = 1;
				break;            
			case 8400:// win8 Release Preview
				DBG_TRACE(DBG_MESSAGE,("win8 Release Preview"));
				IsVersionOk = 2;
				break;            
			case 9200:// win8 RTM
				DBG_TRACE(DBG_MESSAGE,("win8 RTM"));
				IsVersionOk = 2;
				break;            
			default:
            DBG_TRACE(DBG_MESSAGE,("SeImpersonateClientEx: Unsupported Vista Build Number:0x%X", AGetWindowsBuildNumber() ));
            break ;
         }
         break ;
         
         default:
            TokenFlagsOffset = 0 ;
            PrivilegesOffset = 0 ;
            ImpersonationLevelOffset = 0;
            DBG_TRACE(DBG_MESSAGE,("SeImpersonateClientEx: Unsupported Os Number:0x%X", Version ));
            
            break ;
            
      } // switch windows version
      
      //TARGET_OS=WinXP
	  if (1==IsVersionOk)
	  {
#ifdef x86
		  PrivilegesOffset           = 0x48;
		  ImpersonationLevelOffset   = 0xa8;
		  TokenFlagsOffset           = 0xac;
#else
		  PrivilegesOffset           = 0x48;//for amd64
		  ImpersonationLevelOffset   = 0xbc;
		  TokenFlagsOffset           = 0xc0;
#endif
	  }
	  if (2==IsVersionOk)
	  {
		  #ifdef x86
		  PrivilegesOffset           = 0x48;
		  ImpersonationLevelOffset   = 0xac;
		  TokenFlagsOffset           = 0xb0;
		  #else
		  PrivilegesOffset           = 0x48;
		  ImpersonationLevelOffset   = 0xc4;
		  TokenFlagsOffset           = 0xc8;
		  #endif
	  }

      //We assume that offset 0 means - We don't need to change this field.
      if((0 == TokenFlagsOffset) && (0 == PrivilegesOffset) && (0 == ImpersonationLevelOffset))
      {
         break ;
      }
      
      if (SetGetFlags)
      {
         if( 0 != TokenFlagsOffset )
         {
            // Find the pointer to the flags
//            pTokenFlagsPtr = (PULONG32)((PBYTE)pToken+TokenFlagsOffset) ;
            pTokenFlagsPtr = (PUINT32)((INT_PTR)pToken+TokenFlagsOffset) ;
            TokenFlags->TokenFlags = *pTokenFlagsPtr ;
            if( 0 == GET_FLAGS(*pTokenFlagsPtr, TOKEN_HAS_IMPERSONATE_PRIVILEGE ) )
            {
               TokenFlags->bIsTokenFlagsChanged = TRUE ;
               SET_FLAGS(*pTokenFlagsPtr, TOKEN_HAS_IMPERSONATE_PRIVILEGE ) ;
               DBG_TRACE(DBG_MESSAGE,("TokenFlags Added"));
            }
         }
         
         if( 0 != PrivilegesOffset )
         {
            // Find the pointer to the flags
            pPrivilegesPtr = (PUINT32)((INT_PTR)pToken+PrivilegesOffset) ;
            
            TokenFlags->PrivilegesFlags = *pPrivilegesPtr ;
            
            if(0 == GET_FLAGS(*pPrivilegesPtr, UNPROTECTED_DACL_SECURITY_INFORMATION ))
            {
               TokenFlags->bIsPrivilegesChanged = TRUE ;
               SET_FLAGS(*pPrivilegesPtr, UNPROTECTED_DACL_SECURITY_INFORMATION ) ;
               DBG_TRACE(DBG_MESSAGE,("PrivilegesFlags Added"));
            }
         }
         
         if( 0 != ImpersonationLevelOffset )
         {
            // Find the pointer to the flags
            pTokenImpersonationLevelPtr = (PUINT32)((INT_PTR)pToken+ImpersonationLevelOffset) ;
            TokenFlags->ImpersonationLevel = *pTokenImpersonationLevelPtr ;
            
            if(*pTokenImpersonationLevelPtr != SecurityImpersonation)
            {
               TokenFlags->bImpersonationLevelChanged = TRUE ;
               *pTokenImpersonationLevelPtr = SecurityImpersonation;
               DBG_TRACE(DBG_MESSAGE,("ImpersonationLevel Added"));
            }
         }
      }
      else
      {
         if( TokenFlags->bIsTokenFlagsChanged )
         {
            pTokenFlagsPtr = (PUINT32)((INT_PTR)pToken+TokenFlagsOffset) ;
            RESET_FLAGS(TokenFlags->TokenFlags, TOKEN_HAS_IMPERSONATE_PRIVILEGE ) ;
            *pTokenFlagsPtr = TokenFlags->TokenFlags ;
            DBG_TRACE(DBG_MESSAGE,("TokenFlags Removed"));
         }
         
         if( TokenFlags->bIsPrivilegesChanged )
         {
            pPrivilegesPtr = (PUINT32)((INT_PTR)pToken+PrivilegesOffset) ;
            RESET_FLAGS(TokenFlags->PrivilegesFlags, UNPROTECTED_DACL_SECURITY_INFORMATION ) ;
            *pPrivilegesPtr = TokenFlags->PrivilegesFlags ;
            DBG_TRACE(DBG_MESSAGE,("PrivilegesFlags Removed"));
         }
         
         if( TokenFlags->bImpersonationLevelChanged )
         {
            pTokenImpersonationLevelPtr = (PUINT32)((INT_PTR)pToken+ImpersonationLevelOffset) ;
            *pTokenImpersonationLevelPtr = TokenFlags->ImpersonationLevel ;
            DBG_TRACE(DBG_MESSAGE,("ImpersonationLevel Removed"));
         }
      }
      
      break ; // exit the while loop
      
   }while(TRUE);

      
   PsDereferencePrimaryToken( pToken ) ;
   
   DBG_TRACE(DBG_MESSAGE,("imp_GetSetTokenFlags SUCCESS"));

   return TRUE;
}

NTSTATUS imp_ImpersonateClient(IN HANDLE   hSecurity,
                               IN BOOLEAN  Impersonate
                              )
{
   PROC_ENTRY
   NTSTATUS Status = STATUS_SUCCESS ;
   PASECURITY_CONTEXT_OBJECT pObject;

   pObject = (PASECURITY_CONTEXT_OBJECT)hSecurity;
   if (0 == pObject)
   {
      return FALSE;
   }

   if ( ! GET_FLAGS(pObject->Flags, AFLAG_OBJECT_IS_VALID))
   {
      DBG_TRACE(DBG_MESSAGE,("Security context was not initialized"));
      return FALSE;
   }

   if(Impersonate)
   {
      Status = SeImpersonateClientEx(&pObject->ClientContext, NULL) ;
      DBG_TRACE(DBG_MESSAGE,("SeImpersonateClientEx returned 0x%X",Status));
   }
   else
   {
      //SeImpersonateClientEx(NULL,NULL);
      PsRevertToSelf();
   }

   return Status;
}

#define IOCTL_PROMOTE_PRIVILEGE  CTL_CODE(FILE_DEVICE_UNKNOWN, 13 , METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_RESTORE_PRIVILEGE  CTL_CODE(FILE_DEVICE_UNKNOWN, 14 , METHOD_BUFFERED, FILE_ANY_ACCESS)

#define DEVICE_NAME { L'\\', L'D', L'e', L'v', L'i', L'c', L'e', L'\\', L'D', L'I', L'4', L'X',0,L'X',L'4',0}

typedef struct tagCOMMON_DEVICE_EXTENSION
{
   UINT32               Signature ;
   PVOID                pDeviceObject ;      // Ptr to the device object

} COMMON_DEVICE_EXTENSION, * PCOMMON_DEVICE_EXTENSION;

static WCHAR NameBuffer[] = DEVICE_NAME;
NTSTATUS
DeviceCreate(
    IN PDRIVER_OBJECT DriverObject
    )
{
   PROC_ENTRY
      NTSTATUS Status = STATUS_SUCCESS ;
   __try
   {
      UNICODE_STRING DeviceName;
      DeviceName.Buffer = NameBuffer ;
      DeviceName.Length = sizeof(NameBuffer) ;
      DeviceName.MaximumLength = DeviceName.Length;
      
      PDEVICE_OBJECT DeviceObject;
      
      BOOLEAN Exclusive = FALSE;

      Status = IoCreateDevice( DriverObject, sizeof( COMMON_DEVICE_EXTENSION ),
         &DeviceName, FILE_DEVICE_UNKNOWN,
         0, Exclusive, &DeviceObject ) ;
      
      if( ! NT_SUCCESS( Status ))
      {
         DBG_TRACE(DBG_ERROR,("Failed: Status=0x%X", Status));
         __leave;
      }
      
      DriverObject->MajorFunction[IRP_MJ_CREATE] = DeviceOpen;
      DriverObject->MajorFunction[IRP_MJ_CLOSE]  = DeviceClose;
      DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = DeviceIoControl;

      DeviceObject->Flags |= DO_BUFFERED_IO ;
      DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

      PCOMMON_DEVICE_EXTENSION DeviceExt = (PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension ;
   
      RtlZeroMemory( DeviceExt, sizeof(COMMON_DEVICE_EXTENSION) );
      DeviceExt->Signature = SIGNATURE ;
      DeviceExt->pDeviceObject = DeviceObject ;
      
   }
   __finally
   {

   }
  
   return Status; 
}

NTSTATUS
DeviceDestroy(
    IN PDRIVER_OBJECT DriverObject
    )
{
   	PROC_ENTRY
   NTSTATUS Status = STATUS_SUCCESS ;

   IoDeleteDevice(DriverObject->DeviceObject);
return Status; 
}
NTSTATUS
DeviceOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
   	PROC_ENTRY
      NTSTATUS Status = STATUS_SUCCESS ;
      Irp->IoStatus.Information = 0;
      PCOMMON_DEVICE_EXTENSION DeviceExt = (PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension ;

      if (SIGNATURE != DeviceExt->Signature)
      {
         Status = STATUS_UNSUCCESSFUL;
         Irp->IoStatus.Information = FILE_DOES_NOT_EXIST;
         DBG_TRACE(DBG_ERROR,("Got a Open for a Extraneus object!!"));
      }
   
   Irp->IoStatus.Status = Status;
   IoCompleteRequest (Irp, IO_NO_INCREMENT);
return Status; 
};

NTSTATUS
DeviceOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
   	PROC_ENTRY

return DeviceOpenClose(DeviceObject,Irp); 
}

NTSTATUS
DeviceClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
   	PROC_ENTRY

return DeviceOpenClose(DeviceObject,Irp); 
}

/*
#define MAKE_TIMEOUT(ms) (INT64)(-10000i64 * (ms))

VOID
WORKER_THREAD_ROUTINE(
    IN PVOID Parameter
    )
{
   PROC_ENTRY
   pGetSystemContextData pSystemContextData = (pGetSystemContextData) Parameter;

    PETHREAD pThread = PsGetCurrentThread();
    NTSTATUS Status = STATUS_SUCCESS;
    Status = imp_CaptureSecurityContext(pThread, &pSystemContextData->hContext);
	 DBG_TRACE(DBG_MESSAGE,("imp_CaptureSecurityContext returned 0x%X", Status) );
    if( ! NT_SUCCESS( Status )) 
    {
       pSystemContextData->hContext = NULL;
       KeSetEvent(&pSystemContextData->Event1,0,FALSE);
       return;
    }

   KeSetEvent(&pSystemContextData->Event1,0,TRUE);
   LARGE_INTEGER timing;
   timing.QuadPart = MAKE_TIMEOUT(1*60*1000);;
   KeWaitForSingleObject(&pSystemContextData->Event2,Executive,KernelMode,FALSE,&timing);
   imp_ReleaseSecurityContext(&pSystemContextData->hContext);
}

NTSTATUS
GetSystemContext(pGetSystemContextData pSystemContextData)
{
   PROC_ENTRY
   NTSTATUS Status = STATUS_SUCCESS;

   WORK_QUEUE_ITEM  Item;
  
   pSystemContextData->hContext = NULL;
   KeInitializeEvent(&pSystemContextData->Event1,NotificationEvent,FALSE);
   KeInitializeEvent(&pSystemContextData->Event2,NotificationEvent,FALSE);
   ExInitializeWorkItem(&Item,WORKER_THREAD_ROUTINE,pSystemContextData);
   ExQueueWorkItem(&Item,DelayedWorkQueue);
   KeWaitForSingleObject(&pSystemContextData->Event1,Executive,KernelMode,FALSE,NULL);
   if (!pSystemContextData->hContext)
   {
      Status = STATUS_UNSUCCESSFUL;
   }
return Status;
}

NTSTATUS
ReleaseSystemContext(pGetSystemContextData pSystemContextData)
{
   PROC_ENTRY
   NTSTATUS Status = STATUS_SUCCESS;
   KeSetEvent(&pSystemContextData->Event2,0,FALSE);
return Status;
}
*/

HANDLE hContext = NULL;
SecurityFlags TokenSecurityFlags;

NTSTATUS
DeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
   PROC_ENTRY
   NTSTATUS Status = STATUS_SUCCESS ;
   __try 
   {
      Irp->IoStatus.Information = 0;
      PCOMMON_DEVICE_EXTENSION DeviceExt = (PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension ;
      
      if (SIGNATURE != DeviceExt->Signature)
      {
         Status = STATUS_UNSUCCESSFUL;
         Irp->IoStatus.Information = FILE_DOES_NOT_EXIST;
         DBG_TRACE(DBG_ERROR,("Got a Open for a Extraneus object!!"));
         __leave;
      }
      PIO_STACK_LOCATION irpSp;
      ULONG functionCode;
      ULONG outputLength;
      irpSp = IoGetCurrentIrpStackLocation(Irp);
      functionCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
      outputLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
//      char *buffer = Irp->AssociatedIrp.SystemBuffer;
      
      switch(functionCode)
      {
         case IOCTL_PROMOTE_PRIVILEGE:
//              Status = GetSystemContext(&SystemContextData);
//              DBG_TRACE(DBG_MESSAGE,("GetSystemContext returned 0x%X", Status) );
//              if( ! NT_SUCCESS( Status )) 
//              {
//		            break;
//              }
              imp_GetSetTokenFlags(&TokenSecurityFlags,TRUE);
              Status = imp_ImpersonateClient(SystemContextData.hContext,TRUE);
         break;
         
         case IOCTL_RESTORE_PRIVILEGE:
              Status = imp_ImpersonateClient(SystemContextData.hContext,FALSE);
              imp_GetSetTokenFlags(&TokenSecurityFlags,FALSE);
//              ReleaseSystemContext(&SystemContextData);
         break;
         
      default:
         break;
      }
   }
   __finally
   {
      
   }

   Irp->IoStatus.Status = Status;
   IoCompleteRequest (Irp, IO_NO_INCREMENT);
return Status; 
}


/*
dt nt!_token
*/

/*
vista sp1:
nt!_TOKEN
+0x000 TokenSource      : _TOKEN_SOURCE
+0x010 TokenId          : _LUID
+0x018 AuthenticationId : _LUID
+0x020 ParentTokenId    : _LUID
+0x028 ExpirationTime   : _LARGE_INTEGER
+0x030 TokenLock        : Ptr32 _ERESOURCE
+0x034 ModifiedId       : _LUID
+0x040 Privileges       : _SEP_TOKEN_PRIVILEGES
+0x058 AuditPolicy      : _SEP_AUDIT_POLICY
+0x074 SessionId        : Uint4B
+0x078 UserAndGroupCount : Uint4B
+0x07c RestrictedSidCount : Uint4B
+0x080 VariableLength   : Uint4B
+0x084 DynamicCharged   : Uint4B
+0x088 DynamicAvailable : Uint4B
+0x08c DefaultOwnerIndex : Uint4B
+0x090 UserAndGroups    : Ptr32 _SID_AND_ATTRIBUTES
+0x094 RestrictedSids   : Ptr32 _SID_AND_ATTRIBUTES
+0x098 PrimaryGroup     : Ptr32 Void
+0x09c DynamicPart      : Ptr32 Uint4B
+0x0a0 DefaultDacl      : Ptr32 _ACL
+0x0a4 TokenType        : _TOKEN_TYPE
+0x0a8 ImpersonationLevel : _SECURITY_IMPERSONATION_LEVEL
+0x0ac TokenFlags       : Uint4B
+0x0b0 TokenInUse       : UChar
+0x0b4 IntegrityLevelIndex : Uint4B
+0x0b8 MandatoryPolicy  : Uint4B
+0x0bc ProxyData        : Ptr32 _SECURITY_TOKEN_PROXY_DATA
+0x0c0 AuditData        : Ptr32 _SECURITY_TOKEN_AUDIT_DATA
+0x0c4 LogonSession     : Ptr32 _SEP_LOGON_SESSION_REFERENCES
+0x0c8 OriginatingLogonSession : _LUID
+0x0d0 SidHash          : _SID_AND_ATTRIBUTES_HASH
+0x158 RestrictedSidHash : _SID_AND_ATTRIBUTES_HASH
+0x1e0 VariablePart     : Uint4B
*/
/*
Win7 32
lkd> dt nt!_token
+0x000 TokenSource      : _TOKEN_SOURCE
+0x010 TokenId          : _LUID
+0x018 AuthenticationId : _LUID
+0x020 ParentTokenId    : _LUID
+0x028 ExpirationTime   : _LARGE_INTEGER
+0x030 TokenLock        : Ptr32 _ERESOURCE
+0x034 ModifiedId       : _LUID
+0x040 Privileges       : _SEP_TOKEN_PRIVILEGES
+0x058 AuditPolicy      : _SEP_AUDIT_POLICY
+0x074 SessionId        : Uint4B
+0x078 UserAndGroupCount : Uint4B
+0x07c RestrictedSidCount : Uint4B
+0x080 VariableLength   : Uint4B
+0x084 DynamicCharged   : Uint4B
+0x088 DynamicAvailable : Uint4B
+0x08c DefaultOwnerIndex : Uint4B
+0x090 UserAndGroups    : Ptr32 _SID_AND_ATTRIBUTES
+0x094 RestrictedSids   : Ptr32 _SID_AND_ATTRIBUTES
+0x098 PrimaryGroup     : Ptr32 Void
+0x09c DynamicPart      : Ptr32 Uint4B
+0x0a0 DefaultDacl      : Ptr32 _ACL
+0x0a4 TokenType        : _TOKEN_TYPE
+0x0a8 ImpersonationLevel : _SECURITY_IMPERSONATION_LEVEL
+0x0ac TokenFlags       : Uint4B
+0x0b0 TokenInUse       : UChar
+0x0b4 IntegrityLevelIndex : Uint4B
+0x0b8 MandatoryPolicy  : Uint4B
+0x0bc LogonSession     : Ptr32 _SEP_LOGON_SESSION_REFERENCES
+0x0c0 OriginatingLogonSession : _LUID
+0x0c8 SidHash          : _SID_AND_ATTRIBUTES_HASH
+0x150 RestrictedSidHash : _SID_AND_ATTRIBUTES_HASH
+0x1d8 pSecurityAttributes : Ptr32 _AUTHZBASEP_SECURITY_ATTRIBUTES_INFORMATION
+0x1dc VariablePart     : Uint4B
*/

/*
win7 sp1 same
lkd> dt nt!_token
+0x000 TokenSource      : _TOKEN_SOURCE
+0x010 TokenId          : _LUID
+0x018 AuthenticationId : _LUID
+0x020 ParentTokenId    : _LUID
+0x028 ExpirationTime   : _LARGE_INTEGER
+0x030 TokenLock        : Ptr32 _ERESOURCE
+0x034 ModifiedId       : _LUID
+0x040 Privileges       : _SEP_TOKEN_PRIVILEGES
+0x058 AuditPolicy      : _SEP_AUDIT_POLICY
+0x074 SessionId        : Uint4B
+0x078 UserAndGroupCount : Uint4B
+0x07c RestrictedSidCount : Uint4B
+0x080 VariableLength   : Uint4B
+0x084 DynamicCharged   : Uint4B
+0x088 DynamicAvailable : Uint4B
+0x08c DefaultOwnerIndex : Uint4B
+0x090 UserAndGroups    : Ptr32 _SID_AND_ATTRIBUTES
+0x094 RestrictedSids   : Ptr32 _SID_AND_ATTRIBUTES
+0x098 PrimaryGroup     : Ptr32 Void
+0x09c DynamicPart      : Ptr32 Uint4B
+0x0a0 DefaultDacl      : Ptr32 _ACL
+0x0a4 TokenType        : _TOKEN_TYPE
+0x0a8 ImpersonationLevel : _SECURITY_IMPERSONATION_LEVEL
+0x0ac TokenFlags       : Uint4B
+0x0b0 TokenInUse       : UChar
+0x0b4 IntegrityLevelIndex : Uint4B
+0x0b8 MandatoryPolicy  : Uint4B
+0x0bc LogonSession     : Ptr32 _SEP_LOGON_SESSION_REFERENCES
+0x0c0 OriginatingLogonSession : _LUID
+0x0c8 SidHash          : _SID_AND_ATTRIBUTES_HASH
+0x150 RestrictedSidHash : _SID_AND_ATTRIBUTES_HASH
+0x1d8 pSecurityAttributes : Ptr32 _AUTHZBASEP_SECURITY_ATTRIBUTES_INFORMATION
+0x1dc VariablePart     : Uint4B
*/

/*
win8 8400
lkd> dt nt!_token
+0x000 TokenSource      : _TOKEN_SOURCE
+0x010 TokenId          : _LUID
+0x018 AuthenticationId : _LUID
+0x020 ParentTokenId    : _LUID
+0x028 ExpirationTime   : _LARGE_INTEGER
+0x030 TokenLock        : Ptr32 _ERESOURCE
+0x034 ModifiedId       : _LUID
+0x040 Privileges       : _SEP_TOKEN_PRIVILEGES
+0x058 AuditPolicy      : _SEP_AUDIT_POLICY
+0x078 SessionId        : Uint4B
+0x07c UserAndGroupCount : Uint4B
+0x080 RestrictedSidCount : Uint4B
+0x084 VariableLength   : Uint4B
+0x088 DynamicCharged   : Uint4B
+0x08c DynamicAvailable : Uint4B
+0x090 DefaultOwnerIndex : Uint4B
+0x094 UserAndGroups    : Ptr32 _SID_AND_ATTRIBUTES
+0x098 RestrictedSids   : Ptr32 _SID_AND_ATTRIBUTES
+0x09c PrimaryGroup     : Ptr32 Void
+0x0a0 DynamicPart      : Ptr32 Uint4B
+0x0a4 DefaultDacl      : Ptr32 _ACL
+0x0a8 TokenType        : _TOKEN_TYPE
+0x0ac ImpersonationLevel : _SECURITY_IMPERSONATION_LEVEL
+0x0b0 TokenFlags       : Uint4B
+0x0b4 TokenInUse       : UChar
+0x0b8 IntegrityLevelIndex : Uint4B
+0x0bc MandatoryPolicy  : Uint4B
+0x0c0 LogonSession     : Ptr32 _SEP_LOGON_SESSION_REFERENCES
+0x0c4 OriginatingLogonSession : _LUID
+0x0cc SidHash          : _SID_AND_ATTRIBUTES_HASH
+0x154 RestrictedSidHash : _SID_AND_ATTRIBUTES_HASH
+0x1dc pSecurityAttributes : Ptr32 _AUTHZBASEP_SECURITY_ATTRIBUTES_INFORMATION
+0x1e0 Package          : Ptr32 Void
+0x1e4 Capabilities     : Ptr32 _SID_AND_ATTRIBUTES
+0x1e8 CapabilityCount  : Uint4B
+0x1ec CapabilitiesHash : _SID_AND_ATTRIBUTES_HASH
+0x274 LowboxNumberEntry : Ptr32 _SEP_LOWBOX_NUMBER_ENTRY
+0x278 LowboxHandlesEntry : Ptr32 _SEP_LOWBOX_HANDLES_ENTRY
+0x27c pClaimAttributes : Ptr32 _AUTHZBASEP_CLAIM_ATTRIBUTES_COLLECTION
+0x280 VariablePart     : Uint4B
*/
