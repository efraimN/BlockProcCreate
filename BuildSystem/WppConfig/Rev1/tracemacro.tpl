`**********************************************************************`
`* This is an include template file for the tracewpp preprocessor.    *`
`*                                                                    *`
`*    Copyright (c) Microsoft Corporation. All rights reserved.       *`
`**********************************************************************`

// template `TemplateFile`

// This template expects:
//      WPP_THIS_FILE defined (see header.tpl)
//      WPP_LOGGER_ARG  defined
//      WPP_GET_LOGGER  defined
//      WPP_ENABLED() defined

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef WPP_ALREADY_INCLUDED

#ifndef NO_CHECK_FOR_NULL_STRING
#ifndef WPP_CHECK_FOR_NULL_STRING
#define WPP_CHECK_FOR_NULL_STRING 1
#endif
#endif

#define WPP_EVAL(_value_) _value_
#define WPP_(Id) WPP_EVAL(WPP_) ## WPP_EVAL(Id) ## WPP_EVAL(_) ## WPP_EVAL(WPP_THIS_FILE) ## WPP_EVAL(__LINE__)

#if !defined(WPP_INLINE)
#define WPP_INLINE DECLSPEC_NOINLINE __inline
#endif

#else   // #ifndef WPP_ALREADY_INCLUDED

#undef WPP_LOCAL_TraceGuids

#endif  // #ifndef WPP_ALREADY_INCLUDED

#if !defined(WPP_NO_ANNOTATIONS)

`FORALL Msg IN Messages WHERE MsgIsPrivate`
#ifdef WPP_PUBLIC_`Msg.FlagValue`
#define WPP_PUBLIC_ANNOT_`Msg.Name`
#endif
`ENDFOR`

`FORALL Msg IN Messages WHERE MsgIsPublic`
#define WPP_PUBLIC_ANNOT_`Msg.Name`
`ENDFOR`

#ifdef WPP_EMIT_FUNC_NAME
#define WPP_FUNC_NAME L" FUNC=" _WPPW(__FUNCTION__)
#else
#define WPP_FUNC_NAME
#endif

`FORALL Msg IN Messages`
#if !defined(WPP_USER_MSG_GUID)
# define WPP_ANNOTATE_`Msg.Name`_FINAL(P, File, Name, ...)    __annotation(L ## P, L"`Msg.Guid` `CurrentDir` // SRC=" _WPPW(File) L" MJ=`ENV MAJORCOMP` MN=`ENV MINORCOMP`", L"#typev "  _WPPW(Name) L" `Msg.MsgNo` \"`Msg.Text`\" // `Msg.Indent` `Msg.GooPairs`" WPP_FUNC_NAME, __VA_ARGS__)
#else
# define WPP_ANNOTATE_`Msg.Name`_FINAL(P, File, Name, ...)    __annotation(L ## P, WPP_GUID_WTEXT WPP_USER_MSG_GUID L"`CurrentDir` // SRC=" _WPPW(File) L" MJ=`ENV MAJORCOMP` MN=`ENV MINORCOMP`", L"#typev "  _WPPW(Name) L" `Msg.MsgNo` \"`Msg.Text`\" // `Msg.Indent` `Msg.GooPairs`" WPP_FUNC_NAME, __VA_ARGS__)
#endif

#ifdef WPP_PUBLIC_ANNOT_`Msg.Name`
    #define WPP_ANNOTATE_`Msg.Name` WPP_ANNOTATE_`Msg.Name`_FINAL("TMF:", "Unknown_cxx00", "Unknown_cxx00", L"{"`FORALL Arg IN Msg.Arguments`, L"Arg, `Arg.MofType` -- `Arg.No`" `ENDFOR Arg`, L"}", L"PUBLIC_TMF:")
                                        
    #ifndef WPP_PUBLIC_TMC
        #define WPP_PUBLIC_TMC
    #endif
#else
    #define WPP_ANNOTATE_`Msg.Name` WPP_ANNOTATE_`Msg.Name`_FINAL("TMF:", "`SourceFile.Name`", "`Msg.Name`", L"{"`FORALL Arg IN Msg.Arguments`, L"`Arg.Name`, `Arg.MofType` -- `Arg.No`" `ENDFOR Arg`, L"}")
#endif
`ENDFOR`

# define WPP_ANNOTATE(x) WPP_ANNOTATE_ ## x,

#else
# define WPP_ANNOTATE(x)
#endif


#if `TraceGuids.Count`
#if !defined(WPP_USER_MSG_GUID)

#define WPP_LOCAL_TraceGuids WPP_`FORALL Guid IN TraceGuids``Guid.Normalized`_`ENDFOR`Traceguids
extern const __declspec(selectany) GUID WPP_LOCAL_TraceGuids[] = {`FORALL Guid IN TraceGuids` `Guid.Struct`, `ENDFOR`};

#else

#define WPP_LOCAL_MSG_VAR(Guid) WPP_XGLUE3(WPP_, WPP_GUID_NORM Guid, _Traceguids)

#define WPP_LOCAL_MSG_GUID(Guid) \
extern const __declspec(selectany) GUID WPP_LOCAL_MSG_VAR(Guid)[] = { WPP_GUID_STRUCT Guid }

WPP_LOCAL_MSG_GUID(WPP_USER_MSG_GUID);
#define WPP_LOCAL_TraceGuids WPP_LOCAL_MSG_VAR(WPP_USER_MSG_GUID)

#endif
#endif


#ifndef WPP_ALREADY_INCLUDED

#if !defined(WPP_TRACE_OPTIONS)
enum {WPP_TRACE_OPTIONS = TRACE_MESSAGE_SEQUENCE   | TRACE_MESSAGE_GUID
                        | TRACE_MESSAGE_SYSTEMINFO | TRACE_MESSAGE_TIMESTAMP };
#endif

#ifndef WPP_LOGPAIR_SEPARATOR
# define WPP_LOGPAIR_SEPARATOR ,
#endif

#ifndef WPP_LOGPAIR_SIZET
# define WPP_LOGPAIR_SIZET SIZE_T
#endif

#if !defined(WPP_LOGPAIR)
# define WPP_LOGPAIR(_Size, _Addr)     (_Addr),((WPP_LOGPAIR_SIZET)(_Size))WPP_LOGPAIR_SEPARATOR
#endif

#define WPP_LOGTYPEVAL(_Type, _Value) WPP_LOGPAIR(sizeof(_Type), &(_Value))
#define WPP_LOGTYPEPTR(_Value)        WPP_LOGPAIR(sizeof(*(_Value)), (_Value))

// Marshalling macros.

#if !defined(WPP_LOGASTR)
# if !defined(WPP_CHECK_FOR_NULL_STRING)
#  define WPP_LOGASTR(_value)  WPP_LOGPAIR(strlen(_value) + 1, _value )
# else
#  define WPP_LOGASTR(_value)  WPP_LOGPAIR( (_value)?strlen(_value) + 1:5, (_value)?(_value):"NULL" )
# endif
#endif

#if !defined(WPP_LOGWSTR)
# if !defined(WPP_CHECK_FOR_NULL_STRING)
#  define WPP_LOGWSTR(_value)  WPP_LOGPAIR( (wcslen(_value)+1) * sizeof(WCHAR), _value)
# else
#  define WPP_LOGWSTR(_value)  WPP_LOGPAIR( ((_value)?wcslen(_value) + 1:5) * sizeof(WCHAR), (_value)?(_value):L"NULL" )
# endif
#endif

#if !defined(WPP_LOGPGUID)
# define WPP_LOGPGUID(_value) WPP_LOGPAIR( sizeof(GUID), (_value) )
#endif


#if !defined(WPP_LOGPSID)
# if !defined(WPP_CHECK_FOR_NULL_STRING)
# define WPP_LOGPSID(_value)  WPP_LOGPAIR( WPP_GetLengthSid(_value), (_value) )
# else
# define WPP_LOGPSID(_value)  WPP_LOGPAIR( (_value)? (WPP_IsValidSid(_value)? \
                                                                        WPP_GetLengthSid(_value):5):5, \
                                                                        (_value)? (WPP_IsValidSid(_value)?\
                                                                        (_value):"NULL"):"NULL")
#endif
#endif

#if !defined(WPP_LOGCSTR)
# define WPP_LOGCSTR(_x) \
    WPP_LOGPAIR( sizeof((_x).Length), &(_x).Length ) WPP_LOGPAIR( (_x).Length, (_x).Buffer )
#endif

#if !defined(WPP_LOGUSTR)
# define WPP_LOGUSTR(_x) \
    WPP_LOGPAIR( sizeof((_x).Length), &(_x).Length ) WPP_LOGPAIR( (_x).Length, (_x).Buffer )
#endif

#if !defined(WPP_LOGPUSTR)
#if !defined(WPP_CHECK_FOR_NULL_STRING)
# define WPP_LOGPUSTR(_x) WPP_LOGUSTR(*(_x))
#else
# define WPP_LOGPUSTR(_x) WPP_LOGPAIR( sizeof(USHORT), (_x)?&(_x)->Length:L"\x08" ) \
                          WPP_LOGPAIR( (_x)?(_x)->Length:0x08, (_x)?(_x)->Buffer:L"NULL" )
#endif
#endif

#if !defined(WPP_LOGPCSTR)
#if !defined(WPP_CHECK_FOR_NULL_STRING)
# define WPP_LOGPCSTR(_x) WPP_LOGCSTR(*(_x))
#else
# define WPP_LOGPCSTR(_x) WPP_LOGPAIR( sizeof(USHORT), (_x)?&(_x)->Length:L"\x04" ) \
                          WPP_LOGPAIR( (_x)?(_x)->Length:0x04, (_x)?(_x)->Buffer:"NULL" )
#endif
#endif

#if !defined(WPP_LOGSTDSTR)
#define WPP_LOGSTDSTR(_value)  WPP_LOGPAIR( (_value).size()+1, (_value).c_str() )
#endif

#endif  //  #ifndef WPP_ALREADY_INCLUDED

#if !defined(WPP_CHECK_INIT)
# define WPP_CHECK_INIT (WPP_CB != (WPP_CB_TYPE*)&WPP_CB) && 
#endif


`FORALL i IN TypeSigSet WHERE !UnsafeArgs`
#ifndef WPP_SF_`i.Name`_def
#       define WPP_SF_`i.Name`_def
WPP_INLINE void WPP_SF_`i.Name`(WPP_LOGGER_ARG unsigned short id, LPCGUID TraceGuid`i.Arguments`)
   { WPP_TRACE(WPP_GET_LOGGER, WPP_TRACE_OPTIONS, (LPGUID)TraceGuid, id, `i.LogArgs` 0); }
#endif  // #ifndef WPP_SF_`i.Name`_def
`ENDFOR`

`FORALL i IN TypeSigSet WHERE UnsafeArgs`
#ifndef WPP_SF_`i.Name`_def
#       define WPP_SF_`i.Name`_def
WPP_INLINE void WPP_SF_`i.Name`(WPP_LOGGER_ARG unsigned short id, LPCGUID TraceGuid, ...) {
   va_list ap; va_start(ap, TraceGuid); UNREFERENCED_PARAMETER(ap);
   {
     `i.DeclVars`
      WPP_TRACE(WPP_GET_LOGGER, WPP_TRACE_OPTIONS, (LPGUID)TraceGuid, id, `i.LogArgs` 0);

   }
}
#endif  // #ifndef WPP_SF_`i.Name`_def
`ENDFOR`
#ifndef WPP_POST
#  define WPP_POST()
#endif

#ifndef WPP_PRE
#  define WPP_PRE()
#endif

#ifndef WPP_LOG_ALWAYS
#  define WPP_LOG_ALWAYS(...)
#endif      

#ifdef WPP_DEBUG
`FORALL i IN Messages WHERE !MsgArgs`
#ifndef WPP`i.GooId`_POST
#  define WPP`i.GooId`_POST(`i.GooArgs`)
#endif
#ifndef WPP`i.GooId`_PRE
#  define WPP`i.GooId`_PRE(`i.GooArgs`)
#endif
#define WPP_CALL_`i.Name`(`i.FixedArgs``i.MacroArgs`) \
     WPP_LOG_ALWAYS(WPP_EX`i.GooId`(`i.GooVals`),`i.DbgMacroArgs`)\
     WPP`i.GooId`_PRE(`i.GooVals`) \
     WPP_ANNOTATE(`i.Name`) \
     ( ( \
         WPP_CHECK_INIT `i.MsgVal`WPP`i.GooId`_ENABLED(`i.GooVals`) ? \
          WPP_DEBUG((`i.DbgMacroArgs`)), \
          WPP_SF_`i.TypeSig`(WPP`i.GooId`_LOGGER(`i.GooVals`) `i.MsgNo`, \
                             WPP_LOCAL_TraceGuids+0`i.MacroExprs`), 1:0  \
     ) ) \
     WPP`i.GooId`_POST(`i.GooVals`)
`ENDFOR`
#else
`FORALL i IN Messages WHERE !MsgArgs`
#ifndef WPP`i.GooId`_POST
#  define WPP`i.GooId`_POST(`i.GooArgs`)
#endif
#ifndef WPP`i.GooId`_PRE
#  define WPP`i.GooId`_PRE(`i.GooArgs`)
#endif
#define WPP_CALL_`i.Name`(`i.FixedArgs``i.MacroArgs`) \
     WPP_LOG_ALWAYS(WPP_EX`i.GooId`(`i.GooVals`),`i.DbgMacroArgs`) \
     WPP`i.GooId`_PRE(`i.GooVals`) \
     WPP_ANNOTATE(`i.Name`) \
     ( ( \
         WPP_CHECK_INIT `i.MsgVal`WPP`i.GooId`_ENABLED(`i.GooVals`) ? \
            WPP_SF_`i.TypeSig`(WPP`i.GooId`_LOGGER(`i.GooVals`) `i.MsgNo`, \
                               WPP_LOCAL_TraceGuids+0`i.MacroExprs`), 1:0  \
     ) ) \
     WPP`i.GooId`_POST(`i.GooVals`)
`ENDFOR`
#endif


#ifdef WPP_DEBUG
`FORALL i IN Messages WHERE MsgArgs`
#ifndef WPP`i.GooId`_POST
#  define WPP`i.GooId`_POST(`i.GooArgs`)
#endif
#ifndef WPP`i.GooId`_PRE
#  define WPP`i.GooId`_PRE(`i.GooArgs`)
#endif
#define WPP_CALL_`i.Name`(`i.FixedArgs` MSGARGS) \
            WPP_LOG_ALWAYS(WPP_EX`i.GooId`(`i.GooVals`),MSGARGS)\
            WPP`i.GooId`_PRE(`i.GooVals`) \
            WPP_ANNOTATE(`i.Name`) \
            (( \
                WPP_CHECK_INIT  WPP`i.GooId`_ENABLED(`i.GooVals`)? \
                WPP_SF_`i.TypeSig`(WPP`i.GooId`_LOGGER(`i.GooVals`) `i.MsgNo`,WPP_LOCAL_TraceGuids+0 WPP_R`i.ReorderSig` MSGARGS), \
                WPP_DEBUG(MSGARGS),1:0 \
             )) \
             WPP`i.GooId`_POST(`i.GooVals`)
`ENDFOR`
#else
`FORALL i IN Messages WHERE MsgArgs`
#ifndef WPP`i.GooId`_POST
#  define WPP`i.GooId`_POST(`i.GooArgs`)
#endif
#ifndef WPP`i.GooId`_PRE
#  define WPP`i.GooId`_PRE(`i.GooArgs`)
#endif
#define WPP_CALL_`i.Name`(`i.FixedArgs` MSGARGS) \
            WPP_LOG_ALWAYS(WPP_EX`i.GooId`(`i.GooVals`), MSGARGS)\
            WPP`i.GooId`_PRE(`i.GooVals`) \
            WPP_ANNOTATE(`i.Name`) \
            ( \
                WPP_CHECK_INIT  WPP`i.GooId`_ENABLED(`i.GooVals`)?\
                WPP_SF_`i.TypeSig`(WPP`i.GooId`_LOGGER(`i.GooVals`) `i.MsgNo`,WPP_LOCAL_TraceGuids+0 WPP_R`i.ReorderSig` MSGARGS),\
                1:0 \
             ) \
             WPP`i.GooId`_POST(`i.GooVals`)
`ENDFOR`
#endif



#if defined(__cplusplus)
};
#endif
