local ffi = require "ffi"

ffi.cdef[[
	typedef int ImGuiWindowFlags;
	
	enum ImGuiWindowFlags_
	{
    	ImGuiWindowFlags_None = 0,
    	ImGuiWindowFlags_NoTitleBar = 1 << 0,
    	ImGuiWindowFlags_NoResize = 1 << 1,
    	ImGuiWindowFlags_NoMove = 1 << 2,
    	ImGuiWindowFlags_NoScrollbar = 1 << 3,
    	ImGuiWindowFlags_NoScrollWithMouse = 1 << 4,
    	ImGuiWindowFlags_NoCollapse = 1 << 5,
    	ImGuiWindowFlags_AlwaysAutoResize = 1 << 6,
    	ImGuiWindowFlags_NoBackground = 1 << 7,
    	ImGuiWindowFlags_NoSavedSettings = 1 << 8,
    	ImGuiWindowFlags_NoMouseInputs = 1 << 9,
    	ImGuiWindowFlags_MenuBar = 1 << 10,
    	ImGuiWindowFlags_HorizontalScrollbar = 1 << 11,
    	ImGuiWindowFlags_NoFocusOnAppearing = 1 << 12,
    	ImGuiWindowFlags_NoBringToFrontOnFocus = 1 << 13,
    	ImGuiWindowFlags_AlwaysVerticalScrollbar= 1 << 14,
    	ImGuiWindowFlags_AlwaysHorizontalScrollbar=1<< 15,
    	ImGuiWindowFlags_AlwaysUseWindowPadding = 1 << 16,
    	ImGuiWindowFlags_NoNavInputs = 1 << 18,
    	ImGuiWindowFlags_NoNavFocus = 1 << 19,
    	ImGuiWindowFlags_UnsavedDocument = 1 << 20,
    	ImGuiWindowFlags_NoNav = ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
    	ImGuiWindowFlags_NoDecoration = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse,
    	ImGuiWindowFlags_NoInputs = ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
    	ImGuiWindowFlags_NavFlattened = 1 << 23,
    	ImGuiWindowFlags_ChildWindow = 1 << 24,
    	ImGuiWindowFlags_Tooltip = 1 << 25,
    	ImGuiWindowFlags_Popup = 1 << 26,
    	ImGuiWindowFlags_Modal = 1 << 27,
    	ImGuiWindowFlags_ChildMenu = 1 << 28
	};

	bool igBegin(const char* name, bool* p_open, ImGuiWindowFlags flags);
	void igEnd(void);
	void igText(const char* fmt, ...);
	void igNewFrame();
	bool igIsReadyToRender();
]]

local C = ffi.C

imgui = {
	beginWindow = function(name, flags)
		return C.igBegin(name, nil, flags)
	end,
	endWindow = C.igEnd,
	text = C.igText,
	isReadyToRender = C.igIsReadyToRender,
	newFrame = C.igNewFrame,
	WindowFlags = {
    	None = C.ImGuiWindowFlags_None,
    	NoTitleBar = C.ImGuiWindowFlags_NoTitleBar,
    	NoResize = C.ImGuiWindowFlags_NoResize,
    	NoMove = C.ImGuiWindowFlags_NoMove,
    	NoScrollbar = C.ImGuiWindowFlags_NoScrollbar,
    	NoScrollWithMouse = C.ImGuiWindowFlags_NoScrollWithMouse,
    	NoCollapse = C.ImGuiWindowFlags_NoCollapse,
    	AlwaysAutoResize = C.ImGuiWindowFlags_AlwaysAutoResize,
    	NoBackground = C.ImGuiWindowFlags_NoBackground,
    	NoSavedSettings = C.ImGuiWindowFlags_NoSavedSettings,
    	NoMouseInputs = C.ImGuiWindowFlags_NoMouseInputs,
    	MenuBar = C.ImGuiWindowFlags_MenuBar,
    	HorizontalScrollbar = C.ImGuiWindowFlags_HorizontalScrollbar,
    	NoFocusOnAppearing = C.ImGuiWindowFlags_NoFocusOnAppearing,
    	NoBringToFrontOnFocus = C.ImGuiWindowFlags_NoBringToFrontOnFocus,
    	AlwaysVerticalScrollbar= C.ImGuiWindowFlags_AlwaysVerticalScrollbar,
    	AlwaysHorizontalScrollbar= C.ImGuiWindowFlags_AlwaysHorizontalScrollbar,
    	AlwaysUseWindowPadding = C.ImGuiWindowFlags_AlwaysUseWindowPadding,
    	NoNavInputs = C.ImGuiWindowFlags_NoNavInputs,
    	NoNavFocus = C.ImGuiWindowFlags_NoNavFocus,
    	UnsavedDocument = C.ImGuiWindowFlags_UnsavedDocument,
    	NoNav = C.ImGuiWindowFlags_NoNav,
    	NoDecoration = C.ImGuiWindowFlags_NoDecoration,
    	NoInputs = C.ImGuiWindowFlags_NoInputs,
    	NavFlattened = C.ImGuiWindowFlags_NavFlattened,
    	ChildWindow = C.ImGuiWindowFlags_ChildWindow,
    	Tooltip = C.ImGuiWindowFlags_Tooltip,
    	Popup = C.ImGuiWindowFlags_Popup,
    	Modal = C.ImGuiWindowFlags_Modal,
    	ChildMenu = C.ImGuiWindowFlags_ChildMenu
	}
}