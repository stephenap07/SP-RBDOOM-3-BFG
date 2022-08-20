
/**
 * @noSelf
 */
declare namespace imgui {
	enum WindowFlags {
	    None,
	    NoTitleBar,
	    NoResize,
	    NoMove,
	    NoScrollbar,
	    NoScrollWithMouse,
	    NoCollapse,
	    AlwaysAutoResize,
	    NoBackground,
	    NoSavedSettings,
	    NoMouseInputs,
	    MenuBar,
	    HorizontalScrollbar,
	    NoFocusOnAppearing,
	    NoBringToFrontOnFocus,
	    AlwaysVerticalScrollbar,
	    AlwaysHorizontalScrollbar,
	    AlwaysUseWindowPadding,
	    NoNavInputs,
	    NoNavFocus,
	    UnsavedDocument,
	    NoNav,
	    NoDecoration,
	    NoInputs,
	    NavFlattened,
	    ChildWindow,
	    Tooltip,
	    Popup,
	    Modal,
	    ChildMenu
	}
	
	function beginWindow(name: string, windowFlags: WindowFlags): boolean;
	function endWindow(): void;
	function text(...text: string[]): void;
	function newFrame(): void;
	function isReadyToRender(): boolean;
}