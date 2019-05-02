#pragma once

#include "Globals.h"
#include "Application.h"
#include <time.h>

void Log(const char file[], int line, LogTypes mode, bool scripting, const char* format, ...)
{
	bool debugdefined = true;
#ifndef _DEBUG
	debugdefined = false;
#endif

	if (!debugdefined && !scripting && mode == LogTypes::Normal)
		return;

	static char tmp_string[MAX_BUF_SIZE];
	static char tmp_string2[MAX_BUF_SIZE];
	static va_list  ap;

	// Construct the string from variable arguments
	va_start(ap, format);
	vsprintf_s(tmp_string, MAX_BUF_SIZE, format, ap);
	va_end(ap);
	sprintf_s(tmp_string2, MAX_BUF_SIZE, "\n%s(%d) : %s", file, line, tmp_string);
	OutputDebugString(tmp_string2);

	// Send the string to the console
	if (App != nullptr)
	{
		sprintf_s(tmp_string2, MAX_BUF_SIZE, "%s\n", tmp_string);

		switch ((LogTypes)mode)
		{
			case LogTypes::Warning:
			{
				if (strstr(tmp_string2, "Warning: ") == nullptr)
				{
					strcpy(tmp_string, tmp_string2);
					strcpy(tmp_string2, "");
					strcat(tmp_string2, "Warning: ");
					strcat(tmp_string2, tmp_string);
				}
				break;
			}
			case LogTypes::Error:
			{
				if (strstr(tmp_string2, "Error: ") == nullptr)
				{
					strcpy(tmp_string, tmp_string2);
					strcpy(tmp_string2, "");
					strcat(tmp_string2, "Error: ");
					strcat(tmp_string2, tmp_string);
				}
				break;
			}
		}

		App->LogGui(tmp_string2);
	}
}

void OpenInBrowser(char* url)
{
	ShellExecute(0, 0, url, 0, 0, SW_SHOW);
}

void OpenInExplorer()
{
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);
	ShellExecute(NULL, "open", pwd, NULL, NULL, SW_SHOWDEFAULT);
}

bool ApproximatelyEqual(float a, float b, float epsilon)
{
	return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool EssentiallyEqual(float a, float b, float epsilon)
{
	return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

// https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats
float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

#pragma optimize("", off)
void release_mode_breakpoint()
{
	int put_breakpoint_here = 1;
}
#pragma optimize("", on)