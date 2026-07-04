
#ifndef INCLUDEBUILD_H
#define INCLUDEBUILD_H

#if defined(__linux__) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IB_VERSION_MAJOR 2
#define IB_VERSION_MINOR 0
#define IB_VERSION_PATCH 3

#ifndef IB_INITIAL_LIST_CAPACITY
#define IB_INITIAL_LIST_CAPACITY 8
#endif

typedef struct ib_context ib_context;
typedef struct ib_project ib_project;
typedef struct ib_target ib_target;

typedef enum {
    IB_OK = 0,
    IB_ERR_INVALID,
    IB_ERR_IO,
    IB_ERR_TOOLCHAIN,
    IB_ERR_BUILD,
    IB_ERR_NOMEM,
    IB_ERR_NOT_FOUND,
    IB_ERR_DUPLICATE
} ib_status;

typedef enum {
    IB_DIAG_ERROR = 0,
    IB_DIAG_WARN  = 1,
    IB_DIAG_INFO  = 2,
    IB_DIAG_DEBUG = 3
} ib_diag_level;

typedef enum {
    IB_TARGET_EXECUTABLE = 0,
    IB_TARGET_STATIC_LIB = 1,
    IB_TARGET_SHARED_LIB = 2
} ib_target_kind;

typedef enum {
    IB_MODE_DEBUG = 0,
    IB_MODE_RELEASE = 1,
    IB_MODE_CUSTOM = 2
} ib_build_mode;

ib_context* ib_context_create(void);
void ib_context_destroy(ib_context* ctx);

void ib_context_set_log_level(ib_context* ctx, ib_diag_level level);
void ib_context_set_verbose(ib_context* ctx, bool verbose);
void ib_context_set_color_output(ib_context* ctx, bool enabled);
void ib_context_clear_diagnostics(ib_context* ctx);
ib_status ib_context_last_status(const ib_context* ctx);
const char* ib_context_last_message(const ib_context* ctx);
size_t ib_context_diagnostic_count(const ib_context* ctx);
ib_diag_level ib_context_diagnostic_level_at(const ib_context* ctx, size_t index);
ib_status ib_context_diagnostic_status_at(const ib_context* ctx, size_t index);
const char* ib_context_diagnostic_message_at(const ib_context* ctx, size_t index);

ib_project* ib_project_create(ib_context* ctx, const char* root_dir);
void ib_project_destroy(ib_project* project);

ib_status ib_project_set_c_compiler(ib_project* project, const char* compiler);
ib_status ib_project_set_cxx_compiler(ib_project* project, const char* compiler);
ib_status ib_project_set_archiver(ib_project* project, const char* archiver);
ib_status ib_project_set_output_dir(ib_project* project, const char* dir);
ib_status ib_project_set_state_dir(ib_project* project, const char* dir);
ib_status ib_project_set_mode_flags(ib_project* project, ib_build_mode mode, const char* c_flags, const char* cxx_flags);
ib_status ib_project_add_include_dir(ib_project* project, const char* path);
ib_status ib_project_add_shared_source(ib_project* project, const char* path);
ib_status ib_project_scan_shared_dir(ib_project* project, const char* dir, bool recursive);

ib_target* ib_project_add_target(ib_project* project, const char* name, ib_target_kind kind);
ib_status ib_target_set_entry(ib_target* target, const char* path);
ib_status ib_target_add_source(ib_target* target, const char* path);
ib_status ib_target_add_include_dir(ib_target* target, const char* path);
ib_status ib_target_add_cflags(ib_target* target, const char* flags);
ib_status ib_target_add_link_flags(ib_target* target, const char* flags);

ib_status ib_project_build(ib_project* project, ib_build_mode mode);
ib_status ib_project_clean(ib_project* project);
ib_status ib_project_write_compile_commands(ib_project* project, const char* out_path);
ib_status ib_project_write_compile_commands_for_mode(ib_project* project, ib_build_mode mode, const char* out_path);

const char* ib_version(void);

#ifdef __cplusplus
}
#endif

#ifdef INCLUDEBUILD_IMPLEMENTATION

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
  #include <direct.h>
  #include <io.h>
  #define IB_PATH_SEPARATOR '\\'
  #if defined(_MSC_VER)
    #define popen  _popen
    #define pclose _pclose
  #endif
#else
  #include <dirent.h>
  #include <unistd.h>
  #include <sys/wait.h>
  extern FILE* popen(const char* command, const char* mode);
  extern int pclose(FILE* stream);
  #define IB_PATH_SEPARATOR '/'
#endif

typedef struct {
    char** items;
    size_t count;
    size_t capacity;
} ib_strvec;

typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} ib_builder;

typedef struct {
    ib_diag_level level;
    ib_status status;
    char* message;
} ib_diagnostic;

typedef struct {
    ib_diagnostic* items;
    size_t count;
    size_t capacity;
} ib_diagvec;

struct ib_context {
    ib_diagvec diagnostics;
    ib_status last_status;
    char* last_message;
    ib_diag_level log_level;
    bool verbose;
    bool color_output;
};

struct ib_target {
    ib_project* project;
    char* name;
    ib_target_kind kind;
    char* entry_source;
    ib_strvec sources;
    ib_strvec include_dirs;
    ib_strvec cflags;
    ib_strvec link_flags;
};

struct ib_project {
    ib_context* ctx;
    char* root_dir;
    char* output_dir;
    char* state_dir;
    char* compiler_c;
    char* compiler_cxx;
    char* archiver;
    char* debug_cflags;
    char* debug_cxxflags;
    char* release_cflags;
    char* release_cxxflags;
    char* custom_cflags;
    char* custom_cxxflags;
    ib_strvec include_dirs;
    ib_strvec shared_sources;
    ib_target** targets;
    size_t target_count;
    size_t target_capacity;
};

typedef struct {
    ib_target* target;
    char* source_path;
    char* source_arg;
    bool is_cpp;
    char* object_path;
    char* object_arg;
    char* depfile_path;
    char* depfile_arg;
    char* manifest_path;
    char* command;
    unsigned long long command_hash;
} ib_compile_action;

typedef struct {
    ib_target* target;
    char* output_path;
    char* output_arg;
    char* manifest_path;
    char* command;
    unsigned long long command_hash;
    size_t compile_index;
    size_t compile_count;
} ib_link_action;

typedef struct {
    ib_compile_action* compile_actions;
    size_t compile_count;
    size_t compile_capacity;
    ib_link_action* link_actions;
    size_t link_count;
    size_t link_capacity;
} ib_build_plan;

static char* ib_strdup_internal(const char* text);
static bool ib_status_failed(ib_status status);
static bool ib_build_mode_valid(ib_build_mode mode);
static bool ib_stream_is_tty(FILE* stream);
static void ib_context_emit(ib_context* ctx, ib_diag_level level, ib_status status, const char* fmt, ...);
static void ib_strvec_init(ib_strvec* vec);
static void ib_strvec_free(ib_strvec* vec);
static bool ib_strvec_push_owned(ib_strvec* vec, char* value);
static bool ib_strvec_push_copy(ib_strvec* vec, const char* value);
static bool ib_strvec_push_unique_owned(ib_strvec* vec, char* value);
static bool ib_strvec_push_unique_copy(ib_strvec* vec, const char* value);
static bool ib_strvec_contains(const ib_strvec* vec, const char* value);
static void ib_strvec_sort(ib_strvec* vec);
static bool ib_builder_append(ib_builder* builder, const char* text);
static bool ib_builder_appendn(ib_builder* builder, const char* text, size_t length);
static bool ib_builder_append_char(ib_builder* builder, char ch);
static bool ib_builder_appendf(ib_builder* builder, const char* fmt, ...);
static char* ib_builder_take(ib_builder* builder);
static void ib_builder_free(ib_builder* builder);
static bool ib_char_is_sep(char ch);
static bool ib_is_absolute_path(const char* path);
static char* ib_get_cwd(ib_context* ctx);
static char* ib_path_join(ib_context* ctx, const char* a, const char* b);
static char* ib_path_normalize(ib_context* ctx, const char* base_abs, const char* input);
static char* ib_path_dirname(ib_context* ctx, const char* path);
static const char* ib_basename_ptr(const char* path);
static bool ib_path_exists(const char* path);
static bool ib_is_dir(const char* path);
static unsigned long long ib_file_mtime(const char* path);
static bool ib_path_equal(const char* a, const char* b);
static bool ib_path_is_within(const char* root, const char* path);
static char* ib_path_relative_to_root(ib_context* ctx, const char* root, const char* path);
static char* ib_path_display(ib_context* ctx, const char* root, const char* path);
static char* ib_path_append_suffix(ib_context* ctx, const char* path, const char* suffix);
static bool ib_has_source_extension(const char* path);
static bool ib_is_cpp_source(const char* path);
static bool ib_should_skip_source_name(const char* name);
static bool ib_should_skip_dir_name(const char* name);
static bool ib_target_name_is_safe(const char* name);
static ib_status ib_project_validate_state_dir(ib_project* project, const char* path);
static ib_status ib_project_validate_output_dir(ib_project* project, const char* path);
static ib_status ib_remove_recursive(ib_project* project, const char* path);
static ib_status ib_mkdirs(ib_project* project, const char* dir);
static ib_status ib_mkdirs_for_file(ib_project* project, const char* path);
static ib_status ib_collect_sources(ib_project* project, const char* dir, bool recursive, ib_strvec* out);
static unsigned long long ib_hash_string(const char* text);
static const char* ib_mode_name(ib_build_mode mode);
static const char* ib_project_mode_flags(const ib_project* project, bool is_cpp, ib_build_mode mode);
static ib_status ib_set_owned_string(ib_context* ctx, char** dst, const char* value);
static ib_status ib_normalize_existing_source(ib_project* project, const char* input, char** out);
static ib_status ib_normalize_existing_dir(ib_project* project, const char* input, char** out);
static ib_status ib_collect_target_sources(ib_project* project, ib_target* target, ib_strvec* out_sources);
static char* ib_source_object_relpath(ib_project* project, const char* source_path);
static char* ib_target_output_filename(ib_context* ctx, const ib_target* target);
static ib_status ib_target_output_path(ib_project* project, const ib_target* target, char** out_abs, char** out_arg);
static char* ib_shell_quote(ib_context* ctx, const char* text);
static ib_status ib_build_compile_command_for_mode(ib_project* project, ib_target* target, ib_build_mode mode, const char* source_arg, bool is_cpp, const char* object_arg, const char* depfile_arg, char** out_command);
static ib_status ib_build_link_command(ib_project* project, ib_build_plan* plan, ib_target* target, const char* output_arg, size_t compile_index, size_t compile_count, char** out_command);
static ib_status ib_plan_target(ib_project* project, ib_build_mode mode, ib_target* target, ib_build_plan* plan);
static ib_status ib_make_plan(ib_project* project, ib_build_mode mode, ib_build_plan* plan);
static bool ib_read_manifest_hash(const char* path, unsigned long long* out_hash);
static ib_status ib_write_manifest_hash(ib_project* project, const char* path, unsigned long long hash);
static ib_status ib_read_depfile(ib_project* project, const char* depfile_path, ib_strvec* deps);
static ib_status ib_run_command(ib_project* project, const char* command, int* exit_code, char** output);
static bool ib_compile_action_needs_rebuild(ib_project* project, const ib_compile_action* action);
static bool ib_link_action_needs_rebuild(ib_project* project, const ib_build_plan* plan, const ib_link_action* action);
static ib_status ib_execute_compile(ib_project* project, const ib_compile_action* action, bool* rebuilt);
static ib_status ib_execute_link(ib_project* project, const ib_build_plan* plan, const ib_link_action* action, bool* relinked);
static ib_status ib_write_json_string(FILE* fp, const char* text);
static void ib_build_plan_free(ib_build_plan* plan);

static char* ib_strdup_internal(const char* text) {
    size_t len;
    char* copy;
    if (!text) {
        text = "";
    }
    len = strlen(text);
    copy = (char*)malloc(len + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, text, len + 1);
    return copy;
}

static bool ib_status_failed(ib_status status) {
    return status != IB_OK;
}

static bool ib_build_mode_valid(ib_build_mode mode) {
    return mode == IB_MODE_DEBUG || mode == IB_MODE_RELEASE || mode == IB_MODE_CUSTOM;
}

static bool ib_stream_is_tty(FILE* stream) {
    if (!stream) {
        return false;
    }
#ifdef _WIN32
    return _isatty(_fileno(stream)) != 0;
#else
    return isatty(fileno(stream)) != 0;
#endif
}

static const char* ib_diag_prefix(ib_diag_level level) {
    switch (level) {
        case IB_DIAG_ERROR: return "[ERROR] ";
        case IB_DIAG_WARN: return "[WARN]  ";
        case IB_DIAG_INFO: return "[INFO]  ";
        case IB_DIAG_DEBUG: return "[DEBUG] ";
    }
    return "[INFO]  ";
}

static const char* ib_diag_color(ib_diag_level level) {
    switch (level) {
        case IB_DIAG_ERROR: return "\x1b[31m";
        case IB_DIAG_WARN:  return "\x1b[33m";
        case IB_DIAG_INFO:  return "\x1b[32m";
        case IB_DIAG_DEBUG: return "\x1b[36m";
    }
    return "";
}

static void ib_context_emit(ib_context* ctx, ib_diag_level level, ib_status status, const char* fmt, ...) {
    va_list args;
    va_list copy;
    int needed;
    char* message;
    FILE* out;

    if (!ctx || !fmt) {
        return;
    }

    va_start(args, fmt);
    va_copy(copy, args);
    needed = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (needed < 0) {
        va_end(args);
        return;
    }

    message = (char*)malloc((size_t)needed + 1);
    if (!message) {
        va_end(args);
        return;
    }
    vsnprintf(message, (size_t)needed + 1, fmt, args);
    va_end(args);

    if (ctx->diagnostics.count == ctx->diagnostics.capacity) {
        size_t next_cap = ctx->diagnostics.capacity ? ctx->diagnostics.capacity * 2 : IB_INITIAL_LIST_CAPACITY;
        ib_diagnostic* next = (ib_diagnostic*)realloc(ctx->diagnostics.items, next_cap * sizeof(*next));
        if (next) {
            ctx->diagnostics.items = next;
            ctx->diagnostics.capacity = next_cap;
        }
    }

    if (ctx->diagnostics.count < ctx->diagnostics.capacity) {
        ib_diagnostic* diag = &ctx->diagnostics.items[ctx->diagnostics.count++];
        diag->level = level;
        diag->status = status;
        diag->message = ib_strdup_internal(message);
    }

    ctx->last_status = status;
    free(ctx->last_message);
    ctx->last_message = ib_strdup_internal(message);

    if (level <= ctx->log_level) {
        out = (level == IB_DIAG_ERROR) ? stderr : stdout;
        if (ctx->color_output) {
            fprintf(out, "%s%s\x1b[0m%s\n", ib_diag_color(level), ib_diag_prefix(level), message);
        } else {
            fprintf(out, "%s%s\n", ib_diag_prefix(level), message);
        }
    }

    free(message);
}

static bool ib_builder_reserve(ib_builder* builder, size_t extra) {
    size_t needed = builder->length + extra + 1;
    size_t capacity;
    char* next;
    if (needed <= builder->capacity) {
        return true;
    }
    capacity = builder->capacity ? builder->capacity : 64;
    while (capacity < needed) {
        capacity *= 2;
    }
    next = (char*)realloc(builder->data, capacity);
    if (!next) {
        return false;
    }
    builder->data = next;
    builder->capacity = capacity;
    return true;
}

static bool ib_builder_appendn(ib_builder* builder, const char* text, size_t length) {
    if (!length) {
        return true;
    }
    if (!ib_builder_reserve(builder, length)) {
        return false;
    }
    memcpy(builder->data + builder->length, text, length);
    builder->length += length;
    builder->data[builder->length] = '\0';
    return true;
}

static bool ib_builder_append(ib_builder* builder, const char* text) {
    return ib_builder_appendn(builder, text ? text : "", text ? strlen(text) : 0);
}

static bool ib_builder_append_char(ib_builder* builder, char ch) {
    return ib_builder_appendn(builder, &ch, 1);
}

static bool ib_builder_appendf(ib_builder* builder, const char* fmt, ...) {
    va_list args;
    va_list copy;
    int needed;
    va_start(args, fmt);
    va_copy(copy, args);
    needed = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (needed < 0 || !ib_builder_reserve(builder, (size_t)needed)) {
        va_end(args);
        return false;
    }
    vsnprintf(builder->data + builder->length, builder->capacity - builder->length, fmt, args);
    builder->length += (size_t)needed;
    va_end(args);
    return true;
}

static char* ib_builder_take(ib_builder* builder) {
    char* data = builder->data;
    if (!data) {
        return ib_strdup_internal("");
    }
    builder->data = NULL;
    builder->length = 0;
    builder->capacity = 0;
    return data;
}

static void ib_builder_free(ib_builder* builder) {
    if (!builder) {
        return;
    }
    free(builder->data);
    builder->data = NULL;
    builder->length = 0;
    builder->capacity = 0;
}

static void ib_strvec_init(ib_strvec* vec) {
    vec->items = NULL;
    vec->count = 0;
    vec->capacity = 0;
}

static bool ib_strvec_reserve(ib_strvec* vec, size_t needed) {
    size_t capacity;
    char** next;
    if (needed <= vec->capacity) {
        return true;
    }
    capacity = vec->capacity ? vec->capacity : IB_INITIAL_LIST_CAPACITY;
    while (capacity < needed) {
        capacity *= 2;
    }
    next = (char**)realloc(vec->items, capacity * sizeof(*next));
    if (!next) {
        return false;
    }
    vec->items = next;
    vec->capacity = capacity;
    return true;
}
static bool ib_strvec_push_owned(ib_strvec* vec, char* value) {
    if (!ib_strvec_reserve(vec, vec->count + 1)) {
        return false;
    }
    vec->items[vec->count++] = value;
    return true;
}

static bool ib_strvec_push_copy(ib_strvec* vec, const char* value) {
    char* copy = ib_strdup_internal(value);
    if (!copy) {
        return false;
    }
    if (!ib_strvec_push_owned(vec, copy)) {
        free(copy);
        return false;
    }
    return true;
}

static bool ib_strvec_contains(const ib_strvec* vec, const char* value) {
    size_t i;
    for (i = 0; vec && i < vec->count; ++i) {
        if (strcmp(vec->items[i], value) == 0) {
            return true;
        }
    }
    return false;
}

static bool ib_strvec_push_unique_owned(ib_strvec* vec, char* value) {
    if (ib_strvec_contains(vec, value)) {
        free(value);
        return true;
    }
    return ib_strvec_push_owned(vec, value);
}

static bool ib_strvec_push_unique_copy(ib_strvec* vec, const char* value) {
    if (ib_strvec_contains(vec, value)) {
        return true;
    }
    return ib_strvec_push_copy(vec, value);
}

static void ib_strvec_free(ib_strvec* vec) {
    size_t i;
    if (!vec) {
        return;
    }
    for (i = 0; i < vec->count; ++i) {
        free(vec->items[i]);
    }
    free(vec->items);
    vec->items = NULL;
    vec->count = 0;
    vec->capacity = 0;
}

static int ib_qsort_strcmp(const void* lhs, const void* rhs) {
    const char* const* a = (const char* const*)lhs;
    const char* const* b = (const char* const*)rhs;
    return strcmp(*a, *b);
}

static void ib_strvec_sort(ib_strvec* vec) {
    if (vec && vec->count > 1) {
        qsort(vec->items, vec->count, sizeof(*vec->items), ib_qsort_strcmp);
    }
}

static bool ib_char_is_sep(char ch) {
    return ch == '/' || ch == '\\';
}

static bool ib_is_absolute_path(const char* path) {
    if (!path || !path[0]) {
        return false;
    }
    if (ib_char_is_sep(path[0])) {
        return true;
    }
    return isalpha((unsigned char)path[0]) && path[1] == ':' && ib_char_is_sep(path[2]);
}

static char* ib_get_cwd(ib_context* ctx) {
#ifdef _WIN32
    DWORD needed = GetCurrentDirectoryA(0, NULL);
    char* buffer;
    if (needed == 0) {
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to get current working directory");
        return NULL;
    }
    buffer = (char*)malloc((size_t)needed + 1);
    if (!buffer) {
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while reading current working directory");
        return NULL;
    }
    if (GetCurrentDirectoryA(needed + 1, buffer) == 0) {
        free(buffer);
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to get current working directory");
        return NULL;
    }
    return buffer;
#else
    size_t size = 256;
    char* buffer = NULL;
    while (1) {
        char* next = (char*)realloc(buffer, size);
        if (!next) {
            free(buffer);
            ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while reading current working directory");
            return NULL;
        }
        buffer = next;
        if (getcwd(buffer, size) != NULL) {
            return buffer;
        }
        if (errno != ERANGE) {
            free(buffer);
            ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to get current working directory");
            return NULL;
        }
        size *= 2;
    }
#endif
}

static char* ib_path_root(const char* path, size_t* prefix_len) {
    char* root;
    if (isalpha((unsigned char)path[0]) && path[1] == ':' && ib_char_is_sep(path[2])) {
        root = (char*)malloc(4);
        if (!root) {
            return NULL;
        }
        root[0] = path[0];
        root[1] = ':';
        root[2] = IB_PATH_SEPARATOR;
        root[3] = '\0';
        *prefix_len = 3;
        return root;
    }
    if (ib_char_is_sep(path[0])) {
        root = (char*)malloc(2);
        if (!root) {
            return NULL;
        }
        root[0] = IB_PATH_SEPARATOR;
        root[1] = '\0';
        *prefix_len = 1;
        return root;
    }
    *prefix_len = 0;
    return ib_strdup_internal("");
}

static char* ib_path_join(ib_context* ctx, const char* a, const char* b) {
    ib_builder builder;
    size_t len;
    memset(&builder, 0, sizeof(builder));
    if (!a || !a[0]) {
        return ib_strdup_internal(b ? b : "");
    }
    if (!b || !b[0]) {
        return ib_strdup_internal(a);
    }
    len = strlen(a);
    if (!ib_builder_append(&builder, a)) {
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while joining paths");
        return NULL;
    }
    if (a[len - 1] != IB_PATH_SEPARATOR) {
        if (!ib_builder_append_char(&builder, IB_PATH_SEPARATOR)) {
            ib_builder_free(&builder);
            ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while joining paths");
            return NULL;
        }
    }
    while (*b && ib_char_is_sep(*b)) {
        ++b;
    }
    if (!ib_builder_append(&builder, b)) {
        ib_builder_free(&builder);
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while joining paths");
        return NULL;
    }
    return ib_builder_take(&builder);
}

static char* ib_path_normalize(ib_context* ctx, const char* base_abs, const char* input) {
    char* working;
    char* absolute = NULL;
    char* root;
    size_t root_len = 0;
    ib_strvec segments;
    ib_builder result;
    size_t i;
    size_t start;

    if (!input || !input[0]) {
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Expected a non-empty path");
        return NULL;
    }

    memset(&result, 0, sizeof(result));
    ib_strvec_init(&segments);
    working = ib_strdup_internal(input);
    if (!working) {
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while normalizing path");
        return NULL;
    }
    for (i = 0; working[i]; ++i) {
        if (ib_char_is_sep(working[i])) {
            working[i] = IB_PATH_SEPARATOR;
        }
    }
    if (ib_is_absolute_path(working)) {
        absolute = working;
        working = NULL;
    } else {
        if (!base_abs || !base_abs[0]) {
            free(working);
            ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Cannot resolve relative path '%s' without a base directory", input);
            return NULL;
        }
        absolute = ib_path_join(ctx, base_abs, working);
        free(working);
        working = NULL;
        if (!absolute) {
            return NULL;
        }
    }

    root = ib_path_root(absolute, &root_len);
    if (!root) {
        free(absolute);
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while normalizing path");
        return NULL;
    }

    start = root_len;
    while (absolute[start]) {
        size_t end;
        size_t len;
        char* segment;
        while (absolute[start] == IB_PATH_SEPARATOR) {
            ++start;
        }
        if (!absolute[start]) {
            break;
        }
        end = start;
        while (absolute[end] && absolute[end] != IB_PATH_SEPARATOR) {
            ++end;
        }
        len = end - start;
        segment = (char*)malloc(len + 1);
        if (!segment) {
            ib_strvec_free(&segments);
            free(root);
            free(absolute);
            ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while normalizing path");
            return NULL;
        }
        memcpy(segment, absolute + start, len);
        segment[len] = '\0';
        if (strcmp(segment, ".") == 0) {
            free(segment);
        } else if (strcmp(segment, "..") == 0) {
            free(segment);
            if (segments.count > 0) {
                free(segments.items[--segments.count]);
            }
        } else if (!ib_strvec_push_owned(&segments, segment)) {
            free(segment);
            ib_strvec_free(&segments);
            free(root);
            free(absolute);
            ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while normalizing path");
            return NULL;
        }
        start = end;
    }

    if (!ib_builder_append(&result, root)) {
        ib_builder_free(&result);
        ib_strvec_free(&segments);
        free(root);
        free(absolute);
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while normalizing path");
        return NULL;
    }
    for (i = 0; i < segments.count; ++i) {
        if (i > 0 || (root[0] && root[strlen(root) - 1] != IB_PATH_SEPARATOR)) {
            if (!ib_builder_append_char(&result, IB_PATH_SEPARATOR)) {
                ib_builder_free(&result);
                ib_strvec_free(&segments);
                free(root);
                free(absolute);
                ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while normalizing path");
                return NULL;
            }
        }
        if (!ib_builder_append(&result, segments.items[i])) {
            ib_builder_free(&result);
            ib_strvec_free(&segments);
            free(root);
            free(absolute);
            ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while normalizing path");
            return NULL;
        }
    }

    ib_strvec_free(&segments);
    free(root);
    free(absolute);
    return ib_builder_take(&result);
}

static char* ib_path_dirname(ib_context* ctx, const char* path) {
    const char* last = strrchr(path ? path : "", IB_PATH_SEPARATOR);
    char* out;
    size_t len;
    (void)ctx;
    if (!path || !path[0] || !last) {
        return ib_strdup_internal(".");
    }
    if (last == path) {
        return ib_strdup_internal("/");
    }
    len = (size_t)(last - path);
    out = (char*)malloc(len + 1);
    if (!out) {
        return NULL;
    }
    memcpy(out, path, len);
    out[len] = '\0';
    return out;
}

static const char* ib_basename_ptr(const char* path) {
    const char* a = strrchr(path ? path : "", '/');
    const char* b = strrchr(path ? path : "", '\\');
    const char* last = a > b ? a : b;
    return last ? last + 1 : (path ? path : "");
}

static bool ib_path_exists(const char* path) {
    struct stat st;
    return path && stat(path, &st) == 0;
}

static bool ib_is_dir(const char* path) {
    struct stat st;
    if (!path || stat(path, &st) != 0) {
        return false;
    }
#ifdef _WIN32
    return (st.st_mode & _S_IFDIR) != 0;
#else
    return S_ISDIR(st.st_mode);
#endif
}

static unsigned long long ib_file_mtime(const char* path) {
    struct stat st;
    if (!path || stat(path, &st) != 0) {
        return 0;
    }
#ifdef _WIN32
    return (unsigned long long)st.st_mtime * 1000000000ULL;
#elif defined(__APPLE__)
    return ((unsigned long long)st.st_mtimespec.tv_sec * 1000000000ULL) +
           (unsigned long long)st.st_mtimespec.tv_nsec;
#elif defined(__linux__)
    return ((unsigned long long)st.st_mtim.tv_sec * 1000000000ULL) +
           (unsigned long long)st.st_mtim.tv_nsec;
#else
    return (unsigned long long)st.st_mtime * 1000000000ULL;
#endif
}

static bool ib_path_component_equal(const char* a, const char* b, size_t len) {
#ifdef _WIN32
    size_t i;
    for (i = 0; i < len; ++i) {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) {
            return false;
        }
    }
    return true;
#else
    return strncmp(a, b, len) == 0;
#endif
}

static bool ib_path_equal(const char* a, const char* b) {
    size_t len;
    if (!a || !b) {
        return false;
    }
    len = strlen(a);
    return len == strlen(b) && ib_path_component_equal(a, b, len);
}

static bool ib_path_is_within(const char* root, const char* path) {
    size_t root_len;
    size_t path_len;
    if (!root || !path) {
        return false;
    }
    root_len = strlen(root);
    path_len = strlen(path);
    if (root_len == 0 || root_len > path_len || !ib_path_component_equal(root, path, root_len)) {
        return false;
    }
    return path[root_len] == '\0' || ib_char_is_sep(path[root_len]);
}

static char* ib_path_relative_to_root(ib_context* ctx, const char* root, const char* path) {
    const char* start;
    (void)ctx;
    if (!ib_path_is_within(root, path)) {
        return NULL;
    }
    start = path + strlen(root);
    while (*start && ib_char_is_sep(*start)) {
        ++start;
    }
    return ib_strdup_internal(*start ? start : ".");
}

static char* ib_path_display(ib_context* ctx, const char* root, const char* path) {
    char* rel = ib_path_relative_to_root(ctx, root, path);
    return rel ? rel : ib_strdup_internal(path);
}

static char* ib_path_append_suffix(ib_context* ctx, const char* path, const char* suffix) {
    ib_builder builder;
    memset(&builder, 0, sizeof(builder));
    if (!ib_builder_append(&builder, path) || !ib_builder_append(&builder, suffix)) {
        ib_builder_free(&builder);
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while extending path");
        return NULL;
    }
    return ib_builder_take(&builder);
}

static bool ib_has_source_extension(const char* path) {
    const char* ext = strrchr(path, '.');
    return ext && (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 || strcmp(ext, ".cc") == 0 || strcmp(ext, ".cxx") == 0);
}

static bool ib_is_cpp_source(const char* path) {
    const char* ext = strrchr(path, '.');
    return ext && (strcmp(ext, ".cpp") == 0 || strcmp(ext, ".cc") == 0 || strcmp(ext, ".cxx") == 0);
}

static bool ib_should_skip_source_name(const char* name) {
    return strcmp(name, "build.c") == 0 || strcmp(name, "build.cpp") == 0;
}

static bool ib_should_skip_dir_name(const char* name) {
    return strcmp(name, ".git") == 0 || strcmp(name, ".svn") == 0 || strcmp(name, ".idea") == 0 ||
           strcmp(name, ".vscode") == 0 || strcmp(name, ".ibuild") == 0 || strcmp(name, "node_modules") == 0;
}

static bool ib_target_name_is_windows_reserved(const char* name, size_t len) {
    char upper[8];
    size_t i;
    if (len == 3) {
        for (i = 0; i < len; ++i) {
            upper[i] = (char)toupper((unsigned char)name[i]);
        }
        upper[len] = '\0';
        return strcmp(upper, "CON") == 0 || strcmp(upper, "PRN") == 0 || strcmp(upper, "AUX") == 0 || strcmp(upper, "NUL") == 0;
    }
    if (len == 4) {
        for (i = 0; i < len; ++i) {
            upper[i] = (char)toupper((unsigned char)name[i]);
        }
        upper[len] = '\0';
        return (strncmp(upper, "COM", 3) == 0 || strncmp(upper, "LPT", 3) == 0) && upper[3] >= '1' && upper[3] <= '9';
    }
    return false;
}

static bool ib_target_name_is_safe(const char* name) {
    size_t len;
    size_t base_len;
    const char* dot;
    const char* invalid = "<>:\"/\\|?*";
    const char* p;
    if (!name || !name[0]) {
        return false;
    }
    len = strlen(name);
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 || name[0] == '.' || name[len - 1] == '.' || isspace((unsigned char)name[len - 1])) {
        return false;
    }
    for (p = name; *p; ++p) {
        if ((unsigned char)*p < 32 || strchr(invalid, *p) != NULL) {
            return false;
        }
    }
    dot = strchr(name, '.');
    base_len = dot ? (size_t)(dot - name) : len;
    if (base_len == 0 || ib_target_name_is_windows_reserved(name, base_len)) {
        return false;
    }
    return true;
}

static int ib_chdir_to(const char* path) {
#ifdef _WIN32
    return _chdir(path);
#else
    return chdir(path);
#endif
}

static int ib_remove_dir_single(const char* path) {
#ifdef _WIN32
    return _rmdir(path);
#else
    return rmdir(path);
#endif
}

static ib_status ib_mkdirs(ib_project* project, const char* dir) {
    char* normalized;
    char* cursor;
    if (!dir || !dir[0]) {
        return IB_OK;
    }
    normalized = ib_path_normalize(project->ctx, project->root_dir, dir);
    if (!normalized) {
        return project->ctx->last_status;
    }
    if (ib_path_exists(normalized)) {
        free(normalized);
        return IB_OK;
    }
    cursor = normalized;
    while (*cursor) {
        if (*cursor == ':' && cursor[1] == IB_PATH_SEPARATOR) {
            cursor += 2;
            continue;
        }
        if (*cursor == IB_PATH_SEPARATOR && cursor != normalized) {
            char saved = *cursor;
            *cursor = '\0';
            if (normalized[0] && !ib_path_exists(normalized)) {
#ifdef _WIN32
                if (_mkdir(normalized) != 0 && errno != EEXIST) {
#else
                if (mkdir(normalized, 0755) != 0 && errno != EEXIST) {
#endif
                    ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to create directory '%s': %s", normalized, strerror(errno));
                    *cursor = saved;
                    free(normalized);
                    return IB_ERR_IO;
                }
            }
            *cursor = saved;
        }
        ++cursor;
    }
    if (!ib_path_exists(normalized)) {
#ifdef _WIN32
        if (_mkdir(normalized) != 0 && errno != EEXIST) {
#else
        if (mkdir(normalized, 0755) != 0 && errno != EEXIST) {
#endif
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to create directory '%s': %s", normalized, strerror(errno));
            free(normalized);
            return IB_ERR_IO;
        }
    }
    free(normalized);
    return IB_OK;
}

static ib_status ib_mkdirs_for_file(ib_project* project, const char* path) {
    char* dir = ib_path_dirname(project->ctx, path);
    ib_status status;
    if (!dir) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while preparing directories for '%s'", path);
        return IB_ERR_NOMEM;
    }
    status = ib_mkdirs(project, dir);
    free(dir);
    return status;
}

static ib_status ib_remove_recursive(ib_project* project, const char* path) {
    if (!ib_path_exists(path)) {
        return IB_OK;
    }
    if (!ib_is_dir(path)) {
        if (remove(path) != 0 && errno != ENOENT) {
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to remove '%s': %s", path, strerror(errno));
            return IB_ERR_IO;
        }
        return IB_OK;
    }
#ifdef _WIN32
    {
        WIN32_FIND_DATAA fd;
        HANDLE handle;
        char* pattern = ib_path_join(project->ctx, path, "*");
        if (!pattern) {
            return project->ctx->last_status;
        }
        handle = FindFirstFileA(pattern, &fd);
        free(pattern);
        if (handle != INVALID_HANDLE_VALUE) {
            do {
                char* child;
                if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) {
                    continue;
                }
                child = ib_path_join(project->ctx, path, fd.cFileName);
                if (!child) {
                    FindClose(handle);
                    return project->ctx->last_status;
                }
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    ib_status status = ib_remove_recursive(project, child);
                    free(child);
                    if (ib_status_failed(status)) {
                        FindClose(handle);
                        return status;
                    }
                } else {
                    if (DeleteFileA(child) == 0) {
                        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to remove file '%s'", child);
                        free(child);
                        FindClose(handle);
                        return IB_ERR_IO;
                    }
                    free(child);
                }
            } while (FindNextFileA(handle, &fd));
            FindClose(handle);
        }
    }
#else
    {
        DIR* dir = opendir(path);
        struct dirent* ent;
        if (!dir) {
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to open directory '%s': %s", path, strerror(errno));
            return IB_ERR_IO;
        }
        while ((ent = readdir(dir)) != NULL) {
            char* child;
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            child = ib_path_join(project->ctx, path, ent->d_name);
            if (!child) {
                closedir(dir);
                return project->ctx->last_status;
            }
            if (ib_is_dir(child)) {
                ib_status status = ib_remove_recursive(project, child);
                free(child);
                if (ib_status_failed(status)) {
                    closedir(dir);
                    return status;
                }
            } else {
                if (remove(child) != 0 && errno != ENOENT) {
                    ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to remove file '%s': %s", child, strerror(errno));
                    free(child);
                    closedir(dir);
                    return IB_ERR_IO;
                }
                free(child);
            }
        }
        closedir(dir);
    }
#endif
    if (ib_remove_dir_single(path) != 0 && errno != ENOENT) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to remove directory '%s': %s", path, strerror(errno));
        return IB_ERR_IO;
    }
    return IB_OK;
}

static ib_status ib_collect_sources(ib_project* project, const char* dir, bool recursive, ib_strvec* out) {
#ifdef _WIN32
    WIN32_FIND_DATAA fd;
    HANDLE handle;
    char* pattern = ib_path_join(project->ctx, dir, "*");
    if (!pattern) {
        return project->ctx->last_status;
    }
    handle = FindFirstFileA(pattern, &fd);
    free(pattern);
    if (handle == INVALID_HANDLE_VALUE) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to scan directory '%s'", dir);
        return IB_ERR_IO;
    }
    do {
        char* child;
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0 || ib_should_skip_dir_name(fd.cFileName)) {
            continue;
        }
        child = ib_path_join(project->ctx, dir, fd.cFileName);
        if (!child) {
            FindClose(handle);
            return project->ctx->last_status;
        }
        if (ib_path_is_within(project->state_dir, child) || (strcmp(project->output_dir, project->root_dir) != 0 && ib_path_is_within(project->output_dir, child))) {
            free(child);
            continue;
        }
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (recursive) {
                ib_status status = ib_collect_sources(project, child, recursive, out);
                free(child);
                if (ib_status_failed(status)) {
                    FindClose(handle);
                    return status;
                }
            } else {
                free(child);
            }
        } else if (ib_has_source_extension(fd.cFileName) && !ib_should_skip_source_name(fd.cFileName)) {
            if (!ib_strvec_push_unique_owned(out, child)) {
                free(child);
                FindClose(handle);
                ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while collecting sources");
                return IB_ERR_NOMEM;
            }
        } else {
            free(child);
        }
    } while (FindNextFileA(handle, &fd));
    FindClose(handle);
#else
    DIR* handle = opendir(dir);
    struct dirent* ent;
    if (!handle) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to scan directory '%s': %s", dir, strerror(errno));
        return IB_ERR_IO;
    }
    while ((ent = readdir(handle)) != NULL) {
        char* child;
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0 || ib_should_skip_dir_name(ent->d_name)) {
            continue;
        }
        child = ib_path_join(project->ctx, dir, ent->d_name);
        if (!child) {
            closedir(handle);
            return project->ctx->last_status;
        }
        if (ib_path_is_within(project->state_dir, child) || (strcmp(project->output_dir, project->root_dir) != 0 && ib_path_is_within(project->output_dir, child))) {
            free(child);
            continue;
        }
        if (ib_is_dir(child)) {
            if (recursive) {
                ib_status status = ib_collect_sources(project, child, recursive, out);
                free(child);
                if (ib_status_failed(status)) {
                    closedir(handle);
                    return status;
                }
            } else {
                free(child);
            }
        } else if (ib_has_source_extension(ent->d_name) && !ib_should_skip_source_name(ent->d_name)) {
            if (!ib_strvec_push_unique_owned(out, child)) {
                free(child);
                closedir(handle);
                ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while collecting sources");
                return IB_ERR_NOMEM;
            }
        } else {
            free(child);
        }
    }
    closedir(handle);
#endif
    return IB_OK;
}
static unsigned long long ib_hash_string(const char* text) {
    unsigned long long hash = 1469598103934665603ULL;
    while (text && *text) {
        hash ^= (unsigned char)*text++;
        hash *= 1099511628211ULL;
    }
    return hash;
}

static const char* ib_mode_name(ib_build_mode mode) {
    switch (mode) {
        case IB_MODE_DEBUG: return "debug";
        case IB_MODE_RELEASE: return "release";
        case IB_MODE_CUSTOM: return "custom";
    }
    return "custom";
}

static const char* ib_project_mode_flags(const ib_project* project, bool is_cpp, ib_build_mode mode) {
    switch (mode) {
        case IB_MODE_DEBUG: return is_cpp ? project->debug_cxxflags : project->debug_cflags;
        case IB_MODE_RELEASE: return is_cpp ? project->release_cxxflags : project->release_cflags;
        case IB_MODE_CUSTOM: return is_cpp ? project->custom_cxxflags : project->custom_cflags;
    }
    return "";
}

static ib_status ib_set_owned_string(ib_context* ctx, char** dst, const char* value) {
    char* copy = ib_strdup_internal(value ? value : "");
    if (!copy) {
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while updating string value");
        return IB_ERR_NOMEM;
    }
    free(*dst);
    *dst = copy;
    return IB_OK;
}

static ib_status ib_project_validate_state_dir(ib_project* project, const char* path) {
    if (!project || !path || !path[0]) {
        if (project) {
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "State directory must be a non-empty path");
        }
        return IB_ERR_INVALID;
    }
    if (ib_path_equal(path, project->root_dir) || ib_path_is_within(path, project->root_dir)) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "State directory '%s' must not be the project root or one of its parent directories", path);
        return IB_ERR_INVALID;
    }
    if (project->output_dir && ib_path_equal(path, project->output_dir)) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "State directory '%s' must not be the output directory", path);
        return IB_ERR_INVALID;
    }
    return IB_OK;
}

static ib_status ib_project_validate_output_dir(ib_project* project, const char* path) {
    if (!project || !path || !path[0]) {
        if (project) {
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Output directory must be a non-empty path");
        }
        return IB_ERR_INVALID;
    }
    if (project->state_dir && (ib_path_equal(path, project->state_dir) || ib_path_is_within(project->state_dir, path))) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Output directory '%s' must not be inside the state directory", path);
        return IB_ERR_INVALID;
    }
    return IB_OK;
}

static ib_status ib_normalize_existing_source(ib_project* project, const char* input, char** out) {
    char* path = ib_path_normalize(project->ctx, project->root_dir, input);
    if (!path) {
        return project->ctx->last_status;
    }
    if (!ib_path_exists(path)) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOT_FOUND, "Source file '%s' does not exist", input);
        free(path);
        return IB_ERR_NOT_FOUND;
    }
    if (ib_is_dir(path) || !ib_has_source_extension(path)) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Unsupported source path '%s'", input);
        free(path);
        return IB_ERR_INVALID;
    }
    *out = path;
    return IB_OK;
}

static ib_status ib_normalize_existing_dir(ib_project* project, const char* input, char** out) {
    char* path = ib_path_normalize(project->ctx, project->root_dir, input);
    if (!path) {
        return project->ctx->last_status;
    }
    if (!ib_path_exists(path) || !ib_is_dir(path)) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOT_FOUND, "Directory '%s' does not exist", input);
        free(path);
        return IB_ERR_NOT_FOUND;
    }
    *out = path;
    return IB_OK;
}

static ib_status ib_collect_target_sources(ib_project* project, ib_target* target, ib_strvec* out_sources) {
    size_t i;
    ib_strvec_init(out_sources);
    if (target->kind == IB_TARGET_EXECUTABLE && !target->entry_source) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Target '%s' is missing an entry source", target->name);
        return IB_ERR_INVALID;
    }
    if (target->entry_source && !ib_strvec_push_unique_copy(out_sources, target->entry_source)) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while collecting target sources");
        ib_strvec_free(out_sources);
        return IB_ERR_NOMEM;
    }
    for (i = 0; i < project->shared_sources.count; ++i) {
        if (!ib_strvec_push_unique_copy(out_sources, project->shared_sources.items[i])) {
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while collecting target sources");
            ib_strvec_free(out_sources);
            return IB_ERR_NOMEM;
        }
    }
    for (i = 0; i < target->sources.count; ++i) {
        if (!ib_strvec_push_unique_copy(out_sources, target->sources.items[i])) {
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while collecting target sources");
            ib_strvec_free(out_sources);
            return IB_ERR_NOMEM;
        }
    }
    if (out_sources->count == 0) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Target '%s' does not have any sources", target->name);
        ib_strvec_free(out_sources);
        return IB_ERR_INVALID;
    }
    return IB_OK;
}

static char* ib_source_object_relpath(ib_project* project, const char* source_path) {
    char* rel = ib_path_relative_to_root(project->ctx, project->root_dir, source_path);
    if (!rel) {
        ib_builder builder;
        memset(&builder, 0, sizeof(builder));
        if (!ib_builder_append(&builder, "external") ||
            !ib_builder_append_char(&builder, IB_PATH_SEPARATOR) ||
            !ib_builder_appendf(&builder, "%016llx_", ib_hash_string(source_path)) ||
            !ib_builder_append(&builder, ib_basename_ptr(source_path))) {
            ib_builder_free(&builder);
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while mapping source path");
            return NULL;
        }
        rel = ib_builder_take(&builder);
    }
    {
        char* replaced = ib_path_append_suffix(project->ctx, rel, ".o");
        free(rel);
        return replaced;
    }
}

static char* ib_target_output_filename(ib_context* ctx, const ib_target* target) {
    ib_builder builder;
    memset(&builder, 0, sizeof(builder));
    switch (target->kind) {
        case IB_TARGET_EXECUTABLE:
            if (!ib_builder_append(&builder, target->name)) {
                goto oom;
            }
#ifdef _WIN32
            if (!ib_builder_append(&builder, ".exe")) {
                goto oom;
            }
#endif
            break;
        case IB_TARGET_STATIC_LIB:
            if (!ib_builder_append(&builder, "lib") || !ib_builder_append(&builder, target->name) || !ib_builder_append(&builder, ".a")) {
                goto oom;
            }
            break;
        case IB_TARGET_SHARED_LIB:
#ifdef _WIN32
            if (!ib_builder_append(&builder, target->name) || !ib_builder_append(&builder, ".dll")) {
#elif defined(__APPLE__)
            if (!ib_builder_append(&builder, "lib") || !ib_builder_append(&builder, target->name) || !ib_builder_append(&builder, ".dylib")) {
#else
            if (!ib_builder_append(&builder, "lib") || !ib_builder_append(&builder, target->name) || !ib_builder_append(&builder, ".so")) {
#endif
                goto oom;
            }
            break;
    }
    return ib_builder_take(&builder);
oom:
    ib_builder_free(&builder);
    ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while building output file name");
    return NULL;
}

static ib_status ib_target_output_path(ib_project* project, const ib_target* target, char** out_abs, char** out_arg) {
    char* filename = ib_target_output_filename(project->ctx, target);
    char* abs_path;
    char* arg_path;
    if (!filename) {
        return project->ctx->last_status;
    }
    abs_path = ib_path_join(project->ctx, project->output_dir, filename);
    free(filename);
    if (!abs_path) {
        return project->ctx->last_status;
    }
    arg_path = ib_path_display(project->ctx, project->root_dir, abs_path);
    if (!arg_path) {
        free(abs_path);
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while computing output path");
        return IB_ERR_NOMEM;
    }
    *out_abs = abs_path;
    *out_arg = arg_path;
    return IB_OK;
}

static char* ib_shell_quote(ib_context* ctx, const char* text) {
    ib_builder builder;
    const char* p;
    bool needs_quotes = false;
    if (!text || !text[0]) {
        needs_quotes = true;
    }
    for (p = text; !needs_quotes && p && *p; ++p) {
#ifdef _WIN32
        if (isspace((unsigned char)*p) || strchr("\"&|<>()^%!", *p) != NULL) {
#else
        if (isspace((unsigned char)*p) || strchr("'\"\\$`!&|;<>(){}[]*?", *p) != NULL) {
#endif
            needs_quotes = true;
        }
    }
    if (!needs_quotes && text) {
        return ib_strdup_internal(text);
    }
    memset(&builder, 0, sizeof(builder));
#ifdef _WIN32
    if (!ib_builder_append_char(&builder, '"')) {
        goto oom;
    }
    for (p = text; p && *p; ++p) {
        if (*p == '"') {
            if (!ib_builder_append(&builder, "\\\"")) {
                goto oom;
            }
        } else if (*p == '%') {
            if (!ib_builder_append(&builder, "%%")) {
                goto oom;
            }
        } else if (!ib_builder_append_char(&builder, *p)) {
            goto oom;
        }
    }
    if (!ib_builder_append_char(&builder, '"')) {
        goto oom;
    }
#else
    if (!ib_builder_append_char(&builder, '\'')) {
        goto oom;
    }
    for (p = text; p && *p; ++p) {
        if (*p == '\'') {
            if (!ib_builder_append(&builder, "'\"'\"'")) {
                goto oom;
            }
        } else if (!ib_builder_append_char(&builder, *p)) {
            goto oom;
        }
    }
    if (!ib_builder_append_char(&builder, '\'')) {
        goto oom;
    }
#endif
    return ib_builder_take(&builder);
oom:
    ib_builder_free(&builder);
    ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while quoting shell argument");
    return NULL;
}

static ib_status ib_build_compile_command_for_mode(ib_project* project, ib_target* target, ib_build_mode mode, const char* source_arg, bool is_cpp, const char* object_arg, const char* depfile_arg, char** out_command) {
    ib_builder builder;
    char* q_compiler = NULL;
    char* q_depfile = NULL;
    char* q_source = NULL;
    char* q_object = NULL;
    size_t i;
    const char* compiler = is_cpp ? project->compiler_cxx : project->compiler_c;
    const char* mode_flags = ib_project_mode_flags(project, is_cpp, mode);
    memset(&builder, 0, sizeof(builder));
    q_compiler = ib_shell_quote(project->ctx, compiler);
    q_depfile = ib_shell_quote(project->ctx, depfile_arg);
    q_source = ib_shell_quote(project->ctx, source_arg);
    q_object = ib_shell_quote(project->ctx, object_arg);
    if (!q_compiler || !q_depfile || !q_source || !q_object) {
        free(q_compiler); free(q_depfile); free(q_source); free(q_object);
        return project->ctx->last_status;
    }
    if (!ib_builder_append(&builder, q_compiler)) {
        goto oom;
    }
    if (mode_flags && mode_flags[0]) {
        if (!ib_builder_append_char(&builder, ' ') || !ib_builder_append(&builder, mode_flags)) {
            goto oom;
        }
    }
    for (i = 0; i < project->include_dirs.count; ++i) {
        char* q_dir = ib_shell_quote(project->ctx, project->include_dirs.items[i]);
        if (!q_dir || !ib_builder_append(&builder, " -I") || !ib_builder_append(&builder, q_dir)) {
            free(q_dir);
            goto oom;
        }
        free(q_dir);
    }
    for (i = 0; i < target->include_dirs.count; ++i) {
        char* q_dir = ib_shell_quote(project->ctx, target->include_dirs.items[i]);
        if (!q_dir || !ib_builder_append(&builder, " -I") || !ib_builder_append(&builder, q_dir)) {
            free(q_dir);
            goto oom;
        }
        free(q_dir);
    }
#ifndef _WIN32
    if (target->kind == IB_TARGET_SHARED_LIB) {
        if (!ib_builder_append(&builder, " -fPIC")) {
            goto oom;
        }
    }
#endif
    for (i = 0; i < target->cflags.count; ++i) {
        if (!ib_builder_append_char(&builder, ' ') || !ib_builder_append(&builder, target->cflags.items[i])) {
            goto oom;
        }
    }
    if (!ib_builder_append(&builder, " -MMD -MF ") ||
        !ib_builder_append(&builder, q_depfile) ||
        !ib_builder_append(&builder, " -c ") ||
        !ib_builder_append(&builder, q_source) ||
        !ib_builder_append(&builder, " -o ") ||
        !ib_builder_append(&builder, q_object)) {
        goto oom;
    }
    free(q_compiler); free(q_depfile); free(q_source); free(q_object);
    *out_command = ib_builder_take(&builder);
    return IB_OK;
oom:
    ib_builder_free(&builder);
    free(q_compiler); free(q_depfile); free(q_source); free(q_object);
    ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while building compile command");
    return IB_ERR_NOMEM;
}
static ib_status ib_build_link_command(ib_project* project, ib_build_plan* plan, ib_target* target, const char* output_arg, size_t compile_index, size_t compile_count, char** out_command) {
    ib_builder builder;
    char* q_output = NULL;
    char* q_tool = NULL;
    size_t i;
    bool uses_cpp = false;
    memset(&builder, 0, sizeof(builder));
    q_output = ib_shell_quote(project->ctx, output_arg);
    if (!q_output) {
        return project->ctx->last_status;
    }
    if (target->kind == IB_TARGET_STATIC_LIB) {
        q_tool = ib_shell_quote(project->ctx, project->archiver);
        if (!q_tool || !ib_builder_append(&builder, q_tool) || !ib_builder_append(&builder, " rcs ") || !ib_builder_append(&builder, q_output)) {
            goto oom;
        }
    } else {
        for (i = 0; i < compile_count; ++i) {
            if (plan->compile_actions[compile_index + i].is_cpp) {
                uses_cpp = true;
                break;
            }
        }
        q_tool = ib_shell_quote(project->ctx, uses_cpp ? project->compiler_cxx : project->compiler_c);
        if (!q_tool || !ib_builder_append(&builder, q_tool)) {
            goto oom;
        }
        if (target->kind == IB_TARGET_SHARED_LIB) {
#ifdef __APPLE__
            if (!ib_builder_append(&builder, " -dynamiclib")) {
                goto oom;
            }
#else
            if (!ib_builder_append(&builder, " -shared")) {
                goto oom;
            }
#endif
        }
        if (!ib_builder_append(&builder, " -o ") || !ib_builder_append(&builder, q_output)) {
            goto oom;
        }
    }
    for (i = 0; i < compile_count; ++i) {
        char* q_obj = ib_shell_quote(project->ctx, plan->compile_actions[compile_index + i].object_arg);
        if (!q_obj || !ib_builder_append_char(&builder, ' ') || !ib_builder_append(&builder, q_obj)) {
            free(q_obj);
            goto oom;
        }
        free(q_obj);
    }
    if (target->kind != IB_TARGET_STATIC_LIB) {
        for (i = 0; i < target->link_flags.count; ++i) {
            if (!ib_builder_append_char(&builder, ' ') || !ib_builder_append(&builder, target->link_flags.items[i])) {
                goto oom;
            }
        }
    }
    free(q_output);
    free(q_tool);
    *out_command = ib_builder_take(&builder);
    return IB_OK;
oom:
    ib_builder_free(&builder);
    free(q_output);
    free(q_tool);
    ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while building link command");
    return IB_ERR_NOMEM;
}

static bool ib_plan_compile_reserve(ib_build_plan* plan, size_t needed) {
    size_t cap = plan->compile_capacity ? plan->compile_capacity : IB_INITIAL_LIST_CAPACITY;
    ib_compile_action* next;
    if (needed <= plan->compile_capacity) {
        return true;
    }
    while (cap < needed) {
        cap *= 2;
    }
    next = (ib_compile_action*)realloc(plan->compile_actions, cap * sizeof(*next));
    if (!next) {
        return false;
    }
    plan->compile_actions = next;
    plan->compile_capacity = cap;
    return true;
}

static bool ib_plan_link_reserve(ib_build_plan* plan, size_t needed) {
    size_t cap = plan->link_capacity ? plan->link_capacity : IB_INITIAL_LIST_CAPACITY;
    ib_link_action* next;
    if (needed <= plan->link_capacity) {
        return true;
    }
    while (cap < needed) {
        cap *= 2;
    }
    next = (ib_link_action*)realloc(plan->link_actions, cap * sizeof(*next));
    if (!next) {
        return false;
    }
    plan->link_actions = next;
    plan->link_capacity = cap;
    return true;
}

static void ib_build_plan_free(ib_build_plan* plan) {
    size_t i;
    if (!plan) {
        return;
    }
    for (i = 0; i < plan->compile_count; ++i) {
        free(plan->compile_actions[i].source_path);
        free(plan->compile_actions[i].source_arg);
        free(plan->compile_actions[i].object_path);
        free(plan->compile_actions[i].object_arg);
        free(plan->compile_actions[i].depfile_path);
        free(plan->compile_actions[i].depfile_arg);
        free(plan->compile_actions[i].manifest_path);
        free(plan->compile_actions[i].command);
    }
    for (i = 0; i < plan->link_count; ++i) {
        free(plan->link_actions[i].output_path);
        free(plan->link_actions[i].output_arg);
        free(plan->link_actions[i].manifest_path);
        free(plan->link_actions[i].command);
    }
    free(plan->compile_actions);
    free(plan->link_actions);
    memset(plan, 0, sizeof(*plan));
}

static ib_status ib_plan_target(ib_project* project, ib_build_mode mode, ib_target* target, ib_build_plan* plan) {
    ib_strvec sources;
    size_t start_index;
    size_t i;
    ib_status status = ib_collect_target_sources(project, target, &sources);
    if (ib_status_failed(status)) {
        return status;
    }
    start_index = plan->compile_count;
    for (i = 0; i < sources.count; ++i) {
        char* rel = ib_source_object_relpath(project, sources.items[i]);
        char* obj_root;
        char* obj_mode;
        char* obj_target;
        char* object_abs;
        char* dep_abs;
        char* manifest_abs;
        char* source_arg;
        char* object_arg;
        char* dep_arg;
        char* command = NULL;
        ib_compile_action* action;
        if (!rel) {
            ib_strvec_free(&sources);
            return project->ctx->last_status;
        }
        obj_root = ib_path_join(project->ctx, project->state_dir, "obj");
        obj_mode = obj_root ? ib_path_join(project->ctx, obj_root, ib_mode_name(mode)) : NULL;
        free(obj_root);
        obj_target = obj_mode ? ib_path_join(project->ctx, obj_mode, target->name) : NULL;
        free(obj_mode);
        object_abs = obj_target ? ib_path_join(project->ctx, obj_target, rel) : NULL;
        free(obj_target);
        free(rel);
        if (!object_abs) {
            ib_strvec_free(&sources);
            return project->ctx->last_status;
        }
        dep_abs = ib_path_append_suffix(project->ctx, object_abs, ".d");
        manifest_abs = ib_path_append_suffix(project->ctx, object_abs, ".manifest");
        source_arg = ib_path_display(project->ctx, project->root_dir, sources.items[i]);
        object_arg = ib_path_display(project->ctx, project->root_dir, object_abs);
        dep_arg = ib_path_display(project->ctx, project->root_dir, dep_abs);
        if (!dep_abs || !manifest_abs || !source_arg || !object_arg || !dep_arg) {
            free(object_abs); free(dep_abs); free(manifest_abs); free(source_arg); free(object_arg); free(dep_arg);
            ib_strvec_free(&sources);
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while planning target '%s'", target->name);
            return IB_ERR_NOMEM;
        }
        status = ib_build_compile_command_for_mode(project, target, mode, source_arg, ib_is_cpp_source(sources.items[i]), object_arg, dep_arg, &command);
        if (ib_status_failed(status)) {
            free(object_abs); free(dep_abs); free(manifest_abs); free(source_arg); free(object_arg); free(dep_arg);
            ib_strvec_free(&sources);
            return status;
        }
        if (!ib_plan_compile_reserve(plan, plan->compile_count + 1)) {
            free(object_abs); free(dep_abs); free(manifest_abs); free(source_arg); free(object_arg); free(dep_arg); free(command);
            ib_strvec_free(&sources);
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while planning compile actions");
            return IB_ERR_NOMEM;
        }
        action = &plan->compile_actions[plan->compile_count++];
        memset(action, 0, sizeof(*action));
        action->target = target;
        action->source_path = ib_strdup_internal(sources.items[i]);
        action->source_arg = source_arg;
        action->is_cpp = ib_is_cpp_source(sources.items[i]);
        action->object_path = object_abs;
        action->object_arg = object_arg;
        action->depfile_path = dep_abs;
        action->depfile_arg = dep_arg;
        action->manifest_path = manifest_abs;
        action->command = command;
        action->command_hash = ib_hash_string(command);
        if (!action->source_path) {
            ib_strvec_free(&sources);
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while planning compile actions");
            return IB_ERR_NOMEM;
        }
    }
    if (!ib_plan_link_reserve(plan, plan->link_count + 1)) {
        ib_strvec_free(&sources);
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while planning link actions");
        return IB_ERR_NOMEM;
    }
    {
        ib_link_action* link = &plan->link_actions[plan->link_count++];
        char* output_abs = NULL;
        char* output_arg = NULL;
        char* link_root = NULL;
        char* link_mode = NULL;
        char* link_target = NULL;
        memset(link, 0, sizeof(*link));
        status = ib_target_output_path(project, target, &output_abs, &output_arg);
        if (ib_status_failed(status)) {
            ib_strvec_free(&sources);
            return status;
        }
        for (i = 0; i + 1 < plan->link_count; ++i) {
            if (ib_path_equal(plan->link_actions[i].output_path, output_abs)) {
                ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_DUPLICATE, "Target '%s' output path collides with target '%s': %s", target->name, plan->link_actions[i].target->name, output_abs);
                free(output_abs); free(output_arg);
                ib_strvec_free(&sources);
                return IB_ERR_DUPLICATE;
            }
        }
        link_root = ib_path_join(project->ctx, project->state_dir, "link");
        link_mode = link_root ? ib_path_join(project->ctx, link_root, ib_mode_name(mode)) : NULL;
        free(link_root);
        link_target = link_mode ? ib_path_join(project->ctx, link_mode, target->name) : NULL;
        free(link_mode);
        if (!link_target) {
            free(output_abs); free(output_arg);
            ib_strvec_free(&sources);
            return project->ctx->last_status;
        }
        link->target = target;
        link->output_path = output_abs;
        link->output_arg = output_arg;
        link->manifest_path = ib_path_append_suffix(project->ctx, link_target, ".manifest");
        free(link_target);
        link->compile_index = start_index;
        link->compile_count = plan->compile_count - start_index;
        if (!link->manifest_path) {
            ib_strvec_free(&sources);
            return project->ctx->last_status;
        }
        status = ib_build_link_command(project, plan, target, output_arg, link->compile_index, link->compile_count, &link->command);
        if (ib_status_failed(status)) {
            ib_strvec_free(&sources);
            return status;
        }
        link->command_hash = ib_hash_string(link->command);
    }
    ib_strvec_free(&sources);
    return IB_OK;
}

static ib_status ib_make_plan(ib_project* project, ib_build_mode mode, ib_build_plan* plan) {
    size_t i;
    memset(plan, 0, sizeof(*plan));
    if (!project || project->target_count == 0) {
        ib_context_emit(project ? project->ctx : NULL, IB_DIAG_ERROR, IB_ERR_INVALID, "Project has no targets");
        return IB_ERR_INVALID;
    }
    if (!ib_build_mode_valid(mode)) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Invalid build mode %d", (int)mode);
        return IB_ERR_INVALID;
    }
    for (i = 0; i < project->target_count; ++i) {
        ib_status status = ib_plan_target(project, mode, project->targets[i], plan);
        if (ib_status_failed(status)) {
            ib_build_plan_free(plan);
            return status;
        }
    }
    return IB_OK;
}

static ib_status ib_run_command(ib_project* project, const char* command, int* exit_code, char** output) {
    char* saved_cwd = ib_get_cwd(project->ctx);
    char* exec_command;
    FILE* pipe;
    ib_builder captured;
    char buffer[4096];
    int rc;
    memset(&captured, 0, sizeof(captured));
    if (!saved_cwd) {
        return project->ctx->last_status;
    }
    if (ib_chdir_to(project->root_dir) != 0) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to change directory to '%s': %s", project->root_dir, strerror(errno));
        free(saved_cwd);
        return IB_ERR_IO;
    }
    exec_command = ib_path_append_suffix(project->ctx, command, " 2>&1");
    if (!exec_command) {
        ib_chdir_to(saved_cwd);
        free(saved_cwd);
        return project->ctx->last_status;
    }
    if (project->ctx->verbose) {
        ib_context_emit(project->ctx, IB_DIAG_INFO, IB_OK, "Command: %s", command);
    }
    pipe = popen(exec_command, "r");
    free(exec_command);
    if (!pipe) {
        ib_chdir_to(saved_cwd);
        free(saved_cwd);
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_TOOLCHAIN, "Failed to start command: %s", command);
        return IB_ERR_TOOLCHAIN;
    }
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (!ib_builder_append(&captured, buffer)) {
            pclose(pipe);
            ib_chdir_to(saved_cwd);
            free(saved_cwd);
            ib_builder_free(&captured);
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while capturing command output");
            return IB_ERR_NOMEM;
        }
        if (project->ctx->verbose) {
            fputs(buffer, stdout);
        }
    }
    rc = pclose(pipe);
#ifndef _WIN32
    if (rc != -1 && WIFEXITED(rc)) {
        rc = WEXITSTATUS(rc);
    }
#endif
    ib_chdir_to(saved_cwd);
    free(saved_cwd);
    if (exit_code) {
        *exit_code = rc;
    }
    if (output) {
        *output = ib_builder_take(&captured);
    } else {
        ib_builder_free(&captured);
    }
    return IB_OK;
}
static bool ib_read_manifest_hash(const char* path, unsigned long long* out_hash) {
    FILE* fp = fopen(path, "r");
    unsigned long long hash = 0;
    if (!fp) {
        return false;
    }
    if (fscanf(fp, "%llx", &hash) != 1) {
        fclose(fp);
        return false;
    }
    fclose(fp);
    if (out_hash) {
        *out_hash = hash;
    }
    return true;
}

static ib_status ib_write_manifest_hash(ib_project* project, const char* path, unsigned long long hash) {
    FILE* fp;
    ib_status status = ib_mkdirs_for_file(project, path);
    if (ib_status_failed(status)) {
        return status;
    }
    fp = fopen(path, "w");
    if (!fp) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to write manifest '%s': %s", path, strerror(errno));
        return IB_ERR_IO;
    }
    fprintf(fp, "%016llx\n", hash);
    fclose(fp);
    return IB_OK;
}

static ib_status ib_read_depfile(ib_project* project, const char* depfile_path, ib_strvec* deps) {
    FILE* fp = fopen(depfile_path, "r");
    ib_builder content;
    char buffer[4096];
    char* raw;
    char* colon;
    char* cursor;
    ib_strvec_init(deps);
    memset(&content, 0, sizeof(content));
    if (!fp) {
        return IB_ERR_IO;
    }
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (!ib_builder_append(&content, buffer)) {
            fclose(fp);
            ib_builder_free(&content);
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while parsing depfile");
            return IB_ERR_NOMEM;
        }
    }
    fclose(fp);
    raw = ib_builder_take(&content);
    if (!raw) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while parsing depfile");
        return IB_ERR_NOMEM;
    }
    while ((cursor = strstr(raw, "\\\r\n")) != NULL) {
        memmove(cursor, cursor + 3, strlen(cursor + 3) + 1);
        *cursor = ' ';
    }
    while ((cursor = strstr(raw, "\\\n")) != NULL) {
        memmove(cursor, cursor + 2, strlen(cursor + 2) + 1);
        *cursor = ' ';
    }
    colon = NULL;
    for (cursor = raw; *cursor; ++cursor) {
        if (*cursor == ':' && (cursor[1] == '\0' || isspace((unsigned char)cursor[1]))) {
            colon = cursor;
            break;
        }
    }
    if (!colon) {
        free(raw);
        return IB_ERR_IO;
    }
    cursor = colon + 1;
    while (*cursor) {
        ib_builder token;
        char* text;
        char* dep_abs;
        memset(&token, 0, sizeof(token));
        while (*cursor && isspace((unsigned char)*cursor)) {
            ++cursor;
        }
        if (!*cursor) {
            break;
        }
        while (*cursor && !isspace((unsigned char)*cursor)) {
            if (*cursor == '\\' && cursor[1] != '\0') {
                if (!ib_builder_append_char(&token, cursor[1])) {
                    ib_builder_free(&token);
                    ib_strvec_free(deps);
                    free(raw);
                    ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while parsing depfile");
                    return IB_ERR_NOMEM;
                }
                cursor += 2;
            } else if (!ib_builder_append_char(&token, *cursor++)) {
                ib_builder_free(&token);
                ib_strvec_free(deps);
                free(raw);
                ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while parsing depfile");
                return IB_ERR_NOMEM;
            }
        }
        text = ib_builder_take(&token);
        if (!text || !text[0]) {
            free(text);
            continue;
        }
        dep_abs = ib_path_normalize(project->ctx, project->root_dir, text);
        free(text);
        if (!dep_abs) {
            ib_strvec_free(deps);
            free(raw);
            return project->ctx->last_status;
        }
        if (!ib_strvec_push_unique_owned(deps, dep_abs)) {
            free(dep_abs);
            ib_strvec_free(deps);
            free(raw);
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while parsing depfile");
            return IB_ERR_NOMEM;
        }
    }
    free(raw);
    return IB_OK;
}

static bool ib_compile_action_needs_rebuild(ib_project* project, const ib_compile_action* action) {
    unsigned long long manifest_hash = 0;
    ib_strvec deps;
    size_t i;
    unsigned long long obj_mtime;
    if (!ib_path_exists(action->object_path) || !ib_path_exists(action->depfile_path) || !ib_path_exists(action->manifest_path)) {
        return true;
    }
    if (!ib_read_manifest_hash(action->manifest_path, &manifest_hash) || manifest_hash != action->command_hash) {
        return true;
    }
    obj_mtime = ib_file_mtime(action->object_path);
    if (obj_mtime == 0 || ib_status_failed(ib_read_depfile(project, action->depfile_path, &deps))) {
        return true;
    }
    for (i = 0; i < deps.count; ++i) {
        unsigned long long dep_mtime = ib_file_mtime(deps.items[i]);
        if (dep_mtime == 0 || dep_mtime > obj_mtime) {
            ib_strvec_free(&deps);
            return true;
        }
    }
    ib_strvec_free(&deps);
    return false;
}

static bool ib_link_action_needs_rebuild(ib_project* project, const ib_build_plan* plan, const ib_link_action* action) {
    unsigned long long manifest_hash = 0;
    size_t i;
    unsigned long long out_mtime;
    if (!ib_path_exists(action->output_path) || !ib_path_exists(action->manifest_path)) {
        return true;
    }
    if (!ib_read_manifest_hash(action->manifest_path, &manifest_hash) || manifest_hash != action->command_hash) {
        return true;
    }
    out_mtime = ib_file_mtime(action->output_path);
    if (out_mtime == 0) {
        return true;
    }
    for (i = 0; i < action->compile_count; ++i) {
        const ib_compile_action* compile = &plan->compile_actions[action->compile_index + i];
        if (!ib_path_exists(compile->object_path) || ib_file_mtime(compile->object_path) > out_mtime) {
            return true;
        }
    }
    (void)project;
    return false;
}

static ib_status ib_execute_compile(ib_project* project, const ib_compile_action* action, bool* rebuilt) {
    int rc = 0;
    char* output = NULL;
    ib_status status;
    if (rebuilt) {
        *rebuilt = false;
    }
    if (!ib_compile_action_needs_rebuild(project, action)) {
        return IB_OK;
    }
    status = ib_mkdirs_for_file(project, action->object_path);
    if (ib_status_failed(status)) {
        return status;
    }
    ib_context_emit(project->ctx, IB_DIAG_INFO, IB_OK, "Compiling %s for %s", action->source_arg, action->target->name);
    status = ib_run_command(project, action->command, &rc, &output);
    if (ib_status_failed(status)) {
        free(output);
        return status;
    }
    if (rc != 0) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_BUILD, "Compilation failed for %s (exit %d)%s%s", action->source_arg, rc, output && output[0] ? ":\n" : "", output && output[0] ? output : "");
        free(output);
        return IB_ERR_BUILD;
    }
    free(output);
    status = ib_write_manifest_hash(project, action->manifest_path, action->command_hash);
    if (!ib_status_failed(status) && rebuilt) {
        *rebuilt = true;
    }
    return status;
}

static ib_status ib_execute_link(ib_project* project, const ib_build_plan* plan, const ib_link_action* action, bool* relinked) {
    int rc = 0;
    char* output = NULL;
    ib_status status;
    if (relinked) {
        *relinked = false;
    }
    if (!ib_link_action_needs_rebuild(project, plan, action)) {
        return IB_OK;
    }
    status = ib_mkdirs_for_file(project, action->output_path);
    if (ib_status_failed(status)) {
        return status;
    }
    ib_context_emit(project->ctx, IB_DIAG_INFO, IB_OK, "Linking %s", action->target->name);
    status = ib_run_command(project, action->command, &rc, &output);
    if (ib_status_failed(status)) {
        free(output);
        return status;
    }
    if (rc != 0) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_BUILD, "Link failed for %s (exit %d)%s%s", action->target->name, rc, output && output[0] ? ":\n" : "", output && output[0] ? output : "");
        free(output);
        return IB_ERR_BUILD;
    }
    free(output);
    status = ib_write_manifest_hash(project, action->manifest_path, action->command_hash);
    if (!ib_status_failed(status) && relinked) {
        *relinked = true;
    }
    return status;
}

static ib_status ib_write_json_string(FILE* fp, const char* text) {
    const unsigned char* p = (const unsigned char*)text;
    fputc('"', fp);
    while (*p) {
        switch (*p) {
            case '\\': fputs("\\\\", fp); break;
            case '"':  fputs("\\\"", fp); break;
            case '\n': fputs("\\n", fp); break;
            case '\r': fputs("\\r", fp); break;
            case '\t': fputs("\\t", fp); break;
            default:
                if (*p < 0x20) fprintf(fp, "\\u%04x", *p);
                else fputc(*p, fp);
                break;
        }
        ++p;
    }
    fputc('"', fp);
    return IB_OK;
}

ib_context* ib_context_create(void) {
    ib_context* ctx = (ib_context*)calloc(1, sizeof(*ctx));
    if (!ctx) {
        return NULL;
    }
    ctx->last_status = IB_OK;
    ctx->last_message = ib_strdup_internal("");
    ctx->log_level = IB_DIAG_INFO;
    ctx->verbose = false;
    ctx->color_output = ib_stream_is_tty(stdout);
    return ctx;
}

void ib_context_destroy(ib_context* ctx) {
    size_t i;
    if (!ctx) {
        return;
    }
    for (i = 0; i < ctx->diagnostics.count; ++i) {
        free(ctx->diagnostics.items[i].message);
    }
    free(ctx->diagnostics.items);
    free(ctx->last_message);
    free(ctx);
}

void ib_context_set_log_level(ib_context* ctx, ib_diag_level level) { if (ctx) ctx->log_level = level; }
void ib_context_set_verbose(ib_context* ctx, bool verbose) { if (ctx) ctx->verbose = verbose; }
void ib_context_set_color_output(ib_context* ctx, bool enabled) { if (ctx) ctx->color_output = enabled; }

void ib_context_clear_diagnostics(ib_context* ctx) {
    size_t i;
    if (!ctx) return;
    for (i = 0; i < ctx->diagnostics.count; ++i) free(ctx->diagnostics.items[i].message);
    ctx->diagnostics.count = 0;
    ctx->last_status = IB_OK;
    free(ctx->last_message);
    ctx->last_message = ib_strdup_internal("");
}

ib_status ib_context_last_status(const ib_context* ctx) { return ctx ? ctx->last_status : IB_ERR_INVALID; }
const char* ib_context_last_message(const ib_context* ctx) { return (ctx && ctx->last_message) ? ctx->last_message : ""; }
size_t ib_context_diagnostic_count(const ib_context* ctx) { return ctx ? ctx->diagnostics.count : 0; }
ib_diag_level ib_context_diagnostic_level_at(const ib_context* ctx, size_t index) { return (!ctx || index >= ctx->diagnostics.count) ? IB_DIAG_ERROR : ctx->diagnostics.items[index].level; }
ib_status ib_context_diagnostic_status_at(const ib_context* ctx, size_t index) { return (!ctx || index >= ctx->diagnostics.count) ? IB_ERR_INVALID : ctx->diagnostics.items[index].status; }
const char* ib_context_diagnostic_message_at(const ib_context* ctx, size_t index) { return (!ctx || index >= ctx->diagnostics.count || !ctx->diagnostics.items[index].message) ? "" : ctx->diagnostics.items[index].message; }

ib_project* ib_project_create(ib_context* ctx, const char* root_dir) {
    ib_project* project;
    char* cwd;
    if (!ctx) return NULL;
    project = (ib_project*)calloc(1, sizeof(*project));
    if (!project) {
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while creating project");
        return NULL;
    }
    project->ctx = ctx;
    cwd = ib_get_cwd(ctx);
    if (!cwd) { free(project); return NULL; }
    project->root_dir = ib_path_normalize(ctx, cwd, root_dir && root_dir[0] ? root_dir : ".");
    free(cwd);
    if (!project->root_dir || !ib_path_exists(project->root_dir) || !ib_is_dir(project->root_dir)) {
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOT_FOUND, "Project root '%s' does not exist", root_dir && root_dir[0] ? root_dir : ".");
        ib_project_destroy(project);
        return NULL;
    }
    project->output_dir = ib_strdup_internal(project->root_dir);
    project->state_dir = ib_path_join(ctx, project->root_dir, ".ibuild");
#ifdef _WIN32
    project->compiler_c = ib_strdup_internal("gcc");
    project->compiler_cxx = ib_strdup_internal("g++");
#else
    project->compiler_c = ib_strdup_internal("cc");
    project->compiler_cxx = ib_strdup_internal("c++");
#endif
    project->archiver = ib_strdup_internal("ar");
    project->debug_cflags = ib_strdup_internal("-g -O0 -DDEBUG");
    project->debug_cxxflags = ib_strdup_internal("-g -O0 -DDEBUG");
    project->release_cflags = ib_strdup_internal("-O2 -DNDEBUG");
    project->release_cxxflags = ib_strdup_internal("-O2 -DNDEBUG");
    project->custom_cflags = ib_strdup_internal("");
    project->custom_cxxflags = ib_strdup_internal("");
    ib_strvec_init(&project->include_dirs);
    ib_strvec_init(&project->shared_sources);
    if (!project->output_dir || !project->state_dir || !project->compiler_c || !project->compiler_cxx || !project->archiver || !project->debug_cflags || !project->debug_cxxflags || !project->release_cflags || !project->release_cxxflags || !project->custom_cflags || !project->custom_cxxflags) {
        ib_context_emit(ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while initializing project");
        ib_project_destroy(project);
        return NULL;
    }
    ib_context_emit(ctx, IB_DIAG_INFO, IB_OK, "IncludeBuild %s project created at %s", ib_version(), project->root_dir);
    return project;
}

void ib_project_destroy(ib_project* project) {
    size_t i;
    if (!project) return;
    for (i = 0; i < project->target_count; ++i) {
        ib_target* target = project->targets[i];
        if (!target) continue;
        free(target->name);
        free(target->entry_source);
        ib_strvec_free(&target->sources);
        ib_strvec_free(&target->include_dirs);
        ib_strvec_free(&target->cflags);
        ib_strvec_free(&target->link_flags);
        free(target);
    }
    free(project->targets);
    free(project->root_dir); free(project->output_dir); free(project->state_dir);
    free(project->compiler_c); free(project->compiler_cxx); free(project->archiver);
    free(project->debug_cflags); free(project->debug_cxxflags); free(project->release_cflags); free(project->release_cxxflags);
    free(project->custom_cflags); free(project->custom_cxxflags);
    ib_strvec_free(&project->include_dirs); ib_strvec_free(&project->shared_sources);
    free(project);
}
ib_status ib_project_set_c_compiler(ib_project* project, const char* compiler) {
    if (!project || !compiler || !compiler[0]) {
        if (project) ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "C compiler must be a non-empty string");
        return IB_ERR_INVALID;
    }
    return ib_set_owned_string(project->ctx, &project->compiler_c, compiler);
}

ib_status ib_project_set_cxx_compiler(ib_project* project, const char* compiler) {
    if (!project || !compiler || !compiler[0]) {
        if (project) ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "C++ compiler must be a non-empty string");
        return IB_ERR_INVALID;
    }
    return ib_set_owned_string(project->ctx, &project->compiler_cxx, compiler);
}

ib_status ib_project_set_archiver(ib_project* project, const char* archiver) {
    if (!project || !archiver || !archiver[0]) {
        if (project) ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Archiver must be a non-empty string");
        return IB_ERR_INVALID;
    }
    return ib_set_owned_string(project->ctx, &project->archiver, archiver);
}

ib_status ib_project_set_output_dir(ib_project* project, const char* dir) {
    char* normalized;
    ib_status status;
    if (!project || !dir || !dir[0]) {
        if (project) {
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Output directory must be a non-empty path");
        }
        return IB_ERR_INVALID;
    }
    normalized = ib_path_normalize(project->ctx, project->root_dir, dir);
    if (!normalized) return project->ctx->last_status;
    status = ib_project_validate_output_dir(project, normalized);
    if (ib_status_failed(status)) {
        free(normalized);
        return status;
    }
    free(project->output_dir);
    project->output_dir = normalized;
    return IB_OK;
}

ib_status ib_project_set_state_dir(ib_project* project, const char* dir) {
    char* normalized;
    ib_status status;
    if (!project || !dir || !dir[0]) {
        if (project) {
            ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "State directory must be a non-empty path");
        }
        return IB_ERR_INVALID;
    }
    normalized = ib_path_normalize(project->ctx, project->root_dir, dir);
    if (!normalized) return project->ctx->last_status;
    status = ib_project_validate_state_dir(project, normalized);
    if (ib_status_failed(status)) {
        free(normalized);
        return status;
    }
    free(project->state_dir);
    project->state_dir = normalized;
    return IB_OK;
}

ib_status ib_project_set_mode_flags(ib_project* project, ib_build_mode mode, const char* c_flags, const char* cxx_flags) {
    ib_status status = IB_OK;
    if (!project) return IB_ERR_INVALID;
    if (!ib_build_mode_valid(mode)) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Invalid build mode %d", (int)mode);
        return IB_ERR_INVALID;
    }
    switch (mode) {
        case IB_MODE_DEBUG:
            if (c_flags && ib_status_failed((status = ib_set_owned_string(project->ctx, &project->debug_cflags, c_flags)))) return status;
            if (cxx_flags) status = ib_set_owned_string(project->ctx, &project->debug_cxxflags, cxx_flags);
            return status;
        case IB_MODE_RELEASE:
            if (c_flags && ib_status_failed((status = ib_set_owned_string(project->ctx, &project->release_cflags, c_flags)))) return status;
            if (cxx_flags) status = ib_set_owned_string(project->ctx, &project->release_cxxflags, cxx_flags);
            return status;
        case IB_MODE_CUSTOM:
            if (c_flags && ib_status_failed((status = ib_set_owned_string(project->ctx, &project->custom_cflags, c_flags)))) return status;
            if (cxx_flags) status = ib_set_owned_string(project->ctx, &project->custom_cxxflags, cxx_flags);
            return status;
    }
    return IB_ERR_INVALID;
}

ib_status ib_project_add_include_dir(ib_project* project, const char* path) {
    char* dir; ib_status status;
    if (!project) return IB_ERR_INVALID;
    status = ib_normalize_existing_dir(project, path, &dir);
    if (ib_status_failed(status)) return status;
    if (!ib_strvec_push_unique_owned(&project->include_dirs, dir)) {
        free(dir); ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while adding include directory"); return IB_ERR_NOMEM;
    }
    return IB_OK;
}

ib_status ib_project_add_shared_source(ib_project* project, const char* path) {
    char* source; ib_status status;
    if (!project) return IB_ERR_INVALID;
    status = ib_normalize_existing_source(project, path, &source);
    if (ib_status_failed(status)) return status;
    if (!ib_strvec_push_unique_owned(&project->shared_sources, source)) {
        free(source); ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while adding shared source"); return IB_ERR_NOMEM;
    }
    return IB_OK;
}

ib_status ib_project_scan_shared_dir(ib_project* project, const char* dir, bool recursive) {
    char* source_dir; ib_strvec found; size_t i; ib_status status;
    if (!project) return IB_ERR_INVALID;
    status = ib_normalize_existing_dir(project, dir, &source_dir);
    if (ib_status_failed(status)) return status;
    ib_strvec_init(&found);
    status = ib_collect_sources(project, source_dir, recursive, &found);
    free(source_dir);
    if (ib_status_failed(status)) { ib_strvec_free(&found); return status; }
    ib_strvec_sort(&found);
    for (i = 0; i < found.count; ++i) {
        if (!ib_strvec_push_unique_copy(&project->shared_sources, found.items[i])) {
            ib_strvec_free(&found); ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while scanning shared sources"); return IB_ERR_NOMEM;
        }
    }
    ib_strvec_free(&found);
    return IB_OK;
}

static bool ib_targetvec_reserve(ib_project* project, size_t needed) {
    size_t cap = project->target_capacity ? project->target_capacity : IB_INITIAL_LIST_CAPACITY; ib_target** next;
    if (needed <= project->target_capacity) return true;
    while (cap < needed) cap *= 2;
    next = (ib_target**)realloc(project->targets, cap * sizeof(*next));
    if (!next) return false;
    project->targets = next; project->target_capacity = cap; return true;
}

ib_target* ib_project_add_target(ib_project* project, const char* name, ib_target_kind kind) {
    ib_target* target; size_t i;
    if (!project) return NULL;
    if (!ib_target_name_is_safe(name)) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Target name '%s' is not a safe portable file name", name ? name : "");
        return NULL;
    }
    if (kind < IB_TARGET_EXECUTABLE || kind > IB_TARGET_SHARED_LIB) {
        ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Target '%s' uses an invalid target kind", name);
        return NULL;
    }
    for (i = 0; i < project->target_count; ++i) if (strcmp(project->targets[i]->name, name) == 0) { ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_DUPLICATE, "Target '%s' already exists", name); return NULL; }
    if (!ib_targetvec_reserve(project, project->target_count + 1)) { ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while creating target '%s'", name); return NULL; }
    target = (ib_target*)calloc(1, sizeof(*target));
    if (!target) { ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while creating target '%s'", name); return NULL; }
    target->project = project; target->name = ib_strdup_internal(name); target->kind = kind;
    ib_strvec_init(&target->sources); ib_strvec_init(&target->include_dirs); ib_strvec_init(&target->cflags); ib_strvec_init(&target->link_flags);
    if (!target->name) { free(target); ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while creating target '%s'", name); return NULL; }
    project->targets[project->target_count++] = target; return target;
}

ib_status ib_target_set_entry(ib_target* target, const char* path) { char* source; ib_status status; if (!target || !target->project) return IB_ERR_INVALID; status = ib_normalize_existing_source(target->project, path, &source); if (ib_status_failed(status)) return status; free(target->entry_source); target->entry_source = source; return IB_OK; }
ib_status ib_target_add_source(ib_target* target, const char* path) { char* source; ib_status status; if (!target || !target->project) return IB_ERR_INVALID; status = ib_normalize_existing_source(target->project, path, &source); if (ib_status_failed(status)) return status; if (!ib_strvec_push_unique_owned(&target->sources, source)) { free(source); ib_context_emit(target->project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while adding target source"); return IB_ERR_NOMEM; } return IB_OK; }
ib_status ib_target_add_include_dir(ib_target* target, const char* path) { char* dir; ib_status status; if (!target || !target->project) return IB_ERR_INVALID; status = ib_normalize_existing_dir(target->project, path, &dir); if (ib_status_failed(status)) return status; if (!ib_strvec_push_unique_owned(&target->include_dirs, dir)) { free(dir); ib_context_emit(target->project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while adding target include directory"); return IB_ERR_NOMEM; } return IB_OK; }
ib_status ib_target_add_cflags(ib_target* target, const char* flags) { if (!target || !target->project || !flags || !flags[0]) { if (target && target->project) ib_context_emit(target->project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Compile flags must be a non-empty string"); return IB_ERR_INVALID; } if (!ib_strvec_push_copy(&target->cflags, flags)) { ib_context_emit(target->project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while adding target compile flags"); return IB_ERR_NOMEM; } return IB_OK; }
ib_status ib_target_add_link_flags(ib_target* target, const char* flags) { if (!target || !target->project || !flags || !flags[0]) { if (target && target->project) ib_context_emit(target->project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Link flags must be a non-empty string"); return IB_ERR_INVALID; } if (target->kind == IB_TARGET_STATIC_LIB) { ib_context_emit(target->project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Static library targets do not use linker flags"); return IB_ERR_INVALID; } if (!ib_strvec_push_copy(&target->link_flags, flags)) { ib_context_emit(target->project->ctx, IB_DIAG_ERROR, IB_ERR_NOMEM, "Out of memory while adding target link flags"); return IB_ERR_NOMEM; } return IB_OK; }

ib_status ib_project_build(ib_project* project, ib_build_mode mode) {
    ib_build_plan plan; size_t i; size_t compiled = 0; size_t linked = 0; ib_status status;
    if (!project) return IB_ERR_INVALID;
    if (!ib_build_mode_valid(mode)) { ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Invalid build mode %d", (int)mode); return IB_ERR_INVALID; }
    status = ib_project_validate_state_dir(project, project->state_dir); if (ib_status_failed(status)) return status;
    status = ib_project_validate_output_dir(project, project->output_dir); if (ib_status_failed(status)) return status;
    status = ib_make_plan(project, mode, &plan); if (ib_status_failed(status)) return status;
    status = ib_mkdirs(project, project->state_dir); if (ib_status_failed(status)) { ib_build_plan_free(&plan); return status; }
    status = ib_mkdirs(project, project->output_dir); if (ib_status_failed(status)) { ib_build_plan_free(&plan); return status; }
    for (i = 0; i < plan.compile_count; ++i) { bool rebuilt = false; status = ib_execute_compile(project, &plan.compile_actions[i], &rebuilt); if (ib_status_failed(status)) { ib_build_plan_free(&plan); return status; } if (rebuilt) ++compiled; }
    for (i = 0; i < plan.link_count; ++i) { bool relinked = false; status = ib_execute_link(project, &plan, &plan.link_actions[i], &relinked); if (ib_status_failed(status)) { ib_build_plan_free(&plan); return status; } if (relinked) ++linked; }
    ib_context_emit(project->ctx, IB_DIAG_INFO, IB_OK, "Build complete. Recompiled %zu action(s), relinked %zu target(s).", compiled, linked);
    ib_build_plan_free(&plan); return IB_OK;
}

ib_status ib_project_clean(ib_project* project) {
    size_t i; ib_status status;
    if (!project) return IB_ERR_INVALID;
    status = ib_project_validate_state_dir(project, project->state_dir); if (ib_status_failed(status)) return status;
    status = ib_project_validate_output_dir(project, project->output_dir); if (ib_status_failed(status)) return status;
    status = ib_remove_recursive(project, project->state_dir); if (ib_status_failed(status)) return status;
    for (i = 0; i < project->target_count; ++i) {
        char* output_abs = NULL; char* output_arg = NULL;
        status = ib_target_output_path(project, project->targets[i], &output_abs, &output_arg);
        free(output_arg);
        if (ib_status_failed(status)) { free(output_abs); return status; }
        if (ib_path_exists(output_abs)) { status = ib_remove_recursive(project, output_abs); free(output_abs); if (ib_status_failed(status)) return status; } else free(output_abs);
    }
    ib_context_emit(project->ctx, IB_DIAG_INFO, IB_OK, "Cleaned build outputs"); return IB_OK;
}

ib_status ib_project_write_compile_commands(ib_project* project, const char* out_path) {
    return ib_project_write_compile_commands_for_mode(project, IB_MODE_DEBUG, out_path);
}

ib_status ib_project_write_compile_commands_for_mode(ib_project* project, ib_build_mode mode, const char* out_path) {
    ib_build_plan plan; ib_status status; char* output_abs; FILE* fp; size_t i;
    if (!project) return IB_ERR_INVALID;
    if (!ib_build_mode_valid(mode)) { ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_INVALID, "Invalid build mode %d", (int)mode); return IB_ERR_INVALID; }
    status = ib_project_validate_state_dir(project, project->state_dir); if (ib_status_failed(status)) return status;
    status = ib_project_validate_output_dir(project, project->output_dir); if (ib_status_failed(status)) return status;
    output_abs = ib_path_normalize(project->ctx, project->root_dir, (out_path && out_path[0]) ? out_path : "compile_commands.json");
    if (!output_abs) return project->ctx->last_status;
    status = ib_make_plan(project, mode, &plan); if (ib_status_failed(status)) { free(output_abs); return status; }
    status = ib_mkdirs_for_file(project, output_abs); if (ib_status_failed(status)) { ib_build_plan_free(&plan); free(output_abs); return status; }
    fp = fopen(output_abs, "w");
    if (!fp) { ib_build_plan_free(&plan); ib_context_emit(project->ctx, IB_DIAG_ERROR, IB_ERR_IO, "Failed to open '%s' for writing: %s", output_abs, strerror(errno)); free(output_abs); return IB_ERR_IO; }
    fputs("[\n", fp);
    for (i = 0; i < plan.compile_count; ++i) {
        const ib_compile_action* action = &plan.compile_actions[i];
        fputs("  {\n    \"directory\": ", fp); ib_write_json_string(fp, project->root_dir);
        fputs(",\n    \"command\": ", fp); ib_write_json_string(fp, action->command);
        fputs(",\n    \"file\": ", fp); ib_write_json_string(fp, action->source_path);
        fputs("\n  }", fp); if (i + 1 < plan.compile_count) fputs(",", fp); fputs("\n", fp);
    }
    fputs("]\n", fp); fclose(fp); ib_build_plan_free(&plan); ib_context_emit(project->ctx, IB_DIAG_INFO, IB_OK, "Wrote compile_commands.json to %s", output_abs); free(output_abs); return IB_OK;
}

const char* ib_version(void) { return "2.0.3"; }

#endif /* INCLUDEBUILD_IMPLEMENTATION */

#endif /* INCLUDEBUILD_H */
