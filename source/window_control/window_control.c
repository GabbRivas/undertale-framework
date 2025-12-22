#include "window_control.h"
#include "raylib.h"

static unsigned int 	base_game_width = 640;
static unsigned int 	base_game_height = 480;

static unsigned int	border_width;
static unsigned int	border_height;
// Average border texture is 960x540

static const char*	border_image_path;
static Image		border_image;
static Texture2D	border_texture;
static Color		border_color = WHITE;

static bool 		is_border_enabled = false;

RenderTexture2D 	window_buffer;
RenderTexture2D		game_buffer;

static void inline _center_window(void){
	unsigned int _x = (GetMonitorWidth(GetCurrentMonitor()) - GetScreenWidth()) / 2;
	unsigned int _y = (GetMonitorHeight(GetCurrentMonitor()) - GetScreenHeight()) / 2;

	SetWindowPosition(_x, _y);
}

static void inline _init_window_buffer(void){
	if (!IsRenderTextureValid(window_buffer)){
		window_buffer = LoadRenderTexture(get_window_width(), get_window_height());
		return;
	}
}

static void inline _init_game_buffer(void){
	if (!IsRenderTextureValid(game_buffer)){
		game_buffer = LoadRenderTexture(base_game_width, base_game_height);
		return;
	}
	if (game_buffer.texture.width != base_game_width || game_buffer.texture.height != base_game_height){
		game_buffer = LoadRenderTexture(base_game_width, base_game_height);
		return;
	}
}

void inline _draw_border(void){
	if (!is_border_enabled) return;
	if (!IsTextureValid(border_texture)){
		border_texture = LoadTextureFromImage(border_image);
		UnloadImage(border_image);
	}
	DrawTexturePro(border_texture, (Rectangle){0,0,border_width, border_height}, (Rectangle){0,0,border_width, border_height}, (Vector2){0, 0}, 0, border_color);
}

void set_border_path(const char* path){
	if (!IsPathFile(path)) return;
	border_image_path = path;
	if (is_border_enabled) enable_border();
}

void enable_border(void){
	border_image = LoadImage(border_image_path);
	border_width = border_image.width;
	border_height = border_image.height;
	is_border_enabled = true;
}

void draw_init(void){
	_init_window_buffer();
	_init_game_buffer();
	return;
}

void draw_begin(void){
	BeginTextureMode(game_buffer);
	ClearBackground(BLACK);
}

void draw_end(void){
	EndTextureMode();
	BeginTextureMode(window_buffer);

	float _game_offset_x = (window_buffer.texture.width - game_buffer.texture.width)/2.0f;
	float _game_offset_y = (window_buffer.texture.height - game_buffer.texture.height)/2.0f;

	DrawTexturePro(game_buffer.texture,
		(Rectangle){0.0f, 0.0f, game_buffer.texture.width, -game_buffer.texture.height},
		(Rectangle){_game_offset_x, _game_offset_y, game_buffer.texture.width, game_buffer.texture.height}, (Vector2){0,0}, 0.0f, WHITE);

	_draw_border();
	EndTextureMode();

	float _scale_factor = fminf((float)GetScreenWidth()/window_buffer.texture.width, (float)GetScreenHeight()/window_buffer.texture.height);

	BeginDrawing();

	ClearBackground(BLACK);

	float _window_correction_offset_x = ((float)GetScreenWidth() - window_buffer.texture.width*_scale_factor)/2.0f;
	float _window_correction_offset_y = ((float)GetScreenHeight() - window_buffer.texture.height*_scale_factor)/2.0f;

	DrawTexturePro(window_buffer.texture,
		(Rectangle){0.0f,0.0f,window_buffer.texture.width, -window_buffer.texture.height},
		(Rectangle){
			_window_correction_offset_x, _window_correction_offset_y,
			window_buffer.texture.width*_scale_factor, window_buffer.texture.height*_scale_factor},
		(Vector2){0,0}, 0.0f, WHITE);

	EndDrawing();
}

void draw_un_init(void){
	if (IsRenderTextureValid(window_buffer)) UnloadRenderTexture(window_buffer);
	if (IsRenderTextureValid(game_buffer)) UnloadRenderTexture(game_buffer);
}


unsigned int get_window_width(void){
	return is_border_enabled ? border_width : base_game_width;
}
unsigned int get_window_height(void){
	return is_border_enabled ? border_height : base_game_height;
}

// Does not use raylib's implementation since it scales but the window cannot lose focus with Alt+Tab, contrary to this one:
// Scale resizing is implement in the self-made draw evens WIP
void toggle_borderless_fullscreen(void){
	if (IsWindowState(FLAG_WINDOW_UNDECORATED)){
		ClearWindowState(FLAG_WINDOW_UNDECORATED);
		SetWindowSize(get_window_width(), get_window_height());
		_center_window();
		return;
	}
	SetWindowState(FLAG_WINDOW_UNDECORATED);
	SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
	_center_window();
}

bool is_border_active(void){
	return is_border_enabled;
}
