#pragma once


//-------------------------------------------------------------------------
// DEBUG enable bit definitions
// DBG_LEVEL is a bitmask for debug messages 
//
#ifndef DBG_USE_WPP

#ifdef __cplusplus
extern "C"
{
#endif

	extern UINT32 DBG_LEVEL;

#ifdef __cplusplus
}
#endif
/*
// DEBUG FLAG DEFINITIONS
*/


#define DBG_WARNING				   0x00000001
#define DBG_MESSAGE				   0x00000002
#define DBG_ERROR				      0x00000004
#define DBG_BREAK				      0x00000008
#define DBG_TRACE_FUNCTIONS		0x00000010
#define DBG_INIT				      0x00000020
#define DBG_MEM					   0x00000400

#ifndef DBG_INIT_LEVEL
#define DBG_INIT_LEVEL           0xFFFFFFFF
#endif

#define WPP_TO_INCLUDE(_N) "dbg.h"

#else
#define WPP_CONTROL_GUIDS                    \
    WPP_DEFINE_CONTROL_GUID(CtlGuid,         \
(B2FD4369, CE76, 4126, BE3D, 76B2D9D1BC77),  \
        WPP_DEFINE_BIT(DBG_WARNING)          \
        WPP_DEFINE_BIT(DBG_TRACE)            \
        WPP_DEFINE_BIT(DBG_ERROR)            \
        WPP_DEFINE_BIT(DBG_BREAK)            \
        WPP_DEFINE_BIT(DBG_TRACE_FUNCTIONS)  \
        WPP_DEFINE_BIT(DBG_INIT)             \
)

#define WPP_TO_INCLUDE(_N) TO_STR(IntDir\tmh\\ ## _N)
#endif //DBG_USE_WPP

#include WPP_TO_INCLUDE(dbg.h.h)

#define TODO(mes) message(__FILE__ "("  TO_STR(__LINE__) ") TODO > " __FUNCTION__ ":" mes) 
                 
#ifdef USING_DEBUGGER  
#	ifdef WIN64
#		define DBGINT if (!KD_DEBUGGER_NOT_PRESENT) DbgBreakPoint();
#	else
#		define DBGINT if (!KD_DEBUGGER_NOT_PRESENT) __asm{ int 3 }
	#endif
#else
#	define DBGINT
#endif//USING_DEBUGGER

#if DBG_
#	ifdef DBG_USE_WPP
#		ifdef WANT_WPP_DEBUG_PRINTS
#			define WPP_DEBUG(_msg_) DbgPrint _msg_
#		endif//WANT_WPP_DEBUG_PRINTS
#	else
#		define DBG_TRACE(LVL,A) {if (DBG_LEVEL & LVL) {DbgPrint A;DbgPrint ("\n");} }
#	endif//DBG_USE_WPP
#else
#	ifndef DBG_USE_WPP
#		define DBG_TRACE(LVL,A)
#	endif
#endif // DBG

FORCEINLINE void ASSERT_FUNCTION(UINT exp, char* cexp)
{
	if(!(exp))
	{
		DBG_TRACE(DBG_BREAK,("%s \n",cexp));
		DBGINT;
	}
}

#undef ASSERT
#define ASSERT( exp )  ASSERT_FUNCTION( (UINT) (exp) ," ASSERT:" #exp "\n" )

#ifdef __cplusplus
class DebugClass
{
public:
	FORCEINLINE DebugClass(char* Function)
	{
		m_Function = Function;
		DBG_TRACE(DBG_TRACE_FUNCTIONS,(">>>%s Start ",m_Function));
	};

	FORCEINLINE ~DebugClass()
	{
		DBG_TRACE(DBG_TRACE_FUNCTIONS,("<<<%s Finish ",m_Function));
	};
protected:
private:
	char *m_Function;
};

#define PROC_ENTRY DebugClass  m(__FUNCTION__);
#else// if the file is not a cpp file then no PROC_ENTRY
#define PROC_ENTRY
#endif

 
#ifdef __cplusplus
extern "C"
{
#endif
extern void DebugInit(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
extern void DebugStop();
#ifdef __cplusplus
}
#endif

