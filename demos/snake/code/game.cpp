
#include "anyfin/core/arena.hpp"
#include "anyfin/core/math.hpp"
#include "anyfin/core/math/random.hpp"
#include "anyfin/core/result.hpp"
#include "anyfin/core/status_code.hpp"
#include "anyfin/core/strings.hpp"

#include "anyfin/platform/platform.hpp"
#include "anyfin/platform/timers.hpp"
#include "anyfin/platform/files.hpp"
#include "anyfin/platform/win32/common_win32.hpp"

#include "anyfin/rhi/opengl/opengl.hpp"

#include "anyfin/window/window.hpp"
#include "anyfin/window/events.hpp"

#include <wincrypt.h>

static const usize window_width  = 1024;
static const usize window_height = 720;

static String vertex_shader = R"(
#version 460 core

layout (location = 0) in vec2 in_pos;

uniform vec3 fill_color;
uniform mat3 projection;

out vec3 color;

void main () {
  vec2 position = (projection * vec3(in_pos, 1.0)).xy;
  gl_Position = vec4(position, 0.0, 1.0);
  color = fill_color;
}
)";

static String fragment_shader = R"(
#version 460 core

in  vec3 color;
out vec4 frag_color;

void main () {
  frag_color = vec4(color, 1.0f);
}
)";

static const usize density = 15; // Defines the number of cells per row / per column
static const usize board_size = density * density;

enum struct Move_Direction { Up, Left, Right, Down };

static struct Game_State {
  u32 snake_state[board_size];
  u32 snake_length;

  u32 fruit_index;

  Move_Direction direction;
} game_state;

static f32 projection[9];

static void draw_square (const Vec2<f32> position, const f32 size, const Vec3<f32> color) {
  const f32 vertices [] {
    position.x, position.y,
    position.x + size, position.y,
    position.x + size, position.y + size,

    position.x, position.y,
    position.x, position.y + size,
    position.x + size, position.y + size,
  };

  const u32 vertices_count = array_count_elements(vertices);

  GLuint array_buffer;

  glGenBuffers(1, &array_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, array_buffer);

  const auto total_size = vertices_count * sizeof(f32);
  glBufferData(GL_ARRAY_BUFFER, total_size, vertices, GL_STATIC_DRAW);

  GLuint attributes_array;
  glGenVertexArrays(1, &attributes_array);
  glBindVertexArray(attributes_array);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        
  GLuint current_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&current_program));

  // TODO: #efficiency Is this the way to do this? Any way I can upload uniforms to GPU and reuse/bake into the shader?
  glUniform3fv(glGetUniformLocation(current_program, "fill_color"), 1, color.elems);
  glUniformMatrix3fv(glGetUniformLocation(current_program, "projection"), 1, GL_TRUE, projection);

  glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_count));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glDeleteBuffers(1, &array_buffer);
  glDeleteVertexArrays(1, &attributes_array);
}

static void draw_lattice (const Vec2<f32> offset, const f32 cell_size) {
  const usize bars_count = (density - 1);
  const usize vertices_count = bars_count * 2 * 2; // 2 verts per vertical and horizontal bars
  Vec2<f32> lattice_vertices [vertices_count]; 

  { // Fill vertices for each bar
    usize vertex_index = 0;

    // Vertical bars
    f32 horizontal_offset = offset.x + cell_size;
    for (usize idx = 0; idx < bars_count; idx++) {
      lattice_vertices[vertex_index]     = { horizontal_offset, 0 };
      lattice_vertices[vertex_index + 1] = { horizontal_offset, window_height };

      vertex_index += 2;
      horizontal_offset += cell_size;
    }

    // Horizontal bars
    f32 vertical_offset = cell_size;
    for (usize idx = 0; idx < bars_count; idx += 1) {
      lattice_vertices[vertex_index]     = { offset.x, vertical_offset };
      lattice_vertices[vertex_index + 1] = { offset.x + window_height, vertical_offset };

      vertex_index += 2;
      vertical_offset += cell_size;
    }
  }

  GLuint array_buffer;

  glGenBuffers(1, &array_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, array_buffer);

  const auto total_size = vertices_count * sizeof(Vec2<f32>);
  glBufferData(GL_ARRAY_BUFFER, total_size, lattice_vertices, GL_STATIC_DRAW);

  GLuint attributes_array;
  glGenVertexArrays(1, &attributes_array);
  glBindVertexArray(attributes_array);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        
  GLuint current_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint *>(&current_program));

  glUniform3f(glGetUniformLocation(current_program, "fill_color"), 0.0f, 0.0f, 0.0f);
  glUniformMatrix3fv(glGetUniformLocation(current_program, "projection"), 1, GL_TRUE, projection);

  glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices_count));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glDeleteBuffers(1, &array_buffer);
  glDeleteVertexArrays(1, &attributes_array);
}

static s32 pick_fruit_cell () {
  HCRYPTPROV hCryptProv;
  if (!CryptAcquireContext(&hCryptProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
    return -1;
  }

  BYTE randomData[sizeof(DWORD)];
  if (!CryptGenRandom(hCryptProv, sizeof(randomData), randomData)) {
    // Handle error
    CryptReleaseContext(hCryptProv, 0);
    return -1;
  }

  CryptReleaseContext(hCryptProv, 0);

  DWORD randomValue = *reinterpret_cast<DWORD*>(randomData);
  auto normalizedRandom = static_cast<f64>(randomValue) / static_cast<f64>(UINT_MAX);
  return static_cast<u32>(normalizedRandom * ((density * 2) - 1));
}

static u32 get_new_head_position (const Move_Direction dir) {
  const u32 head_position = game_state.snake_state[0];

  const u32 row = head_position / density;
  const u32 col = head_position % density;

  u32 new_head_position;

  switch (dir) {
    case Move_Direction::Up: {
      if (row == 0) new_head_position = (density - 1) * density + col;
      else          new_head_position = (row - 1) * density + col;

      break;
    }
    case Move_Direction::Right: {
      if (col == density - 1) new_head_position = row * density;
      else                    new_head_position = head_position + 1;

      break;
    }
    case Move_Direction::Left: {
      if (col == 0) new_head_position = row * density + (density - 1);
      else          new_head_position = head_position - 1;
      
      break;
    }
    case Move_Direction::Down: {
      if (row == density - 1) new_head_position = col;
      else                    new_head_position = (row + 1) * density + col;
      
      break;
    }
  }

  return new_head_position;
}

static void shift_snake (const u32 new_head_position) {
  if (game_state.snake_length > 1) {
    for (usize idx = game_state.snake_length - 1; idx > 0; idx--) {
      game_state.snake_state[idx] = game_state.snake_state[idx - 1];
    }
  }

  game_state.snake_state[0] = new_head_position;
}

static void pick_new_fruit_index (Linear_Conguential_Generator &lcg) {
  while (true) {
    const u32 position = get_random_in_range(lcg, 0, density * density);

    /*
      Check that the picked index is not taken by the snake's body
     */
    bool pick_another_position = false;
    for (usize idx = 0; idx < game_state.snake_length; idx++) {
      if (game_state.snake_state[idx] == position) {
        pick_another_position = true;
        break;
      }
    }

    if (pick_another_position || position == game_state.fruit_index) continue;

    game_state.fruit_index = position;

    return;
  }
}

static bool try_change_direction (const Move_Direction new_direction) {
  const auto direction = game_state.direction;
  
  if (new_direction == Move_Direction::Left) {
    if (direction != Move_Direction::Left &&
        direction != Move_Direction::Right) {
      game_state.direction = Move_Direction::Left;
      return true;
    }
  }

  if (new_direction == Move_Direction::Up) {
    if (direction != Move_Direction::Up &&
        direction != Move_Direction::Down) {
      game_state.direction = Move_Direction::Up;
      return true;
    }
  }

  if (new_direction == Move_Direction::Right) { 
    if (direction != Move_Direction::Right &&
        direction != Move_Direction::Left) {
      game_state.direction = Move_Direction::Right;
      return true;
    }
  }

  if (new_direction == Move_Direction::Down) {
    if (direction != Move_Direction::Down &&
        direction != Move_Direction::Up) {
      game_state.direction = Move_Direction::Down;             
      return true;
    }
  }

  return false;
}

static Status_Code game_main () {
  auto arena = Memory_Arena { reserve_virtual_memory(megabytes(64)) };

  check_status(enable_high_precision_timer());
  defer { disable_high_precision_timer(); };

  const auto timer_frequency = get_timer_frequency();

  auto font_file_path = make_file_path(arena, "assets", "yeasty.ttf");
  auto font_file      = open_file(font_file_path);
  check_status(font_file);

  auto font_atlas = create_font_atlas(arena, font_file, window_width, window_height);
  // const char raster_queue [] { 's', 'c', 'o', 'r', 'e', ':', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
  // rasterize_ascii_characters(font_atlas, raster_queue, array_count_elements(raster_queue));

  // take the underlying atlas image, convert that into a bitmap image and upload to GPU for the reference

  create_window_system("Snake", window_width, window_height);

  Linear_Conguential_Generator lcg { 42 };

  const auto shader = create_shader({
    .vertex_shader   = vertex_shader,
    .fragment_shader = fragment_shader,
  });
  check_status(shader);

  {
    projection[0] = 2.0f / window_width;
    projection[2] = -1;
    projection[4] = -2.0f / window_height;
    projection[5] = 1;
  }

  const f32 cell_size = static_cast<f32>(window_height) / density;

  glUseProgram(*shader);
  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

  const Vec2<f32> board_offset {  window_width - window_height };

  auto get_cell_offset = [&] (const u64 index) {
    return Vec2<f32> {
      board_offset.x + (index % density) * cell_size,
      (index / density) * cell_size
    };
  };

  game_state.snake_state[game_state.snake_length++] = get_random_in_range(lcg, 0, density * density);
  pick_new_fruit_index(lcg);

  auto start_time      = get_timer_value();
  auto game_counter    = 0;
  const u32 game_speed = 250;

  bool quit_requested = false;
  while (!quit_requested) {
    System_Event *events;
    usize events_count;
    pump_window_events(&events, &events_count);

    Move_Direction direction;
    bool direction_changed = false;

    for (usize idx = 0; idx < events_count; idx++) {
      const auto &event = events[idx];
      switch (event.tag) {
        case System_Event::Quit: {
          quit_requested = true;
          break;
        }
        case System_Event::Keyboard: {
          const auto keyboard = event.keyboard;

          if (keyboard.key_code == 0x25) {
            direction = Move_Direction::Left;
            direction_changed = true;
          }
          else if (keyboard.key_code == 0x26) {
            direction = Move_Direction::Up;
            direction_changed = true;
          }
          else if (keyboard.key_code == 0x27) {
            direction = Move_Direction::Right;
            direction_changed = true;
          }
          else if (keyboard.key_code == 0x28) {
            direction = Move_Direction::Down;             
            direction_changed = true;
          }
          
          break;
        }
      }
    }

    if (bool game_tick = game_counter >= game_speed;
        game_tick || (direction_changed && try_change_direction(direction))) {
      const u32 new_head_position = get_new_head_position(direction);
      if (new_head_position == game_state.fruit_index) {
        game_state.snake_length++;
        pick_new_fruit_index(lcg);
      }

      shift_snake(new_head_position);

      game_counter = 0;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    draw_square(board_offset, window_height, { 1, 1, 1 });

    draw_square(get_cell_offset(game_state.fruit_index), cell_size, { 0.5f, 0.5f, 0.5f });

    for (auto idx = 0; idx < game_state.snake_length; idx++) {
      const auto cell_offset = get_cell_offset(game_state.snake_state[idx]);
      draw_square(cell_offset, cell_size, { 1.0f, 0.0f, 0.0f });
    }

    draw_lattice(board_offset, cell_size);

    check_status(present_frame());

    const auto end_time = get_timer_value();
    const auto elapsed = get_elapsed_millis(timer_frequency, start_time, end_time);

    game_counter += elapsed;
    start_time = end_time;
  }

  return Status_Code::Success;
}

int WinMainCRTStartup () {
  const Status_Code code = game_main();
  ExitProcess(code.error_code);
}

extern "C" int _fltused = 0;
