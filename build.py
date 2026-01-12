import os
from pathlib import Path

# ---- CONFIG -------------------------------------------------

CLANG = r"C:/Program Files/LLVM/bin/clang.exe"
LINKER = r"C:/Program Files/LLVM/bin/clang.exe"
# we use -fuse-ld=lld to use LLVM's linker

MINGW = r"C:/raylib/w64devkit"  # CHANGE if different

BUILD_DIR = Path("build")
OBJ_DIR = BUILD_DIR / "obj"
BIN_DIR = Path("bin")

ENGINE_SRC = [
    "source/asset_loader/asset_loader.c",
    #################
    "source/input/input.c",
    "source/input/input_internal.c",
    #################
    "source/window_controller/window_controller.c",
    "source/window_controller/window_border.c",
    "source/window_controller/window_drawer.c",
    #################
]

APP_SRC = [
    "source/app/app.c",
]

THIRD_SRC = [
    "include/lz4/lz4.c",
    "include/xxhash/xxhash.c",
]

RAYLIB_LIB = "lib/libraylib.a"
EXE_NAME = "undertale-engine.exe"

# ---- ASSET PACKER -------------------------------------------

ASSET_PACKER_SRC = "tools/asset-packer.c"
ASSET_PACKER_OBJ = "build/obj/asset_packer.o"
ASSET_PACKER_EXE = "bin/asset-packer.exe"

ASSET_PACKER_DEPENDENCIES = [
    THIRD_SRC[0],
    THIRD_SRC[1],
]

ASSET_INPUT_DIR = "assets"
ASSET_OUTPUT_BASE = "bin/data0"

# -------------------------------------------------------------

ALL_SRC = ENGINE_SRC + APP_SRC + THIRD_SRC


def obj_name(src):
    return OBJ_DIR / (Path(src).stem + ".o")


BUILD_DIR.mkdir(exist_ok=True)
OBJ_DIR.mkdir(parents=True, exist_ok=True)
BIN_DIR.mkdir(exist_ok=True)

with open("build.ninja", "w") as f:
    f.write(f"""
cc = "{CLANG}"
ld = "{LINKER}"

cflags = -std=c11 -O2 -g -Wall -Wextra --target=x86_64-w64-mingw32 --sysroot={MINGW} -Iinclude -Icommon -Isource -DDEBUG

ldflags = --target=x86_64-w64-mingw32 -static -lopengl32 -lgdi32 -lwinmm -lwinpthread

rule cc
  command = $cc $cflags -c $in -o $out
  description = CC $in

rule link
  command = $ld $in {RAYLIB_LIB} -o $out $ldflags -fuse-ld=lld
  description = LINK $out

rule link_tool
  command = $ld $in $deps -o $out --target=x86_64-w64-mingw32 -static -fuse-ld=lld

""")

    # Compile rules

    _packer_obj = obj_name(ASSET_PACKER_SRC)
    f.write(f"build {_packer_obj}: cc {ASSET_PACKER_SRC}\n")
    f.write(
        f"build {ASSET_PACKER_EXE}: link_tool {_packer_obj} {' '.join(ASSET_PACKER_DEPENDENCIES)}\n"
    )

    objects = []

    for src in ALL_SRC:
        obj = obj_name(src)
        objects.append(str(obj))
        f.write(f"build {obj}: cc {src}\n")

    # Link
    f.write(f"""
build {BIN_DIR / EXE_NAME}: link {" ".join(objects)}
default {BIN_DIR / EXE_NAME} {ASSET_PACKER_EXE}
""")

print("Generated build.ninja successfully.")
