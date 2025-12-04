#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOBDEF static inline
#include "vendor/nob/nob.h"

#define CAP_IMPLEMENTATION
#define CAPDEF static inline
#include "vendor/cap/cap.h"

#define SHIFT(arr, len) ((len)--, *(arr)++)
#ifndef UNUSED
#define UNUSED(x) (void)x
#endif
#define ARRAY_LENGTH(arr) (sizeof((arr)) / sizeof(*(arr)))

#define DO_OR_FAIL(...)                         \
    do {                                        \
        if (!(__VA_ARGS__)) {                   \
            return_val = false;                 \
            goto done;                          \
        }                                       \
    } while (0)


#define SRC_DIR               "src"
#define DATA_DIR              "data"
#define BUILD_DIR             "build"
#define BIN_DIR               BUILD_DIR"/bin"
#define OBJECT_DIR            BUILD_DIR"/obj"
#define GENERATED_DIR         BUILD_DIR"/generated"
#define AOC2025_GENERATED_DIR GENERATED_DIR"/aoc2025"

#define AOC2025_DISTRIBUTION_DIR          BUILD_DIR"/"AOC2025_DISTRIBUTION_DIR_NAME

#define LEGAL_DIR      AOC2025_DISTRIBUTION_DIR"/legal"
#define CACHE_DIR      BUILD_DIR"/cache"

#define AOC2025_OBJECT_DIR    OBJECT_DIR"/aoc2025"

#ifdef _MSC_VER
#define AOC2025_BIN_NAME "aoc2025.exe"
#else
#define AOC2025_BIN_NAME "aoc2025"
#endif

#define AOC2025_BIN           BIN_DIR"/"AOC2025_BIN_NAME

#ifdef _MSC_VER
#define AOC2025_DISTRIBUTION_DIR_NAME "aoc2025_windows_x64"
#else
#define AOC2025_DISTRIBUTION_DIR_NAME "aoc2025_linux_x86_64"
#endif
#ifdef _MSC_VER
#define AOC2025_DISTRIBUTION_ARCHIVE_NAME AOC2025_DISTRIBUTION_DIR_NAME".zip"
#else
#define AOC2025_DISTRIBUTION_ARCHIVE_NAME AOC2025_DISTRIBUTION_DIR_NAME".tar.gz"
#endif
#define AOC2025_DISTRIBUTION_HASH_FILE_NAME  AOC2025_DISTRIBUTION_ARCHIVE_NAME".sha256"
#define AOC2025_DISTRIBUTION_ARCHIVE   BUILD_DIR"/"AOC2025_DISTRIBUTION_ARCHIVE_NAME
#define AOC2025_DISTRIBUTION_HASH_FILE BUILD_DIR"/"AOC2025_DISTRIBUTION_HASH_FILE_NAME

const char *CXX = NULL;
const char *CC  = NULL;
#ifdef _MSC_VER
#define CCACHE_BIN "tools/ccache/ccache-windows-x86_64/ccache"
#define OBJ_FILE_EXT ".obj"
#else
#define CCACHE_BIN "tools/ccache/ccache-linux-x86_64/ccache"
#define OBJ_FILE_EXT ".o"
#endif
#define CCACHE_CACHE_DIR CACHE_DIR"/ccache"

#ifdef _MSC_VER
static const char *common_compile_options[] = {
    "/nologo", "/c", "/EHsc", "/utf-8"
};
static const char *debug_compile_options[] = {
    "/Z7", "/Od", "/MTd",
};
static const char *release_compile_options[] = {
    "/O2", "/GL", "/MT", "/Gy", "/Gw"
};
static const char *aoc2025_compile_options[] = {
    "/W3"
};
static const char *debug_definitions[] = {
    "/D_CRT_SECURE_NO_WARNINGS"
};
static const char *release_definitions[] = {
    "/DNDEBUG", "/D_CRT_SECURE_NO_WARNINGS"
};
static const char *common_link_options[] = {
    "/nologo", "/link", "/INCREMENTAL:NO", "user32.lib", "gdi32.lib", "shell32.lib", "Winmm.lib", "/SUBSYSTEM:WINDOWS", "/OPT:REF", "/OPT:ICF"
};
static const char *debug_link_options[] = {
    "/DEBUG"
};
static const char *release_link_options[] = {
    "/LTCG"
};
#else
static const char *common_compile_options[] = {
    "-c", "-fPIC", "-Wno-unused-result"
};
static const char *debug_compile_options[] = {
    "-O0", "-ggdb", "-fno-omit-frame-pointer"
};
static const char *release_compile_options[] = {
    "-O3", "-ffunction-sections", "-fdata-sections", "-flto", "-fvisibility=hidden"
};
static const char *aoc2025_compile_options[] = {
    "-Wall", "-Wextra"
};
static const char *debug_definitions[] = {
};
static const char *release_definitions[] = {
    "-DNDEBUG"
};
static const char *common_link_options[] = {
    "-ldl", "-lpthread", "-lm", "-Wl,--gc-sections", "-Wl,--as-needed", "-Wl,-O1", "-flto=auto"
};
static const char *debug_link_options[] = {

};
static const char *release_link_options[] = {

};
#endif

typedef enum {
#ifdef _MSC_VER
    COMPILER_CL,
    COMPILER_CLANG_CL
#else
    COMPILER_GCC,
    COMPILER_CLANG
#endif
} Compiler;

static struct {
    CapContext *ctx;

    bool debug;
    int  jobs;
    bool run;
    bool clean;
    bool package;
    bool emit_compile_commands;
    bool emit_vscode_tasks;
    bool asan;

    Compiler compiler;

    bool verbose;

    bool cache;

#ifndef _MSC_VER
    bool ubsan; // ubsan is not supported on windows
#endif

    int    remainder_argc;
    char **remainder_argv;
} cli;

static inline int
get_cpu_count(void)
{
    int count;

#ifdef _MSC_VER
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    count = (int)sys_info.dwNumberOfProcessors;
#else
    count = sysconf(_SC_NPROCESSORS_ONLN);
#endif

    if (count < 1) count = 1;

    return count;
}

static inline void
init_cli(int argc, char **argv)
{
    cli.ctx = cap_context_new();

    cap_set_program_description(cli.ctx, "Build system for Aoc2025");

    cap_capture_remainder(cli.ctx,
                          &cli.remainder_argc, &cli.remainder_argv,
                          "Passed to Aoc2025 binary if `--run` is specified");

    cap_flag(cli.ctx, &cli.verbose)
        ->long_name("verbose")
        ->short_name('v')
        ->description("Enable verbose output.")
        ->done();

    cap_flag(cli.ctx, &cli.debug)
        ->long_name("debug")
        ->short_name('d')
        ->description("Build without optimizations and generate debug symbols.")
        ->done();

    cap_option_int(cli.ctx, &cli.jobs)
        ->long_name("jobs")
        ->short_name('j')
        ->description("Specify max amount of concurrent processes. Higher values may yield faster builds, but use more system resources. Values of 0 or less imply 1.")
        ->default_value(get_cpu_count())
        ->done();

    cap_flag(cli.ctx, &cli.run)
        ->long_name("run")
        ->short_name('r')
        ->description("Run Aoc2025 after building.")
        ->done();

    cap_flag(cli.ctx, &cli.cache)
        ->long_name("no-cache")
        ->invert()
        ->description("Do not cache build steps, or use existing cache.")
        ->done();

    cap_option_enum(cli.ctx, Compiler, &cli.compiler)
        ->long_name("compiler")
        ->description("Choose compiler to build with. "
                      "Note that this option does not take a path to a compiler, "
                      "but rather a name. Ensure it is in PATH. "
                      #ifndef _MSC_VER
                      "A value of 'gcc' means using gcc and g++. "
                      "A value of 'clang' means using clang clang++."
                      #endif
            )
        ->metavar("name")
#ifdef _MSC_VER
        ->entry(COMPILER_CL, "cl")
        ->entry(COMPILER_CLANG_CL, "clang-cl")
        ->default_value(COMPILER_CL)
#else
        ->entry(COMPILER_GCC, "gcc")
        ->entry(COMPILER_CLANG, "clang")
        ->default_value(COMPILER_GCC)
#endif
        ->done();

    cap_flag(cli.ctx, &cli.package)
        ->long_name("package")
        ->short_name('p')
        ->description("Package distribution after build.")
        ->done();

    cap_flag(cli.ctx, &cli.emit_compile_commands)
        ->long_name("emit-compile-commands")
        ->description("Emit a clangd-compatible compile_commands.json file.")
        ->done();

    cap_flag(cli.ctx, &cli.emit_vscode_tasks)
        ->long_name("emit-vscode-tasks")
        ->description("emit task files for building and debugging Aoc2025 in VSCode.")
        ->done();

    cap_flag(cli.ctx, &cli.asan)
        ->long_name("asan")
        ->description("Build and link with address sanitizer.")
        ->done();

#ifndef _MSC_VER
    cap_flag(cli.ctx, &cli.ubsan)
        ->long_name("ubsan")
        ->description("Build and link with undefined behavior sanitizer.")
        ->done();
#endif

    cap_flag(cli.ctx, &cli.clean)
        ->long_name("clean")
        ->short_name('c')
        ->description("Clean cache and build artifacts.")
        ->done();

    int exit_code;
    if (cap_parse_and_handle(cli.ctx, argc, argv, &exit_code) == CAP_EXIT) {
        cap_context_free(cli.ctx);
        exit(exit_code);
    }

    if (cli.verbose) {
        NOB_NO_ECHO = false;
    }

    switch (cli.compiler) {
#ifdef _MSC_VER
    case COMPILER_CL: {
        CC = "cl";
        CXX = "cl";
    } break;
    case COMPILER_CLANG_CL: {
        CC = "clang-cl";
        CXX = "clang-cl";
    } break;
#else
    case COMPILER_GCC: {
        CC = "gcc";
        CXX = "g++";
    } break;
    case COMPILER_CLANG: {
        CC = "clang";
        CXX = "clang++";
    } break;
#endif
    }
}

static inline void
destroy_cli(void)
{
    cap_context_free(cli.ctx);
}

typedef struct {
    const char **items;
    size_t      capacity;
    size_t      count;
} Strings;

typedef struct {
    const char *source;
    const char *object;
} Target;

typedef struct {
    Target *items;
    size_t  capacity;
    size_t  count;
} Targets;

typedef struct {
    Strings  options;
    Strings  definitions;
    Strings  include_directories;
    Targets  targets;
} CompilationBlock;

typedef struct {
    CompilationBlock *items;
    size_t           capacity;
    size_t           count;
} CompilationBlocks;

typedef struct {
    Cmd    *items;
    size_t  capacity;
    size_t  count;
} Cmds;

static inline void
add_sanitizer_option(Cmd *cmd)
{
#ifdef _MSC_VER
    if (cli.asan) {
        cmd_append(cmd, "/fsanitize=address");
    }
#else
    if (cli.asan && cli.ubsan) cmd_append(cmd, "-fsanitize=address,undefined");
    else if (cli.asan)         cmd_append(cmd, "-fsanitize=address");
    else if (cli.ubsan)        cmd_append(cmd, "-fsanitize=undefined");
#endif
}

static Cmds
generate_compile_commands(CompilationBlocks *blocks)
{
    Cmds compile_commands = {0};

    da_foreach(CompilationBlock, block, blocks) {
        da_foreach(Target, target, &block->targets) {
            Cmd cmd = {0};

            if (cli.cache) {
                da_append(&cmd, CCACHE_BIN);
            }

            da_append(&cmd, CC);
#ifdef _MSC_VER
            da_append(&cmd, "/std:c17");
#else
            da_append(&cmd, "-std=c17");
#endif

            for (size_t i = 0; i < ARRAY_LENGTH(common_compile_options); ++i) {
                da_append(&cmd, common_compile_options[i]);
            }
            if (cli.debug) {
                for (size_t i = 0; i < ARRAY_LENGTH(debug_compile_options); ++i) {
                    da_append(&cmd, debug_compile_options[i]);
                }
            } else {
                for (size_t i = 0; i < ARRAY_LENGTH(release_compile_options); ++i) {
                    da_append(&cmd, release_compile_options[i]);
                }
            }
#ifdef _MSC_VER
            if (cli.compiler == COMPILER_CLANG_CL) da_append(&cmd, "-Wno-unused-command-line-argument");
#endif

            add_sanitizer_option(&cmd);

            da_foreach(const char *, option, &block->options) {
                da_append(&cmd, *option);
            }

            if (cli.debug) {
                for (size_t i = 0; i < ARRAY_LENGTH(debug_definitions); ++i) {
                    da_append(&cmd, debug_definitions[i]);
                }
            } else {
                for (size_t i = 0; i < ARRAY_LENGTH(release_definitions); ++i) {
                    da_append(&cmd, release_definitions[i]);
                }
            }

            da_foreach(const char *, definition, &block->definitions) {
#ifdef _MSC_VER
                da_append(&cmd, temp_sprintf("/D%s", *definition));
#else
                da_append(&cmd, temp_sprintf("-D%s", *definition));
#endif
            }

            da_foreach(const char *, dir, &block->include_directories) {
#ifdef _MSC_VER
                da_append(&cmd, temp_sprintf("/I%s", *dir));
#else
                da_append(&cmd, temp_sprintf("-I%s", *dir));
#endif
            }

#ifdef _MSC_VER
            da_append(&cmd, temp_sprintf("/Fo%s", target->object));
            da_append(&cmd, target->source);
#else
            da_append(&cmd, "-o");
            da_append(&cmd, target->object);
            da_append(&cmd, target->source);
#endif

            da_append(&compile_commands, cmd);
        }
    }

    return compile_commands;
}


typedef struct {
    const char *name;
    const char *license_path;
} ThirdPartyLicense;

typedef struct {
    ThirdPartyLicense *items;
    size_t             capacity;
    size_t             count;
} ThirdPartyLicenses;

static ThirdPartyLicenses third_party_licenses = {0};


#define WAYLAND_DIR           "vendor/wayland"
#define WAYLAND_GENERATED_DIR GENERATED_DIR"/wayland"

static inline bool
generate_wayland_files(void)
{
#ifdef _MSC_VER
    // Windows does not user wayland
    return true;
#else

    bool return_val = true;

    Cmd cmd = {0};
    Procs procs = {0};
    size_t chk = temp_save();

    DO_OR_FAIL(mkdir_if_not_exists(WAYLAND_GENERATED_DIR));

    static struct {
        const char *xml;
        const char *header;
        const char *code;
    } wayland_exports[] = {
        { .xml    = WAYLAND_DIR"/fractional-scale-v1.xml",
          .header = WAYLAND_GENERATED_DIR"/fractional-scale-v1-client-protocol.h",
          .code   = WAYLAND_GENERATED_DIR"/fractional-scale-v1-client-protocol-code.h" },
        { .xml    = WAYLAND_DIR"/idle-inhibit-unstable-v1.xml",
          .header = WAYLAND_GENERATED_DIR"/idle-inhibit-unstable-v1-client-protocol.h",
          .code   = WAYLAND_GENERATED_DIR"/idle-inhibit-unstable-v1-client-protocol-code.h" },
        { .xml    = WAYLAND_DIR"/pointer-constraints-unstable-v1.xml",
          .header = WAYLAND_GENERATED_DIR"/pointer-constraints-unstable-v1-client-protocol.h",
          .code   = WAYLAND_GENERATED_DIR"/pointer-constraints-unstable-v1-client-protocol-code.h" },
        { .xml    = WAYLAND_DIR"/relative-pointer-unstable-v1.xml",
          .header = WAYLAND_GENERATED_DIR"/relative-pointer-unstable-v1-client-protocol.h",
          .code   = WAYLAND_GENERATED_DIR"/relative-pointer-unstable-v1-client-protocol-code.h" },
        { .xml    = WAYLAND_DIR"/viewporter.xml",
          .header = WAYLAND_GENERATED_DIR"/viewporter-client-protocol.h",
          .code   = WAYLAND_GENERATED_DIR"/viewporter-client-protocol-code.h" },
        { .xml    = WAYLAND_DIR"/wayland.xml",
          .header = WAYLAND_GENERATED_DIR"/wayland-client-protocol.h",
          .code   = WAYLAND_GENERATED_DIR"/wayland-client-protocol-code.h" },
        { .xml    = WAYLAND_DIR"/xdg-activation-v1.xml",
          .header = WAYLAND_GENERATED_DIR"/xdg-activation-v1-client-protocol.h",
          .code   = WAYLAND_GENERATED_DIR"/xdg-activation-v1-client-protocol-code.h" },
        { .xml    = WAYLAND_DIR"/xdg-decoration-unstable-v1.xml",
          .header = WAYLAND_GENERATED_DIR"/xdg-decoration-unstable-v1-client-protocol.h",
          .code   = WAYLAND_GENERATED_DIR"/xdg-decoration-unstable-v1-client-protocol-code.h" },
        { .xml    = WAYLAND_DIR"/xdg-shell.xml",
          .header = WAYLAND_GENERATED_DIR"/xdg-shell-client-protocol.h",
          .code   = WAYLAND_GENERATED_DIR"/xdg-shell-client-protocol-code.h" }
    };

    for (size_t i = 0; i < ARRAY_LENGTH(wayland_exports); ++i) {
        cmd.count = 0;
        const char *xml    = wayland_exports[i].xml;
        const char *header = wayland_exports[i].header;
        const char *code   = wayland_exports[i].code;

        cmd_append(&cmd, "wayland-scanner", "client-header", xml, header);
        DO_OR_FAIL(cmd_run(&cmd, .async = &procs, .max_procs = cli.jobs));

        cmd_append(&cmd, "wayland-scanner", "private-code", xml, code);
        DO_OR_FAIL(cmd_run(&cmd, .async = &procs, .max_procs = cli.jobs));
    }
    DO_OR_FAIL(procs_flush(&procs));

    printf("Generated Wayland headers\n"); fflush(stdout);

done:
    temp_rewind(chk);
    da_free(procs);
    da_free(cmd);
    fflush(stdout);

    ThirdPartyLicense license = {"Wayland", WAYLAND_DIR"/COPYING"};
    da_append(&third_party_licenses, license);

    return return_val;
#endif
}


#define GLFW_DIR          "vendor/glfw"
#define GLFW_SRC_DIR      GLFW_DIR"/src"
#define GLFW_INCLUDE_DIR  GLFW_DIR"/include"
#define GLFW_OBJECT_DIR   OBJECT_DIR"/glfw"

static inline bool
prepare_glfw(CompilationBlocks *blocks)
{
    if (!mkdir_if_not_exists(GLFW_OBJECT_DIR)) return false;

    CompilationBlock block = {0};

    static Target glfw_targets[] = {
        { .source = GLFW_SRC_DIR"/context.c",
          .object = GLFW_OBJECT_DIR"/context"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/egl_context.c",
          .object = GLFW_OBJECT_DIR"/egl_context"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/init.c",
          .object = GLFW_OBJECT_DIR"/init"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/input.c",
          .object = GLFW_OBJECT_DIR"/input"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/monitor.c",
          .object = GLFW_OBJECT_DIR"/monitor"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/null_init.c",
          .object = GLFW_OBJECT_DIR"/null_init"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/null_joystick.c",
          .object = GLFW_OBJECT_DIR"/null_joystick"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/null_monitor.c",
          .object = GLFW_OBJECT_DIR"/null_monitor"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/null_window.c",
          .object = GLFW_OBJECT_DIR"/null_window"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/osmesa_context.c",
          .object = GLFW_OBJECT_DIR"/osmesa_context"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/platform.c",
          .object = GLFW_OBJECT_DIR"/platform"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/vulkan.c",
          .object = GLFW_OBJECT_DIR"/vulkan"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/wgl_context.c",
          .object = GLFW_OBJECT_DIR"/wgl_context"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/window.c",
          .object = GLFW_OBJECT_DIR"/window"OBJ_FILE_EXT},
#ifdef _MSC_VER
        { .source = GLFW_SRC_DIR"/win32_init.c",
          .object = GLFW_OBJECT_DIR"/win32_init"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/win32_joystick.c",
          .object = GLFW_OBJECT_DIR"/win32_joystick"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/win32_module.c",
          .object = GLFW_OBJECT_DIR"/win32_module"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/win32_monitor.c",
          .object = GLFW_OBJECT_DIR"/win32_monitor"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/win32_thread.c",
          .object = GLFW_OBJECT_DIR"/win32_thread"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/win32_time.c",
          .object = GLFW_OBJECT_DIR"/win32_time"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/win32_window.c",
          .object = GLFW_OBJECT_DIR"/win32_window"OBJ_FILE_EXT}
#else
        { .source = GLFW_SRC_DIR"/glx_context.c",
          .object = GLFW_OBJECT_DIR"/glx_context"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/linux_joystick.c",
          .object = GLFW_OBJECT_DIR"/linux_joystick"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/posix_module.c",
          .object = GLFW_OBJECT_DIR"/posix_module"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/posix_poll.c",
          .object = GLFW_OBJECT_DIR"/posix_poll"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/posix_thread.c",
          .object = GLFW_OBJECT_DIR"/posix_thread"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/posix_time.c",
          .object = GLFW_OBJECT_DIR"/posix_time"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/wl_init.c",
          .object = GLFW_OBJECT_DIR"/wl_init"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/wl_monitor.c",
          .object = GLFW_OBJECT_DIR"/wl_monitor"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/wl_window.c",
          .object = GLFW_OBJECT_DIR"/wl_window"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/x11_init.c",
          .object = GLFW_OBJECT_DIR"/x11_init"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/x11_monitor.c",
          .object = GLFW_OBJECT_DIR"/x11_monitor"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/x11_window.c",
          .object = GLFW_OBJECT_DIR"/x11_window"OBJ_FILE_EXT},
        { .source = GLFW_SRC_DIR"/xkb_unicode.c",
          .object = GLFW_OBJECT_DIR"/xkb_unicode"OBJ_FILE_EXT}
#endif
    };

    for (size_t i = 0; i < ARRAY_LENGTH(glfw_targets); ++i) {
        da_append(&block.targets, glfw_targets[i]);
    }

    da_append(&block.include_directories, GLFW_INCLUDE_DIR);

#ifdef _MSC_VER
    da_append(&block.definitions, "_GLFW_WIN32");
    da_append(&block.options, "/wd4005");
#else
    da_append(&block.include_directories, WAYLAND_GENERATED_DIR);
    da_append(&block.definitions, "_POSIX_C_SOURCE=200809L");
    da_append(&block.definitions, "_GLFW_WAYLAND");
    da_append(&block.definitions, "_GLFW_X11");
#endif

    da_append(blocks, block);

    ThirdPartyLicense license = {"GLFW", GLFW_DIR"/LICENSE.md"};
    da_append(&third_party_licenses, license);

    return true;
}


#define RAYLIB_DIR "vendor/raylib-5.5/"
#define RAYLIB_SRC_DIR RAYLIB_DIR"/src"
#define RAYLIB_OBJECT_DIR OBJECT_DIR"/raylib"

static inline bool
prepare_raylib(CompilationBlocks *blocks)
{
    if (!mkdir_if_not_exists(RAYLIB_OBJECT_DIR)) return false;

    CompilationBlock block = {0};

    static Target raylib_targets[] = {
        { .source = RAYLIB_SRC_DIR"/raudio.c",
          .object = RAYLIB_OBJECT_DIR"/raudio"OBJ_FILE_EXT},
        { .source = RAYLIB_SRC_DIR"/rcore.c",
          .object = RAYLIB_OBJECT_DIR"/rcore"OBJ_FILE_EXT},
        { .source = RAYLIB_SRC_DIR"/rmodels.c",
          .object = RAYLIB_OBJECT_DIR"/rmodels"OBJ_FILE_EXT},
        { .source = RAYLIB_SRC_DIR"/rshapes.c",
          .object = RAYLIB_OBJECT_DIR"/rshapes"OBJ_FILE_EXT},
        { .source = RAYLIB_SRC_DIR"/rtext.c",
          .object = RAYLIB_OBJECT_DIR"/rtext"OBJ_FILE_EXT},
        { .source = RAYLIB_SRC_DIR"/rtextures.c",
          .object = RAYLIB_OBJECT_DIR"/rtextures"OBJ_FILE_EXT},
        { .source = RAYLIB_SRC_DIR"/utils.c",
          .object = RAYLIB_OBJECT_DIR"/utils"OBJ_FILE_EXT}
    };
    for (size_t i = 0; i < ARRAY_LENGTH(raylib_targets); ++i) {
        da_append(&block.targets, raylib_targets[i]);
    }

    da_append(&block.definitions, "PLATFORM_DESKTOP_GLFW");
    da_append(&block.include_directories, GLFW_INCLUDE_DIR);

#ifdef _MSC_VER
    da_append(&block.options, "/wd4005");
#endif

    ThirdPartyLicense license = {"raylib", RAYLIB_DIR"/LICENSE"};
    da_append(&third_party_licenses, license);

    da_append(blocks, block);

    return true;
}


#define CLAY_DIR "vendor/clay-0.14/"
#define CLAY_OBJECT_DIR OBJECT_DIR"/clay"

static inline bool
prepare_clay(CompilationBlocks *blocks)
{
    if (!mkdir_if_not_exists(CLAY_OBJECT_DIR)) return false;

    CompilationBlock block = {0};

    static Target clay_targets[] = {
        { .source = CLAY_DIR"/clay.c",
          .object = CLAY_OBJECT_DIR"/clay"OBJ_FILE_EXT}
    };
    for (size_t i = 0; i < ARRAY_LENGTH(clay_targets); ++i) {
        da_append(&block.targets, clay_targets[i]);
    }

    da_append(&block.include_directories, RAYLIB_SRC_DIR);
    da_append(&block.definitions, "PLATFORM_DESKTOP_GLFW");
    da_append(&block.include_directories, GLFW_INCLUDE_DIR);

    // Clay produces a lot of warnings, let's ignore them
#ifdef _MSC_VER
    da_append(&block.options, "/w");
#else
    da_append(&block.options, "-w");
#endif


    ThirdPartyLicense license = {"clay", CLAY_DIR"/LICENSE.md"};
    da_append(&third_party_licenses, license);

    da_append(blocks, block);

    return true;
}


#define SV_DIR "vendor/sv/"
#define SV_OBJECT_DIR OBJECT_DIR"/sv"

static inline bool
prepare_sv(CompilationBlocks *blocks)
{
    if (!mkdir_if_not_exists(SV_OBJECT_DIR)) return false;

    CompilationBlock block = {0};

    static Target sv_targets[] = {
        { .source = SV_DIR"/sv.c",
          .object = SV_OBJECT_DIR"/sv"OBJ_FILE_EXT}
    };
    for (size_t i = 0; i < ARRAY_LENGTH(sv_targets); ++i) {
        da_append(&block.targets, sv_targets[i]);
    }


    ThirdPartyLicense license = {"sv", SV_DIR"/LICENSE"};
    da_append(&third_party_licenses, license);

    da_append(blocks, block);

    return true;
}


static inline bool
prepare_aoc2025(CompilationBlocks *blocks)
{
    if (!mkdir_if_not_exists(AOC2025_OBJECT_DIR)) return false;

    CompilationBlock block = {0};

    Target targets[] = {
        { .source = SRC_DIR"/aoc2025.c",
          .object = AOC2025_OBJECT_DIR"/aoc2025"OBJ_FILE_EXT},
        { .source = SRC_DIR"/basic.c",
          .object = AOC2025_OBJECT_DIR"/basic"OBJ_FILE_EXT},
        { .source = SRC_DIR"/day1.c",
          .object = AOC2025_OBJECT_DIR"/day1"OBJ_FILE_EXT},
        { .source = SRC_DIR"/day2.c",
          .object = AOC2025_OBJECT_DIR"/day2"OBJ_FILE_EXT},
        { .source = SRC_DIR"/day3.c",
          .object = AOC2025_OBJECT_DIR"/day3"OBJ_FILE_EXT},
        { .source = SRC_DIR"/day4.c",
          .object = AOC2025_OBJECT_DIR"/day4"OBJ_FILE_EXT},
#ifdef _MSC_VER
        { .source = SRC_DIR"/win32_aoc2025.c",
          .object = AOC2025_OBJECT_DIR"/win32_aoc2025"OBJ_FILE_EXT},
#else
        { .source = SRC_DIR"/linux_aoc2025.c",
          .object = AOC2025_OBJECT_DIR"/linux_aoc2025"OBJ_FILE_EXT},
#endif
    };
    for (size_t i = 0; i < ARRAY_LENGTH(targets); ++i) {
        da_append(&block.targets, targets[i]);
    }

    for (size_t i = 0; i < ARRAY_LENGTH(aoc2025_compile_options); ++i) {
        da_append(&block.options, aoc2025_compile_options[i]);
    }

    da_append(&block.include_directories, "vendor/cap/");

    da_append(&block.include_directories, RAYLIB_SRC_DIR);
    da_append(&block.include_directories, CLAY_DIR);
    da_append(&block.include_directories, SV_DIR);
    da_append(&block.include_directories, AOC2025_GENERATED_DIR);

#ifdef _MSC_VER
    da_append(&block.options, "/wd4244");
    da_append(&block.options, "/wd4305");
#endif

    da_append(blocks, block);

    return true;
}

static inline bool
build_objects(Cmds *compile_commands)
{
    bool return_val = true;

    Procs procs = {0};
    da_foreach(Cmd, compile_command, compile_commands) {
        // Awkward if formatting, but whatever
        if (
#ifdef _MSC_VER
        cli.compiler == COMPILER_CLANG_CL
#else
        cli.compiler == COMPILER_GCC || cli.compiler == COMPILER_CLANG
#endif
            ) {
            size_t chk = temp_save();
            printf("%s\n", temp_file_name(compile_command->items[compile_command->count-1]));
            fflush(stdout);
            temp_rewind(chk);
        }

        DO_OR_FAIL(cmd_run(compile_command, .async = &procs, .max_procs = cli.jobs));
    }

    DO_OR_FAIL(procs_flush(&procs));

done:
    da_free(procs);

    return return_val;
}


static inline bool
generate_win32_resource_file(void)
{
#ifndef _MSC_VER
    return true;
#endif

    bool return_val = true;

    Cmd cmd = {0};

    const char *cpp_header_file_content = "#define IDI_APP_ICON 101\n";
    const char *resource_file_content =
        "#include \"win32_resource.h\"\n"
        "IDI_APP_ICON ICON \"data/win32_icon.ico\"\n";
    DO_OR_FAIL(write_entire_file(AOC2025_GENERATED_DIR"/win32_resource.h", cpp_header_file_content, strlen(cpp_header_file_content)));
    DO_OR_FAIL(write_entire_file(AOC2025_GENERATED_DIR"/win32_resource.rc", resource_file_content, strlen(resource_file_content)));

    const char *rc_command[] = {"rc", "/fo", AOC2025_GENERATED_DIR"/win32_resource.res", AOC2025_GENERATED_DIR"/win32_resource.rc"};
    da_append_many(&cmd, rc_command, ARRAY_LENGTH(rc_command));
    DO_OR_FAIL(cmd_run(&cmd));

done:
    da_free(cmd);
    return return_val;
}

static inline bool
link_aoc2025(CompilationBlocks *blocks)
{
    bool return_val = true;

    printf("\nLinking \""AOC2025_BIN"\"...\n"); fflush(stdout);

    Cmd cmd = {0};

    da_append(&cmd, CXX);

#ifdef _MSC_VER
    da_append(&cmd, temp_sprintf("/Fe:%s", AOC2025_BIN));
#else
    da_append(&cmd, "-o");
    da_append(&cmd,  AOC2025_BIN);
#endif

    da_foreach(CompilationBlock, block, blocks) {
        da_foreach(Target, target, &block->targets) {
            da_append(&cmd, target->object);
        }
    }

#ifdef _MSC_VER
    da_append(&cmd, AOC2025_GENERATED_DIR"/win32_resource.res");
#endif

    for (size_t i = 0; i < ARRAY_LENGTH(common_link_options); ++i) {
        da_append(&cmd, common_link_options[i]);
    }
    if (cli.debug) {
        for (size_t i = 0; i < ARRAY_LENGTH(debug_link_options); ++i) {
            da_append(&cmd, debug_link_options[i]);
        }
    } else {
        for (size_t i = 0; i < ARRAY_LENGTH(release_link_options); ++i) {
            da_append(&cmd, release_link_options[i]);
        }
    }

#ifndef _MSC_VER
    // gcc/clang wants sanitizers to be specified both during object building and linking,
    // while cl is happy just being informed during object building.
    add_sanitizer_option(&cmd);
#endif

    DO_OR_FAIL(cmd_run(&cmd));
done:
    da_free(cmd);

    return return_val;
}

static inline bool
generate_compilation_database(Cmds *compile_commands)
{
    size_t checkpoint = temp_save();

    String_Builder sb = {0};

    char *cwd = (char *)get_current_dir_temp();
    const size_t cwd_len = strlen(cwd);
#ifdef _MSC_VER
    for (size_t i = 0; i < cwd_len; ++i) {
        char *c = &cwd[i];
        if (*c == '\\') *c = '/';
    }
#endif

    sb_append_cstr(&sb, "[\n");

    for (size_t i = 0; i < compile_commands->count; ++i) {
        Cmd compile_command = compile_commands->items[i];

        sb_append_cstr(&sb, "  {\n");

        sb_appendf(&sb, "    \"directory\": \"%s\",\n", cwd);

        sb_append_cstr(&sb, "    \"arguments\": [");
        for (size_t j = 0; j < compile_command.count; ++j) {
            if (j > 0) {
                sb_append_cstr(&sb, ", ");
            }
            sb_appendf(&sb, "\"%s\"", compile_command.items[j]);
        }
        sb_append_cstr(&sb, "],\n");

        sb_appendf(&sb, "    \"file\": \"%s\"\n", compile_command.items[compile_command.count-1]);

        sb_append_cstr(&sb, "  }");
        if (i < compile_commands->count-1) {
            sb_append_cstr(&sb, ",");
        }
        sb_append_cstr(&sb, " \n");
    }

    sb_append_cstr(&sb, "]\n");

    bool return_val = write_entire_file("compile_commands.json", sb.items, sb.count);

    sb_free(sb);
    temp_rewind(checkpoint);
    return return_val;
}

static inline bool
generate_vscode_tasks(void)
{
    bool return_val = true;

    DO_OR_FAIL(mkdir_if_not_exists(".vscode"));

#ifdef _MSC_VER
    static const char *tasks_content =
        "{\n"
        "  \"version\": \"2.0.0\",\n"
        "  \"tasks\": [\n"
        "    {\n"
        "      \"label\": \"Build C++ project\",\n"
        "      \"type\": \"shell\",\n"
        "      \"command\": \"./bs.exe\",\n"
        "      \"args\": [\n"
        "        \"--emit-compile-commands\",\n"
        "        \"--emit-vscode-tasks\",\n"
        "        \"--debug\"\n"
        "      ],\n"
        "      \"problemMatcher\": [],\n"
        "      \"presentation\": {\n"
        "        \"reveal\": \"always\",\n"
        "        \"focus\": false,\n"
        "        \"panel\": \"dedicated\",\n"
        "        \"showReuseMessage\": false,\n"
        "        \"clear\": false,\n"
        "        \"close\": false\n"
        "      },\n"
        "      \"runOptions\": {\n"
        "        \"reevaluateOnRerun\": false\n"
        "      },\n"
        "      \"options\": {\n"
        "        \"cwd\": \"${workspaceFolder}\"\n"
        "      },\n"
        "      \"group\": {\n"
        "        \"kind\": \"build\",\n"
        "        \"isDefault\": true\n"
        "      },\n"
        "      \"detail\": \"Custom C++ build command\"\n"
        "    }\n"
        "  ]\n"
        "}\n";
#else
    static const char *tasks_content =
        "{\n"
        "  \"version\": \"2.0.0\",\n"
        "  \"tasks\": [\n"
        "    {\n"
        "      \"label\": \"Build C++ project\",\n"
        "      \"type\": \"shell\",\n"
        "      \"command\": \"./bs\",\n"
        "      \"args\": [\n"
        "        \"--emit-compile-commands\",\n"
        "        \"--emit-vscode-tasks\",\n"
        "        \"--debug\"\n"
        "      ],\n"
        "      \"options\": {\n"
        "        \"cwd\": \"${workspaceFolder}\"\n"
        "      },\n"
        "      \"group\": {\n"
        "        \"kind\": \"build\",\n"
        "        \"isDefault\": true\n"
        "      },\n"
        "      \"detail\": \"Custom C++ build command\"\n"
        "    }\n"
        "  ]\n"
        "}\n";
#endif

    DO_OR_FAIL(write_entire_file(".vscode/tasks.json", tasks_content, strlen(tasks_content)));

#ifdef _MSC_VER
    static const char *launch_content =
        "{\n"
        "  \"version\": \"0.2.0\",\n"
        "  \"configurations\": [\n"
        "    {\n"
        "      \"name\": \"Run C++ (Windows, MSVC)\",\n"
        "      \"type\": \"cppvsdbg\",\n"
        "      \"request\": \"launch\",\n"
        "      \"program\": \"${workspaceFolder}/"AOC2025_BIN"\",\n"
        "      \"args\": [],\n"
        "      \"cwd\": \"${workspaceFolder}\",\n"
        "      \"environment\": [],\n"
        "      \"console\": \"integratedTerminal\"\n"
        "    }\n"
        "  ]\n"
        "}\n";

#else
    static const char *launch_content =
        "{\n"
        "  \"version\": \"0.2.0\",\n"
        "  \"configurations\": [\n"
        "    {\n"
        "      \"name\": \"Run C++ (Linux, gdb)\",\n"
        "      \"type\": \"cppdbg\",\n"
        "      \"request\": \"launch\",\n"
        "      \"program\": \"${workspaceFolder}/"AOC2025_BIN"\",\n"
        "      \"args\": [],\n"
        "      \"cwd\": \"${workspaceFolder}\",\n"
        "      \"environment\": [],\n"
        "      \"externalConsole\": false,\n"
        "      \"MIMode\": \"gdb\",\n"
        "      \"miDebuggerPath\": \"/usr/bin/gdb\",\n"
        "      \"setupCommands\": [\n"
        "        { \"text\": \"-enable-pretty-printing\", \"description\": \"Enable pretty printing\", \"ignoreFailures\": true }\n"
        "      ],\n"
        "    }\n"
        "  ]\n"
        "}\n";
#endif

    DO_OR_FAIL(write_entire_file(".vscode/launch.json", launch_content, strlen(launch_content)));

done:

    return return_val;
}

static inline bool
generate_stuff(CompilationBlocks *blocks, Cmds *out_compile_commands)
{
    bool return_val = true;

    DO_OR_FAIL(mkdir_if_not_exists(BUILD_DIR));
    DO_OR_FAIL(mkdir_if_not_exists(OBJECT_DIR));
    DO_OR_FAIL(mkdir_if_not_exists(GENERATED_DIR));
    DO_OR_FAIL(mkdir_if_not_exists(AOC2025_GENERATED_DIR));

    DO_OR_FAIL(generate_win32_resource_file());
    DO_OR_FAIL(generate_wayland_files());

    DO_OR_FAIL(prepare_aoc2025(blocks));
    DO_OR_FAIL(prepare_raylib(blocks));
    DO_OR_FAIL(prepare_glfw(blocks));
    DO_OR_FAIL(prepare_sv(blocks));
    DO_OR_FAIL(prepare_clay(blocks));

    *out_compile_commands = generate_compile_commands(blocks);

    if (cli.emit_compile_commands) {
        DO_OR_FAIL(generate_compilation_database(out_compile_commands));
    }

    if (cli.emit_vscode_tasks) {
        DO_OR_FAIL(generate_vscode_tasks());
    }

done:

    return return_val;
}

static inline bool
generate_third_party_licenses_file(void)
{
    String_Builder sb = {0};

    bool return_val = true;

    sb_append_cstr(&sb, "=====================================================\n");
    sb_append_cstr(&sb, "LICENSES FOR ALL THIRD-PARTY MATERIAL USED IN AOC2025\n");
    sb_append_cstr(&sb, "=====================================================\n");

    da_foreach(ThirdPartyLicense, license, &third_party_licenses) {
        sb_appendf(&sb, "\n\n==================== %s ====================\n\n", license->name);
        DO_OR_FAIL(read_entire_file(license->license_path, &sb));
        sb_append_cstr(&sb, "\n");
    }

    DO_OR_FAIL(write_entire_file(LEGAL_DIR"/third_party_licenses.txt", sb.items, sb.count));

done:
    sb_free(sb);

    return return_val;
}

static inline bool
package_distribution(void)
{
    printf("Packaging distribution...\n");
    fflush(stdout);
    Cmd cmd = {0};
    int return_val = true;


    // Make and copy everything into distribution directory
    if (file_exists(AOC2025_DISTRIBUTION_DIR)) DO_OR_FAIL(nob_delete_tree(AOC2025_DISTRIBUTION_DIR));
    DO_OR_FAIL(mkdir_if_not_exists(AOC2025_DISTRIBUTION_DIR));
    DO_OR_FAIL(copy_directory_recursively(DATA_DIR, AOC2025_DISTRIBUTION_DIR"/data"));
    DO_OR_FAIL(copy_file(AOC2025_BIN, AOC2025_DISTRIBUTION_DIR"/"AOC2025_BIN_NAME));
#ifndef _MSC_VER
    cmd_append(&cmd, "strip", "-s", AOC2025_DISTRIBUTION_DIR"/"AOC2025_BIN_NAME);
    DO_OR_FAIL(cmd_run_sync_and_reset(&cmd));
#endif

    if (file_exists(LEGAL_DIR)) DO_OR_FAIL(delete_tree(LEGAL_DIR));
    DO_OR_FAIL(mkdir_if_not_exists(LEGAL_DIR));
    DO_OR_FAIL(copy_file("LICENSE", LEGAL_DIR"/LICENSE.txt"));
    // Generate things
    DO_OR_FAIL(generate_third_party_licenses_file());

    // Archive
    if (file_exists(AOC2025_DISTRIBUTION_ARCHIVE)) DO_OR_FAIL(delete_file(AOC2025_DISTRIBUTION_ARCHIVE));
#ifdef _MSC_VER
    cmd_append(&cmd, "powershell", "Compress-Archive", AOC2025_DISTRIBUTION_DIR"/*", AOC2025_DISTRIBUTION_ARCHIVE);
#else
    cmd_append(&cmd, "tar", "-czf", AOC2025_DISTRIBUTION_ARCHIVE, "-C", BUILD_DIR, AOC2025_DISTRIBUTION_DIR_NAME);
#endif
    DO_OR_FAIL(cmd_run_sync_and_reset(&cmd));


    // Make checksum of distribution archive
    {
        const char *cwd = get_current_dir_temp();
        DO_OR_FAIL(set_current_dir(BUILD_DIR));
        if (file_exists(AOC2025_DISTRIBUTION_HASH_FILE_NAME)) DO_OR_FAIL(delete_file(AOC2025_DISTRIBUTION_HASH_FILE_NAME));
#ifdef _MSC_VER
        cmd_append(&cmd, "powershell", "-Command", "$h=(Get-FileHash '"AOC2025_DISTRIBUTION_ARCHIVE_NAME"' -Algorithm SHA256).Hash.ToLower(); Write-Output \"$h  "AOC2025_DISTRIBUTION_ARCHIVE_NAME"\"");
#else
        cmd_append(&cmd, "sha256sum", AOC2025_DISTRIBUTION_ARCHIVE_NAME);
#endif
        bool ok = cmd_run(&cmd, .stdout_path = AOC2025_DISTRIBUTION_HASH_FILE_NAME);
        set_current_dir(cwd);
        if (!ok) {
            return_val = false;
            goto done;
        }
    }

done:
    da_free(cmd);

    return return_val;
}

int
main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    init_cli(argc, argv);

    Cmd cmd = {0};
    CompilationBlocks blocks = {0};
    Cmds compile_commands = {0};

    int return_val = EXIT_SUCCESS;
    if (cli.clean) {
        if (file_exists(BUILD_DIR))               delete_tree(BUILD_DIR);
        if (file_exists("compile_commands.json")) delete_file("compile_commands.json");
        if (file_exists(".vscode"))               delete_tree(".vscode");
        if (file_exists(".cache"))                delete_tree(".cache");
        printf("Deleted all build artifacts and cleared build cache.\n");
        goto done;
    }

    mkdir_if_not_exists(BUILD_DIR);

    if (cli.cache) {
        mkdir_if_not_exists(CACHE_DIR);
        mkdir_if_not_exists(CCACHE_CACHE_DIR);
#ifdef _MSC_VER
        _putenv_s("CCACHE_DIR", CCACHE_CACHE_DIR);
#else
        setenv("CCACHE_DIR", CCACHE_CACHE_DIR, 1);
#endif

        // Clear ccache stats
        cmd_append(&cmd, CCACHE_BIN, "-z");
        if (!cmd_run(&cmd,
                     .stdout_path = CCACHE_CACHE_DIR"/garbage_out",
                     .stderr_path = CCACHE_CACHE_DIR"/garbage_err")) {
            return_val = EXIT_FAILURE;
        }
    }

    const uint64_t before_generate_stuff_ns = nanos_since_unspecified_epoch();
    if (!generate_stuff(&blocks, &compile_commands)) {
        return_val = EXIT_FAILURE;
        goto done;
    }
    const uint64_t after_generate_stuff_ns = nanos_since_unspecified_epoch();
    const uint64_t generate_stuff_time = after_generate_stuff_ns - before_generate_stuff_ns;

    if (!mkdir_if_not_exists(BIN_DIR)) {
        return_val = EXIT_FAILURE;
        goto done;
    }

    const uint64_t before_compilation_ns = nanos_since_unspecified_epoch();
    if (!build_objects(&compile_commands)) {
        return_val = EXIT_FAILURE;
        goto done;
    }
    const uint64_t after_compilation_ns = nanos_since_unspecified_epoch();
    const uint64_t compilation_time = after_compilation_ns - before_compilation_ns;

    const uint64_t before_linking_ns = nanos_since_unspecified_epoch();
    if(!link_aoc2025(&blocks)) {
        return_val = EXIT_FAILURE;
        goto done;
    }
    const uint64_t after_linking_ns = nanos_since_unspecified_epoch();
    const uint64_t linking_time = after_linking_ns - before_linking_ns;

    const uint64_t before_package_ns = nanos_since_unspecified_epoch();
    if (cli.package) {
        if (!package_distribution()) {
            return_val = EXIT_FAILURE;
            goto done;
        }
    }
    const uint64_t after_package_ns = nanos_since_unspecified_epoch();
    const uint64_t package_time = after_package_ns - before_package_ns;

    printf("\n==== BUILD FINISHED ===\n\n");

    if (cli.cache) {
        // Print ccache stats for this build
        cmd_append(&cmd, CCACHE_BIN, "-s");
        const char *ccache_stats_file = CCACHE_CACHE_DIR"/ccache_stats.txt";
        if (cmd_run(&cmd, .stdout_path = ccache_stats_file)) {
            String_Builder sb = {0};
            if (read_entire_file(ccache_stats_file, &sb)) {
                sb_append_null(&sb);
                printf(" == CACHE ==\n%s\n", sb.items);
            }
            sb_free(sb);
        }
        cmd.count = 0;
    }

    size_t checkpoint = temp_save();

    const char     *generate_stuff_time_fmt     = temp_sprintf("%.4f", (double)generate_stuff_time  / NANOS_PER_SEC);
    const size_t    generate_stuff_time_fmt_len = strlen(generate_stuff_time_fmt);
    const char     *compilation_time_fmt        = temp_sprintf("%.4f", (double)compilation_time / NANOS_PER_SEC);
    const size_t    compilation_time_fmt_len    = strlen(compilation_time_fmt);
    const char     *linking_time_fmt            = temp_sprintf("%.4f", (double)linking_time / NANOS_PER_SEC);
    const size_t    linking_time_fmt_len        = strlen(linking_time_fmt);
    const char     *package_time_fmt            = temp_sprintf("%.4f", (double)package_time     / NANOS_PER_SEC);
    const size_t    package_time_fmt_len        = strlen(package_time_fmt);
    const uint64_t  total_time                  = generate_stuff_time + compilation_time + linking_time + (cli.package ? package_time : 0);
    const char     *total_time_fmt              = temp_sprintf("%.4f", (double)total_time  / NANOS_PER_SEC);
    const size_t    total_time_fmt_len          = strlen(total_time_fmt);
    size_t          longest_num                 = generate_stuff_time_fmt_len;
    if (compilation_time_fmt_len > longest_num) longest_num = compilation_time_fmt_len;
    if (linking_time_fmt_len > longest_num) longest_num = linking_time_fmt_len;
    if (cli.package && package_time_fmt_len > longest_num) longest_num = package_time_fmt_len;
    if (total_time_fmt_len > longest_num) longest_num = total_time_fmt_len;
    const size_t generate_stuff_time_pad  = longest_num - generate_stuff_time_fmt_len;
    const size_t compilation_time_pad = longest_num - compilation_time_fmt_len;
    const size_t linking_time_pad = longest_num - linking_time_fmt_len;
    const size_t package_time_pad     = longest_num - package_time_fmt_len;
    const size_t total_time_pad       = longest_num - total_time_fmt_len;
    printf("\n == Timing ==\n");
    printf("Generation..: %*s%s seconds\n", (int)generate_stuff_time_pad, "", generate_stuff_time_fmt);
    printf("Compilation.: %*s%s seconds\n", (int)compilation_time_pad, "", compilation_time_fmt);
    printf("Linking.....: %*s%s seconds\n", (int)linking_time_pad, "", linking_time_fmt);
    if (cli.package) printf("Packaging...: %*s%s seconds\n", (int)package_time_pad, "", package_time_fmt);
    printf("              "); for (size_t i = 0; i < longest_num; ++i) printf("%c", '-'); printf("--------\n");
    printf("Total.......: %*s%s seconds\n", (int)total_time_pad, "", total_time_fmt);
    fflush(stdout);
    fflush(stderr);

    temp_rewind(checkpoint);

    printf("\n\n == BUILD ARTIFACTS ==\n");
    printf("Built binary: \""AOC2025_BIN"\". (Run from root directory of source tree)\n");
    if (cli.package) {
        printf("Archived distribution: \""AOC2025_DISTRIBUTION_ARCHIVE"\"\n");
        printf("SHA256 sum of archive: \""AOC2025_DISTRIBUTION_HASH_FILE"\"\n");
    }
    printf("\n"); fflush(stdout);

    if (cli.run) {
        cmd_append(&cmd, AOC2025_BIN);
        printf("\nRunning :: "AOC2025_BIN" ");
        for (int i = 0; i < cli.remainder_argc; ++i) {
            cmd_append(&cmd, cli.remainder_argv[(size_t)i]);
            printf(" %s", cli.remainder_argv[(size_t)i]);
        }
        printf("\n");
        fflush(stdout);
        fflush(stderr);
        if (!cmd_run_sync_and_reset(&cmd)) return_val = 1;
        goto done;
    }

done:
    da_free(compile_commands);
    da_free(blocks);
    da_free(cmd);
    destroy_cli();

    return return_val;
}
