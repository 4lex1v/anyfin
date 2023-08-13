
#include "anyfin/rhi/opengl/opengl.hpp"

#include "anyfin/core/strings.hpp"
#include "anyfin/core/result.hpp"
#include "anyfin/core/prelude.hpp"

#include "anyfin/platform/platform.hpp"

static const char *errStrings [] {
  "GL_INVALID_ENUM",
  "GL_INVALID_VALUE",
  "GL_INVALID_OPERATION",
  "GL_STACK_OVERFLOAT",
  "GL_STACK_UNDERFLOW",
  "GL_OUT_OF_MEMORY",
  "GL_INVALID_FRAMEBUFFER_OPERATION"
};

Status_Code check_opengl_errors () {
  GLenum e;
  while ((e = glGetError()) != GL_NO_ERROR) {
    const int errOfs = e - GL_INVALID_ENUM;
    return Status_Code::Runtime_Error;
  }

  return Status_Code::Success;
}

static Result<u32> compile_shader (const String &code, const GLenum shader_type) {
  const GLuint shader_id = glCreateShader(shader_type);
  glShaderSource(shader_id, 1, (const GLchar **)&code.value, reinterpret_cast<const GLint *>(&code.length));
  check_status(check_opengl_errors());

  glCompileShader(shader_id);
  check_status(check_opengl_errors());

  int success;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
  check_status(check_opengl_errors());

  if (!success) {
#ifdef DEV_TOOLS_ENABLED
    enum { max_info_log_size = 512 };
    GLsizei info_log_size = 0;
    char info_log[max_info_log_size];
    glGetShaderInfoLog(shader_id, max_info_log_size, &info_log_size, info_log);

    // TODO: This is unsafe and should be changes with a proper logging solution.
    send_bytes_to_stdout(Memory_Region{reinterpret_cast<u8 *>(info_log), static_cast<u32>(info_log_size)});
#endif

    return Status_Code {
      Status_Code::Runtime_Error,
      static_cast<u32>(OpenGL_Error_Codes::Compilation_Error)
    };
  }

  return shader_id;
}

using Shader_Source = Create_Shader_Configuration::Shader_Source;

static Status_Code load_shader (const u32 shader_program_id, const Shader_Source &source, const GLenum shader_type) {
  switch (source.type) {
    case Shader_Source::Type::Undef: return Status_Code::Success;
    case Shader_Source::Type::Literal: {
      const auto &code = *reinterpret_cast<const String *>(source.source);
      auto [compile_status, id] = compile_shader(code, shader_type);
      check_status(compile_status);

      glAttachShader(shader_program_id, id);
      check_status(check_opengl_errors());

      return Status_Code::Success;
    }
    case Shader_Source::Type::File: {
      return Status_Code::Success;
    }
    case Shader_Source::Type::Binary: {
      return Status_Code::Success;
    }
  }
}

Result<u32> create_shader (const Create_Shader_Configuration &config) {
  GLuint program_id = glCreateProgram();
  
  check_status(load_shader(program_id, config.vertex_shader,   GL_VERTEX_SHADER));
  check_status(load_shader(program_id, config.fragment_shader, GL_FRAGMENT_SHADER));

  glLinkProgram(program_id);
  check_status(check_opengl_errors());

  int status;
  glGetProgramiv(program_id, GL_LINK_STATUS, &status);
  check_status(check_opengl_errors());
  if (!status) {
#ifdef DEV_TOOLS_ENABLED
    enum { max_info_log_size = 512 };
    GLsizei info_log_size = 0;
    char info_log[max_info_log_size];
    glGetProgramInfoLog(program_id, max_info_log_size, &info_log_size, info_log);

    // TODO: This is unsafe and should be changes with a proper logging solution.
    send_bytes_to_stdout(Memory_Region{reinterpret_cast<u8 *>(info_log), static_cast<u32>(info_log_size)});
#endif

    return Status_Code { Status_Code::Runtime_Error };
  }

  return program_id;
}

