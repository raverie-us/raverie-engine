import wasmUrl from "../Build/Active/Code/Editor/RaverieEditor/RaverieEditor.wasm?url";
import {
  ToWorkerMessageType,
  MessageYieldDraw,
  MessageYieldComplete,
  MessageMouseTrap,
  MessageMouseSetCursor,
  MessageDownloadFile,
  ToMainMessageType,
  MessageCopyData,
  MessageOpenFileDialog,
  MessageProgressUpdate,
  MessageProjectSave,
  MessageInitialize,
  MessageOpenUrl,
  MessageGamepadVibrate,
  MessageAudioOutput,
  ToAudioMessageType,
  AudioConstants,
  ToWorkerAudioMessageType
} from "./shared";

const mainPostMessage = <T extends ToMainMessageType>(message: T) => {
postMessage(message);
};

const modulePromise = WebAssembly.compileStreaming(fetch(wasmUrl));

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

const start = async (initMessage: MessageInitialize) => {
  // We need to buffer any messages we receive until we connect up our message 
  const bufferedMessages: MessageEvent<ToWorkerMessageType>[] = [];
  const onBufferMessage = (event: MessageEvent<ToWorkerMessageType>) => {
    bufferedMessages.push(event);
  }
  addEventListener("message", onBufferMessage);

  const module = await modulePromise;

  const gl = initMessage.canvas.getContext("webgl2", {
    antialias: false,
    alpha: false,
    preserveDrawingBuffer: false
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

    public delete(id: number): T | null {
      if (id === 0) {
        return null;
      }

      const value = this.map[id];
      if (!value) {
        throw new Error(`Double delete or attempt to delete invalid id: ${id}`);
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
  let yieldedThisFrame = false;

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
        if (programWithLocations) {
          gl.deleteProgram(programWithLocations);
          programWithLocations.locations.clear();
          delete (programWithLocations as Partial<WebGLProgramWithLocations>).locations;
        }
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
        gl.lineWidth(Math.max(width, 0.0000001));
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

        const MILLISECOND_TO_NANOSECOND = 1000000;

        switch (clockId) {
          case CLOCKID_REALTIME:
            return Date.now() * MILLISECOND_TO_NANOSECOND;
          case CLOCKID_MONOTONIC:
            return performance.now() * MILLISECOND_TO_NANOSECOND;
          default: throw new Error(`Invalid clockId: ${clockId}`);
        }
      },
      ImportYield: () => {
        const width = gl.drawingBufferWidth;
        const height = gl.drawingBufferHeight;
        const pixels = new Uint8ClampedArray(width * height * 4);
        gl.readPixels(
          0,
          0,
          width,
          height,
          gl.RGBA,
          gl.UNSIGNED_BYTE,
          pixels,
        );

        const halfHeight = Math.floor(height / 2);
        const bytesPerRow = width * 4;

        const temp = new Uint8ClampedArray(width * 4);
        for (let y = 0; y < halfHeight; ++y) {
          const topOffset = y * bytesPerRow;
          const bottomOffset = (height - y - 1) * bytesPerRow;
          temp.set(pixels.subarray(topOffset, topOffset + bytesPerRow));
          pixels.copyWithin(topOffset, bottomOffset, bottomOffset + bytesPerRow);
          pixels.set(temp, bottomOffset);
        }

        mainPostMessage<MessageYieldDraw>({
          type: "yieldDraw",
          pixels,
          width,
          height
        });
        yieldedThisFrame = true;

        // Eventually we would like to take advantage of this:
        // https://github.com/WebAssembly/js-promise-integration/blob/main/proposals/js-promise-integration/Overview.md
        // https://v8.dev/blog/jspi
      },
      ImportMouseTrap: (valueBool: number) => {
        mainPostMessage<MessageMouseTrap>({
          type: "mouseTrap",
          value: Boolean(valueBool)
        });
      },
      ImportMouseSetCursor: (cursor: number) => {
        mainPostMessage<MessageMouseSetCursor>({
          type: "mouseSetCursor",
          cursor
        });
      },
      ImportDownloadFile: (fileNamePointer: number, dataPointer: number, dataLength: number) => {
        const filename = readNullTerminatedString(fileNamePointer);
        const buffer = readBuffer(dataPointer, dataLength);
        mainPostMessage<MessageDownloadFile>({
          type: "downloadFile",
          filename,
          buffer
        });
      },
      ImportRandomUnique: (): BigInt => {
        const buffer = new Uint8Array(8);
        crypto.getRandomValues(buffer);
        const view = new DataView(buffer.buffer);
        return view.getBigUint64(0);
      },
      ImportOpenFileDialog: (dialog: number, multipleBool: number, acceptCharPtr: number) => {
        mainPostMessage<MessageOpenFileDialog>({
          type: "openFileDialog",
          dialog,
          multiple: Boolean(multipleBool),
          accept: readNullTerminatedString(acceptCharPtr)
        });
      },
      ImportProgressUpdate: (textCharPtr: number, percent: number) => {
        mainPostMessage<MessageProgressUpdate>({
          type: "progressUpdate",
          text: textCharPtr === 0
            ? null
            : readNullTerminatedString(textCharPtr),
          percent
        });
      },
      ImportSaveProject: (nameCharPtr: number, projectBytePtr: number, projectLength: number, builtContentBytePtr: number, builtContentLength: number) => {
        mainPostMessage<MessageProjectSave>({
          type: "projectSave",
          name: readNullTerminatedString(nameCharPtr),
          projectArchive: readBuffer(projectBytePtr, projectLength),
          builtContentArchive: readBuffer(builtContentBytePtr, builtContentLength)
        });
      },
      ImportOpenUrl: (urlCharPtr: number) => {
        mainPostMessage<MessageOpenUrl>({
          type: "openUrl",
          url: readNullTerminatedString(urlCharPtr)
        });
      },
      ImportGamepadVibrate: (gamepadIndex: number, duration: number, intensity: number) => {
        mainPostMessage<MessageGamepadVibrate>({
          type: "gamepadVibrate",
          gamepadIndex,
          duration,
          intensity
        });
      },
    }
  };

  const instance = await WebAssembly.instantiate(module, imports);

  const memory = instance.exports.memory as WebAssembly.Memory;

  const ExportAllocate = instance.exports.ExportAllocate as (size: number) => number;
  const ExportFree = instance.exports.ExportFree as (pointer: number) => void;
  const ExportInitialize = instance.exports.ExportInitialize as (argumentsCharPtr: number, clientWidth: number, clientHeight: number, focusedBool: number, projectBytePtrSteal: number, projectLength: number, builtContentBytePtrSteal: number, builtContentLength: number) => void;
  const ExportRunIteration = instance.exports.ExportRunIteration as () => void;
  const ExportHandleCrash = instance.exports.ExportHandleCrash as () => void;
  const ExportMouseMove = instance.exports.ExportMouseMove as (clientX: number, clientY: number, dx: number, dy: number) => void;
  const ExportMouseScroll = instance.exports.ExportMouseScroll as (clientX: number, clientY: number, scrollX: number, scrollY: number) => void;
  const ExportMouseButtonChanged = instance.exports.ExportMouseButtonChanged as (clientX: number, clientY: number, button: number, state: number) => void;
  const ExportTextTyped = instance.exports.ExportTextTyped as (rune: number) => void;
  const ExportKeyboardButtonChanged = instance.exports.ExportKeyboardButtonChanged as (key: number, state: number) => void;
  const ExportQuit = instance.exports.ExportQuit as () => void;
  const ExportCopy = instance.exports.ExportCopy as (isCutBool: number) => number;
  const ExportPaste = instance.exports.ExportPaste as (charPtr: number) => void;
  const ExportFileCreate = instance.exports.ExportFileCreate as (filePathCharPtr: number, dataBytePtr: number, dataLength: number) => void;
  const ExportFileDelete = instance.exports.ExportFileDelete as (filePathCharPtr: number) => void;
  const ExportFileDropAdd = instance.exports.ExportFileDropAdd as (filePathCharPtr: number) => void;
  const ExportFileDropFinish = instance.exports.ExportFileDropFinish as (clientX: number, clientY: number) => void;
  const ExportOpenFileDialogAdd = instance.exports.ExportOpenFileDialogAdd as (dialog: number, filePathCharPtr: number) => void;
  const ExportOpenFileDialogFinish = instance.exports.ExportOpenFileDialogFinish as (dialog: number) => void;
  const ExportSizeChanged = instance.exports.ExportSizeChanged as (clientWidth: number, clientHeight: number) => void;
  const ExportFocusChanged = instance.exports.ExportFocusChanged as (focusedBool: number) => void;
  const ExportGamepadConnectionChanged = instance.exports.ExportGamepadConnectionChanged as (gamepadIndex: number, idCharPtr: number, connectedBool: number) => void;
  const ExportGamepadButtonChanged = instance.exports.ExportGamepadButtonChanged as (gamepadIndex: number, buttonIndex: number, pressedBool: number, touchedBool: number, value: number) => void;
  const ExportGamepadAxisChanged = instance.exports.ExportGamepadAxisChanged as (gamepadIndex: number, axisIndex: number, value: number) => void;
  const ExportAudioOutput = instance.exports.ExportAudioOutput as (framesRequested: number) => number;


  const audioPostMessage = <T extends ToAudioMessageType>(message: T) => {
    initMessage.audioPort.postMessage(message);
  };
  initMessage.audioPort.onmessage = (event: MessageEvent<ToWorkerAudioMessageType>) => {
    const data = event.data;

    // It always outputs all the data it can, including 0s if needed
    const floatPtr = ExportAudioOutput(data.framesRequested);
    audioPostMessage<MessageAudioOutput>({
      type: "audioOutput",
      id: data.id,
      samplesPerChannel: new Float32Array(readBuffer(floatPtr, data.framesRequested * 4/*sizeof(float)*/ * AudioConstants.Channels)),
    });
  };

  const allocateAndCopy = (buffer: Uint8Array | null) => {
    if (!buffer) {
      return 0;
    }
    const pointer = ExportAllocate(buffer.byteLength);
    new Uint8Array(memory.buffer).set(buffer, pointer);
    return pointer;
  }
  const allocateNullTerminatedString = (str: string) => {
    return allocateAndCopy(encoder.encode(`${str}\0`))
  }

  const onMessage = (event: MessageEvent<ToWorkerMessageType>) => {
    const data = event.data;
    switch (data.type) {
      case "mouseMove":
        ExportMouseMove(data.clientX, data.clientY, data.dx, data.dy);
        break;
      case "mouseScroll":
        ExportMouseScroll(data.clientX, data.clientY, data.scrollX, data.scrollY);
        break;
      case "mouseButtonChanged":
        ExportMouseButtonChanged(data.clientX, data.clientY, data.button, data.state);
        break;
      case "keyboardButtonChanged":
        ExportKeyboardButtonChanged(data.button, data.state);
        break;
      case "textTyped":
        ExportTextTyped(data.rune);
        break;
      case "copy":
        mainPostMessage<MessageCopyData>({
          type: "copyData",
          text: readNullTerminatedString(ExportCopy(Number(data.isCut)))
        });
        break;
      case "paste": {
        const charPtr = allocateNullTerminatedString(data.text);
        ExportPaste(charPtr);
        ExportFree(charPtr);
        break;
      }
      case "filesDropped": {
        for (const file of data.files) {
          // When dropping files, we write them all to /tmp/ and add the absolute file paths to a drop context
          const filePathCharPtr = allocateNullTerminatedString(`/tmp/${file.fileName}`);
          const dataBytePtr = allocateAndCopy(new Uint8Array(file.buffer));
          ExportFileCreate(filePathCharPtr, dataBytePtr, file.buffer.byteLength);
          ExportFileDropAdd(filePathCharPtr);
          ExportFree(filePathCharPtr);
        }
        
        // Once all files have been added, we complete the drop with the coordinates
        ExportFileDropFinish(data.clientX, data.clientY);
        break;
      }
      case "openFileDialogFinish": {
        for (const file of data.files) {
          // When opening files, we write them all to /tmp/ and add the absolute file paths to a dialog context
          const filePathCharPtr = allocateNullTerminatedString(`/tmp/${file.fileName}`);
          const dataBytePtr = allocateAndCopy(new Uint8Array(file.buffer));
          ExportFileCreate(filePathCharPtr, dataBytePtr, file.buffer.byteLength);
          ExportOpenFileDialogAdd(data.dialog, filePathCharPtr);
          ExportFree(filePathCharPtr);
        }
        
        // Once all files have been added, we complete the dialog
        ExportOpenFileDialogFinish(data.dialog);
        break;
      }
      case "sizeChanged":
        initMessage.canvas.width = data.clientWidth;
        initMessage.canvas.height = data.clientHeight;
        ExportSizeChanged(data.clientWidth, data.clientHeight);
        // Since the OffscreenCanvas clears the back buffer to black/transparent upon any resize
        // we run another engine iteration immediately to render out a frame to avoid resize flicker
        ExportRunIteration();
        break;
      case "focusChanged":
        ExportFocusChanged(Number(data.focused));
        break;
      case "gamepadConnectionChanged": {
        const idCharPtr = allocateNullTerminatedString(data.id);
        ExportGamepadConnectionChanged(data.gamepadIndex, idCharPtr, Number(data.connected));
        ExportFree(idCharPtr);
        break;
      }
      case "gamepadButtonChanged":
        ExportGamepadButtonChanged(data.gamepadIndex, data.buttonIndex, Number(data.pressed), Number(data.touched), data.value);
        break;
      case "gamepadAxisChanged":
        ExportGamepadAxisChanged(data.gamepadIndex, data.axisIndex, data.value);
        break;
    }
  };

  let requestedAnimationFrame = -1;
  const handleCrash = () => {
    cancelAnimationFrame(requestedAnimationFrame);
    removeEventListener("message", onMessage);
    ExportHandleCrash();
  };
  addEventListener("error", handleCrash);
  addEventListener("unhandledrejection", handleCrash);

  const decoder = new TextDecoder();
  const encoder = new TextEncoder();
  const readLengthString = (startPointer: number, lengthBytes: number) =>
    decoder.decode(new DataView(memory.buffer, startPointer, lengthBytes));
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
  const readBuffer = (startPointer: number, lengthBytes: number) => {
    return memory.buffer.slice(startPointer, startPointer + lengthBytes)
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

  const commandLineCharPtr = allocateNullTerminatedString(initMessage.args);
  const projectArchiveBytePtr = allocateAndCopy(initMessage.projectArchive);
  const builtContentArchiveBytePtr = allocateAndCopy(initMessage.builtContentArchive);
  ExportInitialize(
    commandLineCharPtr,
    initMessage.canvas.width,
    initMessage.canvas.height,
    Number(initMessage.focused),
    projectArchiveBytePtr,
    initMessage.projectArchive?.byteLength || 0,
    builtContentArchiveBytePtr,
    initMessage.builtContentArchive?.byteLength || 0);
  ExportFree(commandLineCharPtr);

  removeEventListener("message", onBufferMessage);
  addEventListener("message", onMessage);
  for (const bufferedMessage of bufferedMessages) {
    onMessage(bufferedMessage);
  }

  let mustSendYieldComplete = false;
  const doUpdate = () => {
    requestedAnimationFrame = requestAnimationFrame(doUpdate);
    ExportRunIteration();

    if (yieldedThisFrame) {
      yieldedThisFrame = false;
      mustSendYieldComplete = true;
    } else if (mustSendYieldComplete) {
      mustSendYieldComplete = false;
      // We only send yield complete when we complete an entire frame without yielding
      mainPostMessage<MessageYieldComplete>({
        type: "yieldComplete"
      });
    }
  };

  doUpdate();
}

const onCanvasMessage = (event: MessageEvent<ToWorkerMessageType>) => {
  const data = event.data;
  if (data.type === "initialize") {
    removeEventListener("message", onCanvasMessage);
    start(data);
  }
};
addEventListener("message", onCanvasMessage);
