/*
 * TODO:
 * - Implement history function
 * - Vertical (and horizontal?) scrolling with softbuttons
 */

#include <string.h>
#include "LCDUI.h"
#include "g15-chat.h"

#define LINES 4

CLCDConnection Connection; // The connection
CLCDPage monoPage; // The page
CLCDText Lines[LINES]; // The text lines on the page
wchar_t* Strings[LINES]; // Buffer with strings printed
unsigned char FirstLine; // Indicates the newest line in the buffer

int __declspec(dllexport) LcdInit( wchar_t* name )
{
	// Set up the connection context
    lgLcdConnectContextEx ConnectCtx;

	ConnectCtx.appFriendlyName = name;
	ConnectCtx.dwAppletCapabilitiesSupported = LGLCD_APPLET_CAP_BW;
    ConnectCtx.isAutostartable = FALSE;
	ConnectCtx.isPersistent = FALSE;
    ConnectCtx.onConfigure.configCallback = NULL;
    ConnectCtx.onConfigure.configContext = NULL;
    ConnectCtx.onNotify.notificationCallback = NULL;
    ConnectCtx.onNotify.notifyContext = NULL;

	// Initialize the connection, if it fails return errorcode 1
	if(Connection.Initialize(ConnectCtx) == FALSE)
	{
		return 1;
	}

	// Select monochrome output
	CLCDOutput* monoOutput = Connection.MonoOutput();

	//Initialize the text lines and add them to the page
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

	//Show the page and update the display
	monoOutput->ShowPage(&monoPage);
	Connection.Update();

	return 0;
}

void __declspec(dllexport) LcdClose( void )
{
	// Close the connection
    Connection.Shutdown();
}

void __declspec(dllexport) LcdPrint( wchar_t* text )
{
	// Increase starting position
	FirstLine++;
	//Wrap around the buffer
	FirstLine%=LINES;

	// If the new starting position already has a string, free the memory
	if(Strings[FirstLine] != NULL) free(Strings[FirstLine]);

	// Allocate memory and copy the given string into it
	Strings[FirstLine] = (WCHAR*)malloc(sizeof(WCHAR) * (wcslen(text)+1));
	wcscpy(Strings[FirstLine], text);
	
	// Set the text lines from oldest (top) to newest (bottom)
	for(int i=LINES-1, j=FirstLine; i>=0; i--, j--)
	{
		// Wrap around the buffer
		if(j<0) j=LINES-1;

		// Set the line
		Lines[i].SetText(Strings[j]);
	}

	// Update the display
	Connection.Update();
}
