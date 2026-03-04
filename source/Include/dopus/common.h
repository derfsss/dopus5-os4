/* AmigaOS includes & definitions */

#ifndef DOPUS_COMMON_H
#define DOPUS_COMMON_H

// Temporary defines
#undef __chip
#define __chip
// End Temporary defines

#define __ARGS(x) x
#define DOPUS_WIN_NAME "dOpUs5.win"

#ifdef __amigaos4__
#include <amiga_compiler.h>
#endif
#include <SDI/SDI_compiler.h>
#include <SDI/SDI_lib.h>
#include <SDI/SDI_stdarg.h>

#ifdef __amigaos4__
#undef DEPRECATED
#define DEPRECATED
/* OS4 FilePart/PathPart return CONST_STRPTR but callers modify the buffer */
#define MutableFilePart(p) ((char *)FilePart(p))
#define MutablePathPart(p) ((char *)PathPart(p))
#else
#define MutableFilePart(p) FilePart(p)
#define MutablePathPart(p) PathPart(p)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/locale.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/wb.h>
#include <proto/icon.h>
#include <proto/iffparse.h>
#include <proto/locale.h>
#include <proto/commodities.h>
#include <proto/asl.h>
#include <proto/gadtools.h>
#include <proto/console.h>
#include <proto/utility.h>
#include <proto/timer.h>
#include <proto/diskfont.h>
#include <proto/input.h>
#include <proto/datatypes.h>
#include <proto/layers.h>
#include <proto/rexxsyslib.h>
#if defined(__amigaos4__) || defined(__MORPHOS__) || defined(__AROS__)
#include <cybergraphx/cybergraphics.h>
#else
#include <cybergraphics/cybergraphics.h>
#endif
#include <proto/cybergraphics.h>	

#ifdef __amigaos4__
#include <dos/obsolete.h>
#endif
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <dos/dostags.h>
#include <dos/filehandler.h>
#include <devices/trackdisk.h>
#include <graphics/gfxmacros.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <intuition/cghooks.h>
#include <intuition/sghooks.h>
#include <workbench/startup.h>
#include <workbench/icon.h>
#include <prefs/wbpattern.h>
#include <prefs/prefhdr.h>
#include <prefs/font.h>
#include <datatypes/animationclass.h>
#include <datatypes/textclass.h>

#ifdef __amigaos3__
#include <clib/alib_protos.h>
#endif

#ifdef __MORPHOS__
#include <intuition/intuitionbase.h>
#include <clib/alib_protos.h>
#endif

#ifdef __AROS__
#include <proto/alib.h>
#include <graphics/gfxbase.h>
#endif

#include <dopus/debug.h>
#include <dopus/version.h>
#include <dopus/stack.h>

#ifdef __amigaos4__
#include <exec/emulation.h>     // necessary 68k emul-based parts
#include <dos/stdio.h>          // for #define Flush(x) FFlush(x)
#include <graphics/composite.h> // CompositeTags, COMPTAG_*, COMPFLAG_*
#include <graphics/blitattr.h>  // BltBitMapTags, BLITA_*, BLITT_*
#define REG68K_d0 REG68K_D0     // make macros work for all builds
#define REG68K_d1 REG68K_D1
#define REG68K_a0 REG68K_A0
#define REG68K_a1 REG68K_A1
#define REG68K_a2 REG68K_A2
#define REG68K_a3 REG68K_A3
#define REG68K_a4 REG68K_A4
#endif


/* Long word alignement (mainly used to get
 * FIB or DISK_INFO as auto variables)
 */
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
					   type *name = (type *)((LONG)(a_##name+3) & ~3);


// no need for old functions
#undef UDivMod32
#define UDivMod32(x,y)( ((ULONG) x) / ((ULONG) y) ) 
#undef SDivMod32
#define SDivMod32(x,y) ( ((LONG) x) / ((LONG) y) )  
#undef UMult32
#define UMult32(x,y) ( ((ULONG) x) * ((ULONG) y) )


/* Replacement functions for functions not available in some SDKs/GCCs */
#if !defined(__MORPHOS__) && !defined(__AROS__)
#undef stccpy
int stccpy(char *p, const char *q, int n);
#endif


#if defined(__amigaos3__)
#define lsprintf(buf,fmt,...) \
	({ \
		static ULONG StuffChar = 0x16c04e75; \
		IPTR vargs[] = { __VA_ARGS__ }; \
		RawDoFmt((STRPTR)fmt, (APTR)&vargs, (void (*))&StuffChar, (APTR)buf); \
	})
#define LSprintf(buffer, string, data) \
	({ \
		static ULONG StuffChar = 0x16c04e75; \
		RawDoFmt(string, data, (void (*))&StuffChar, buffer); \
	})
#else
#define lsprintf(buf,fmt,...) \
	({ \
		IPTR vargs[] = { __VA_ARGS__ }; \
		RawDoFmt((STRPTR)fmt, (APTR)&vargs, NULL, (APTR)buf); \
	})
#define LSprintf(buffer, string, data) \
	RawDoFmt(string, data, NULL, buffer)
#endif


/*
 * AmigaOS 4 SDK modernization helpers.
 *
 * ExamineDir/ExamineData replaces the deprecated Examine/ExNext loop.
 * The helper below copies ExamineData fields into a legacy FileInfoBlock
 * so that existing loop bodies continue to work unchanged.
 *
 * Usage pattern:
 *   APTR context = ObtainDirContextTags(EX_LockInput, lock, TAG_END);
 *   if (context) {
 *       struct ExamineData *exdata;
 *       while ((exdata = ExamineDir(context))) {
 *           ExamineData_to_FIB(exdata, fib);
 *           // ... existing loop body using fib-> fields ...
 *       }
 *       ReleaseDirContext(context);
 *   }
 */
#ifdef __amigaos4__
#include <dos/exall.h>

static inline void ExamineData_to_FIB(
	const struct ExamineData *exdata,
	struct FileInfoBlock *fib)
{
	/* Name */
	strncpy(fib->fib_FileName, exdata->Name ? exdata->Name : "", sizeof(fib->fib_FileName)-1);
	fib->fib_FileName[sizeof(fib->fib_FileName)-1] = '\0';

	/* Comment */
	strncpy(fib->fib_Comment, exdata->Comment ? exdata->Comment : "", sizeof(fib->fib_Comment)-1);
	fib->fib_Comment[sizeof(fib->fib_Comment)-1] = '\0';

	/* Type — map EXD types to legacy DirEntryType values */
	if (EXD_IS_DIRECTORY(exdata))
		fib->fib_DirEntryType = ST_USERDIR;
	else if (EXD_IS_SOFTLINK(exdata))
		fib->fib_DirEntryType = ST_SOFTLINK;
	else if (EXD_IS_LINK(exdata))
		fib->fib_DirEntryType = ST_LINKFILE;
	else
		fib->fib_DirEntryType = ST_FILE;

	/* Size — clamp 64-bit to 32-bit for legacy field */
	if (exdata->FileSize > 0x7FFFFFFF)
		fib->fib_Size = 0x7FFFFFFF;
	else
		fib->fib_Size = (LONG)exdata->FileSize;

	/* Protection, owner, date */
	fib->fib_Protection = exdata->Protection;
	fib->fib_OwnerUID = (UWORD)exdata->OwnerUID;
	fib->fib_OwnerGID = (UWORD)exdata->OwnerGID;
	fib->fib_Date = exdata->Date;
}

/*
 * Examine a directory lock's own metadata via ExamineObjectTags.
 * Fills the FIB with the directory's name, date, type etc.
 * Returns TRUE on success.
 */
static inline BOOL ExamineObject_to_FIB(
	BPTR lock,
	struct FileInfoBlock *fib)
{
	struct ExamineData *exdata;

	if ((exdata = ExamineObjectTags(EX_LockInput, lock, TAG_END)))
	{
		ExamineData_to_FIB(exdata, fib);
		FreeDosObject(DOS_EXAMINEDATA, exdata);
		return TRUE;
	}
	return FALSE;
}

static inline BOOL ExamineObject_to_FIB_FH(
	BPTR fh,
	struct FileInfoBlock *fib)
{
	struct ExamineData *exdata;

	if ((exdata = ExamineObjectTags(EX_FileHandleInput, fh, TAG_END)))
	{
		ExamineData_to_FIB(exdata, fib);
		FreeDosObject(DOS_EXAMINEDATA, exdata);
		return TRUE;
	}
	return FALSE;
}
#endif /* __amigaos4__ */


#endif /* DOPUS_COMMON_H */

