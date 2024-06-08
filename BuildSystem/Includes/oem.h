#pragma once
#include <ProjectVer.h>

#ifndef _TO_STR
#define _TO_STR(_N) #_N
#endif
#ifndef TO_STR
#define TO_STR(_N) _TO_STR(_N)
#endif

#ifndef FileDescription
#define FileDescription TO_STR(TARGET_NAME)
#endif

#define MAKE_VER_STRING(maj, min, submin, bld)    _TO_STR(maj) "." _TO_STR(min) "." _TO_STR(submin) "." _TO_STR(bld)

#define  COMPANYNAME              "Acme"
#define  LEGALCOPYRIGHT           "Copyright Acme (C) 2020 All rights reserved."
#define  PRODUCTNAME              TO_STR(TARGET_NAME)

#define  FILEDESCRIPTION          FileDescription " " TO_STR(OS_VERSION) " "
#define  INTERNALNAME             TO_STR(TARGET_NAME)
#define  ORIGINALFILENAME         TO_STR(TARGET_NAME)


// Define our driver version
#define VER_FILEVERSION         OEM_MAJOR_VERSION,OEM_MINOR_VERSION,OEM_MAJ_BUILD_VERSION,OEM_MIN_BUILD_VERSION
#define VER_FILEVERSION_STR     MAKE_VER_STRING(OEM_MAJOR_VERSION,OEM_MINOR_VERSION,OEM_MAJ_BUILD_VERSION,OEM_MIN_BUILD_VERSION)

#define VENDORDESCRIPTOR        PRODUCTNAME

