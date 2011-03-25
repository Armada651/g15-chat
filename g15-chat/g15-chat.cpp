/*
 * TODO:
 * - Implement history function
 * - Make code thread-safe
 * - Vertical (and horizontal?) scrolling with softbuttons
 */

#include <string.h>
#include "LCDUI.h"
#include "g15-chat.h"

#define LINES 4

CLCDConnection Connection;
CLCDPage monoPage;
CLCDText Lines[LINES];
WCHAR* Strings[LINES];
UCHAR FirstLine;

int __stdcall LcdInit( WCHAR* name )
{
    lgLcdConnectContextEx ConnectCtx;

	ConnectCtx.appFriendlyName = name;
	ConnectCtx.dwAppletCapabilitiesSupported = LGLCD_APPLET_CAP_BW;
    ConnectCtx.isAutostartable = FALSE;
	ConnectCtx.isPersistent = FALSE;
    ConnectCtx.onConfigure.configCallback = NULL;
    ConnectCtx.onConfigure.configContext = NULL;
    ConnectCtx.onNotify.notificationCallback = NULL;
    ConnectCtx.onNotify.notifyContext = NULL;

	if(Connection.Initialize(ConnectCtx) == FALSE)
	{
		return 1;
	}

	CLCDOutput* monoOutput = Connection.MonoOutput();

	monoOutput->ShowPage(&monoPage);
	
	FirstLine = 0;
	memset(Strings, NULL, sizeof(Strings));
	for(int i=0; i<LINES; i++)
	{
		Lines[i].Initialize();
		Lines[i].SetOrigin(0,i*10);
		Lines[i].SetSize(160, 13);
		Lines[i].SetFontFaceName(_T("Microsoft Sans Serif"));
		Lines[i].SetFontWeight(4);
		Lines[i].SetFontPointSize(7);
		monoPage.AddObject(&Lines[i]);
	}

	Connection.Update();

	return 0;
}

void __stdcall LcdClose( void )
{
    Connection.Shutdown();
}

void __stdcall LcdPrint( WCHAR* text )
{
	if(Strings[FirstLine] != NULL) free(Strings[FirstLine]);
	Strings[FirstLine] = (WCHAR*)malloc(sizeof(WCHAR) * (wcslen(text)+1));
	wcscpy(Strings[FirstLine], text);
	
	int j = FirstLine;
	for(int i=LINES-1; i>=0; i--)
	{
		Lines[i].SetText(Strings[j]);
		j--;
		if(j<0) j=LINES-1;
	}
	
	FirstLine++;
	FirstLine%=LINES;

	Connection.Update();
}
