
#define INCLUDE_FILE(TAIL) TO_STR( CAT_2(FILE__NAME,TAIL) )

#pragma message ( TO_STR(INT_DIR) "\\" INCLUDE_FILE(.tmh) "(0) : WPP MESSAGE: File Included")

#pragma warning(push)
#pragma warning(disable:26440 26493 26494 26477 26485 26489)
#include INCLUDE_FILE(.tmh)
#pragma warning(pop)

#undef INCLUDE_FILE





