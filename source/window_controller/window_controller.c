#include <window_controller/window_controller.h>
#include <window_controller/window_definitions.h>
#include <raylib.h>
#include <debug.h>

WindowControlData window_control_data;

void window_center(void)
{
	unsigned int target_x = (GetMonitorWidth(GetCurrentMonitor()) - GetScreenWidth()) / 2;
	unsigned int target_y = (GetMonitorHeight(GetCurrentMonitor()) - GetScreenHeight()) / 2;

	SetWindowPosition(target_x, target_y);
}

void init_window_control(uint64_t _base_width, uint64_t _base_height)
{
	window_control_data.base_width = _base_width;
	window_control_data.base_height = _base_height;
	window_control_data.virtual_width = _base_width;
	window_control_data.virtual_height = _base_height;
	window_control_data.border_enabled = false;
	debug_print("[%s] Initialized system\n", WINDOW_CONTROLLER_SIGNATURE);
}

WindowControlData* get_window_control_data(void)
{
	return &window_control_data;
}

void toggle_borderless_fullscreen(void)
{
	if (IsWindowState(FLAG_WINDOW_UNDECORATED)){
		ClearWindowState(FLAG_WINDOW_UNDECORATED);
		SetWindowSize(window_control_data.virtual_width, window_control_data.virtual_height);
		window_center();
		return;
	}
	SetWindowState(FLAG_WINDOW_UNDECORATED);
	SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
	window_center();
}
