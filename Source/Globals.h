#pragma once

// Warning disabled ---
#pragma warning( disable : 4577 ) // Warning that exceptions are disabled
#pragma warning( disable : 4530 ) // Warning that exceptions are disabled

#include <windows.h>
#include <stdio.h>

enum class LogTypes
{
	Normal,
	Warning,
	Error
};

#define CONSOLE_LOG(format, ...) Log(__FILE__, __LINE__, LogTypes::Normal, format, __VA_ARGS__);
#define CONSOLE_LOG(mode, format, ...) Log(__FILE__, __LINE__, mode, false, format, __VA_ARGS__);
#define CONSOLE_SCRIPTING_LOG(mode, format, ...) Log(__FILE__, __LINE__, mode, true, format, __VA_ARGS__);

void Log(const char file[], int line, LogTypes mode, bool scripting, const char* format, ...);

void OpenInBrowser(char* url);

void OpenInExplorer();

#define EPSILON 0.00001
bool ApproximatelyEqual(float a, float b, float epsilon = EPSILON);

bool EssentiallyEqual(float a, float b, float epsilon = EPSILON);

float RandomFloat(float a, float b);

#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define PI 3.14159265358979323846264338327950288

#define MSTOSECONDS 0.001

#define RELEASE(x) \
    { \
    if (x != nullptr) \
      delete x; \
	x = nullptr; \
	} \

#define RELEASE_ARRAY(x) \
    { \
    if (x != nullptr) \
      delete[] x; \
	x = nullptr; \
	} \

#pragma optimize("", off)
void release_mode_breakpoint();
#pragma optimize("", on)

typedef unsigned int uint;
typedef unsigned char uchar;

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

#define MAX_BUF_SIZE 4096
#define OPEN_GL_BUF_SIZE 512
#define DEFAULT_BUF_SIZE 256
#define INPUT_BUF_SIZE 128

#define EXTENSION_MESH ".nekoMesh"
#define EXTENSION_TEXTURE ".nekoDDS"
#define EXTENSION_BONE ".nekoBone"
#define EXTENSION_ANIMATION ".nekoAnimation"
#define EXTENSION_SCENE ".scn"
#define EXTENSION_FONT ".ttf"
#define EXTENSION_VERTEX_SHADER_OBJECT ".vsh"
#define EXTENSION_FRAGMENT_SHADER_OBJECT ".fsh"
#define EXTENSION_GEOMETRY_SHADER_OBJECT ".gsh"
#define EXTENSION_SHADER_PROGRAM ".psh"
#define EXTENSION_PREFAB ".pfb"
#define EXTENSION_META ".meta"
#define EXTENSION_SCRIPT ".cs"
#define EXTENSION_MATERIAL ".mat"
#define EXTENSION_ANIMATOR ".ani"
#define EXTENSION_AVATAR ".ava"
#define EXTENSION_AUDIOBANK ".bnk"

#define UILAYER 1