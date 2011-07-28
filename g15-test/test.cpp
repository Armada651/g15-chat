#include "g15-chat.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	LcdInit(L"g15-test");

	while(true)
	{
		wchar_t str[1024];
		wcin >> str;
		LcdPrint(str);
	}
}