//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "stdafx.h"

//TODO: x-plat definitions
#ifdef _WIN32

//TODO: #include <string> gets angry about this being redefined in yvals.h?
//      As a workaround we undef it for this file where we include string.
//      Also strange that string is already included in Linux build but not in Windows?????
#ifdef _STRINGIZE
#undef _STRINGIZE
#endif
#include <string>

#define TTDPathSeparator _u("\\")
#else
#define TTDPathSeparator "/"
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
        fwprintf(stderr, _u(": %s"), pTemp);
#endif
        fprintf(stderr, "msg is: %s", msg);

        AssertMsg(false, "IO Error!!!");
    }
}

void Helpers::CreateDirectoryIfNeeded(size_t uriByteLength, const byte* uriBytes)
{
#ifdef _WIN32
    char16 opath[MAX_PATH];
    memcpy_s(opath, MAX_PATH * sizeof(char16), uriBytes, uriByteLength);
    char16* context = nullptr;

    struct _stat statVal;
    char16* token = wcstok_s(opath, TTDPathSeparator, &context);
    std::wstring cpath(token);

    //At least 1 part of the path must exist so iterate until we find it
    while(_wstat(cpath.c_str(), &statVal) == -1)
    {
        token = wcstok_s(nullptr, TTDPathSeparator, &context);
        cpath.append(TTDPathSeparator);
        cpath.append(token);
    }

    //Now continue until we hit the part that doesn't exist (or the end of the path)
    while(token != nullptr && _wstat(cpath.c_str(), &statVal) != -1)
    {
        token = wcstok_s(nullptr, TTDPathSeparator, &context);
        if(token != nullptr)
        {
            cpath.append(TTDPathSeparator);
            cpath.append(token);
        }
    }

    //Now if there is path left then continue build up the directory tree as we go
    while(token != nullptr)
    {
        _wmkdir(cpath.c_str());

        token = wcstok_s(nullptr, TTDPathSeparator, &context);
        if(token != nullptr)
        {
            cpath.append(TTDPathSeparator);
            cpath.append(token);
        }
    }
#else
    AssertMsg(false, "Not x-plat yet!!!");
#endif
}

void Helpers::DeleteDirectory(size_t uriByteLength, const byte* uriBytes)
{
#ifdef _WIN32
    intptr_t hFile;
    struct _wfinddata_t FileInformation;

    std::wstring strPattern((const char16*)uriBytes, uriByteLength / sizeof(char16));
    strPattern.append(_u("*.*"));

    hFile = _wfindfirst(strPattern.c_str(), &FileInformation);
    if(hFile != -1)
    {
        do
        {
            if(FileInformation.name[0] != '.')
            {
                std::wstring strFilePath((const char16*)uriBytes, uriByteLength / sizeof(char16));

                if(FileInformation.attrib & FILE_ATTRIBUTE_DIRECTORY)
                {
                    DeleteDirectory(strFilePath.length() * sizeof(char16), (const byte*)strFilePath.c_str());
                    _wrmdir(strFilePath.c_str());
                }
                else
                {
                    // Set file attributes
                    _wchmod(strFilePath.c_str(), S_IREAD | _S_IWRITE);
                    _wremove(strFilePath.c_str());
                }
            }
        } while(_wfindnext(hFile, &FileInformation) == TRUE);

        // Close handle
        _findclose(hFile);
    }
#else
    AssertMsg(false, "Not x-plat yet!!!");
#endif
}

void Helpers::GetTTDDirectory(const char16* curi, size_t* uriByteLength, byte** uriBytes)
{
#ifdef _WIN32
    std::wstring turi;
    if(curi[0] != _u('!'))
    {
        turi.append(curi);
        turi.append(TTDPathSeparator);
    }
    else
    {
        char16 cexeLocation[MAX_PATH];
        GetModuleFileName(NULL, cexeLocation, MAX_PATH);

        char16 drive[_MAX_DRIVE];
        char16 dir[_MAX_DIR];
        char16 name[_MAX_FNAME];
        char16 ext[_MAX_EXT];
        _wsplitpath_s(cexeLocation, drive, dir, name, ext);

        char16 rootPath[MAX_PATH];
        _wmakepath_s(rootPath, drive, dir, nullptr, nullptr);

        turi.append(rootPath);

        turi.append(_u("_ttdlog"));
        turi.append(TTDPathSeparator);

        turi.append(curi + 1);
        turi.append(TTDPathSeparator);
    }

    *uriBytes = (byte*)CoTaskMemAlloc(MAX_PATH * sizeof(char16));
    memset(*uriBytes, 0, MAX_PATH * sizeof(char16));
    char16* nuri = (char16*)(*uriBytes);

    _wfullpath(nuri, turi.c_str(), MAX_PATH);
    *uriByteLength = wcslen(nuri) * sizeof(char16); //include null terminator in size computation
#else
    AssertMsg(false, "Not x-plat yet!!!");
#endif
}

void CALLBACK Helpers::TTInitializeForWriteLogStreamCallback(size_t uriByteLength, const byte* uriBytes)
{
    //If the directory does not exist then we want to create it
    Helpers::CreateDirectoryIfNeeded(uriByteLength, uriBytes);

    //Clear the logging directory so it is ready for us to write into
    Helpers::DeleteDirectory(uriByteLength, uriBytes);
}

JsTTDStreamHandle Helpers::TTCreateStreamCallback(size_t uriByteLength, const byte* uriBytes, const char* asciiResourceName, bool read, bool write)
{
    AssertMsg((read | write) & (!read | !write), "Read/Write streams not supported yet -- defaulting to read only");

    FILE* res = nullptr;
#ifdef _WIN32
    std::wstring path((const char16*)uriBytes, uriByteLength / sizeof(char16));

    size_t slen = strlen(asciiResourceName);
    for(size_t i = 0; i < slen; ++i)
    {
        char16 c = asciiResourceName[i];
        path.push_back(c);
    }

    _wfopen_s(&res, path.c_str(), read ? _u("r+b") : _u("w+b"));
#else
    AssertMsg(false, "Not x-plat yet!!!");
#endif

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

#ifdef _WIN32
    *readCount = fread_s(buff, size, 1, size, (FILE*)handle);
    ok = (*readCount != 0);
#else
    AssertMsg(false, "Not x-plat yet!!!");
#endif

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

#ifdef _WIN32
    *writtenCount = fwrite(buff, 1, size, (FILE*)handle);
    ok = (*writtenCount == size);
#else
    AssertMsg(false, "Not x-plat yet!!!");
#endif

    Helpers::TTReportLastIOErrorAsNeeded(ok, "Failed Read!!!");

    return ok ? true : false;
}

void CALLBACK Helpers::TTFlushAndCloseStreamCallback(JsTTDStreamHandle handle, bool read, bool write)
{
    fflush((FILE*)handle);
    fclose((FILE*)handle);
}

