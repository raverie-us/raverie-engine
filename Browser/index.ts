import wasmUrl from "../Build/Active/Code/Editor/RaverieEditor/RaverieEditor.wasm?url";

const module = await WebAssembly.compileStreaming(fetch(wasmUrl));

// GL
type GLfloat = number;
type GLint = number;
type GLsizei = number;
type GLintptr = number;
type GLsizeiptr = number;
type GLboolean = boolean;
type GLbitfield = number;
type GLenum = number;
type GLuint = number;

// GL Pointers
type VoidPointer = number;
type GLcharPointer = number;
type GLcharPointerPointer = number;
type GLubytePointer = number;
type GLuintPointer = number;
type GLintPointer = number;
type GLenumPointer = number;
type GLfloatPointer = number;
type GLsizeiPointer = number;

// Platform
type CharPointer = number;

const decoder = new TextDecoder();

const imports: WebAssembly.Imports = {
  env: {
    // GL
    ImportGlActiveTexture: (texture: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlAttachShader: (program: GLuint, shader: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlBindAttribLocation: (program: GLuint, index: GLuint, name: GLcharPointer): void => { throw new Error("Not implemented"); },
    ImportGlBindBuffer: (target: GLenum, buffer: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlBindFramebuffer: (target: GLenum, framebuffer: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlBindSampler: (unit: GLuint, sampler: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlBindTexture: (target: GLenum, texture: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlBindVertexArray: (array: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlBlendEquation: (mode: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlBlendEquationi: (buf: GLuint, mode: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlBlendEquationSeparate: (modeRGB: GLenum, modeAlpha: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlBlendEquationSeparatei: (buf: GLuint, modeRGB: GLenum, modeAlpha: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlBlendFunc: (sfactor: GLenum, dfactor: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlBlendFunci: (buf: GLuint, src: GLenum, dst: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlBlendFuncSeparate: (sfactorRGB: GLenum, dfactorRGB: GLenum, sfactorAlpha: GLenum, dfactorAlpha: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlBlendFuncSeparatei: (buf: GLuint, srcRGB: GLenum, dstRGB: GLenum, srcAlpha: GLenum, dstAlpha: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlBlitFramebuffer: (srcX0: GLint, srcY0: GLint, srcX1: GLint, srcY1: GLint, dstX0: GLint, dstY0: GLint, dstX1: GLint, dstY1: GLint, mask: GLbitfield, filter: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlBufferData: (target: GLenum, size: GLsizeiptr, data: VoidPointer, usage: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlBufferSubData: (target: GLenum, offset: GLintptr, size: GLsizeiptr, data: VoidPointer): void => { throw new Error("Not implemented"); },
    ImportGlCheckFramebufferStatus: (target: GLenum): GLenum => { throw new Error("Not implemented"); },
    ImportGlClear: (mask: GLbitfield): void => { throw new Error("Not implemented"); },
    ImportGlClearColor: (red: GLfloat, green: GLfloat, blue: GLfloat, alpha: GLfloat): void => { throw new Error("Not implemented"); },
    ImportGlClearDepth: (d: GLfloat): void => { throw new Error("Not implemented"); },
    ImportGlClearStencil: (s: GLint): void => { throw new Error("Not implemented"); },
    ImportGlCompileShader: (shader: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlCompressedTexImage2D: (target: GLenum, level: GLint, internalformat: GLenum, width: GLsizei, height: GLsizei, border: GLint, imageSize: GLsizei, data: VoidPointer): void => { throw new Error("Not implemented"); },
    ImportGlCreateProgram: (): GLuint => { throw new Error("Not implemented"); },
    ImportGlCreateShader: (type: GLenum): GLuint => { throw new Error("Not implemented"); },
    ImportGlCullFace: (mode: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlDeleteBuffers: (n: GLsizei, buffers: GLuintPointer): void => { throw new Error("Not implemented"); },
    ImportGlDeleteFramebuffers: (n: GLsizei, framebuffers: GLuintPointer): void => { throw new Error("Not implemented"); },
    ImportGlDeleteProgram: (program: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlDeleteSamplers: (count: GLsizei, samplers: GLuintPointer): void => { throw new Error("Not implemented"); },
    ImportGlDeleteShader: (shader: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlDeleteTextures: (n: GLsizei, textures: GLuintPointer): void => { throw new Error("Not implemented"); },
    ImportGlDeleteVertexArrays: (n: GLsizei, arrays: GLuintPointer): void => { throw new Error("Not implemented"); },
    ImportGlDepthFunc: (func: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlDepthMask: (flag: GLboolean): void => { throw new Error("Not implemented"); },
    ImportGlDetachShader: (program: GLuint, shader: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlDisable: (cap: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlDisablei: (target: GLenum, index: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlDrawArrays: (mode: GLenum, first: GLint, count: GLsizei): void => { throw new Error("Not implemented"); },
    ImportGlDrawBuffers: (n: GLsizei, bufs: GLenumPointer): void => { throw new Error("Not implemented"); },
    ImportGlDrawElements: (mode: GLenum, count: GLsizei, type: GLenum, indices: VoidPointer): void => { throw new Error("Not implemented"); },
    ImportGlEnable: (cap: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlEnablei: (target: GLenum, index: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlEnableVertexAttribArray: (index: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlFramebufferTexture2D: (target: GLenum, attachment: GLenum, textarget: GLenum, texture: GLuint, level: GLint): void => { throw new Error("Not implemented"); },
    ImportGlGenBuffers: (n: GLsizei, buffers: GLuintPointer): void => { throw new Error("Not implemented"); },
    ImportGlGenerateMipmap: (target: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlGenFramebuffers: (n: GLsizei, framebuffers: GLuintPointer): void => { throw new Error("Not implemented"); },
    ImportGlGenSamplers: (count: GLsizei, samplers: GLuintPointer): void => { throw new Error("Not implemented"); },
    ImportGlGenTextures: (n: GLsizei, textures: GLuintPointer): void => { throw new Error("Not implemented"); },
    ImportGlGenVertexArrays: (n: GLsizei, arrays: GLuintPointer): void => { throw new Error("Not implemented"); },
    ImportGlGetFloatv: (pname: GLenum, data: GLfloatPointer): void => { throw new Error("Not implemented"); },
    ImportGlGetIntegerv: (pname: GLenum, data: GLintPointer): void => { throw new Error("Not implemented"); },
    ImportGlGetProgramInfoLog: (program: GLuint, bufSize: GLsizei, length: GLsizeiPointer, infoLog: GLcharPointer): void => { throw new Error("Not implemented"); },
    ImportGlGetProgramiv: (program: GLuint, pname: GLenum, params: GLintPointer): void => { throw new Error("Not implemented"); },
    ImportGlGetShaderInfoLog: (shader: GLuint, bufSize: GLsizei, length: GLsizeiPointer, infoLog: GLcharPointer): void => { throw new Error("Not implemented"); },
    ImportGlGetShaderiv: (shader: GLuint, pname: GLenum, params: GLintPointer): void => { throw new Error("Not implemented"); },
    ImportGlGetString: (name: GLenum): GLubytePointer => { throw new Error("Not implemented"); },
    ImportGlGetUniformLocation: (program: GLuint, name: GLcharPointer): GLint => { throw new Error("Not implemented"); },
    ImportGlLineWidth: (width: GLfloat): void => { throw new Error("Not implemented"); },
    ImportGlLinkProgram: (program: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlPixelStorei: (pname: GLenum, param: GLint): void => { throw new Error("Not implemented"); },
    ImportGlReadPixels: (x: GLint, y: GLint, width: GLsizei, height: GLsizei, format: GLenum, type: GLenum, pixels: VoidPointer): void => { throw new Error("Not implemented"); },
    ImportGlSamplerParameteri: (sampler: GLuint, pname: GLenum, param: GLint): void => { throw new Error("Not implemented"); },
    ImportGlScissor: (x: GLint, y: GLint, width: GLsizei, height: GLsizei): void => { throw new Error("Not implemented"); },
    ImportGlShaderSource: (shader: GLuint, count: GLsizei, string: GLcharPointerPointer, length: GLintPointer): void => { throw new Error("Not implemented"); },
    ImportGlStencilFunc: (func: GLenum, ref: GLint, mask: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlStencilFuncSeparate: (face: GLenum, func: GLenum, ref: GLint, mask: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlStencilMask: (mask: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlStencilMaskSeparate: (face: GLenum, mask: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlStencilOp: (fail: GLenum, zfail: GLenum, zpass: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlStencilOpSeparate: (face: GLenum, sfail: GLenum, dpfail: GLenum, dppass: GLenum): void => { throw new Error("Not implemented"); },
    ImportGlTexImage2D: (target: GLenum, level: GLint, internalformat: GLint, width: GLsizei, height: GLsizei, border: GLint, format: GLenum, type: GLenum, pixels: VoidPointer): void => { throw new Error("Not implemented"); },
    ImportGlTexParameterf: (target: GLenum, pname: GLenum, param: GLfloat): void => { throw new Error("Not implemented"); },
    ImportGlTexParameteri: (target: GLenum, pname: GLenum, param: GLint): void => { throw new Error("Not implemented"); },
    ImportGlTexSubImage2D: (target: GLenum, level: GLint, xoffset: GLint, yoffset: GLint, width: GLsizei, height: GLsizei, format: GLenum, type: GLenum, pixels: VoidPointer): void => { throw new Error("Not implemented"); },
    ImportGlUniform1fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => { throw new Error("Not implemented"); },
    ImportGlUniform1iv: (location: GLint, count: GLsizei, value: GLintPointer): void => { throw new Error("Not implemented"); },
    ImportGlUniform2fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => { throw new Error("Not implemented"); },
    ImportGlUniform2iv: (location: GLint, count: GLsizei, value: GLintPointer): void => { throw new Error("Not implemented"); },
    ImportGlUniform3fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => { throw new Error("Not implemented"); },
    ImportGlUniform3iv: (location: GLint, count: GLsizei, value: GLintPointer): void => { throw new Error("Not implemented"); },
    ImportGlUniform4fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => { throw new Error("Not implemented"); },
    ImportGlUniform4iv: (location: GLint, count: GLsizei, value: GLintPointer): void => { throw new Error("Not implemented"); },
    ImportGlUniformMatrix3fv: (location: GLint, count: GLsizei, transpose: GLboolean, value: GLfloatPointer): void => { throw new Error("Not implemented"); },
    ImportGlUniformMatrix4fv: (location: GLint, count: GLsizei, transpose: GLboolean, value: GLfloatPointer): void => { throw new Error("Not implemented"); },
    ImportGlUseProgram: (program: GLuint): void => { throw new Error("Not implemented"); },
    ImportGlVertexAttribIPointer: (index: GLuint, size: GLint, type: GLenum, stride: GLsizei, pointer: VoidPointer): void => { throw new Error("Not implemented"); },
    ImportGlVertexAttribPointer: (index: GLuint, size: GLint, type: GLenum, normalized: GLboolean, stride: GLsizei, pointer: VoidPointer): void => { throw new Error("Not implemented"); },
    ImportGlViewport: (x: GLint, y: GLint, width: GLsizei, height: GLsizei): void => { throw new Error("Not implemented"); },

    // Platform
    ImportPrintLine: (fd: number, str: CharPointer, length: number): void => {
      const FD_STDOUT = 1;
      const FD_STDERR = 2;

      const string = decoder.decode(new DataView(memory.buffer, str, length));
      switch (fd) {
        case FD_STDOUT:
          console.log(string);
          break;
        case FD_STDERR:
          console.error(string);
          break;
        default: throw new Error(`Invalid fd: ${fd}`);
      }
    },
    ImportClock: (clockId: number): number => {
      const CLOCKID_REALTIME = 0;
      const CLOCKID_MONOTONIC = 1;

      switch (clockId) {
        case CLOCKID_REALTIME:
          return Date.now();
        case CLOCKID_MONOTONIC:
          return performance.now();
        default: throw new Error(`Invalid clockId: ${clockId}`);
      }
    }
  }
};

const instance = await WebAssembly.instantiate(module, imports);

const memory = instance.exports.memory as WebAssembly.Memory;
const initialize = instance.exports.ExportInitialize as Function;
const runIteration = instance.exports.ExportRunIteration as Function;

initialize();
console.log("after initialize");

console.log("iteration 0");
runIteration();
console.log("iteration 1");
runIteration();
console.log("iteration 2");
runIteration();
console.log("iteration 3");
runIteration();
console.log("iteration 4");
runIteration();
console.log("iteration 5");
runIteration();
console.log("iteration 6");
runIteration();
console.log("iteration 7");
runIteration();
console.log("iteration 8");
runIteration();
