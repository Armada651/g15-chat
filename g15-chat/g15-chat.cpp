#include <Windows.h>
#include <string.h>
#include <time.h>
#include "LCDUI.h"
#include "g15-chat.h"

#define LINES 4
#define RESET_DELAY 2000 * 10000

CLCDConnection connection; // The connection
CLCDPage monoPage; // The page
CLCDText lines[LINES]; // The text lines on the page

wchar_t** strings; // Buffer with strings printed
wchar_t** writePtr; // Points to the string in the buffer to write
wchar_t** readPtr; // Points to the string in the buffer to read
unsigned int history; // Size of strings buffer
unsigned int offset; // Offset int the string to start drawing (horizontal scrolling)

HANDLE hResetTimer; // Timer that resets the display
HANDLE hMutex; // Thread-safety mutex

VOID CALLBACK OnResetCallback(LPVOID lpArgToCompletionRoutine,DWORD dwTimerLowValue,DWORD dwTimerHighValue)
{
	// Acquire the mutex
	WaitForSingleObject(hMutex, 10000);

	// Reset scrolling
	readPtr = writePtr;
	offset = 0;

	// Update the display
	LcdDraw();
}

DWORD CALLBACK OnLCDButtonsCallback(int device, DWORD dwButtons, const PVOID pContext)
{
	// Acquire the mutex
	if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT) return 1;

	// Set the timer to reset the display after one second
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -(RESET_DELAY);
	SetWaitableTimer(hResetTimer, &dueTime, 0, OnResetCallback, NULL, FALSE);

	switch(dwButtons)
	{
		case NULL: // Button released
			{
				CancelWaitableTimer(hResetTimer);
			}
		break;
		case LGLCDBUTTON_BUTTON0: // Up
		case LGLCDBUTTON_UP:
			{
				// Keep enough distance to write pointer and empty values so the display does not wrap around
				wchar_t** current = readPtr;
				bool reached = false;
				for(int i=0; i<LINES && !reached; i++)
				{
					// Decrement the pointer
					current--;

					// Wrap around the buffer
					if(current<strings) current=strings+history-1;

					// Check if the write pointer or an empty value has been reached
					if(current == writePtr || *current == NULL) reached = true;
				}

				if(!reached)
				{
					// Decrement the read pointer
					if(readPtr==strings) readPtr = strings+history-1;
					else readPtr--;
				}

				// Reset the offset
				offset=0;

				// Update the display
				LcdDraw();
			}
		break;
		case LGLCDBUTTON_BUTTON1: // Down
		case LGLCDBUTTON_DOWN:
			{
				// Read pointer can not move past write pointer
				if(readPtr != writePtr)
				{
					// Increment the read pointer
					readPtr++;

					// Wrap around the buffer
					if(readPtr==strings+history) readPtr = strings;

				}

				// Reset the offset
				offset=0;

				// Update the display
				LcdDraw();
			}
		break;
		case LGLCDBUTTON_BUTTON2: // Left
		case LGLCDBUTTON_LEFT:
			{
				if(offset-25>=0) offset-=25;

				// Update the display
				LcdDraw();
			}
		break;
		case LGLCDBUTTON_BUTTON3: // Right
		case LGLCDBUTTON_RIGHT:
			{
				wchar_t** current = readPtr;
				int count = 0;
				for(int i=0; i<LINES; i++)
				{
					// Check if the end of the longest string on the display has been reached
					if(*current != NULL)
					{
						unsigned int len = wcslen(*current);
						if(len<offset+25) count++;
					}
					else count++;

					// Decrement the pointer
					current--;

					// Wrap around the buffer
					if(current<strings) current=strings+history-1;

				}

				if(count<4) offset+=25;

				// Update the display
				LcdDraw();
			}
		break;
	}
	
	// Release the mutex
	ReleaseMutex(hMutex);
	
	return 0;
}

int LcdInit( wchar_t* name, unsigned int historySize )
{
	// Check if already connected
	if(connection.IsConnected()) return 1;

	// Create the mutex
	hMutex = CreateMutex(NULL, true, NULL);
	if(hMutex == NULL) return 3;

	// Create the display reset timer
	hResetTimer = CreateWaitableTimer(NULL, FALSE, NULL);

	// Set up the connection and softbutton context
	lgLcdConnectContextEx ConnectCtx;
	lgLcdSoftbuttonsChangedContext ButtonCtx;

	ConnectCtx.appFriendlyName = name;
	ConnectCtx.dwAppletCapabilitiesSupported = LGLCD_APPLET_CAP_BW;
	ConnectCtx.isAutostartable = FALSE;
	ConnectCtx.isPersistent = FALSE;
	ConnectCtx.onConfigure.configCallback = NULL;
	ConnectCtx.onConfigure.configContext = NULL;
	ConnectCtx.onNotify.notificationCallback = NULL;
	ConnectCtx.onNotify.notifyContext = NULL;

	ButtonCtx.softbuttonsChangedCallback = OnLCDButtonsCallback;

	// Initialize the connection
	if(connection.Initialize(ConnectCtx, &ButtonCtx) == FALSE)
	{
		// Destroy the mutex
		CloseHandle(hMutex);
		
		return 2;
	}

	// Select monochrome output
	CLCDOutput* monoOutput = connection.MonoOutput();
	
	// Initialize the buffer and pointers
	history = historySize;
	strings = (wchar_t**)calloc(historySize, sizeof(wchar_t*));
	readPtr = strings;
	writePtr = strings;

	// Reset the offset
	offset = 0;
	
	// Initialize the text lines and add them to the page
	for(int i=0; i<LINES; i++)
	{
		lines[i].Initialize();
		lines[i].SetOrigin(0,i*10);
		lines[i].SetSize(160, 13);
		lines[i].SetFontFaceName(_T("Microsoft Sans Serif"));
		lines[i].SetFontWeight(4);
		lines[i].SetFontPointSize(7);
		monoPage.AddObject(&lines[i]);
	}

	// Show the page and update the display
	monoOutput->ShowPage(&monoPage);
	connection.Update();

	// Release the mutex
	ReleaseMutex(hMutex);

	return 0;
}

int LcdClose( void )
{
	// Check if connected
	if(!connection.IsConnected()) return 1;
	
	// Acquire the mutex
	if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT) return 2;

	// Close the connection
	connection.Shutdown();

	// Destroy the mutex
	CloseHandle(hMutex);

	return 0;
}

int LcdPrint( wchar_t* text )
{
	// Acquire the mutex
	if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT) return 2;

	// Increment read pointer
	if(readPtr == writePtr)
	{
		// Increment the read pointer
		readPtr++;
		
		// Reset the offset
		offset=0;
	}
	else 
	{
		// Keep enough distance to read pointer
		wchar_t** current = writePtr;
		for(int i=0; i<LINES; i++)
		{
			// Increment the pointer
			current++;

			// Wrap around the buffer
			if(current>=strings+history) current=strings;

			// Check if the read pointer has been reached
			if(current == readPtr)
			{
				// Increment the read pointer
				readPtr++;

				// Reset the offset
				offset=0;

				// Exit the loop
				break;
			}
		}
	}

	// Increment write pointer
	writePtr++;

	// Wrap around the buffer
	if(readPtr == strings+history) readPtr = strings;
	if(writePtr == strings+history) writePtr = strings;

	// If the new starting position already has a string, free the memory
	if(*writePtr != NULL) free(*writePtr);

	// Allocate memory and copy the given string into it
	int size = wcslen(text) + 1; // Extra element for the NULL character
	*writePtr = (wchar_t*)malloc(sizeof(wchar_t) * size);
	wcscpy_s(*writePtr, size, text);
	
	// Draw the display
	LcdDraw();

	// Release the mutex
	ReleaseMutex(hMutex);
	
	return 0;
}

int LcdDraw( void )
{
	// Acquire the mutex
	if(WaitForSingleObject(hMutex, 10000)==WAIT_TIMEOUT) return 2;

	// Set the text lines from oldest (top) to newest (bottom)
	wchar_t** current = readPtr;
	for(int i=LINES-1; i>=0; i--)
	{
		// Display the string
		if(*current != NULL)
		{
			// Start printing the string starting from the offset
			if(offset < wcslen(*current)) lines[i].SetText(*current+offset);
			else lines[i].SetText(L"");
		}
		else lines[i].SetText(L"");

		// Decrement the pointer
		current--;

		// Wrap around the buffer
		if(current<strings) current=strings+history-1;
	}

	// Update the display
	connection.Update();

	// Release the mutex
	ReleaseMutex(hMutex);

	return 0;
}