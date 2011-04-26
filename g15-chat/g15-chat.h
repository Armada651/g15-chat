#ifdef __cplusplus
extern "C" {
#endif
	int __declspec(dllexport) LcdInit( wchar_t* name );
	void __declspec(dllexport) LcdClose( void );
	void __declspec(dllexport) LcdPrint( wchar_t* text );
#ifdef __cplusplus
}
#endif