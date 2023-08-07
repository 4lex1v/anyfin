
#pragma once

#include "anyfin/base.hpp"

#ifdef PLATFORM_WIN32
  #include "anyfin/platform/win32/common_win32.hpp"
  #include <wingdi.h>
  #include <gl/GL.h>
#endif

typedef char           GLchar;
typedef ptrdiff_t      GLsizeiptr;
typedef ptrdiff_t      GLintptr;

enum {
  GL_VERTEX_SHADER            = 0x8B31,
  GL_COMPILE_STATUS           = 0x8B81,
  GL_FRAGMENT_SHADER          = 0x8B30,
  GL_LINK_STATUS              = 0x8B82,
  GL_DEBUG_OUTPUT             = 0x92E0,
  GL_DEBUG_OUTPUT_SYNCHRONOUS = 0x8242,
  GL_ARRAY_BUFFER             = 0x8892,
  GL_STATIC_DRAW              = 0x88E4,
  GL_DEBUG_TYPE_ERROR         = 0x824C,
  GL_R8                       = 0x8229,
  GL_CLAMP_TO_EDGE            = 0x812F, 
  GL_TEXTURE0                 = 0x84C0,
  GL_CONTEXT_FLAGS            = 0x821E,

  GL_CONTEXT_FLAG_DEBUG_BIT = 0x00000002,
};

#ifdef OPENGL_LOADER
  #define declare_opengl_proc(NAME) type_##NAME *NAME
#else
  #define declare_opengl_proc(NAME) extern type_##NAME *NAME
#endif

typedef GLboolean type_glIsEnabled (GLenum cap);
typedef GLint type_glGetAttribLocation (GLuint program, const GLchar * name);
typedef GLint type_glGetUniformLocation (GLuint program, const GLchar * name);
typedef GLuint type_glCreateProgram ();
typedef GLuint type_glCreateShader (GLenum type);
typedef const GLubyte *type_glGetString (GLenum name);
typedef const GLubyte *type_glGetStringi (GLenum name, GLuint index);
typedef void (*gl_debug_proc_type_t)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam);
typedef void type_glActiveTexture (GLenum texture);
typedef void type_glAttachShader (GLuint program, GLuint shader);
typedef void type_glBindBuffer (GLenum target, GLuint buffer);
typedef void type_glBindSampler (GLuint unit, GLuint sampler);
typedef void type_glBindTexture (GLenum target, GLuint texture);
typedef void type_glBindVertexArray (GLuint array);
typedef void type_glBlendEquation (GLenum mode);
typedef void type_glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha);
typedef void type_glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void type_glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void type_glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data);
typedef void type_glCompileShader (GLuint shader);

typedef void type_glDebugMessageCallback (gl_debug_proc_type_t callback, const void * userParam);
declare_opengl_proc(glDebugMessageCallback);

typedef void type_glDebugMessageControl (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
declare_opengl_proc(glDebugMessageControl);

typedef void type_glDeleteBuffers (GLsizei n, const GLuint *buffers);
typedef void type_glDeleteProgram (GLuint program);
typedef void type_glDeleteShader (GLuint shader);
typedef void type_glDeleteTextures (GLsizei n, const GLuint *textures);
typedef void type_glDeleteVertexArrays (GLsizei n, const GLuint *arrays);
typedef void type_glDetachShader (GLuint program, GLuint shader);
typedef void type_glDisable (GLenum cap);
typedef void type_glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void type_glDrawElementsBaseVertex (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
typedef void type_glEnable (GLenum cap);
typedef void type_glEnableVertexAttribArray (GLuint index);
typedef void type_glGenBuffers (GLsizei n, GLuint *buffers);
typedef void type_glGenTextures (GLsizei n, GLuint *textures);
typedef void type_glGenVertexArrays (GLsizei n, GLuint *arrays);
typedef void type_glGetIntegerv (GLenum pname, GLint *data);
typedef void type_glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void type_glGetProgramiv (GLuint program, GLenum pname, GLint *params);
typedef void type_glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void type_glGetShaderInfoLog (GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void type_glGetShaderiv (GLuint shader, GLenum pname, GLint *params);
typedef void type_glLinkProgram (GLuint program);
typedef void type_glPixelStorei (GLenum pname, GLint param);
typedef void type_glPolygonMode (GLenum face, GLenum mode);
typedef void type_glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
typedef void type_glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void type_glShaderSource (GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void type_glTexParameteri (GLenum target, GLenum pname, GLint param);

typedef void type_glUniform1i (GLint location, GLint v0);
declare_opengl_proc(glUniform1i);

typedef void type_glUniform2ui(GLint location, GLuint v0, GLuint v1);
declare_opengl_proc(glUniform2ui);

typedef void type_glUniform2uiv(GLint location, GLsizei count, const GLuint *value);
declare_opengl_proc(glUniform2uiv);

typedef void type_glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
declare_opengl_proc(glUniform3f);

typedef void type_glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
declare_opengl_proc(glUniform4f);

typedef void type_glUniform4fv (GLint location, GLsizei count, const GLfloat * value);
declare_opengl_proc(glUniform4fv);

typedef void type_glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
declare_opengl_proc(glUniformMatrix2fv);

typedef void type_glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
declare_opengl_proc(glUniformMatrix3fv);

typedef void type_glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
declare_opengl_proc(glUniformMatrix4fv);

typedef void type_glUseProgram (GLuint program);
declare_opengl_proc(glUseProgram);

typedef void type_glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
declare_opengl_proc(glVertexAttribPointer);

typedef void type_glTexStorage2D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

//typedef void type_glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * data);

declare_opengl_proc(glTexStorage2D);
declare_opengl_proc(glActiveTexture);
declare_opengl_proc(glAttachShader);
declare_opengl_proc(glBindBuffer);
declare_opengl_proc(glBindVertexArray);
declare_opengl_proc(glBufferData);
declare_opengl_proc(glBufferSubData);
declare_opengl_proc(glCompileShader);
declare_opengl_proc(glCreateProgram);
declare_opengl_proc(glCreateShader);
declare_opengl_proc(glDeleteBuffers);
declare_opengl_proc(glDeleteProgram);
declare_opengl_proc(glDeleteShader);
declare_opengl_proc(glDeleteVertexArrays);
declare_opengl_proc(glEnableVertexAttribArray);
declare_opengl_proc(glGenBuffers);
declare_opengl_proc(glGenVertexArrays);
declare_opengl_proc(glGetProgramiv);
declare_opengl_proc(glGetShaderInfoLog);
declare_opengl_proc(glGetShaderiv);
declare_opengl_proc(glGetUniformLocation);
declare_opengl_proc(glLinkProgram);
declare_opengl_proc(glShaderSource);

// typedef void type_glTextureParameteri (GLuint texture, GLenum pname, GLint param);
// declare_opengl_proc(glTextureParameteri);

// typedef void type_glTextureStorage2D (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
// declare_opengl_proc(glTextureStorage2D);

// declare_opengl_proc(glTextureSubImage2D);

typedef void type_glCreateBuffers (GLsizei n, GLuint *buffers);
declare_opengl_proc(glCreateBuffers);

typedef void type_glNamedBufferStorage (GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags);
declare_opengl_proc(glNamedBufferStorage);

// typedef void type_glNamedBufferData (GLuint buffer, GLsizeiptr size, const void *data, GLenum usage);
// declare_opengl_proc(glNamedBufferData);

// typedef void type_glCreateVertexArrays (GLsizei n, GLuint *arrays);
// declare_opengl_proc(glCreateVertexArrays);

// typedef void type_glEnableVertexArrayAttrib (GLuint vaobj, GLuint index);
// declare_opengl_proc(glEnableVertexArrayAttrib);

// typedef void type_glVertexArrayVertexBuffer (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
// declare_opengl_proc(glVertexArrayVertexBuffer);

// typedef void type_glVertexArrayAttribBinding (GLuint vaobj, GLuint attribindex, GLuint bindingindex);
// declare_opengl_proc(glVertexArrayAttribBinding);

// typedef void type_glVertexArrayAttribFormat (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
// declare_opengl_proc(glVertexArrayAttribFormat);

// typedef void type_glVertexArrayBindingDivisor (GLuint vaobj, GLuint bindingindex, GLuint divisor);
// declare_opengl_proc(glVertexArrayBindingDivisor);
