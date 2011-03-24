#include <WinDef.h>

#ifdef __cplusplus
extern "C" {
#endif
	int __stdcall LcdInit( WCHAR* name );
	void __stdcall LcdClose( void );
	void __stdcall LcdPrint( WCHAR* text );
#ifdef __cplusplus
}
#endif