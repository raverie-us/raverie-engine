// MIT Licensed (see LICENSE.md).
#pragma once
#include "Foundation/Common/CommonStandard.hpp"

extern "C" {
// GL
typedef char GLchar;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef signed long int GLintptr;
typedef signed long int GLsizeiptr;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef unsigned int GLbitfield;
typedef unsigned int GLenum;
typedef unsigned int GLuint;

#define GL_ALWAYS                                     0x0207
#define GL_ARRAY_BUFFER                               0x8892
#define GL_BACK                                       0x0405
#define GL_BLEND                                      0x0BE2
#define GL_CLAMP_TO_EDGE                              0x812F
#define GL_COLOR_ATTACHMENT0                          0x8CE0
#define GL_COLOR_BUFFER_BIT                           0x00004000
#define GL_COMPARE_REF_TO_TEXTURE                     0x884E
#define GL_COMPILE_STATUS                             0x8B81
#define GL_COMPRESSED_RED_RGTC1                       0x8DBB
#define GL_COMPRESSED_RG_RGTC2                        0x8DBD
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB     0x8E8F
#define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB             0x8E8C
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT              0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT              0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT              0x83F3
#define GL_CULL_FACE                                  0x0B44
#define GL_DECR                                       0x1E03
#define GL_DECR_WRAP                                  0x8508
#define GL_DEPTH_ATTACHMENT                           0x8D00
#define GL_DEPTH_BUFFER_BIT                           0x00000100
#define GL_DEPTH_COMPONENT                            0x1902
#define GL_DEPTH_COMPONENT16                          0x81A5
#define GL_DEPTH_COMPONENT24                          0x81A6
#define GL_DEPTH_COMPONENT32                          0x81A7
#define GL_DEPTH_COMPONENT32F                         0x8CAC
#define GL_DEPTH_STENCIL                              0x84F9
#define GL_DEPTH_STENCIL_ATTACHMENT                   0x821A
#define GL_DEPTH_TEST                                 0x0B71
#define GL_DEPTH24_STENCIL8                           0x88F0
#define GL_DEPTH32F_STENCIL8                          0x8CAD
#define GL_DST_ALPHA                                  0x0304
#define GL_DST_COLOR                                  0x0306
#define GL_ELEMENT_ARRAY_BUFFER                       0x8893
#define GL_EQUAL                                      0x0202
#define GL_EXTENSIONS                                 0x1F03
#define GL_FALSE                                      0
#define GL_FLOAT                                      0x1406
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV             0x8DAD
#define GL_FRAGMENT_SHADER                            0x8B30
#define GL_FRAMEBUFFER                                0x8D40
#define GL_FRAMEBUFFER_COMPLETE                       0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT          0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT  0x8CD7
#define GL_FRAMEBUFFER_UNSUPPORTED                    0x8CDD
#define GL_FRONT                                      0x0404
#define GL_FUNC_ADD                                   0x8006
#define GL_FUNC_REVERSE_SUBTRACT                      0x800B
#define GL_FUNC_SUBTRACT                              0x800A
#define GL_GEOMETRY_SHADER                            0x8DD9
#define GL_GEQUAL                                     0x0206
#define GL_GREATER                                    0x0204
#define GL_HALF_FLOAT                                 0x140B
#define GL_INCR                                       0x1E02
#define GL_INCR_WRAP                                  0x8507
#define GL_INFO_LOG_LENGTH                            0x8B84
#define GL_INVERT                                     0x150A
#define GL_KEEP                                       0x1E00
#define GL_LEQUAL                                     0x0203
#define GL_LESS                                       0x0201
#define GL_LINEAR                                     0x2601
#define GL_LINEAR_MIPMAP_LINEAR                       0x2703
#define GL_LINEAR_MIPMAP_NEAREST                      0x2701
#define GL_LINES                                      0x0001
#define GL_LINK_STATUS                                0x8B82
#define GL_MAX                                        0x8008
#define GL_MAX_COLOR_ATTACHMENTS                      0x8CDF
#define GL_MAX_TEXTURE_IMAGE_UNITS                    0x8872
#define GL_MAX_TEXTURE_MAX_ANISOTROPY                 0x84FF
#define GL_MAX_VERTEX_ATTRIBS                         0x8869
#define GL_MIN                                        0x8007
#define GL_MIRRORED_REPEAT                            0x8370
#define GL_NEAREST                                    0x2600
#define GL_NEAREST_MIPMAP_NEAREST                     0x2700
#define GL_NEVER                                      0x0200
#define GL_NONE                                       0
#define GL_NOTEQUAL                                   0x0205
#define GL_ONE                                        1
#define GL_ONE_MINUS_DST_ALPHA                        0x0305
#define GL_ONE_MINUS_DST_COLOR                        0x0307
#define GL_ONE_MINUS_SRC_ALPHA                        0x0303
#define GL_ONE_MINUS_SRC_COLOR                        0x0301
#define GL_PACK_ALIGNMENT                             0x0D05
#define GL_POINTS                                     0x0000
#define GL_R16I                                       0x8233
#define GL_R16F                                       0x822D
#define GL_R32F                                       0x822E
#define GL_R8                                         0x8229
#define GL_READ_FRAMEBUFFER                           0x8CA8
#define GL_RED                                        0x1903
#define GL_RENDERER                                   0x1F01
#define GL_REPEAT                                     0x2901
#define GL_REPLACE                                    0x1E01
#define GL_RG                                         0x8227
#define GL_RG16I                                      0x8239
#define GL_RG16F                                      0x822F
#define GL_RG32F                                      0x8230
#define GL_RG8                                        0x822B
#define GL_RGB                                        0x1907
#define GL_RGB16I                                     0x8D89
#define GL_RGB16F                                     0x881B
#define GL_RGB32F                                     0x8815
#define GL_RGB8                                       0x8051
#define GL_RGBA                                       0x1908
#define GL_RGBA16I                                    0x8D88
#define GL_RGBA16F                                    0x881A
#define GL_RGBA32F                                    0x8814
#define GL_RGBA8                                      0x8058
#define GL_SCISSOR_TEST                               0x0C11
#define GL_SHADING_LANGUAGE_VERSION                   0x8B8C
#define GL_SRC_ALPHA                                  0x0302
#define GL_SRC_ALPHA_SATURATE                         0x0308
#define GL_SRC_COLOR                                  0x0300
#define GL_SRGB8                                      0x8C41
#define GL_SRGB8_ALPHA8                               0x8C43
#define GL_STATIC_DRAW                                0x88E4
#define GL_STENCIL_BUFFER_BIT                         0x00000400
#define GL_STENCIL_TEST                               0x0B90
#define GL_STREAM_DRAW                                0x88E0
#define GL_TEXTURE_2D                                 0x0DE1
#define GL_TEXTURE_COMPARE_FUNC                       0x884D
#define GL_TEXTURE_COMPARE_MODE                       0x884C
#define GL_TEXTURE_CUBE_MAP                           0x8513
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X                0x8516
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                0x8518
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                0x851A
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X                0x8515
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y                0x8517
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z                0x8519
#define GL_TEXTURE_CUBE_MAP_SEAMLESS                  0x884F
#define GL_TEXTURE_MAG_FILTER                         0x2800
#define GL_TEXTURE_MAX_ANISOTROPY                     0x84FE
#define GL_TEXTURE_MAX_LEVEL                          0x813D
#define GL_TEXTURE_MIN_FILTER                         0x2801
#define GL_TEXTURE_WRAP_S                             0x2802
#define GL_TEXTURE_WRAP_T                             0x2803
#define GL_TEXTURE0                                   0x84C0
#define GL_TRIANGLES                                  0x0004
#define GL_UNPACK_ALIGNMENT                           0x0CF5
#define GL_UNSIGNED_BYTE                              0x1401
#define GL_UNSIGNED_INT                               0x1405
#define GL_UNSIGNED_INT_24_8                          0x84FA
#define GL_UNSIGNED_SHORT                             0x1403
#define GL_VENDOR                                     0x1F00
#define GL_VERTEX_SHADER                              0x8B31
#define GL_ZERO                                       0

void ZeroImportNamed(ImportGlActiveTexture)(GLenum texture);
void ZeroImportNamed(ImportGlAttachShader)(GLuint program, GLuint shader);
void ZeroImportNamed(ImportGlBindAttribLocation)(GLuint program, GLuint index, const GLchar* name);
void ZeroImportNamed(ImportGlBindBuffer)(GLenum target, GLuint buffer);
void ZeroImportNamed(ImportGlBindFramebuffer)(GLenum target, GLuint framebuffer);
void ZeroImportNamed(ImportGlBindSampler)(GLuint unit, GLuint sampler);
void ZeroImportNamed(ImportGlBindTexture)(GLenum target, GLuint texture);
void ZeroImportNamed(ImportGlBindVertexArray)(GLuint array);
void ZeroImportNamed(ImportGlBlendEquation)(GLenum mode);
void ZeroImportNamed(ImportGlBlendEquationi)(GLuint buf, GLenum mode);
void ZeroImportNamed(ImportGlBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
void ZeroImportNamed(ImportGlBlendEquationSeparatei)(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
void ZeroImportNamed(ImportGlBlendFunc)(GLenum sfactor, GLenum dfactor);
void ZeroImportNamed(ImportGlBlendFunci)(GLuint buf, GLenum src, GLenum dst);
void ZeroImportNamed(ImportGlBlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
void ZeroImportNamed(ImportGlBlendFuncSeparatei)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
void ZeroImportNamed(ImportGlBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
void ZeroImportNamed(ImportGlBufferData)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
void ZeroImportNamed(ImportGlBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
GLenum ZeroImportNamed(ImportGlCheckFramebufferStatus)(GLenum target);
void ZeroImportNamed(ImportGlClear)(GLbitfield mask);
void ZeroImportNamed(ImportGlClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ZeroImportNamed(ImportGlClearDepth)(GLfloat d);
void ZeroImportNamed(ImportGlClearStencil)(GLint s);
void ZeroImportNamed(ImportGlCompileShader)(GLuint shader);
void ZeroImportNamed(ImportGlCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei imageSize, const void* data);
GLuint ZeroImportNamed(ImportGlCreateProgram)();
GLuint ZeroImportNamed(ImportGlCreateShader)(GLenum type);
void ZeroImportNamed(ImportGlCullFace)(GLenum mode);
void ZeroImportNamed(ImportGlDeleteBuffer)(GLuint buffer);
void ZeroImportNamed(ImportGlDeleteFramebuffer)(GLuint framebuffer);
void ZeroImportNamed(ImportGlDeleteProgram)(GLuint program);
void ZeroImportNamed(ImportGlDeleteSamplers)(GLsizei count, const GLuint* samplers);
void ZeroImportNamed(ImportGlDeleteShader)(GLuint shader);
void ZeroImportNamed(ImportGlDeleteTexture)(GLuint texture);
void ZeroImportNamed(ImportGlDeleteVertexArray)(GLuint array);
void ZeroImportNamed(ImportGlDepthFunc)(GLenum func);
void ZeroImportNamed(ImportGlDepthMask)(GLboolean flag);
void ZeroImportNamed(ImportGlDetachShader)(GLuint program, GLuint shader);
void ZeroImportNamed(ImportGlDisable)(GLenum cap);
void ZeroImportNamed(ImportGlDisablei)(GLenum target, GLuint index);
void ZeroImportNamed(ImportGlDrawArrays)(GLenum mode, GLint first, GLsizei count);
void ZeroImportNamed(ImportGlDrawBuffers)(GLsizei n, const GLenum* bufs);
void ZeroImportNamed(ImportGlDrawElements)(GLenum mode, GLsizei count, GLenum type, const void* indices);
void ZeroImportNamed(ImportGlEnable)(GLenum cap);
void ZeroImportNamed(ImportGlEnablei)(GLenum target, GLuint index);
void ZeroImportNamed(ImportGlEnableVertexAttribArray)(GLuint index);
void ZeroImportNamed(ImportGlFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GLuint ZeroImportNamed(ImportGlGenBuffer)();
void ZeroImportNamed(ImportGlGenerateMipmap)(GLenum target);
GLuint ZeroImportNamed(ImportGlGenFramebuffer)();
void ZeroImportNamed(ImportGlGenSamplers)(GLsizei count, GLuint* samplers);
GLuint ZeroImportNamed(ImportGlGenTexture)();
GLuint ZeroImportNamed(ImportGlGenVertexArray)();
void ZeroImportNamed(ImportGlGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
void ZeroImportNamed(ImportGlGetProgramiv)(GLuint program, GLenum pname, GLint* params);
void ZeroImportNamed(ImportGlGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
void ZeroImportNamed(ImportGlGetShaderiv)(GLuint shader, GLenum pname, GLint* params);
GLint ZeroImportNamed(ImportGlGetUniformLocation)(GLuint program, const GLchar* name);
void ZeroImportNamed(ImportGlLineWidth)(GLfloat width);
void ZeroImportNamed(ImportGlLinkProgram)(GLuint program);
void ZeroImportNamed(ImportGlPixelStorei)(GLenum pname, GLint param);
void ZeroImportNamed(ImportGlReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
void ZeroImportNamed(ImportGlSamplerParameteri)(GLuint sampler, GLenum pname, GLint param);
void ZeroImportNamed(ImportGlScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
void ZeroImportNamed(ImportGlShaderSource)(GLuint shader, const GLchar* string, GLint length);
void ZeroImportNamed(ImportGlStencilFunc)(GLenum func, GLint ref, GLuint mask);
void ZeroImportNamed(ImportGlStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
void ZeroImportNamed(ImportGlStencilMask)(GLuint mask);
void ZeroImportNamed(ImportGlStencilMaskSeparate)(GLenum face, GLuint mask);
void ZeroImportNamed(ImportGlStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
void ZeroImportNamed(ImportGlStencilOpSeparate)(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass);
void ZeroImportNamed(ImportGlTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
void ZeroImportNamed(ImportGlTexParameterf)(GLenum target, GLenum pname, GLfloat param);
void ZeroImportNamed(ImportGlTexParameteri)(GLenum target, GLenum pname, GLint param);
void ZeroImportNamed(ImportGlTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
void ZeroImportNamed(ImportGlUniform1fv)(GLint location, GLsizei count, const GLfloat* value);
void ZeroImportNamed(ImportGlUniform1iv)(GLint location, GLsizei count, const GLint* value);
void ZeroImportNamed(ImportGlUniform2fv)(GLint location, GLsizei count, const GLfloat* value);
void ZeroImportNamed(ImportGlUniform2iv)(GLint location, GLsizei count, const GLint* value);
void ZeroImportNamed(ImportGlUniform3fv)(GLint location, GLsizei count, const GLfloat* value);
void ZeroImportNamed(ImportGlUniform3iv)(GLint location, GLsizei count, const GLint* value);
void ZeroImportNamed(ImportGlUniform4fv)(GLint location, GLsizei count, const GLfloat* value);
void ZeroImportNamed(ImportGlUniform4iv)(GLint location, GLsizei count, const GLint* value);
void ZeroImportNamed(ImportGlUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void ZeroImportNamed(ImportGlUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void ZeroImportNamed(ImportGlUseProgram)(GLuint program);
void ZeroImportNamed(ImportGlVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
void ZeroImportNamed(ImportGlVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
void ZeroImportNamed(ImportGlViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

// Platform
void ZeroImportNamed(ImportPrintLine)(int32_t fd, const char* str, size_t length);
double ZeroImportNamed(ImportClock)(int32_t clockId);
void ZeroImportNamed(ImportYield)();
void ZeroImportNamed(ImportMouseTrap)(bool value);
void ZeroImportNamed(ImportMouseSetCursor)(Zero::Cursor::Enum cursor);
//void (*CaptureMouse)(bool capture);

char* ZeroExportNamed(ExportInitialize)(size_t argumentsLength);
void ZeroExportNamed(ExportRunIteration)();
void ZeroExportNamed(ExportHandleCrash)();
void ZeroExportNamed(ExportMouseMove)(int32_t x, int32_t y, int32_t dx, int32_t dy);
void ZeroExportNamed(ExportKeyDown)(Zero::Keys::Enum key, uint osKey, bool repeated);
void ZeroExportNamed(ExportQuit)();

}
