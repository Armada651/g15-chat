#ifdef __cplusplus
extern "C" {
#endif
	int __stdcall LcdInit( wchar_t* name );
	void __stdcall LcdClose( void );
	void __stdcall LcdPrint( wchar_t* text );
#ifdef __cplusplus
}
#endif