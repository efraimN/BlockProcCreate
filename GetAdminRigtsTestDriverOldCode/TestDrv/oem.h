#pragma once

#define _TT(n) #n
#define T(n) _TT(n)

#define  OEM_MAJOR_VERSION       4
#define  OEM_MINOR_VERSION       0
#define  OEM_MAJ_BUILD_VERSION   1
#define  OEM_MIN_BUILD_VERSION   000

//#define  INF_VERSION $(OEM_MAJOR_VERSION).$(OEM_MINOR_VERSION).$(OEM_MAJ_BUILD_VERSION).$(OEM_MIN_BUILD_VERSION)
//#define  INF_DATE 01/02/2005

#define  COMPANYNAME              "Friendly Technologies, Ltd."
#define  LEGALCOPYRIGHT           "2006,2008 Friendly Technologies, Ltd."
#define  PRODUCTNAME              "Friendly Technologies  DialerApi Driver"
#define  PROGRAMERS_NAME          "Efraim Neuberger"


#define  FILEDESCRIPTION          "DialerApi Driver Friendly Technologies"
#define  INTERNALNAME             T(TARGET_NAME)
#define  ORIGINALFILENAME         T(TARGET_NAME)

#define MAKE_VER_STRING(maj, min, submin, bld)    _TT(maj) "." _TT(min) "." _TT(submin) "." _TT(bld)

// Define our driver version
#define VER_FILEVERSION         OEM_MAJOR_VERSION,OEM_MINOR_VERSION,OEM_MAJ_BUILD_VERSION,OEM_MIN_BUILD_VERSION
#define VER_FILEVERSION_STR     MAKE_VER_STRING(OEM_MAJOR_VERSION,OEM_MINOR_VERSION,OEM_MAJ_BUILD_VERSION,OEM_MIN_BUILD_VERSION)

#define VENDORDESCRIPTOR        PRODUCTNAME

