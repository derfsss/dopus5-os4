# Directory Opus 5.93 — AmigaOS 4 Fork

A working AmigaOS 4 build of Directory Opus 5, based on the open-source release from the [DOpus5 All Amigas](https://sourceforge.net/projects/dopus5allamigas/) SourceForge project.

**Current version:** 5.93 [OS4] (Program 5.93, Library 73.1, Commands 65.0)

## Origin

The source code in this repository was copied from the SourceForge SVN repository at:

    https://sourceforge.net/projects/dopus5allamigas/

The original code was released under the **AROS Public License (APL) version 1.1** by Jonathan Potter & GP Software, with subsequent porting work by the DOPUS5 Open Source Team (Ilkka Lehtoranta, Roman Kargin, Szilard Biro, Xenic, and others). See the `AUTHORS` file for full credits.

The SourceForge codebase already contained `#ifdef __amigaos4__` support for AmigaOS 4, contributed by the open-source team. However, it did not compile or run cleanly on modern AmigaOS 4.1 systems. This fork contains fixes to get it building and running correctly.

The bug fixes and modernisation work in this fork were developed with the assistance of [Claude](https://claude.ai) (Anthropic) via [Claude Code](https://claude.com/claude-code).

## License

AROS Public License (APL) version 1.1 — see `COPYING`.

Directory Opus is a registered trademark of GP Software. Use of the trademark is permitted only for software derived from this source code running on Amiga-family platforms (AmigaOS, MorphOS, AROS). See the original `README` file for the full trademark notice.

## Building for AmigaOS 4

### Prerequisites

- Docker with the `walkero/amigagccondocker:os4-gcc11` image (PPC cross-compiler toolchain)
- WSL2 or native Linux for running Docker

### Build Commands

From WSL2:

```sh
# Mount the project and build (debug build, serial output via DebugPrintF)
docker run --rm \
  -v /path/to/dopus5allamigas-code:/project \
  -w /project/source \
  walkero/amigagccondocker:os4-gcc11 \
  make -j$(nproc) -f makefile os4 all

# Non-debug (release) build
docker run --rm \
  -v /path/to/dopus5allamigas-code:/project \
  -w /project/source \
  walkero/amigagccondocker:os4-gcc11 \
  make -j$(nproc) -f makefile os4 debug=no all
```

### Creating a Release Archive

The build system uses `basedata.lha` (in `archive/`) as a base containing icon `.info` files and default configuration. A clean release requires separate clean and build steps to avoid parallel build races:

```sh
# Step 1: Clean
docker run --rm -v ...:/project -w /project/source \
  walkero/amigagccondocker:os4-gcc11 make -f makefile os4 cleanall

# Step 2: Build
docker run --rm -v ...:/project -w /project/source \
  walkero/amigagccondocker:os4-gcc11 make -j$(nproc) -f makefile os4 debug=no all

# Step 3: Create archive (manual, to avoid race condition in 'release' target)
docker run --rm -v ...:/project -w /project/source \
  walkero/amigagccondocker:os4-gcc11 sh -c '
    mkdir -p Dopus5/Libs Dopus5/Modules Dopus5/C Dopus5/Help Dopus5/Documents Dopus5/WBStartup
    cp ../archive/DoWBStartup Dopus5/WBStartup/
    cp ../archive/DoWBStartup.info Dopus5/WBStartup/
    cp bin.os4/dopus5.library Dopus5/Libs/
    cp bin.os4/*.module Dopus5/Modules/
    cp bin.os4/DirectoryOpus Dopus5/
    cp bin.os4/DOpusRT5 Dopus5/C/
    cp bin.os4/LoadDB Dopus5/C/
    cp bin.os4/viewfont Dopus5/C/
    cp ../documents/*.guide Dopus5/Help/
    cp ../documents/*.pdf Dopus5/Documents/
    cp ../archive/basedata.lha ../releases/Dopus5_93_os4.lha
    lha ao5q ../releases/Dopus5_93_os4.lha Dopus5
    rm -rf Dopus5
  '
```

### Build Output

All 25 binaries are produced in `source/bin.os4/`:

| Binary | Description |
|--------|-------------|
| `DirectoryOpus` | Main program |
| `dopus5.library` | Shared library |
| `DOpusRT5` | Shell utility |
| `LoadDB` | Database loader |
| `viewfont` | Font viewer |
| `*.module` (20) | Function modules (about, cleanup, configopus, diskcopy, diskinfo, filetype, fixicons, format, ftp, icon, join, listerformat, misc, pathformat, play, print, read, show, themes, xadopus) |

## Changes Made for AmigaOS 4

The following changes were made on top of the SourceForge codebase to fix compilation, crashes, and runtime issues on AmigaOS 4.1.

### Bug Fixes

#### Load Environment Deadlock
- **File:** `Program/display.c`
- The `IPC_ListCommand` calls in `close_display()` that broadcast `IPC_HIDE` to all process lists must be **asynchronous** (last parameter `FALSE`). Making them synchronous caused a deadlock when `environment_proc` (which is on the process list) triggered `close_display` via `MAINCMD_CLOSE_DISPLAY` — the environment process waited for the main process to reply, while the main process waited for the environment process to reply to `IPC_HIDE`.

#### Environment Process Parent/Child Exit Error
- **File:** `Program/environment.c`
- Changed `MAINCMD_OPEN_DISPLAY` from asynchronous (fire-and-forget) to **synchronous** (`(struct MsgPort *)-1`). On AmigaOS 4, `IPC_Launch` uses `NP_Child=TRUE`, so the environment process is tracked as a parent. If it exits before the listers it triggers are fully launched, AmigaOS reports "Parent process has tried to exit before all children have".

#### Screen Close Abort Button
- **File:** `Program/display.c`
- Added an "Abort" button to the "Unable to close screen" requester. If another application holds the screen open, the user can now abort instead of being stuck in an infinite retry loop.

#### NULL Backdrop Guard
- **File:** `Program/display.c`
- Added NULL checks for `GUI->backdrop` before accessing `backdrop->window_lock` in `close_display()`. Prevents crash if display closes before backdrop is initialised.

#### DOPUS5: Assign Leak on Exit
- **File:** `Program/cleanup.c`
- Added `AssignLock("DOPUS5", 0)` to the quit path. The `DOPUS5:` assign was created at startup but never freed, leaving a lock on the program directory after exit.

#### D5THEMES: Assign Leak on Exit
- **File:** `Program/cleanup.c`
- Added `AssignLock("D5THEMES", 0)` to the quit path (same issue as above).

#### Memory Allocation MEMF_PRIVATE Fallback
- **File:** `Library/memory.c`
- When the memory pool handle has no specific memory type set (type bits = 0), the code now defaults to `MEMF_PRIVATE` instead of passing 0 to `AllocVecTags`. On AmigaOS 4, a zero memory type is invalid and returns NULL.

#### ModuleInfo Flexible Array Fix
- **Files:** 6 module `libinit.c` files
- Fixed `ModuleInfo` structure declarations that used `function[1]` (single-element array) to properly size the flexible array member for each module's function count. Prevents out-of-bounds access.

#### Anti-Aliased Font Rendering Fix (SourceForge #6)
- **Files:** `Library/image_class.c`, `Library/listview_class.c`, `Modules/configopus/config_environment_output.c`
- Text and checkmark rendering used JAM1 draw mode without clearing the background first. With legacy bitmap fonts this was fine, but with anti-aliased fonts (standard on AmigaOS 4), each redraw blends with existing pixels instead of replacing them, causing text and checkmarks to become progressively bolder with each toggle or refresh.
- Fixed checkmark rendering (`IM_CHECK`/`IM_CROSS`) by clearing the background rectangle before drawing lines.
- Fixed listview title rendering by switching from JAM1 to JAM2 with proper background pen.
- Fixed config preview instruction text by switching from JAM1 to JAM2 with proper background pen.

#### Defensive Lock Validation for USB/FAT32 (SourceForge #63 mitigation)
- **File:** `Program/function_runprog.c`
- On AmigaOS 4, double-clicking files on FAT32 USB drives could cause a DSI crash in `dos.library` during `UnLock()`. The underlying issue is in the OS4 FAT filesystem handler which can invalidate locks unexpectedly. Added a defensive check on OS4: before calling `UnLock()`, `NameFromLock()` is used to verify the lock is still valid. If the lock has been invalidated (`ERROR_INVALID_LOCK`), the `UnLock()` call is skipped, preventing the crash.

#### 255-Character Filename Support
- **Files:** `Include/dopus/common.h`, `Program/environment.c`, `Program/function_readdir.c`, `Modules/configopus/config_environment.c`
- On AmigaOS 4, the modern DOS API (`ExamineData->Name`) supports filenames up to 255 characters, but DOpus5 clamped filenames to 107 characters — matching the legacy `FileInfoBlock.fib_FileName[108]` field. The internal data structures already supported longer names (`DirEntry.de_NameLen` is `unsigned char`, entry allocation is dynamic), so only the artificial clamps needed raising.
- Added `MAX_FILENAME_LEN` constant (255 on OS4, 107 on other platforms) in `common.h`.
- Raised the filename length clamp in `environment.c` and the config UI bounds in `config_environment.c` from 107 to `MAX_FILENAME_LEN`.
- In the OS4 directory reading path (`function_readdir.c`), bypassed the `FileInfoBlock` name truncation by calling `create_file_entry()` directly with `exdata->Name` instead of going through `create_file_entry_fib()` which reads the truncated 107-char FIB field.
- Replaced 7 hardcoded `108` buffer size literals across `function_filechange.c`, `backdrop_leftout.c`, and `ftp_lister.c` with `sizeof(fib->fib_FileName)` for self-documentation.

### Modernisation (BltBitMapTags)

Replaced 6 legacy `BltBitMapRastPort` calls with `BltBitMapTags` under `#ifdef __amigaos4__` guards. All 6 used minterm `0xc0` (simple copy). The legacy calls are preserved in `#else` branches for other platforms.

| File | Context |
|------|---------|
| `Library/drag_routines.c` (x3) | Drag sprite background save/restore and image drawing |
| `Library/images.c` | Bitmap image rendering |
| `Program/clock_task.c` | Moon phase decoration in title bar |
| `Modules/show/show.c` | DataTypes fullscreen image display |

### Pre-existing AmigaOS 4 Support

The SourceForge codebase already contained extensive `#ifdef __amigaos4__` support contributed by the open-source team, including:

- **Library interface management** — `GetInterface`/`DropInterface` for all OS4 library access across 62+ files
- **Memory management** — `AllocVecTags`/`FreeVec` replacing classic `AllocMem`/`FreeMem`
- **Icon handling** — `PutIconTags`/`GetIconTags` replacing `PutDiskObject`/`GetDiskObject` across 27 files
- **File I/O** — `ChangeFilePosition` replacing `Seek` across 7 files
- **CONST_STRPTR handling** — `MutableFilePart`/`MutablePathPart` wrapper macros
- **IPC process management** — `NP_Child=TRUE` for child process tracking
- **Global libbase access** — `dopuslibbase_global` pattern for library functions
- **Build system** — Complete makefile support for `os4` target using `ppc-amigaos-gcc`

## Project Structure

```
dopus5allamigas-code/
  AUTHORS         Original author credits
  COPYING         AROS Public License v1.1
  ChangeLog       SVN commit history from SourceForge
  README          Original release notes from Jonathan Potter
  archive/        Base archive (icons, WBStartup) and support files
  catalogs/       Locale translation catalogs
  documents/      AmigaGuide help and PDF documentation
  source/         All source code
    Include/      Shared headers (dopus/, inline4/)
    Library/      dopus5.library source
    Misc/         Shell utilities (DOpusRT5, LoadDB, viewfont)
    Modules/      Function module source (20 modules)
    Program/      Main DirectoryOpus program source
    makefile      Top-level build system
```
