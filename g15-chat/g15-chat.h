extern "C" {
	int __declspec(dllexport) LcdInit( wchar_t* name, unsigned int historySize = 30 );
	int __declspec(dllexport) LcdClose( void );
	int __declspec(dllexport) LcdPrint( wchar_t* text );
	int __declspec(dllexport) LcdDraw( void );
}
