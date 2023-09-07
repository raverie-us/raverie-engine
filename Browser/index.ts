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
type GLuintPointer = number;
type GLintPointer = number;
type GLenumPointer = number;
type GLfloatPointer = number;
type GLsizeiPointer = number;

// Platform
type CharPointer = number;

const canvas = document.createElement("canvas");
canvas.style.backgroundColor = "#555";
canvas.width = 512;
canvas.height = 512;
document.body.append(canvas);
const gl = canvas.getContext("webgl2", {
  antialias: false
})!;

const EXT_texture_filter_anisotropic = gl.getExtension("EXT_texture_filter_anisotropic");
const GL_TEXTURE_MAX_ANISOTROPY = 0x84FE;

const requireExtension = (name: string) => {
  if (!gl.getExtension(name)) {
    throw new Error(`Needs ${name} to function`);
  }
}

requireExtension("EXT_color_buffer_float");
requireExtension("OES_texture_float_linear");

class ObjectMap<T> {
  private map: Record<number, T> = {};
  // Start at 1 since 0 is reserved
  private counter = 1;

  public allocate(value: T | null): number {
    if (!value) {
      throw new Error("Failed to allocate type");
    }

    const id = this.counter++;
    this.map[id] = value;
    return id;
  }

  public delete(id: number): T {
    const value = this.map[id];
    if (!value) {
      throw new Error("Double delete or attempt to delete invalid id");
    }
    delete this.map[id];
    return value;
  }

  public get(id: number): T | null {
    if (id === 0) {
      return null;
    }
    return this.getRequired(id);
  }

  public getRequired(id: number): T {
    const value = this.map[id];
    if (!value) {
      throw new Error("Attempt to get value by invalid id");
    }
    return value;
  }
}

class LazyObjectMap<T> {
  private mapToId = new Map<T, number>();
  private mapToValue: Record<number, T> = {};
  // Note: 0 is not reserved here, -1 is invalid
  private counter = 0;

  public allocateOrGet(value: T | null): number {
    if (!value) {
      return -1;
    }

    let id = this.mapToId.get(value);
    if (id === undefined) {
      id = this.counter++;
      this.mapToId.set(value, id);
      this.mapToValue[id] = value;
    }
    return id;
  }

  public clear(): void {
    this.mapToId.clear();
    this.mapToValue = {};
  }

  public get(id: number): T | null {
    if (id === -1) {
      return null;
    }
    const value = this.mapToValue[id];
    if (!value) {
      throw new Error("Attempt to get value by invalid id");
    }
    return value;
  }
}

interface WebGLProgramWithLocations extends WebGLProgram {
  locations: LazyObjectMap<WebGLUniformLocation>;
}

const vaoMap = new ObjectMap<WebGLVertexArrayObject>();
const bufferMap = new ObjectMap<WebGLBuffer>();
const frameBufferMap = new ObjectMap<WebGLFramebuffer>();
const programMap = new ObjectMap<WebGLProgramWithLocations>();
const shaderMap = new ObjectMap<WebGLShader>();
const textureMap = new ObjectMap<WebGLTexture>();
const samplerMap = new ObjectMap<WebGLSampler>();

let usedProgram: WebGLProgramWithLocations | null = null;

const imports: WebAssembly.Imports = {
  env: {
    // GL
    ImportGlActiveTexture: (texture: GLenum): void => {
      gl.activeTexture(texture);
    },
    ImportGlAttachShader: (program: GLuint, shader: GLuint): void => {
      gl.attachShader(programMap.getRequired(program), shaderMap.getRequired(shader));
    },
    ImportGlBindAttribLocation: (program: GLuint, index: GLuint, name: GLcharPointer): void => {
      const nameString = readNullTerminatedString(name);
      gl.bindAttribLocation(programMap.getRequired(program), index, nameString);
    },
    ImportGlBindBuffer: (target: GLenum, buffer: GLuint): void => {
      gl.bindBuffer(target, bufferMap.get(buffer));
    },
    ImportGlBindFramebuffer: (target: GLenum, framebuffer: GLuint): void => {
      gl.bindFramebuffer(target, frameBufferMap.get(framebuffer));
    },
    ImportGlBindSampler: (unit: GLuint, sampler: GLuint): void => {
      gl.bindSampler(unit, samplerMap.get(sampler));
    },
    ImportGlBindTexture: (target: GLenum, texture: GLuint): void => {
      gl.bindTexture(target, textureMap.get(texture));
    },
    ImportGlBindVertexArray: (array: GLuint): void => {
      gl.bindVertexArray(vaoMap.get(array));
    },
    ImportGlBlendEquation: (mode: GLenum): void => {
      gl.blendEquation(mode);
    },
    // To support all these, we must have extension OES_draw_buffers_indexed
    // and we must set mDriverSupport.mMultiTargetBlend
    ImportGlBlendEquationi: (buf: GLuint, mode: GLenum): void => {
      throw new Error("Not implemented");
    },
    ImportGlBlendEquationSeparate: (modeRGB: GLenum, modeAlpha: GLenum): void => {
      gl.blendEquationSeparate(modeRGB, modeAlpha);
    },
    ImportGlBlendEquationSeparatei: (buf: GLuint, modeRGB: GLenum, modeAlpha: GLenum): void => {
      throw new Error("Not implemented");
    },
    ImportGlBlendFunc: (sfactor: GLenum, dfactor: GLenum): void => {
      gl.blendFunc(sfactor, dfactor);
    },
    ImportGlBlendFunci: (buf: GLuint, src: GLenum, dst: GLenum): void => {
      throw new Error("Not implemented");
    },
    ImportGlBlendFuncSeparate: (sfactorRGB: GLenum, dfactorRGB: GLenum, sfactorAlpha: GLenum, dfactorAlpha: GLenum): void => {
      gl.blendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    },
    ImportGlBlendFuncSeparatei: (buf: GLuint, srcRGB: GLenum, dstRGB: GLenum, srcAlpha: GLenum, dstAlpha: GLenum): void => {
      throw new Error("Not implemented");
    },
    ImportGlBlitFramebuffer: (srcX0: GLint, srcY0: GLint, srcX1: GLint, srcY1: GLint, dstX0: GLint, dstY0: GLint, dstX1: GLint, dstY1: GLint, mask: GLbitfield, filter: GLenum): void => {
      gl.blitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    },
    ImportGlBufferData: (target: GLenum, size: GLsizeiptr, data: VoidPointer, usage: GLenum): void => {
      const view = new DataView(memory.buffer, data, size);
      gl.bufferData(target, view, usage);
    },
    ImportGlBufferSubData: (target: GLenum, offset: GLintptr, size: GLsizeiptr, data: VoidPointer): void => {
      const view = new DataView(memory.buffer, data, size);
      gl.bufferSubData(target, offset, view);
    },
    ImportGlCheckFramebufferStatus: (target: GLenum): GLenum => {
      return gl.checkFramebufferStatus(target);
    },
    ImportGlClear: (mask: GLbitfield): void => {
      gl.clear(mask);
    },
    ImportGlClearColor: (red: GLfloat, green: GLfloat, blue: GLfloat, alpha: GLfloat): void => {
      gl.clearColor(red, green, blue, alpha);
    },
    ImportGlClearDepth: (d: GLfloat): void => {
      gl.clearDepth(d);
    },
    ImportGlClearStencil: (s: GLint): void => {
      gl.clearStencil(s);
    },
    ImportGlCompileShader: (shader: GLuint): void => {
      gl.compileShader(shaderMap.getRequired(shader));
    },
    // For this we probably need extension EXT_texture_compression_bptc and EXT_texture_compression_rgtc
    // and possibly WEBGL_compressed_texture_s3tc and WEBGL_compressed_texture_s3tc_srgb
    ImportGlCompressedTexImage2D: (target: GLenum, level: GLint, internalformat: GLenum, width: GLsizei, height: GLsizei, imageSize: GLsizei, data: VoidPointer): void => {
      throw new Error("Not implemented");
    },
    ImportGlCreateProgram: (): GLuint => {
      const program = gl.createProgram() as WebGLProgramWithLocations;
      program.locations = new LazyObjectMap<WebGLUniformLocation>();
      return programMap.allocate(program);
    },
    ImportGlCreateShader: (type: GLenum): GLuint => {
      return shaderMap.allocate(gl.createShader(type));
    },
    ImportGlCullFace: (mode: GLenum): void => {
      gl.cullFace(mode);
    },
    ImportGlDeleteBuffer: (buffer: GLuint): void => {
      gl.deleteBuffer(bufferMap.delete(buffer));
    },
    ImportGlDeleteFramebuffer: (framebuffer: GLuint): void => {
      gl.deleteFramebuffer(frameBufferMap.delete(framebuffer));
    },
    ImportGlDeleteProgram: (program: GLuint): void => {
      const programWithLocations = programMap.delete(program);
      gl.deleteProgram(programWithLocations);
      programWithLocations.locations.clear();
      delete (programWithLocations as Partial<WebGLProgramWithLocations>).locations;
    },
    ImportGlDeleteSamplers: (count: GLsizei, samplers: GLuintPointer): void => {
      throw new Error("Not implemented");
    },
    ImportGlDeleteShader: (shader: GLuint): void => {
      gl.deleteShader(shaderMap.delete(shader));
    },
    ImportGlDeleteTexture: (texture: GLuint): void => {
      gl.deleteTexture(textureMap.delete(texture));
    },
    ImportGlDeleteVertexArray: (array: GLuint): void => {
      gl.deleteVertexArray(vaoMap.delete(array));
    },
    ImportGlDepthFunc: (func: GLenum): void => {
      gl.depthFunc(func);
    },
    ImportGlDepthMask: (flag: GLboolean): void => {
      gl.depthMask(flag);
    },
    ImportGlDetachShader: (program: GLuint, shader: GLuint): void => {
      gl.detachShader(programMap.getRequired(program), shaderMap.getRequired(shader));
    },
    ImportGlDisable: (cap: GLenum): void => {
      gl.disable(cap);
    },
    ImportGlDisablei: (target: GLenum, index: GLuint): void => {
      throw new Error("Not implemented");
    },
    ImportGlDrawArrays: (mode: GLenum, first: GLint, count: GLsizei): void => {
      gl.drawArrays(mode, first, count);
    },
    ImportGlDrawBuffers: (n: GLsizei, bufs: GLenumPointer): void => {
      gl.drawBuffers(new Uint32Array(memory.buffer, bufs, n));
    },
    ImportGlDrawElements: (mode: GLenum, count: GLsizei, type: GLenum, indicesOrOffset: VoidPointer): void => {
      gl.drawElements(mode, count, type, indicesOrOffset);
    },
    ImportGlEnable: (cap: GLenum): void => {
      gl.enable(cap);
    },
    ImportGlEnablei: (target: GLenum, index: GLuint): void => {
      throw new Error("Not implemented");
    },
    ImportGlEnableVertexAttribArray: (index: GLuint): void => {
      gl.enableVertexAttribArray(index);
    },
    ImportGlFramebufferTexture2D: (target: GLenum, attachment: GLenum, textarget: GLenum, texture: GLuint, level: GLint): void => {
      gl.framebufferTexture2D(target, attachment, textarget, textureMap.get(texture), level);
    },
    ImportGlGenBuffer: (): GLuint => {
      return bufferMap.allocate(gl.createBuffer());
    },
    ImportGlGenerateMipmap: (target: GLenum): void => {
      gl.generateMipmap(target);
    },
    ImportGlGenFramebuffer: (): GLuint => {
      return frameBufferMap.allocate(gl.createFramebuffer());
    },
    ImportGlGenSamplers: (count: GLsizei, samplers: GLuintPointer): void => {
      throw new Error("Not implemented");
    },
    ImportGlGenTexture: (): GLuint => {
      return textureMap.allocate(gl.createTexture());
    },
    ImportGlGenVertexArray: (): GLuint => {
      return vaoMap.allocate(gl.createVertexArray());
    },
    ImportGlGetProgramInfoLog: (program: GLuint, bufSize: GLsizei, length: GLsizeiPointer, infoLog: GLcharPointer): void => {
      throw new Error("Not implemented");
    },
    ImportGlGetProgramiv: (program: GLuint, pname: GLenum, params: GLintPointer): void => {
      throw new Error("Not implemented");
    },
    ImportGlGetShaderInfoLog: (shader: GLuint, bufSize: GLsizei, length: GLsizeiPointer, infoLog: GLcharPointer): void => {
      throw new Error("Not implemented");
    },
    ImportGlGetShaderiv: (shader: GLuint, pname: GLenum, params: GLintPointer): void => {
      throw new Error("Not implemented");
    },
    ImportGlGetUniformLocation: (program: GLuint, name: GLcharPointer): GLint => {
      const programWithLocations = programMap.getRequired(program);
      const nameString = readNullTerminatedString(name);
      const location = gl.getUniformLocation(programWithLocations, nameString);
      return programWithLocations.locations.allocateOrGet(location);
    },
    ImportGlLineWidth: (width: GLfloat): void => {
      gl.lineWidth(width);
    },
    ImportGlLinkProgram: (program: GLuint): void => {
      const programWithLocations = programMap.getRequired(program);
      programWithLocations.locations.clear();
      gl.linkProgram(programWithLocations);
    },
    ImportGlPixelStorei: (pname: GLenum, param: GLint): void => {
      gl.pixelStorei(pname, param);
    },
    ImportGlReadPixels: (x: GLint, y: GLint, width: GLsizei, height: GLsizei, format: GLenum, type: GLenum, pixels: VoidPointer): void => {
      gl.readPixels(x, y, width, height, format, type, getPixelsView(width, height, format, type, pixels));
    },
    ImportGlSamplerParameteri: (sampler: GLuint, pname: GLenum, param: GLint): void => {
      throw new Error("Not implemented");
    },
    ImportGlScissor: (x: GLint, y: GLint, width: GLsizei, height: GLsizei): void => {
      gl.scissor(x, y, width, height);
    },
    ImportGlShaderSource: (shader: GLuint, str: GLcharPointer, length: GLint): void => {
      const string = readLengthString(str, length);
      gl.shaderSource(shaderMap.getRequired(shader), string);
    },
    ImportGlStencilFunc: (func: GLenum, ref: GLint, mask: GLuint): void => {
      gl.stencilFunc(func, ref, mask);
    },
    ImportGlStencilFuncSeparate: (face: GLenum, func: GLenum, ref: GLint, mask: GLuint): void => {
      gl.stencilFuncSeparate(face, func, ref, mask);
    },
    ImportGlStencilMask: (mask: GLuint): void => {
      gl.stencilMask(mask);
    },
    ImportGlStencilMaskSeparate: (face: GLenum, mask: GLuint): void => {
      gl.stencilMaskSeparate(face, mask);
    },
    ImportGlStencilOp: (fail: GLenum, zfail: GLenum, zpass: GLenum): void => {
      gl.stencilOp(fail, zfail, zpass);
    },
    ImportGlStencilOpSeparate: (face: GLenum, sfail: GLenum, zfail: GLenum, zpass: GLenum): void => {
      gl.stencilOpSeparate(face, sfail, zfail, zpass);
    },
    ImportGlTexImage2D: (target: GLenum, level: GLint, internalformat: GLint, width: GLsizei, height: GLsizei, format: GLenum, type: GLenum, pixels: VoidPointer): void => {
      gl.texImage2D(target, level, internalformat, width, height, 0, format, type, getPixelsView(width, height, format, type, pixels));
    },
    ImportGlTexParameterf: (target: GLenum, pname: GLenum, param: GLfloat): void => {
      if (pname === GL_TEXTURE_MAX_ANISOTROPY) {
        if (EXT_texture_filter_anisotropic) {
          gl.texParameterf(target, pname, param);
        }
      } else {
        gl.texParameterf(target, pname, param);
      }
    },
    ImportGlTexParameteri: (target: GLenum, pname: GLenum, param: GLint): void => {
      gl.texParameteri(target, pname, param);
    },
    ImportGlTexSubImage2D: (target: GLenum, level: GLint, xoffset: GLint, yoffset: GLint, width: GLsizei, height: GLsizei, format: GLenum, type: GLenum, pixels: VoidPointer): void => {
      gl.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, getPixelsView(width, height, format, type, pixels));
    },
    ImportGlUniform1fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => {
      gl.uniform1fv(usedProgram!.locations.get(location), new Float32Array(memory.buffer, value, count));
    },
    ImportGlUniform1iv: (location: GLint, count: GLsizei, value: GLintPointer): void => {
      gl.uniform1iv(usedProgram!.locations.get(location), new Int32Array(memory.buffer, value, count));
    },
    ImportGlUniform2fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => {
      gl.uniform2fv(usedProgram!.locations.get(location), new Float32Array(memory.buffer, value, count * 2));
    },
    ImportGlUniform2iv: (location: GLint, count: GLsizei, value: GLintPointer): void => {
      gl.uniform2iv(usedProgram!.locations.get(location), new Int32Array(memory.buffer, value, count * 2));
    },
    ImportGlUniform3fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => {
      gl.uniform3fv(usedProgram!.locations.get(location), new Float32Array(memory.buffer, value, count * 3));
    },
    ImportGlUniform3iv: (location: GLint, count: GLsizei, value: GLintPointer): void => {
      gl.uniform3iv(usedProgram!.locations.get(location), new Int32Array(memory.buffer, value, count * 3));
    },
    ImportGlUniform4fv: (location: GLint, count: GLsizei, value: GLfloatPointer): void => {
      gl.uniform4fv(usedProgram!.locations.get(location), new Float32Array(memory.buffer, value, count * 4));
    },
    ImportGlUniform4iv: (location: GLint, count: GLsizei, value: GLintPointer): void => {
      gl.uniform4iv(usedProgram!.locations.get(location), new Int32Array(memory.buffer, value, count * 4));
    },
    ImportGlUniformMatrix3fv: (location: GLint, count: GLsizei, transpose: GLboolean, value: GLfloatPointer): void => {
      gl.uniformMatrix3fv(usedProgram!.locations.get(location), transpose, new Float32Array(memory.buffer, value, count * 3 * 3));
    },
    ImportGlUniformMatrix4fv: (location: GLint, count: GLsizei, transpose: GLboolean, value: GLfloatPointer): void => {
      gl.uniformMatrix4fv(usedProgram!.locations.get(location), transpose, new Float32Array(memory.buffer, value, count * 4 * 4));
    },
    ImportGlUseProgram: (program: GLuint): void => {
      const programWithLocations = programMap.get(program);
      usedProgram = programWithLocations;
      gl.useProgram(programWithLocations);
    },
    ImportGlVertexAttribIPointer: (index: GLuint, size: GLint, type: GLenum, stride: GLsizei, pointerOrOffset: VoidPointer): void => {
      gl.vertexAttribIPointer(index, size, type, stride, pointerOrOffset);
    },
    ImportGlVertexAttribPointer: (index: GLuint, size: GLint, type: GLenum, normalized: GLboolean, stride: GLsizei, pointerOrOffset: VoidPointer): void => {
      gl.vertexAttribPointer(index, size, type, normalized, stride, pointerOrOffset);
    },
    ImportGlViewport: (x: GLint, y: GLint, width: GLsizei, height: GLsizei): void => {
      gl.viewport(x, y, width, height);
    },

    // Platform
    ImportPrintLine: (fd: number, str: CharPointer, length: number): void => {
      const FD_STDOUT = 1;
      const FD_STDERR = 2;

      const string = readLengthString(str, length);
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

const decoder = new TextDecoder();
const readLengthString = (startPointer: number, length: number) =>
  decoder.decode(new DataView(memory.buffer, startPointer, length));
const readNullTerminatedString = (startPointer: number) => {
  const buffer = new Uint8Array(memory.buffer);
  let length = 0;
  for (let i = startPointer; i < buffer.byteLength; ++i) {
    if (buffer[i] === 0) {
      length = i - startPointer;
      break;
    }
  }
  return decoder.decode(new DataView(memory.buffer, startPointer, length));
}

const getPixelsView = (width: GLsizei, height: GLsizei, format: GLenum, type: GLenum, pixels: VoidPointer): ArrayBufferView | null => {
  if (pixels === 0) {
    return null;
  }

  let componentCount = 0;
  switch (format) {
    case gl.DEPTH_COMPONENT: componentCount = 1; break;
    case gl.DEPTH_STENCIL: componentCount = 1; break;
    case gl.RED: componentCount = 1; break;
    case gl.RG: componentCount = 2; break;
    case gl.RGB: componentCount = 3; break;
    case gl.RGBA: componentCount = 4; break;
    default: throw new Error(`Unhandled format ${format}`);
  }

  const elements = componentCount * width * height;

  switch (type) {
    case gl.UNSIGNED_BYTE:
      return new Uint8Array(memory.buffer, pixels, elements)
    case gl.UNSIGNED_SHORT_5_6_5:
    case gl.UNSIGNED_SHORT_4_4_4_4:
    case gl.UNSIGNED_SHORT_5_5_5_1:
    case gl.UNSIGNED_SHORT:
    case gl.HALF_FLOAT:
      return new Uint16Array(memory.buffer, pixels, elements)
    case gl.UNSIGNED_INT:
    case gl.UNSIGNED_INT_24_8:
      return new Uint32Array(memory.buffer, pixels, elements)
    case gl.FLOAT:
      return new Float32Array(memory.buffer, pixels, elements)
    default: throw new Error(`Unhandled type ${type}`);
  }
}

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
