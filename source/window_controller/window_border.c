#include <window_controller/window_definitions.h>
#include <window_controller/window_border.h>
#include <window_controller/window_controller.h>
#include <asset_loader/asset_loader.h>
#include <debug.h>
#include <raylib.h>

static inline bool reset_window(WindowControlData *data)
{
	SetWindowSize(data->virtual_width, data->virtual_height);
	if (IsRenderTextureValid(data->window_buffer))
	{
		UnloadRenderTexture(data->window_buffer);
		data->window_buffer = LoadRenderTexture(data->virtual_width, data->virtual_height);

		if (!IsRenderTextureValid(data->window_buffer))
		{
			debug_print("[%s] Critical: Unable to load new window buffer\n", BORDER_SIGNATURE);
			return false;
		}
	}
	window_center();
	return true;
}

static inline bool set_window_border_attributes(WindowControlData* data)
{
	if (!IsImageValid(data->border->border_image))
	{
		debug_print("[%s] Unable to retrieve border image, canceling window transformation\n", BORDER_SIGNATURE);
		return false;
	}
	data->virtual_width = data->border->border_image.width;
	data->virtual_height = data->border->border_image.height;
	debug_print("[%s] Retrieved, and successfully unloaded border image\n", BORDER_SIGNATURE);

	return IsWindowReady() ? reset_window(data) : true;
}

WindowBorder window_new_border(const char *path)
{

	WindowBorder border =
	{
		.border_image = asset_retrieve_image(path),
		.color = WHITE,
		.alpha = 1,
		.border_texture = (Texture2D){0}
	};

	return border;
}

bool window_set_current_border(WindowBorder *border)
{
	WindowControlData* data = get_window_control_data();
	if (!data) return false;

	data->border = border;

	if (data->border_enabled) return set_window_border_attributes(data);

	debug_print(&(data->border)!=NULL ? "[%s] Sucesfully set new border" : "[%s] Failed to set new border\n", BORDER_SIGNATURE);
	return &(data->border)!=NULL ? true : false;
}

void window_draw_border(WindowBorder* border)
{
	WindowControlData* data = get_window_control_data();
	if (!IsRenderTextureValid(data->window_buffer))
	{
		debug_print("[%s] Unable to draw border; window buffer is invalid\n", BORDER_SIGNATURE);
		return;
	}
	BeginTextureMode(data->window_buffer);

	if (!IsTextureValid(border->border_texture))
	{
		border->border_texture = LoadTextureFromImage(border->border_image);
		if (!IsTextureValid(border->border_texture))
		{
			debug_print("[%s] Failed to load border texture, skipping draw process\n", BORDER_SIGNATURE);
			return;
		}
		UnloadImage(border->border_image); //Preserve image
	}

	DrawTextureEx(border->border_texture, (Vector2){0, 0}, 0, 1, ColorAlpha(border->color, border->alpha));
	EndTextureMode();
}

void window_enable_border(void)
{
	WindowControlData *data = get_window_control_data();
	if (!data) return;

	if (!set_window_border_attributes(data)) return;
	data->border_enabled = true;
	debug_print("[%s] Border enabled\n", BORDER_SIGNATURE);
}

void window_disable_border(void)
{
	WindowControlData *data = get_window_control_data();
	if (!data) return;

	data->virtual_width = data->base_width;
	data->virtual_height = data->base_height;

	reset_window(data);

	data->border_enabled = false;
	debug_print("[%s] Border disabled\n", BORDER_SIGNATURE);
}

bool is_border_enabled(void)
{
	WindowControlData *data = get_window_control_data();
	if (!data) return false;

	return data->border_enabled;
}
