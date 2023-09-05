import wasmUrl from "../Build/Active/Code/Editor/RaverieEditor/RaverieEditor.wasm?url";
import {WASI, init} from "@wasmer/wasi";

const modulePromise = WebAssembly.compileStreaming(fetch(wasmUrl));
const initPromise = init();

const [module] = await Promise.all([modulePromise, initPromise]);

const wasi = new WASI({
  env: {
      // 'ENVVAR1': '1',
      // 'ENVVAR2': '2'
  },
  args: [
      // 'command', 'arg1', 'arg2'
  ],
});

console.log("instantiating...");

type GLfloat = number;
type GLint = number;
type GLsizei = number;
type GLintptr = number;
type GLsizeiptr = number;
type GLboolean = boolean;
type GLbitfield = number;
type GLenum = number;
type GLuint = number;

type VoidPointer = number;
type GLcharPointer = number;
type GLcharPointerPointer = number;
type GLubytePointer = number;
type GLuintPointer = number;
type GLintPointer = number;
type GLenumPointer = number;
type GLfloatPointer = number;
type GLsizeiPointer = number;

const imports: WebAssembly.Imports = {
  ...wasi.getImports(module),
  env: {
    ImportGlActiveTexture: (texture: GLenum): void => {},
    ImportGlAttachShader: (program: GLuint, shader: GLuint): void => {},
    ImportGlBindAttribLocation: (program: GLuint, index: GLuint, name: GLcharPointer): void => {},
    ImportGlBindBuffer: (target: GLenum, buffer: GLuint): void => {},
    ImportGlBindFramebuffer: (target: GLenum, framebuffer: GLuint): void => {},
    ImportGlBindSampler: (unit: GLuint, sampler: GLuint): void => {},
    ImportGlBindTexture: (target: GLenum, texture: GLuint): void => {},
    ImportGlBindVertexArray: (array: GLuint): void => {},
    ImportGlBlendEquation: (mode: GLenum): void => {},
    ImportGlBlendEquationi: (buf: GLuint, mode: GLenum): void => {},
    ImportGlBlendEquationSeparate: (modeRGB: GLenum, modeAlpha: GLenum): void => {},
    ImportGlBlendEquationSeparatei: (buf: GLuint, modeRGB: GLenum, modeAlpha: GLenum): void => {},
    ImportGlBlendFunc: (sfactor: GLenum, dfactor: GLenum): void => {},
    ImportGlBlendFunci: (buf: GLuint, src: GLenum, dst: GLenum): void => {},
    ImportGlBlendFuncSeparate: (sfactorRGB: GLenum, dfactorRGB: GLenum, sfactorAlpha: GLenum, dfactorAlpha: GLenum): void => {},
    ImportGlBlendFuncSeparatei: (buf: GLuint, srcRGB: GLenum, dstRGB: GLenum, srcAlpha: GLenum, dstAlpha: GLenum): void => {},
    ImportGlBlitFramebuffer: (srcX0: GLint, srcY0: GLint, srcX1: GLint, srcY1: GLint, dstX0: GLint, dstY0: GLint, dstX1: GLint, dstY1: GLint, mask: GLbitfield, filter: GLenum): void => {},
    ImportGlBufferData: (target: GLenum, size: GLsizeiptr, data: VoidPointer, usage: GLenum): void => {},
    ImportGlBufferSubData: (target: GLenum, offset: GLintptr, size: GLsizeiptr, data: VoidPointer): void => {},
    ImportGlCheckFramebufferStatus: (target: GLenum): GLenum => { return 0; },
    ImportGlClear: (mask: GLbitfield): void => {},
    ImportGlClearColor: (red: GLfloat, green: GLfloat, blue: GLfloat, alpha: GLfloat): void => {},
    ImportGlClearDepth: (d: GLfloat): void => {},
    ImportGlClearStencil: (s: GLint): void => {},
    ImportGlCompileShader: (shader: GLuint): void => {},
    ImportGlCompressedTexImage2D: (target: GLenum, level: GLint, internalformat: GLenum, width: GLsizei, height: GLsizei, border: GLint, imageSize: GLsizei, data: VoidPointer): void => {},
    ImportGlCreateProgram: (): GLuint => { return 0; },
    ImportGlCreateShader: (type: GLenum): GLuint => { return 0; },
    ImportGlCullFace: (mode: GLenum): void => {},
    ImportGlDeleteBuffers: (n: GLsizei, buffers: GLuintPointer): void => {},
    ImportGlDeleteFramebuffers: (n: GLsizei, framebuffers: GLuintPointer): void => {},
    ImportGlDeleteProgram: (program: GLuint): void => {},
    ImportGlDeleteSamplers: (count: GLsizei, samplers: GLuintPointer): void => {},
    ImportGlDeleteShader: (shader: GLuint): void => {},
    ImportGlDeleteTextures: (n: GLsizei, textures: GLuintPointer): void => {},
    ImportGlDeleteVertexArrays: (n: GLsizei, arrays: GLuintPointer): void => {},
    ImportGlDepthFunc: (func: GLenum): void => {},
    ImportGlDepthMask: (flag: GLboolean): void => {},
    ImportGlDetachShader: (program: GLuint, shader: GLuint): void => {},
    ImportGlDisable: (cap: GLenum): void => {},
    ImportGlDisablei: (target: GLenum, index: GLuint): void => {},
    ImportGlDrawArrays: (mode: GLenum, first: GLint, count: GLsizei): void => {},
    ImportGlDrawBuffers: (n: GLsizei, bufs: GLenumPointer): void => {},
    ImportGlDrawElements: (mode: GLenum, count: GLsizei, type: GLenum, indices: VoidPointer): void => {},
    ImportGlEnable: (cap: GLenum): void => {},
    ImportGlEnablei: (target: GLenum, index: GLuint): void => {},
    ImportGlEnableVertexAttribArray: (index: GLuint): void => {},
    ImportGlFramebufferTexture2D: (target: GLenum, attachment: GLenum, textarget: GLenum, texture: GLuint, level: GLint): void => {},
    ImportGlGenBuffers: (n: GLsizei, buffers: GLuintPointer): void => {},
    ImportGlGenerateMipmap: (target: GLenum): void => {},
    ImportGlGenFramebuffers: (n: GLsizei, framebuffers: GLuintPointer): void => {},
    ImportGlGenSamplers: (count: GLsizei, samplers: GLuintPointer): void => {},
    ImportGlGenTextures: (n: GLsizei, textures: GLuintPointer): void => {},
    ImportGlGenVertexArrays: (n: GLsizei, arrays: GLuintPointer): void => {},
    ImportGlGetFloatv: (pname: GLenum, data: GLfloatPointer): void => {},
    ImportGlGetIntegerv: (pname: GLenum, data: GLintPointer): void => {},
    ImportGlGetProgramInfoLog: (program: GLuint, bufSize: GLsizei, length: GLsizeiPointer, infoLog: GLcharPointer): void => {},
    ImportGlGetProgramiv: (program: GLuint, pname: GLenum, params: GLintPointer): void => {},
    ImportGlGetShaderInfoLog: (shader: GLuint, bufSize: GLsizei, length: GLsizeiPointer, infoLog: GLcharPointer): void => {},
    ImportGlGetShaderiv: (shader: GLuint, pname: GLenum, params: GLintPointer): void => {},
    ImportGlGetString: (name: GLenum): GLubytePointer => { return 0; },
    ImportGlGetUniformLocation: (program: GLuint, name: GLcharPointer): GLint => { return 0; },
    ImportGlLineWidth: (width: GLfloat): void => {},
    ImportGlLinkProgram: (program: GLuint): void => {},
    ImportGlPixelStorei: (pname: GLenum, param: GLint): void => {},
    ImportGlReadPixels: (x: GLint, y: GLint, width: GLsizei, height: GLsizei, format: GLenum, type: GLenum, pixels: VoidPointer): void => {},
    ImportGlSamplerParameteri: (sampler: GLuint, pname: GLenum, param: GLint): void => {},
    ImportGlScissor: (x: GLint, y: GLint, width: GLsizei, height: GLsizei): void => {},
    ImportGlShaderSource: (shader: GLuint, count: GLsizei, string: GLcharPointerPointer, length: GLintPointer): void => {},
    ImportGlStencilFunc: (func: GLenum, ref: GLint, mask: GLuint): void => {},
    ImportGlStencilFuncSeparate: (face: GLenum, func: GLenum, ref: GLint, mask: GLuint): void => {},
    ImportGlStencilMask: (mask: GLuint): void => {},
    ImportGlStencilMaskSeparate: (face: GLenum, mask: GLuint): void => {},
    ImportGlStencilOp: (fail: GLenum, zfail: GLenum, zpass: GLenum): void => {},
    ImportGlStencilOpSeparate: (face: GLenum, sfail: GLenum, dpfail: GLenum, dppass: GLenum): void => {},
    ImportGlTexImage2D: (target: GLenum, level: GLint, internalformat: GLint, width: GLsizei, height: GLsizei, border: GLint, format: GLenum, type: GLenum, pixels: VoidPointer): void => {},
    ImportGlTexParameterf: (target: GLenum, pname: GLenum, param: GLfloat): void => {},
    ImportGlTexParameteri: (target: GLenum, pname: GLenum, param: GLint): void => {},
    ImportGlTexSubImage2D: (target: GLenum, level: GLint, xoffset: GLint, yoffset: GLint, width: GLsizei, height: GLsizei, format: GLenum, type: GLenum, pixels: VoidPointer): void => {},
    ImportGlUniform1fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => {},
    ImportGlUniform1iv: (location: GLint, count: GLsizei, value: GLintPointer): void => {},
    ImportGlUniform2fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => {},
    ImportGlUniform2iv: (location: GLint, count: GLsizei, value: GLintPointer): void => {},
    ImportGlUniform3fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => {},
    ImportGlUniform3iv: (location: GLint, count: GLsizei, value: GLintPointer): void => {},
    ImportGlUniform4fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => {},
    ImportGlUniform4iv: (location: GLint, count: GLsizei, value: GLintPointer): void => {},
    ImportGlUniformMatrix3fv: (location: GLint, count: GLsizei, transpose: GLboolean, value: GLfloatPointer): void => {},
    ImportGlUniformMatrix4fv: (location: GLint, count: GLsizei, transpose: GLboolean, value: GLfloatPointer): void => {},
    ImportGlUseProgram: (program: GLuint): void => {},
    ImportGlVertexAttribIPointer: (index: GLuint, size: GLint, type: GLenum, stride: GLsizei, pointer: VoidPointer): void => {},
    ImportGlVertexAttribPointer: (index: GLuint, size: GLint, type: GLenum, normalized: GLboolean, stride: GLsizei, pointer: VoidPointer): void => {},
    ImportGlViewport: (x: GLint, y: GLint, width: GLsizei, height: GLsizei): void => {},
  }
};

const instance = await WebAssembly.instantiate(module, imports);
try {
  wasi.start(instance);
} catch {
}
const stdout = wasi.getStdoutString();

console.log("stdout", stdout);
