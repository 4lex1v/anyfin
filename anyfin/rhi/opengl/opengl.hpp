
#pragma once

#include "anyfin/base.hpp"
#include "anyfin/rhi/opengl/opengl_bindings.hpp"

template <typename T> struct Result;

struct String;
struct File_Path;
struct Memory_Region;

enum struct OpenGL_Error_Codes: u32 {
  Success,
  Compilation_Error,
  Linkage_Error,
};

Status_Code check_opengl_errors ();

struct Create_Shader_Configuration {
  struct Shader_Source {
    enum struct Type: u32 { Undef, Literal, File, Binary };
    using enum Type;

    Type  type   = Type::Undef;
    const void *source = nullptr; // It should have been a typed reference, but it's a microsoft's ext. so :shrug:

    fin_forceinline Shader_Source (const String        &value): type { Literal }, source { &value } {}
    fin_forceinline Shader_Source (const File_Path     &value): type { File    }, source { &value } {}
    fin_forceinline Shader_Source (const Memory_Region &value): type { Binary  }, source { &value } {}
  };

  using Type = Shader_Source::Type;
  
  Shader_Source vertex_shader;
  Shader_Source fragment_shader;
};

Result<u32> create_shader (const Create_Shader_Configuration &config);

