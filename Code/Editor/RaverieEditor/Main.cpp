// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include <wasi/api.h>

using namespace Zero;

static GameOrEditorStartup* startup = nullptr;

extern "C" void __wasm_call_ctors();
#define WASI_STDOUT 1
#define WASI_STDERR 2

void ZeroExportNamed(ExportInitialize)() {
  __wasm_call_ctors();
  startup = new GameOrEditorStartup();
}

void ZeroExportNamed(ExportRunIteration)() {
  startup->RunIteration();
}

ZeroNoReturn void fatal(const char* str) {
  ImportPrintLine(WASI_STDERR, str, strlen(str));
  abort();
}

#define FATAL_NO_CALL(name) fatal(#name " should not be called");

extern "C" {

int32_t __imported_wasi_snapshot_preview1_args_get(uint8_t** argv, uint8_t* argv_buf) __attribute__((
  __import_module__("wasi_snapshot_preview1"),
  __import_name__("args_get")
)) {
  *argv = 0;
  *argv_buf = 0;
  return 0;
}

int32_t __imported_wasi_snapshot_preview1_args_sizes_get(__wasi_size_t* retptr0, __wasi_size_t* retptr1) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("args_sizes_get")
)) {
  *retptr0 = 0;
  *retptr1 = 0;
  return 0;
}

int32_t __imported_wasi_snapshot_preview1_environ_get(uint8_t** environ, uint8_t* environ_buf) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("environ_get")
)) {
  *environ = 0;
  *environ_buf = 0;
  return 0;
}

int32_t __imported_wasi_snapshot_preview1_environ_sizes_get(__wasi_size_t* retptr0, __wasi_size_t* retptr1) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("environ_sizes_get")
)) {
  *retptr0 = 0;
  *retptr1 = 0;
  return 0;
}

int32_t __imported_wasi_snapshot_preview1_fd_close(__wasi_fd_t fd) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("fd_close")
)) {
  FATAL_NO_CALL(fd_close);
}

int32_t __imported_wasi_snapshot_preview1_fd_fdstat_get(__wasi_fd_t fd, __wasi_fdstat_t* retptr0) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("fd_fdstat_get")
)) {
  if (fd != WASI_STDOUT && fd != WASI_STDERR) {
    fatal("fd_fdstat_get should only be called on stdout or stderr");
  }

  // This is called by __isatty, and since we don't care we can short circuit and return BADF
  return __WASI_ERRNO_BADF;
}

int32_t __imported_wasi_snapshot_preview1_fd_fdstat_set_flags(__wasi_fd_t fd, __wasi_fdflags_t flags) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("fd_fdstat_set_flags")
)) {
  FATAL_NO_CALL(fd_fdstat_set_flags);
}

int32_t __imported_wasi_snapshot_preview1_fd_prestat_get(__wasi_fd_t fd, __wasi_prestat_t* retptr0) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("fd_prestat_get")
)) {
  // WASI relies on hitting BADF to know when it's done, since we have no prestat files we just always return BADF
  return __WASI_ERRNO_BADF;
}

int32_t __imported_wasi_snapshot_preview1_fd_prestat_dir_name(__wasi_fd_t fd, uint8_t* path, __wasi_size_t path_len) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("fd_prestat_dir_name")
)) {
  FATAL_NO_CALL(fd_prestat_dir_name);
}

// We keep buffers because we don't flush until we hit a newline (stdin, stdout, stderr)
// For now we're not using stdin, just keeping it there so the fd indices line up
std::vector<uint8_t> stdBuffer[3];
int32_t __imported_wasi_snapshot_preview1_fd_write(__wasi_fd_t fd, const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("fd_write")
)) {
  if (fd != WASI_STDOUT && fd != WASI_STDERR) {
    fatal("fd_write should only be called on stdout or stderr");
  }

  std::vector<uint8_t>& buffer = stdBuffer[fd];

  *nwritten = 0;
  for (size_t i = 0; i < iovs_len; ++i) {
    const __wasi_ciovec_t& io = iovs[i];
    *nwritten += io.buf_len;

    size_t prevSize = buffer.size();
    buffer.resize(buffer.size() + io.buf_len);

    memcpy(buffer.data() + prevSize, io.buf, io.buf_len);
  }

  bool lineFound = false;
  do {
    lineFound = false;
    for (size_t i = 0; i < buffer.size(); ++i) {
      if (buffer[i] == '\n') {
        ImportPrintLine(fd, (const char*)buffer.data(), i);
        buffer.erase(buffer.begin(), buffer.begin() + i + 1); // + 1 for the newline
        lineFound = true;
        break;
      }
    }
  } while(lineFound);

  return __WASI_ERRNO_SUCCESS;
}

int32_t __imported_wasi_snapshot_preview1_fd_read(__wasi_fd_t fd, const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("fd_read")
)) {
  FATAL_NO_CALL(fd_read);
}

int32_t __imported_wasi_snapshot_preview1_fd_seek(__wasi_fd_t fd, __wasi_filedelta_t offset, __wasi_whence_t whence, __wasi_filesize_t *retptr0) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("fd_seek")
)) {
  FATAL_NO_CALL(fd_seek);
}


int32_t __imported_wasi_snapshot_preview1_path_create_directory(__wasi_fd_t fd, const char *path, int32_t path_len) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("path_create_directory")
)) {
  FATAL_NO_CALL(path_create_directory);
}

int32_t __imported_wasi_snapshot_preview1_path_filestat_get(__wasi_fd_t fd, __wasi_lookupflags_t flags, const char *path, int32_t path_len, __wasi_filestat_t* retptr0) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("path_filestat_get")
)) {
  FATAL_NO_CALL(path_filestat_get);
}

int32_t __imported_wasi_snapshot_preview1_path_open(__wasi_fd_t fd, __wasi_lookupflags_t dirflags, const char *path, size_t path_len, __wasi_oflags_t oflags, __wasi_rights_t fs_rights_base, __wasi_rights_t fs_rights_inheriting, __wasi_fdflags_t fdflags, __wasi_fd_t* retptr0) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("path_open")
)) {
  FATAL_NO_CALL(path_open);
}

int32_t __imported_wasi_snapshot_preview1_path_remove_directory(__wasi_fd_t fd, const char* path, size_t path_len) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("path_remove_directory")
)) {
  FATAL_NO_CALL(path_remove_directory);
}

int32_t __imported_wasi_snapshot_preview1_path_unlink_file(__wasi_fd_t fd, const char* path, size_t path_len) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("path_unlink_file")
)) {
  FATAL_NO_CALL(path_unlink_file);
}

_Noreturn void __imported_wasi_snapshot_preview1_proc_exit(__wasi_exitcode_t rval) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("proc_exit")
)) {
  FATAL_NO_CALL(proc_exit);
}

int32_t __imported_wasi_snapshot_preview1_clock_time_get(__wasi_clockid_t id, __wasi_timestamp_t precision, __wasi_timestamp_t* ticks) __attribute__((
    __import_module__("wasi_snapshot_preview1"),
    __import_name__("clock_time_get")
)) {
  if (id != __WASI_CLOCKID_REALTIME && id != __WASI_CLOCKID_MONOTONIC) {
    fatal("clock_time_get should only be called with REALTIME or MONOTONIC clocks ids");
  }

  *ticks = ImportClock(id);
  return __WASI_ERRNO_SUCCESS;
}

/*
static void InitializeOnce() {
  for (;;) {
    StartupPhase::Enum phase = startup->RunIteration();

    // Once we reach the engine update phase, we can stop initialization
    if (phase == StartupPhase::EngineUpdate) {
      break;
    }
  }
}

void __wasm_call_ctors();
void __wasm_call_dtors();
//void __wasi_proc_exit(int);
int __main_void();

// This function's export name `wizer.initialize` is specially
// recognized by Wizer. It is the direct entry point for pre-init.
__attribute__((export_name("wizer.initialize"))) void __wizer_initialize() {
    // `__wasm_call_ctors()` is generated by `wasm-ld` and invokes all
    // of the global constructors. It is safe (and in fact necessary)
    // to manually invoke it here because `wizer.initialize` is the
    // direct entry point, and no libc startup (crt1.o or equivalent)
    // is executed before this code does.
    __wasm_call_ctors();

    // Perform our own initialization
    InitializeOnce();
}

// This function replaces `_start` (the WASI-specified entry point) in
// the pre-initialized Wasm module.
__attribute__((export_name("wizer.resume"))) void __wizer_resume() {
    // `__main_void()` is defined by the WASI SDK toolchain due to
    // special semantics in C/C++ for the `main()` function, i.e., ito
    // can either take argc/argv or not. It collects arguments using
    // the appropriate WASI calls and then invokes the user program's
    // `main()`. This may change in the future; when it does, we will
    // coordinate with the WASI-SDK toolchain to implement this entry
    // point in an alternate way.
    int r = __main_void();
    // Because we are replacing `_start()`, we need to manually invoke
    // destructors as well.
    __wasm_call_dtors();
    // If main returned non-zero code, call `__wasi_proc_exit`.
    if (r != 0) {
      __wasi_proc_exit(r);
    }
}
*/

int main(int argc, char* argv[])
{

  CommandLineToStringArray(gCommandLineArguments, argv, argc);
  return 0;
}

}
