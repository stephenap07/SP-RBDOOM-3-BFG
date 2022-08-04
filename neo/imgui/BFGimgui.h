
#ifndef NEO_IMGUI_BFGIMGUI_H_
#define NEO_IMGUI_BFGIMGUI_H_

#include "libs/imgui/imgui.h"

#include "../idlib/math/Vector.h"

// add custom functions for imgui
namespace ImGui
{

bool DragVec3( const char* label, idVec3& v, float v_speed = 1.0f,
			   float v_min = 0.0f, float v_max = 0.0f,
			   const char* display_format = "%.1f",
			   float power = 1.0f, bool ignoreLabelWidth = true );

bool DragVec3fitLabel( const char* label, idVec3& v, float v_speed = 1.0f,
					   float v_min = 0.0f, float v_max = 0.0f,
					   const char* display_format = "%.1f", float power = 1.0f );

IMGUI_API bool  InputText( const char* label, idStr* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL );
IMGUI_API bool  InputTextMultiline( const char* label, idStr* str, const ImVec2& size = ImVec2( 0, 0 ), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL );
IMGUI_API bool  InputTextWithHint( const char* label, const char* hint, idStr* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL );
}

#endif /* NEO_IMGUI_BFGIMGUI_H_ */
