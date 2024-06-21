!if  exist($(OBJ_PATH)\$O\_objects.mac)
!if [-copy $(OBJ_PATH)\$O\_objects.mac $(MCPU)\_objects.mac > nul:]
!endif
!if [-del $(OBJ_PATH)\$O\*.* /q]
!endif
!if [-rd $(OBJ_PATH)\obj$(BUILD_ALT_DIR) /s /q]
!endif
!endif

O=$(MCPU)

!if ("$(DDK_TARGET_OS)"!="WinXP") && ("$(DDK_TARGET_OS)"!="WinNET") && ("$(DDK_TARGET_OS)"!="Win2K") && ("$(DDK_TARGET_OS)"!="WinLH")
!ERROR Wrong DDK_TARGET_OS !!! the setenv file was not called correctly!
!endif

TARGETTYPE=DRIVER
#TARGETLIBS=$(DDK_LIB_PATH)\ndis.lib 
TARGETPATH=..\$(MYDIR)

#MSC_OPTIMIZATION=/Od /Oi

USER_C_FLAGS=$(USER_C_FLAGS) /GS /$(DRIVER_COMPILE_FLAGS)

C_DEFINES=$(C_DEFINES) /DTARGET_NAME=$(TARGETNAME)


PRECOMPILED_CXX=1
PRECOMPILED_INCLUDE=precompile.h
PRECOMPILED_PCH=precompile.pch
PRECOMPILED_OBJ=precompile.obj

INCLUDES=..;$(BASE_INC_PATH)

!if [-copy ..\resource.rc1 $O\resource.rc > nul:]
!endif
PASS0_HEADERDIR=$(MYBASEDIR)
PASS0_SOURCEDIR=$O


#C_DEFINES=$(C_DEFINES) /DDBG_USE_WPP
#C_DEFINES=$(C_DEFINES) /DWANT_WPP_DEBUG_PRINTS
#C_DEFINES=$(C_DEFINES) /DUSING_DEBUGGER
#C_DEFINES=$(C_DEFINES) /DDBG_

SOURCES=\
                 resource.rc			\
				..\messages.mc			\
				..\message.cpp			\
				..\Precompile.inc.cpp	\
				..\dbg.c				\
				..\DialerApiDriver.cpp	\
