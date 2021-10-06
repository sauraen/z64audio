#ifndef __HERMOSAUHU_LIB_H__
#define __HERMOSAUHU_LIB_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

typedef signed char s8;
typedef unsigned char u8;
typedef signed short int s16;
typedef unsigned short int u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long int s64;
typedef unsigned long long int u64;
typedef float f32;
typedef double f64;

typedef enum {
	PSL_DEBUG = -1,
	PSL_NONE,
	PSL_NO_INFO,
	PSL_NO_WARNING,
	PSL_NO_ERROR,
} PrintfSuppressLevel;

typedef enum {
	SWAP_U8  = 1,
	SWAP_U16 = 2,
	SWAP_U32 = 4,
	SWAP_U64 = 8,
	SWAP_F80 = 10
} SwapSize;

#ifndef __HERMOSAUHU_DATA_T__
#define __HERMOSAUHU_DATA_T__

typedef struct {
	union {
		void* data;
		union {
			u8*  u8;
			u16* u16;
			u32* u32;
			u64* u64;
		} cast;
	};
	u32 memSize;
	u32 dataSize;
} MemFile;

#endif

/* 👺 PRINTF 👺 */
void printf_SetSuppressLevel(PrintfSuppressLevel lvl);
void printf_debug(const char* fmt, ...);
void printf_warning(const char* fmt, ...);
void printf_error(const char* fmt, ...);
void printf_info(const char* fmt, ...);

/* 👺 LIB 👺 */
void* Lib_MemMem(const void* haystack, size_t haystackSize, const void* needle, size_t needleSize);
void* Lib_Malloc(s32 size);
void Lib_ByteSwap(void* src, s32 size);

/* 👺 FILE 👺 */
s32 File_LoadToMem(void** dst, char* filepath);
void File_WriteToFromMem(char* filepath, void* src, s32 size);
s32 File_LoadToMem_ReqExt(void** dst, char* filepath, const char* ext);
void File_WriteToFromMem_ReqExt(char* filepath, void* src, s32 size, const char* ext);

/* 👺 MEMFILE 👺 */
void MemFile_LoadToMemFile(MemFile* memFile, char* filepath);
void MemFile_WriteToFromMemFile(MemFile* memFile, char* filepath);
void MemFile_LoadToMemFile_ReqExt(MemFile* memFile, char* filepath, const char* ext);
void MemFile_WriteToFromMemFile_ReqExt(MemFile* memFile, char* filepath, s32 size, const char* ext);
void MemFile_Free(MemFile* memFile);

/* 👺 STRING 👺 */
u32 String_ToHex(char* string);
s32 String_GetLineCount(char* str);
char* String_GetLine(char* str, s32 line);
char* String_GetWord(char* str, s32 word);
void String_CaseToLow(char* s, s32 i);
void String_CaseToUp(char* s, s32 i);
void String_GetPath(char* dst, char* src);
void String_GetBasename(char* dst, char* src);
void String_GetFilename(char* dst, char* src);

#define ABS(val)      (val < 0 ? -val : val)
#define ByteSwap16(x) x = __bswap_16(x)
#define ByteSwap32(x) x = __bswap_32(x)
#define ByteSwap64(x) x = __bswap_64(x)

#endif /* __HERMOSAUHU_LIB_H__ */