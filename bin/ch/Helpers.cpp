//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "stdafx.h"

//TODO: x-plat definitions
#ifdef _WIN32
typedef char16 TTDHostCharType;
typedef struct _wfinddata_t TTDHostFileInfo;
typedef intptr_t TTDHostFindHandle;
typedef struct _stat TTDHostStatType;

#define TTDHostPathSeparator _u("\\")
#define TTDHostPathSeparatorChar _u('\\')
#define TTDHostFindInvalid -1

size_t TTDHostStringLength(const TTDHostCharType* str)
{
    return wcslen(str);
}

void TTDHostInitEmpty(TTDHostCharType* dst)
{
    dst[0] = _u('\0');
}

void TTDHostInitFromUriBytes(TTDHostCharType* dst, const byte* uriBytes, size_t uriBytesLength)
{
    memcpy_s(dst, MAX_PATH * sizeof(TTDHostCharType), uriBytes, uriBytesLength);
    dst[uriBytesLength / sizeof(TTDHostCharType)] = _u('\0');

    AssertMsg(wcslen(dst) == (uriBytesLength / sizeof(TTDHostCharType)), "We have an null in the uri or our math is wrong somewhere.");
}

void TTDHostAppend(TTDHostCharType* dst, const TTDHostCharType* src)
{
    size_t dpos = TTDHostStringLength(dst);
    size_t srcLength = TTDHostStringLength(src);
    size_t srcByteLength = srcLength * sizeof(TTDHostCharType);

    memcpy_s(dst + dpos, srcByteLength, src, srcByteLength);
    dst[dpos + srcLength] = _u('\0');
}

void TTDHostAppendChar16(TTDHostCharType* dst, const char16* src)
{
    size_t dpos = TTDHostStringLength(dst);
    size_t srcLength = wcslen(src);
    size_t srcByteLength = srcLength * sizeof(char16);

    memcpy_s(dst + dpos, srcByteLength, src, srcByteLength);
    dst[dpos + srcLength] = _u('\0');
}

void TTDHostAppendAscii(TTDHostCharType* dst, const char* src)
{
    size_t dpos = TTDHostStringLength(dst);
    size_t srcLength = strlen(src);
    for(size_t i = 0; i < srcLength; ++i)
    {
        dst[dpos + i] = (char16)src[i];
    }
    dst[dpos + srcLength] = _u('\0');
}

void TTDHostBuildCurrentExeDirectory(TTDHostCharType* path, size_t pathBufferLength)
{
    TTDHostCharType exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    size_t i = TTDHostStringLength(exePath) - 1;
    while(exePath[i] != TTDHostPathSeparatorChar)
    {
        --i;
    }

    memcpy_s(path, MAX_PATH, exePath, (i + 1) * sizeof(TTDHostCharType));
    path[i + 1] = _u('\0');
}

JsTTDStreamHandle TTDHostOpen(const TTDHostCharType* path, bool isWrite)
{
    JsTTDStreamHandle res = nullptr;
    _wfopen_s(&res, path, isWrite ? _u("w+b") : _u("r+b"));

    return res;
}

#define TTDHostCWD(dst) _wgetcwd(dst, MAX_PATH)
#define TTDDoPathInit(dst)
#define TTDHostTok(opath, TTDHostPathSeparator, context) wcstok_s(opath, TTDHostPathSeparator, context)
#define TTDHostStat(cpath, statVal) _wstat(cpath, statVal)

#define TTDHostMKDir(cpath) _wmkdir(cpath)
#define TTDHostCHMod(cpath, flags) _wchmod(cpath, flags)
#define TTDHostRMFile(cpath) _wremove(cpath)

#define TTDHostFindFirst(strPattern, FileInformation) _wfindfirst(strPattern, FileInformation)
#define TTDHostFindNext(hFile, FileInformation) _wfindnext(hFile, FileInformation)
#define TTDHostFindClose(hFile) _findclose(hFile)

#define TTDHostDirInfoName(FileInformation) FileInformation.name

#define TTDHostRead(buff, size, handle) fread_s(buff, size, 1, size, (FILE*)handle);
#define TTDHostWrite(buff, size, handle) fwrite(buff, 1, size, (FILE*)handle)
#else
#include <unistd.h>
#include <cstring>
#include <libgen.h>
#include <dirent.h>

#include <sys/stat.h>

typedef char TTDHostCharType;
typedef struct dirent* TTDHostFileInfo;
typedef DIR* TTDHostFindHandle;
typedef struct stat TTDHostStatType;

#define TTDHostPathSeparator "/"
#define TTDHostPathSeparatorChar '/'
#define TTDHostFindInvalid nullptr

size_t TTDHostStringLength(const TTDHostCharType* str)
{
    return strlen(str);
}

void TTDHostInitEmpty(TTDHostCharType* dst)
{
    dst[0] = '\0';
}

void TTDHostInitFromUriBytes(TTDHostCharType* dst, const byte* uriBytes, size_t uriBytesLength)
{
    memcpy_s(dst, MAX_PATH * sizeof(TTDHostCharType), uriBytes, uriBytesLength);
    dst[uriBytesLength / sizeof(TTDHostCharType)] = '\0';

    AssertMsg(TTDHostStringLength(dst) == (uriBytesLength / sizeof(TTDHostCharType)), "We have an null in the uri or our math is wrong somewhere.");
}

void TTDHostAppend(TTDHostCharType* dst, const TTDHostCharType* src)
{
    size_t dpos = TTDHostStringLength(dst);
    size_t srcLength = TTDHostStringLength(src);
    size_t srcByteLength = srcLength * sizeof(TTDHostCharType);

    memcpy_s(dst + dpos, srcByteLength, src, srcByteLength);
    dst[dpos + srcLength] = '\0';
}

void TTDHostAppendChar16(TTDHostCharType* dst, const char16* src)
{
    size_t dpos = TTDHostStringLength(dst);
    size_t srcLength = wcslen(src);
    utf8::EncodeIntoAndNullTerminate((utf8char_t*)(dst + dpos), src, srcLength);
}

void TTDHostAppendAscii(TTDHostCharType* dst, const char* src)
{
    size_t dpos = TTDHostStringLength(dst);
    size_t srcLength = strlen(src);
    size_t srcByteLength = srcLength * sizeof(TTDHostCharType);

    memcpy_s(dst + dpos, srcByteLength, src, srcByteLength);
    dst[dpos + srcLength] = '\0';
}

void TTDHostBuildCurrentExeDirectory(TTDHostCharType* path, size_t pathBufferLength)
{
    TTDHostCharType exePath[MAX_PATH];
    readlink("/proc/self/exe", exePath, MAX_PATH);

    size_t i = TTDHostStringLength(exePath) - 1;
    while(exePath[i] != TTDHostPathSeparatorChar)
    {
        --i;
    }

    memcpy_s(path, MAX_PATH, exePath, (i + 1) * sizeof(TTDHostCharType));
    path[i + 1] = '\0';
}

JsTTDStreamHandle TTDHostOpen(const TTDHostCharType* path, bool isWrite)
{
    return fopen(path, isWrite ? "w+b" : "r+b");
}

#define TTDHostCWD(dst) getcwd(dst, MAX_PATH)
#define TTDDoPathInit(dst) TTDHostAppend(dst, TTDHostPathSeparator)
#define TTDHostTok(opath, TTDHostPathSeparator, context) strtok(opath, TTDHostPathSeparator)
#define TTDHostStat(cpath, statVal) stat(cpath, statVal)

#define TTDHostMKDir(cpath) mkdir(cpath, 0777)
#define TTDHostCHMod(cpath, flags) chmod(cpath, flags)
#define TTDHostRMFile(cpath) remove(cpath)

#define TTDHostFindFirst(strPattern, FileInformation) opendir(strPattern)
#define TTDHostFindNext(hFile, FileInformation) (*FileInformation = readdir(hFile))
#define TTDHostFindClose(hFile) closedir(hFile)

#define TTDHostDirInfoName(FileInformation) FileInformation->d_name

#define TTDHostRead(buff, size, handle) fread(buff, 1, size, (FILE*)handle)
#define TTDHostWrite(buff, size, handle) fwrite(buff, 1, size, (FILE*)handle)
#endif

HRESULT Helpers::LoadScriptFromFile(LPCSTR filename, LPCSTR& contents, UINT* lengthBytesOut /*= nullptr*/)
{
    HRESULT hr = S_OK;
    BYTE * pRawBytes = nullptr;
    UINT lengthBytes = 0;
    contents = nullptr;
    FILE * file = NULL;

    //
    // Open the file as a binary file to prevent CRT from handling encoding, line-break conversions,
    // etc.
    //
    if (fopen_s(&file, filename, "rb") != 0)
    {
#ifdef _WIN32
        DWORD lastError = GetLastError();
        char16 wszBuff[512];
        fprintf(stderr, "Error in opening file '%s' ", filename);
        wszBuff[0] = 0;
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            lastError,
            0,
            wszBuff,
            _countof(wszBuff),
            nullptr))
        {
            fwprintf(stderr, _u(": %s"), wszBuff);
        }
        fwprintf(stderr, _u("\n"));
#elif defined(_POSIX_VERSION)
        fprintf(stderr, "Error in opening file: ");
        perror(filename);
#endif
        IfFailGo(E_FAIL);
    }

    if (file != NULL)
    {
        // Determine the file length, in bytes.
        fseek(file, 0, SEEK_END);
        lengthBytes = ftell(file);
        fseek(file, 0, SEEK_SET);
        const size_t bufferLength = lengthBytes + sizeof(BYTE);
        pRawBytes = (LPBYTE)malloc(bufferLength);
        if (nullptr == pRawBytes)
        {
            fwprintf(stderr, _u("out of memory"));
            IfFailGo(E_OUTOFMEMORY);
        }

        //
        // Read the entire content as a binary block.
        //
        size_t readBytes = fread(pRawBytes, sizeof(BYTE), lengthBytes, file);
        if (readBytes < lengthBytes * sizeof(BYTE))
        {
            IfFailGo(E_FAIL);
        }

        pRawBytes[lengthBytes] = 0; // Null terminate it. Could be UTF16

        //
        // Read encoding to make sure it's supported
        //
        // Warning: The UNICODE buffer for parsing is supposed to be provided by the host.
        // This is not a complete read of the encoding. Some encodings like UTF7, UTF1, EBCDIC, SCSU, BOCU could be
        // wrongly classified as ANSI
        //
        {
            C_ASSERT(sizeof(WCHAR) == 2);
            if (bufferLength > 2)
            {
                if ((pRawBytes[0] == 0xFE && pRawBytes[1] == 0xFF) ||
                    (pRawBytes[0] == 0xFF && pRawBytes[1] == 0xFE) ||
                    (bufferLength > 4 && pRawBytes[0] == 0x00 && pRawBytes[1] == 0x00 &&
                        ((pRawBytes[2] == 0xFE && pRawBytes[3] == 0xFF) ||
                         (pRawBytes[2] == 0xFF && pRawBytes[3] == 0xFE))))

                {
                    // unicode unsupported
                    fwprintf(stderr, _u("unsupported file encoding. Only ANSI and UTF8 supported"));
                    IfFailGo(E_UNEXPECTED);
                }
            }
        }
    }

    contents = reinterpret_cast<LPCSTR>(pRawBytes);

Error:
    if (SUCCEEDED(hr))
    {
        if (lengthBytesOut)
        {
            *lengthBytesOut = lengthBytes;
        }
    }

    if (file != NULL)
    {
        fclose(file);
    }

    if (pRawBytes && reinterpret_cast<LPCSTR>(pRawBytes) != contents)
    {
        free(pRawBytes);
    }

    return hr;
}

LPCWSTR Helpers::JsErrorCodeToString(JsErrorCode jsErrorCode)
{
    bool hasException = false;
    ChakraRTInterface::JsHasException(&hasException);
    if (hasException)
    {
        WScriptJsrt::PrintException("", JsErrorScriptException);
    }

    switch (jsErrorCode)
    {
    case JsNoError:
        return _u("JsNoError");
        break;

    case JsErrorInvalidArgument:
        return _u("JsErrorInvalidArgument");
        break;

    case JsErrorNullArgument:
        return _u("JsErrorNullArgument");
        break;

    case JsErrorNoCurrentContext:
        return _u("JsErrorNoCurrentContext");
        break;

    case JsErrorInExceptionState:
        return _u("JsErrorInExceptionState");
        break;

    case JsErrorNotImplemented:
        return _u("JsErrorNotImplemented");
        break;

    case JsErrorWrongThread:
        return _u("JsErrorWrongThread");
        break;

    case JsErrorRuntimeInUse:
        return _u("JsErrorRuntimeInUse");
        break;

    case JsErrorBadSerializedScript:
        return _u("JsErrorBadSerializedScript");
        break;

    case JsErrorInDisabledState:
        return _u("JsErrorInDisabledState");
        break;

    case JsErrorCannotDisableExecution:
        return _u("JsErrorCannotDisableExecution");
        break;

    case JsErrorHeapEnumInProgress:
        return _u("JsErrorHeapEnumInProgress");
        break;

    case JsErrorOutOfMemory:
        return _u("JsErrorOutOfMemory");
        break;

    case JsErrorScriptException:
        return _u("JsErrorScriptException");
        break;

    case JsErrorScriptCompile:
        return _u("JsErrorScriptCompile");
        break;

    case JsErrorScriptTerminated:
        return _u("JsErrorScriptTerminated");
        break;

    case JsErrorFatal:
        return _u("JsErrorFatal");
        break;

    default:
        return _u("<unknown>");
        break;
    }
}

void Helpers::LogError(__in __nullterminated const char16 *msg, ...)
{
    va_list args;
    va_start(args, msg);
    wprintf(_u("ERROR: "));
    vfwprintf(stderr, msg, args);
    wprintf(_u("\n"));
    fflush(stdout);
    va_end(args);
}

void Helpers::TTReportLastIOErrorAsNeeded(BOOL ok, const char* msg)
{
    if(!ok)
    {
#ifdef _WIN32
        DWORD lastError = GetLastError();
        LPTSTR pTemp = NULL;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY, NULL, lastError, 0, (LPTSTR)&pTemp, 0, NULL);
        fwprintf(stderr, _u("Error is: %s\n"), pTemp);
#else
        fprintf(stderr, "Error is: %i %s\n", errno, strerror(errno));
#endif
        fprintf(stderr, "Message is: %s\n", msg);

        AssertMsg(false, "IO Error!!!");
    }
}

void Helpers::CreateDirectoryIfNeeded(size_t uriByteLength, const byte* uriBytes)
{
    TTDHostCharType opath[MAX_PATH];
    TTDHostInitFromUriBytes(opath, uriBytes, uriByteLength);

    TTDHostCharType cpath[MAX_PATH];
    TTDHostInitEmpty(cpath);
    TTDDoPathInit(cpath);

    TTDHostStatType statVal;
    TTDHostCharType* context = nullptr;
    TTDHostCharType* token = TTDHostTok(opath, TTDHostPathSeparator, &context);
    TTDHostAppend(cpath, token);

    //At least 1 part of the path must exist so iterate until we find it
    while(TTDHostStat(cpath, &statVal) == -1)
    {
        token = TTDHostTok(nullptr, TTDHostPathSeparator, &context);
        TTDHostAppend(cpath, TTDHostPathSeparator);
        TTDHostAppend(cpath, token);
    }

    //Now continue until we hit the part that doesn't exist (or the end of the path)
    while(token != nullptr && TTDHostStat(cpath, &statVal) != -1)
    {
        token = TTDHostTok(nullptr, TTDHostPathSeparator, &context);
        if(token != nullptr)
        {
            TTDHostAppend(cpath, TTDHostPathSeparator);
            TTDHostAppend(cpath, token);
        }
    }

    //Now if there is path left then continue build up the directory tree as we go
    while(token != nullptr)
    {
        TTDHostMKDir(cpath);

        token = TTDHostTok(nullptr, TTDHostPathSeparator, &context);
        if(token != nullptr)
        {
            TTDHostAppend(cpath, TTDHostPathSeparator);
            TTDHostAppend(cpath, token);
        }
    }
}

void Helpers::CleanDirectory(size_t uriByteLength, const byte* uriBytes)
{
    TTDHostFindHandle hFile;
    TTDHostFileInfo FileInformation;

    TTDHostCharType strPattern[MAX_PATH];
    TTDHostInitFromUriBytes(strPattern, uriBytes, uriByteLength);
    TTDHostAppendAscii(strPattern, "*.*");

    hFile = TTDHostFindFirst(strPattern, &FileInformation);
    if(hFile != TTDHostFindInvalid)
    {
        do
        {
            if(TTDHostDirInfoName(FileInformation)[0] != '.')
            {
                TTDHostCharType strFilePath[MAX_PATH];
                TTDHostInitFromUriBytes(strFilePath, uriBytes, uriByteLength);
                TTDHostAppend(strFilePath, TTDHostDirInfoName(FileInformation));

                // Set file attributes
                TTDHostCHMod(strFilePath, S_IREAD | S_IWRITE);
                TTDHostRMFile(strFilePath);
            }
        } while(TTDHostFindNext(hFile, &FileInformation) != TTDHostFindInvalid);

        // Close handle
        TTDHostFindClose(hFile);
    }
}

void Helpers::GetTTDDirectory(const char16* curi, size_t* uriByteLength, byte** uriBytes)
{
    TTDHostCharType turi[MAX_PATH];
    TTDHostInitEmpty(turi);

    if(curi[0] != _u('~'))
    {
        TTDHostCWD(turi);
        TTDHostAppend(turi, TTDHostPathSeparator);

        TTDHostAppendChar16(turi, curi);
    }
    else
    {
        TTDHostBuildCurrentExeDirectory(turi, MAX_PATH);

        TTDHostAppendAscii(turi, "_ttdlog");
        TTDHostAppend(turi, TTDHostPathSeparator);

        TTDHostAppendChar16(turi, curi + 1);
    }

    //add a path separator if one is not already present
    if(curi[wcslen(curi) - 1] != (wchar)TTDHostPathSeparator[0])
    {
        TTDHostAppend(turi, TTDHostPathSeparator);
    }

    size_t turiLength = TTDHostStringLength(turi);

    size_t byteLengthWNull = (turiLength + 1) * sizeof(TTDHostCharType);
    *uriBytes = (byte*)CoTaskMemAlloc(byteLengthWNull);
    memcpy_s(*uriBytes, byteLengthWNull, turi, byteLengthWNull);

    *uriByteLength = turiLength * sizeof(TTDHostCharType);
}

void CALLBACK Helpers::TTInitializeForWriteLogStreamCallback(size_t uriByteLength, const byte* uriBytes)
{
    //If the directory does not exist then we want to create it
    Helpers::CreateDirectoryIfNeeded(uriByteLength, uriBytes);

    //Clear the logging directory so it is ready for us to write into
    Helpers::CleanDirectory(uriByteLength, uriBytes);
}

JsTTDStreamHandle Helpers::TTCreateStreamCallback(size_t uriByteLength, const byte* uriBytes, const char* asciiResourceName, bool read, bool write)
{
    AssertMsg((read | write) & (!read | !write), "Read/Write streams not supported yet -- defaulting to read only");

    FILE* res = nullptr;
    TTDHostCharType path[MAX_PATH];
    TTDHostInitFromUriBytes(path, uriBytes, uriByteLength);
    TTDHostAppendAscii(path, asciiResourceName);

    res = TTDHostOpen(path, write);
    if(res == nullptr)
    {
#if _WIN32
        fwprintf(stderr, _u("Filename: %ls\n"), (char16*)path);
#else
        fprintf(stderr, "Filename: %s\n", (char*)path);
#endif
    }

    Helpers::TTReportLastIOErrorAsNeeded(res != nullptr, "Failed File Open");
    return res;
}

bool CALLBACK Helpers::TTReadBytesFromStreamCallback(JsTTDStreamHandle handle, byte* buff, size_t size, size_t* readCount)
{
    AssertMsg(handle != nullptr, "Bad file handle.");

    if(size > MAXDWORD)
    {
        *readCount = 0;
        return false;
    }

    BOOL ok = FALSE;

    *readCount = TTDHostRead(buff, size, handle);
    ok = (*readCount != 0);

    Helpers::TTReportLastIOErrorAsNeeded(ok, "Failed Read!!!");

    return ok ? true : false;
}

bool CALLBACK Helpers::TTWriteBytesToStreamCallback(JsTTDStreamHandle handle, byte* buff, size_t size, size_t* writtenCount)
{
    AssertMsg(handle != nullptr, "Bad file handle.");

    if(size > MAXDWORD)
    {
        *writtenCount = 0;
        return false;
    }

    BOOL ok = FALSE;

    *writtenCount = TTDHostWrite(buff, size, (FILE*)handle);
    ok = (*writtenCount == size);

    Helpers::TTReportLastIOErrorAsNeeded(ok, "Failed Read!!!");

    return ok ? true : false;
}

void CALLBACK Helpers::TTFlushAndCloseStreamCallback(JsTTDStreamHandle handle, bool read, bool write)
{
    fflush((FILE*)handle);
    fclose((FILE*)handle);
}

