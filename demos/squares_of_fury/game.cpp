
#include "anyfin/base.hpp"

#include "anyfin/core/prelude.hpp"
#include "anyfin/core/status_code.hpp"
#include "anyfin/core/result.hpp"
#include "anyfin/core/math.hpp"
#include "anyfin/core/strings.hpp"

#include "anyfin/window/window.hpp"
#include "anyfin/window/events.hpp"

#include "anyfin/platform/platform.hpp"

#include "anyfin/rhi/opengl/opengl.hpp"

static const usize window_width  = 1024;
static const usize window_height = 720;

static String simple_vertex_shader_code = R"(
#version 460 core

layout (location = 0) in vec2 in_pos;

uniform mat3 projection;

void main () {
  vec2 position = (projection * vec3(in_pos, 1.0)).xy;
  gl_Position = vec4(position, 0.0, 1.0);
}
)";

static String simple_fragment_shader_code = R"(
#version 460 core

//in vec3 color;

out vec4 frag_color;

void main () {
  frag_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)";

static f32 projection[9];

static void draw_square (const Vec2<f32> position, const f32 size) {
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

  //glBufferSubData(GL_ARRAY_BUFFER, 0, positions_size, vertices_positions);
  //glBufferSubData(GL_ARRAY_BUFFER, positions_size, colors_size, vertices_colors);

  GLuint attributes_array;
  glGenVertexArrays(1, &attributes_array);
  glBindVertexArray(attributes_array);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        
  // glEnableVertexAttribArray(1);
  // glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) (positions_size));

  GLuint current_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&current_program));

  // TODO: #efficiency Is this the way to do this? Any way I can upload uniforms to GPU and reuse/bake into the shader?
  // glUniform2ui(glGetUniformLocation(render_device.shaders.draw_filled_mesh, "pos_offset"), command->offset.x, command->offset.y);
  glUniformMatrix3fv(glGetUniformLocation(current_program, "projection"), 1, GL_TRUE, projection);

  glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_count));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glDeleteBuffers(1, &array_buffer);
  glDeleteVertexArrays(1, &attributes_array);
}

static Status_Code game_main () {
  use(Status_Code);
  
  create_window_system("Squares of Fury", window_width, window_height);

  auto simple_square_shader = create_shader({
    .vertex_shader   = simple_vertex_shader_code,
    .fragment_shader = simple_fragment_shader_code,
  });
  check_status(simple_square_shader);

  glViewport(0, 0, window_width, window_height);

  glUseProgram(simple_square_shader);
  check_status(check_opengl_errors());

  {
    projection[0] = 2.0f / window_width;
    projection[2] = -1;
    projection[4] = -2.0f / window_height;
    projection[5] = 1;
  }

  Vec2<f32> position { 50, 50 };
  const f32 speed = 10.5f;

  bool quit_requested = false;
  while (!quit_requested) {
    System_Event *events;
    usize events_count;
    pump_window_events(&events, &events_count);

    for (usize idx = 0; idx < events_count; idx++) {
      switch (events[idx].tag) {
        case System_Event::Quit: {
          quit_requested = true;
          break;
        }
        case System_Event::Keyboard: {
          const auto &keyboard = events[idx].keyboard;

          if      (keyboard.key_code == 0x25) position += { -speed, 0.0f };
          else if (keyboard.key_code == 0x26) position += { 0.0f, -speed };
          else if (keyboard.key_code == 0x27) position += { speed, 0.0f };
          else if (keyboard.key_code == 0x28) position += { 0.0f, speed };

          break;
        }
        default: {}
      }
    }

    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    draw_square(position, 100);
    
    check_status(present_frame());
  }

  return Success;
}

int WinMainCRTStartup () {
#ifdef DEV_TOOLS_ENABLED
  attach_console();
#endif

  auto status = game_main();
  ExitProcess(static_cast<int>(status.error_code));
}

#ifdef PLATFORM_WIN32
extern "C" int _fltused = 0;
#endif
