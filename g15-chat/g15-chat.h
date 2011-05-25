extern "C" {
	int __declspec(dllexport) LcdInit( wchar_t* name );
	int __declspec(dllexport) LcdClose( void );
	int __declspec(dllexport) LcdPrint( wchar_t* text );
}
