
#ifndef NEO_IMGUI_IMGUI_HOOKS_H_
#define NEO_IMGUI_IMGUI_HOOKS_H_

#include "../sys/sys_public.h"

#include <stdio.h>
#include <stdint.h>
#if defined _WIN32 || defined __CYGWIN__
	#ifdef CIMGUI_NO_EXPORT
		#define IMGUIAPI
	#else
		#define IMGUIAPI __declspec(dllexport)
	#endif
	#ifndef __GNUC__
		#define snprintf sprintf_s
	#endif
#else
	#define API
#endif

#if defined __cplusplus
	#define EXTERN extern "C"
#else
	#include <stdarg.h>
	#include <stdbool.h>
	#define EXTERN extern
#endif

#define CIMGUI_API EXTERN IMGUIAPI
#define CONST const

namespace ImGuiHook
{

bool	Init( int windowWidth, int windowHeight );

bool	IsInitialized();

// tell imgui that the (game) window size has changed
void	NotifyDisplaySizeChanged( int width, int height );

// inject a sys event (keyboard, mouse, unicode character)
bool	InjectSysEvent( const sysEvent_t* keyEvent );

// inject the current mouse wheel delta for scrolling
bool	InjectMouseWheel( int delta );

// call this once per frame *before* calling ImGui::* commands to draw widgets etc
// (but ideally after getting all new events)
void	NewFrame();

// call this to enable custom ImGui windows which are not editors
bool	IsReadyToRender();

// call this once per frame (at the end) - it'll render all ImGui::* commands
// since NewFrame()
void	Render();

void	Destroy();

} //namespace ImGuiHook

CIMGUI_API void igHookNewFrame();
CIMGUI_API bool	igIsReadyToRender();


#endif /* NEO_IMGUI_IMGUI_HOOKS_H_ */
