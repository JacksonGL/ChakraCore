//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
/// \mainpage Chakra Hosting Debugging API Reference
///
/// Chakra is Microsoft's JavaScript engine. It is an integral part of Internet Explorer but can
/// also be hosted independently by other applications. This reference describes the APIs available
/// to applications to debug JavaScript.

/// \file
/// \brief The Chakra hosting debugging API.
///
/// This file contains a flat C API layer. This is the API exported by ChakraCore.dll.

#ifdef _MSC_VER
#pragma once
#endif  // _MSC_VER

#ifndef _CHAKRADEBUG_H_
#define _CHAKRADEBUG_H_

#include "ChakraCommon.h"

#ifdef _WIN32
typedef __int64 int64_t;
#endif
//Other platforms should include <stdint.h> and have this defined automatically

    /// <summary>
    ///     Debug events reported from ChakraCore engine.
    /// </summary>
    typedef enum _JsDiagDebugEvent
    {
        /// <summary>
        ///     Indicates a new script being compiled, this includes script, eval, new function.
        /// </summary>
        JsDiagDebugEventSourceCompile = 0,
        /// <summary>
        ///     Indicates compile error for a script.
        /// </summary>
        JsDiagDebugEventCompileError = 1,
        /// <summary>
        ///     Indicates a break due to a breakpoint.
        /// </summary>
        JsDiagDebugEventBreakpoint = 2,
        /// <summary>
        ///     Indicates a break after completion of step action.
        /// </summary>
        JsDiagDebugEventStepComplete = 3,
        /// <summary>
        ///     Indicates a break due to debugger statement.
        /// </summary>
        JsDiagDebugEventDebuggerStatement = 4,
        /// <summary>
        ///     Indicates a break due to async break.
        /// </summary>
        JsDiagDebugEventAsyncBreak = 5,
        /// <summary>
        ///     Indicates a break due to a runtime script exception.
        /// </summary>
        JsDiagDebugEventRuntimeException = 6
    } JsDiagDebugEvent;

    /// <summary>
    ///     Break on Exception attributes.
    /// </summary>
    typedef enum _JsDiagBreakOnExceptionAttributes
    {
        /// <summary>
        ///     Don't break on any exception.
        /// </summary>
        JsDiagBreakOnExceptionAttributeNone = 0x0,
        /// <summary>
        ///     Break on uncaught exception.
        /// </summary>
        JsDiagBreakOnExceptionAttributeUncaught = 0x1,
        /// <summary>
        ///     Break on first chance exception.
        /// </summary>
        JsDiagBreakOnExceptionAttributeFirstChance = 0x2
    } JsDiagBreakOnExceptionAttributes;

    /// <summary>
    ///     Stepping types.
    /// </summary>
    typedef enum _JsDiagStepType
    {
        /// <summary>
        ///     Perform a step operation to next statement.
        /// </summary>
        JsDiagStepTypeStepIn = 0,
        /// <summary>
        ///     Perform a step out from the current function.
        /// </summary>
        JsDiagStepTypeStepOut = 1,
        /// <summary>
        ///     Perform a single step over after a debug break if the next statement is a function call, else behaves as a stepin.
        /// </summary>
        JsDiagStepTypeStepOver = 2,
        /// <summary>
        ///     Perform a single step back to the previous statement (only applicable in TTD mode).
        /// </summary>
        JsDiagStepTypeStepBack = 3
    } JsDiagStepType;

    /// <summary>
    ///     User implemented callback routine for debug events.
    /// </summary>
    /// <remarks>
    ///     Use <c>JsDiagStartDebugging</c> to register the callback.
    /// </remarks>
    /// <param name="debugEvent">The type of JsDiagDebugEvent event.</param>
    /// <param name="eventData">Additional data related to the debug event.</param>
    /// <param name="callbackState">The state passed to <c>JsDiagStartDebugging</c>.</param>
    typedef void (CHAKRA_CALLBACK * JsDiagDebugEventCallback)(_In_ JsDiagDebugEvent debugEvent, _In_ JsValueRef eventData, _In_opt_ void* callbackState);

    /// <summary>
    ///     Starts debugging in the given runtime.
    /// </summary>
    /// <param name="runtimeHandle">Runtime to put into debug mode.</param>
    /// <param name="debugEventCallback">Registers a callback to be called on every JsDiagDebugEvent.</param>
    /// <param name="callbackState">User provided state that will be passed back to the callback.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The runtime should be active on the current thread and should not be in debug state.
    /// </remarks>
    CHAKRA_API
        JsDiagStartDebugging(
            _In_ JsRuntimeHandle runtimeHandle,
            _In_ JsDiagDebugEventCallback debugEventCallback,
            _In_opt_ void* callbackState);

    /// <summary>
    ///     Stops debugging in the given runtime.
    /// </summary>
    /// <param name="runtimeHandle">Runtime to stop debugging.</param>
    /// <param name="callbackState">User provided state that was passed in JsDiagStartDebugging.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The runtime should be active on the current thread and in debug state.
    /// </remarks>
    CHAKRA_API
        JsDiagStopDebugging(
            _In_ JsRuntimeHandle runtimeHandle,
            _Out_ void** callbackState);

    /// <summary>
    ///     Request the runtime to break on next JavaScript statement.
    /// </summary>
    /// <param name="runtimeHandle">Runtime to request break.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The runtime should be in debug state. This API can be called from another runtime.
    /// </remarks>
    CHAKRA_API
        JsDiagRequestAsyncBreak(
            _In_ JsRuntimeHandle runtimeHandle);

    /// <summary>
    ///     List all breakpoints in the current runtime.
    /// </summary>
    /// <param name="breakpoints">Array of breakpoints.</param>
    /// <remarks>
    ///     <para>
    ///     [{
    ///         "breakpointId" : 1,
    ///         "scriptId" : 1,
    ///         "line" : 0,
    ///         "column" : 62
    ///     }]
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can be called when runtime is at a break or running.
    /// </remarks>
    CHAKRA_API
        JsDiagGetBreakpoints(
            _Out_ JsValueRef *breakpoints);

    /// <summary>
    ///     Sets breakpoint in the specified script at give location.
    /// </summary>
    /// <param name="scriptId">Id of script from JsDiagGetScripts or JsDiagGetSource to put breakpoint.</param>
    /// <param name="lineNumber">0 based line number to put breakpoint.</param>
    /// <param name="columnNumber">0 based column number to put breakpoint.</param>
    /// <param name="breakpoint">Breakpoint object with id, line and column if success.</param>
    /// <remarks>
    ///     <para>
    ///     {
    ///         "breakpointId" : 1,
    ///         "line" : 2,
    ///         "column" : 4
    ///     }
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can be called when runtime is at a break or running.
    /// </remarks>
    CHAKRA_API
        JsDiagSetBreakpoint(
            _In_ unsigned int scriptId,
            _In_ unsigned int lineNumber,
            _In_ unsigned int columnNumber,
            _Out_ JsValueRef *breakpoint);

    /// <summary>
    ///     Remove a breakpoint.
    /// </summary>
    /// <param name="breakpointId">Breakpoint id returned from JsDiagSetBreakpoint.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can be called when runtime is at a break or running.
    /// </remarks>
    CHAKRA_API
        JsDiagRemoveBreakpoint(
            _In_ unsigned int breakpointId);

    /// <summary>
    ///     Sets break on exception handling.
    /// </summary>
    /// <param name="runtimeHandle">Runtime to set break on exception attributes.</param>
    /// <param name="exceptionAttributes">Mask of JsDiagBreakOnExceptionAttributes to set.</param>
    /// <remarks>
    ///     <para>
    ///         If this API is not called the default value is set to JsDiagBreakOnExceptionAttributeUncaught in the runtime.
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The runtime should be in debug state. This API can be called from another runtime.
    /// </remarks>
    CHAKRA_API
        JsDiagSetBreakOnException(
            _In_ JsRuntimeHandle runtimeHandle,
            _In_ JsDiagBreakOnExceptionAttributes exceptionAttributes);

    /// <summary>
    ///     Gets break on exception setting.
    /// </summary>
    /// <param name="runtimeHandle">Runtime from which to get break on exception attributes, should be in debug mode.</param>
    /// <param name="exceptionAttributes">Mask of JsDiagBreakOnExceptionAttributes.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The runtime should be in debug state. This API can be called from another runtime.
    /// </remarks>
    CHAKRA_API
        JsDiagGetBreakOnException(
            _In_ JsRuntimeHandle runtimeHandle,
            _Out_ JsDiagBreakOnExceptionAttributes* exceptionAttributes);

    /// <summary>
    ///     Sets the step type in the runtime after a debug break.
    /// </summary>
    /// <remarks>
    ///     Requires to be at a debug break.
    /// </remarks>
    /// <param name="resumeType">Type of JsDiagStepType.</param>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can only be called when runtime is at a break.
    /// </remarks>
    CHAKRA_API
        JsDiagSetStepType(
            _In_ JsDiagStepType stepType);

    /// <summary>
    ///     Gets list of scripts.
    /// </summary>
    /// <param name="scriptsArray">Array of script objects.</param>
    /// <remarks>
    ///     <para>
    ///     [{
    ///         "scriptId" : 2,
    ///         "fileName" : "c:\\Test\\Test.js",
    ///         "lineCount" : 4,
    ///         "sourceLength" : 111
    ///       }, {
    ///         "scriptId" : 3,
    ///         "parentScriptId" : 2,
    ///         "scriptType" : "eval code",
    ///         "lineCount" : 1,
    ///         "sourceLength" : 12
    ///     }]
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can be called when runtime is at a break or running.
    /// </remarks>
    CHAKRA_API
        JsDiagGetScripts(
            _Out_ JsValueRef *scriptsArray);

    /// <summary>
    ///     Gets source for a specific script identified by scriptId from JsDiagGetScripts.
    /// </summary>
    /// <param name="scriptId">Id of the script.</param>
    /// <param name="source">Source object.</param>
    /// <remarks>
    ///     <para>
    ///     {
    ///         "scriptId" : 1,
    ///         "fileName" : "c:\\Test\\Test.js",
    ///         "lineCount" : 12,
    ///         "sourceLength" : 15154,
    ///         "source" : "var x = 1;"
    ///     }
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can be called when runtime is at a break or running.
    /// </remarks>
    CHAKRA_API
        JsDiagGetSource(
            _In_ unsigned int scriptId,
            _Out_ JsValueRef *source);

    /// <summary>
    ///     Gets the source information for a function object.
    /// </summary>
    /// <param name="function">JavaScript function.</param>
    /// <param name="functionPosition">Function position - scriptId, start line, start column, line number of first statement, column number of first statement.</param>
    /// <remarks>
    ///     <para>
    ///     {
    ///         "scriptId" : 1,
    ///         "fileName" : "c:\\Test\\Test.js",
    ///         "line" : 1,
    ///         "column" : 2,
    ///         "firstStatementLine" : 6,
    ///         "firstStatementColumn" : 0
    ///     }
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     This API can be called when runtime is at a break or running.
    /// </remarks>
    CHAKRA_API
        JsDiagGetFunctionPosition(
            _In_ JsValueRef function,
            _Out_ JsValueRef *functionPosition);

    /// <summary>
    ///     Gets the stack trace information.
    /// </summary>
    /// <param name="stackTrace">Stack trace information.</param>
    /// <remarks>
    ///     <para>
    ///     [{
    ///         "index" : 0,
    ///         "scriptId" : 2,
    ///         "line" : 3,
    ///         "column" : 0,
    ///         "sourceLength" : 9,
    ///         "sourceText" : "var x = 1",
    ///         "functionHandle" : 1
    ///     }]
    ///    </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can only be called when runtime is at a break.
    /// </remarks>
    CHAKRA_API
        JsDiagGetStackTrace(
            _Out_ JsValueRef *stackTrace);

    /// <summary>
    ///     Gets the list of properties corresponding to the frame.
    /// </summary>
    /// <param name="stackFrameIndex">Index of stack frame from JsDiagGetStackTrace.</param>
    /// <param name="properties">Object of properties array (properties, scopes and globals).</param>
    /// <remarks>
    ///     <para>
    ///     propertyAttributes is a bit mask of
    ///         NONE = 0x1,
    ///         HAVE_CHILDRENS = 0x2,
    ///         READ_ONLY_VALUE = 0x4,
    ///     </para>
    /// </remarks>
    /// <remarks>
    ///     <para>
    ///     {
    ///         "thisObject": {
    ///             "name": "this",
    ///             "type" : "object",
    ///             "className" : "Object",
    ///             "display" : "{...}",
    ///             "propertyAttributes" : 1,
    ///             "handle" : 306
    ///         },
    ///         "exception" : {
    ///             "name" : "{exception}",
    ///             "type" : "object",
    ///             "display" : "'a' is undefined",
    ///             "className" : "Error",
    ///             "propertyAttributes" : 1,
    ///             "handle" : 307
    ///         }
    ///         "arguments" : {
    ///             "name" : "arguments",
    ///             "type" : "object",
    ///             "display" : "{...}",
    ///             "className" : "Object",
    ///             "propertyAttributes" : 1,
    ///             "handle" : 190
    ///         },
    ///         "returnValue" : {
    ///             "name" : "[Return value]",
    ///             "type" : "undefined",
    ///             "propertyAttributes" : 0,
    ///             "handle" : 192
    ///         },
    ///         "functionCallsReturn" : [{
    ///                 "name" : "[foo1 returned]",
    ///                 "type" : "number",
    ///                 "value" : 1,
    ///                 "propertyAttributes" : 2,
    ///                 "handle" : 191
    ///             }
    ///         ],
    ///         "locals" : [],
    ///         "scopes" : [{
    ///                 "index" : 0,
    ///                 "handle" : 193
    ///             }
    ///         ],
    ///         "globals" : {
    ///             "handle" : 194
    ///         }
    ///     }
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can only be called when runtime is at a break.
    /// </remarks>
    CHAKRA_API
        JsDiagGetStackProperties(
            _In_ unsigned int stackFrameIndex,
            _Out_ JsValueRef *properties);

    /// <summary>
    ///     Gets the list of children of a handle.
    /// </summary>
    /// <param name="objectHandle">Handle of object.</param>
    /// <param name="fromCount">0-based from count of properties, usually 0.</param>
    /// <param name="totalCount">Number of properties to return.</param>
    /// <param name="propertiesObject">Array of properties.</param>
    /// <remarks>Handle should be from objects returned from call to JsDiagGetStackProperties.</remarks>
    /// <remarks>For scenarios where object have large number of properties totalCount can be used to control how many properties are given.</remarks>
    /// <remarks>
    ///     <para>
    ///     {
    ///         "totalPropertiesOfObject": 10,
    ///         "properties" : [{
    ///                 "name" : "__proto__",
    ///                 "type" : "object",
    ///                 "display" : "{...}",
    ///                 "className" : "Object",
    ///                 "propertyAttributes" : 1,
    ///                 "handle" : 156
    ///             }
    ///         ],
    ///         "debuggerOnlyProperties" : [{
    ///                 "name" : "[Map]",
    ///                 "type" : "string",
    ///                 "value" : "size = 0",
    ///                 "propertyAttributes" : 2,
    ///                 "handle" : 157
    ///             }
    ///         ]
    ///     }
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can only be called when runtime is at a break.
    /// </remarks>
    CHAKRA_API
        JsDiagGetProperties(
            _In_ unsigned int objectHandle,
            _In_ unsigned int fromCount,
            _In_ unsigned int totalCount,
            _Out_ JsValueRef *propertiesObject);

    /// <summary>
    ///     Gets the object corresponding to handle.
    /// </summary>
    /// <param name="objectHandle">Handle of object.</param>
    /// <param name="handleObject">Object corresponding to the handle.</param>
    /// <remarks>
    ///     <para>
    ///     {
    ///         "scriptId" : 24,
    ///          "line" : 1,
    ///          "column" : 63,
    ///          "name" : "foo",
    ///          "type" : "function",
    ///          "handle" : 2
    ///     }
    ///    </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can only be called when runtime is at a break.
    /// </remarks>
    CHAKRA_API
        JsDiagGetObjectFromHandle(
            _In_ unsigned int objectHandle,
            _Out_ JsValueRef *handleObject);

#ifdef _WIN32
    /// <summary>
    ///     Evaluates an expression on given frame.
    /// </summary>
    /// <param name="expression">Expression to evaluate.</param>
    /// <param name="stackFrameIndex">Index of stack frame on which to evaluate the expression.</param>
    /// <param name="evalResult">Result of evaluation.</param>
    /// <remarks>
    ///     <para>
    ///     evalResult when evaluating 'this' and return is JsNoError
    ///     {
    ///         "name" : "this",
    ///         "type" : "object",
    ///         "className" : "Object",
    ///         "display" : "{...}",
    ///         "propertyAttributes" : 1,
    ///         "handle" : 18
    ///     }
    ///
    ///     evalResult when evaluating a script which throws JavaScript error and return is JsErrorScriptException
    ///     {
    ///         "name" : "a.b.c",
    ///         "type" : "object",
    ///         "className" : "Error",
    ///         "display" : "'a' is undefined",
    ///         "propertyAttributes" : 1,
    ///         "handle" : 18
    ///     }
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, evalResult will contain the result
    ///     The code <c>JsErrorScriptException</c> if evaluate generated a JavaScript exception, evalResult will contain the error details
    ///     Other error code for invalid parameters or API was not called at break
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can only be called when runtime is at a break.
    /// </remarks>
    CHAKRA_API
        JsDiagEvaluate(
            _In_ const wchar_t *expression,
            _In_ unsigned int stackFrameIndex,
            _Out_ JsValueRef *evalResult);

#endif // _WIN32

    /// <summary>
    ///     Evaluates an expression on given frame.
    /// </summary>
    /// <param name="expression">Expression to evaluate.</param>
    /// <param name="stackFrameIndex">Index of stack frame on which to evaluate the expression.</param>
    /// <param name="evalResult">Result of evaluation.</param>
    /// <remarks>
    ///     <para>
    ///     evalResult when evaluating 'this' and return is JsNoError
    ///     {
    ///         "name" : "this",
    ///         "type" : "object",
    ///         "className" : "Object",
    ///         "display" : "{...}",
    ///         "propertyAttributes" : 1,
    ///         "handle" : 18
    ///     }
    ///
    ///     evalResult when evaluating a script which throws JavaScript error and return is JsErrorScriptException
    ///     {
    ///         "name" : "a.b.c",
    ///         "type" : "object",
    ///         "className" : "Error",
    ///         "display" : "'a' is undefined",
    ///         "propertyAttributes" : 1,
    ///         "handle" : 18
    ///     }
    ///     </para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, evalResult will contain the result
    ///     The code <c>JsErrorScriptException</c> if evaluate generated a JavaScript exception, evalResult will contain the error details
    ///     Other error code for invalid parameters or API was not called at break
    /// </returns>
    /// <remarks>
    ///     The current runtime should be in debug state. This API can only be called when runtime is at a break.
    /// </remarks>
    CHAKRA_API JsDiagEvaluateUtf8(
        _In_ const char *expression,
        _In_ unsigned int stackFrameIndex,
        _Out_ JsValueRef *evalResult);

    /////////////////////
    /// <summary>
    ///     TimeTravel move options as bit flag enum.
    /// </summary>
    typedef enum _JsTTDMoveModes : UINT64
    {
        /// <summary>
        ///     Indicates no special actions needed for move.
        /// </summary>
        JsTTDMoveNone = 0x0,

        /// <summary>
        ///     Indicates that we want to move to the first/last event.
        /// </summary>
        JsTTDMoveFirstEvent = 0x1,
        JsTTDMoveLastEvent = 0x2,

        /// <summary>
        ///     Indicates that we want to move to the kth event -- top 32 bits are event count.
        /// </summary>
        JsTTDMoveKthEvent = 0x4,

        /// <summary>
        ///     Indicates if we want to scan the snapshot interval containing the event to populate debug info before moving to execute event.
        /// </summary>
        JsTTDMoveScanIntervalBeforeDebugExecute = 0x10,

        /// <summary>
        ///     Indicates if we are doing the scan for a continue operation
        /// </summary>
        JsTTDMoveScanIntervalForContinue = 0x20,

        /// <summary>
        ///     Indicates if we want to set break on entry or just run and let something else trigger breakpoints.
        /// </summary>
        JsTTDMoveBreakOnEntry = 0x100
    } JsTTDMoveMode;

    /// <summary>
    ///     A handle for URI's that TTD information is written to/read from.
    /// </summary>
    typedef void* JsTTDStreamHandle;

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Ensure that the location specified for outputting the TTD data is clean. Specifically, ensure that any previous TTD
    ///     in the location has been removed.
    /// </summary>
    /// <param name="uriByteLength">The length of the uriBytes array that the host passed in for storing log info.</param>
    /// <param name="uriBytes">The bytes of the URI that the host passed in for storing log info.</param>
    typedef void (CHAKRA_CALLBACK *JsTTDInitializeForWriteLogStreamCallback)(_In_ size_t uriByteLength, _In_reads_(uriByteLength) const byte* uriBytes);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Construct a JsTTDStreamHandle that will be used to read/write the event log portion of the TTD data based on the uri
    ///     provided by JsTTDInitializeUriCallback.
    /// </summary>
    /// <remarks>
    ///     <para>Exactly one of read or write will be set to true.</para>
    /// </remarks>
    /// <param name="uriByteLength">The length of the uriBytes array that the host passed in for storing log info.</param>
    /// <param name="uriBytes">The bytes of the URI that the host passed in for storing log info.</param>
    /// <param name="asciiResourceName">A null terminated ascii string giving a unique name to the resource that the JsTTDStreamHandle will be created for.</param>
    /// <param name="read">If the handle should be opened for reading.</param>
    /// <param name="write">If the handle should be opened for writing.</param>
    /// <returns>A JsTTDStreamHandle opened in read/write mode as specified.</returns>
    typedef JsTTDStreamHandle (CHAKRA_CALLBACK *TTDOpenResourceStreamCallback)(_In_ size_t uriByteLength, _In_reads_(uriByteLength) const byte* uriBytes, _In_z_ const char* asciiResourceName, _In_ bool read, _In_ bool write);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     A callback for reading data from a handle.
    /// </summary>
    /// <param name="handle">The JsTTDStreamHandle to read the data from.</param>
    /// <param name="buff">The buffer to place the data into.</param>
    /// <param name="size">The max number of bytes that should be read.</param>
    /// <param name="readCount">The actual number of bytes read and placed in the buffer.</param>
    /// <returns>true if the read was successful false otherwise.</returns>
    typedef bool (CHAKRA_CALLBACK *JsTTDReadBytesFromStreamCallback)(_In_ JsTTDStreamHandle handle, _Out_writes_(size) byte* buff, _In_ size_t size, _Out_ size_t* readCount);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     A callback for writing data to a handle.
    /// </summary>
    /// <param name="handle">The JsTTDStreamHandle to write the data to.</param>
    /// <param name="buff">The buffer to copy the data from.</param>
    /// <param name="size">The max number of bytes that should be written.</param>
    /// <param name="readCount">The actual number of bytes written to the HANDLE.</param>
    /// <returns>true if the write was successful false otherwise.</returns>
    typedef bool (CHAKRA_CALLBACK *JsTTDWriteBytesToStreamCallback)(_In_ JsTTDStreamHandle handle, _In_reads_(size) byte* buff, _In_ size_t size, _Out_ size_t* writtenCount);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Flush and close the stream represented by the HANDLE as needed.
    /// </summary>
    /// <remarks>
    ///     <para>Exactly one of read or write will be set to true.</para>
    /// </remarks>
    /// <param name="handle">The JsTTDStreamHandle to close.</param>
    /// <param name="read">If the handle was opened for reading.</param>
    /// <param name="write">If the handle was opened for writing.</param>
    typedef void (CHAKRA_CALLBACK *JsTTDFlushAndCloseStreamCallback)(_In_ JsTTDStreamHandle handle, _In_ bool read, _In_ bool write);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Creates a new runtime in Record Mode.
    /// </summary>
    /// <param name="attributes">The attributes of the runtime to be created.</param>
    /// <param name="infoUri">The uri where the recorded Time-Travel data should be stored.</param>
    /// <param name="snapInterval">The interval to wait between snapshots (measured in millis).</param>
    /// <param name="snapHistoryLength">The amount of history to maintain before discarding -- measured in number of snapshots and controls how far back in time a trace can be reversed.</param>
    /// <param name="threadService">The thread service for the runtime. Can be null.</param>
    /// <param name="runtime">The runtime created.</param>
    /// <remarks>
    ///     <para>See <c>JsCreateRuntime</c> for additional information.</para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    CHAKRA_API
        JsTTDCreateRecordRuntime(
            _In_ JsRuntimeAttributes attributes,
            _In_reads_(infoUriCount) const byte* infoUri,
            _In_ size_t infoUriCount,
            _In_ size_t snapInterval,
            _In_ size_t snapHistoryLength,
            _In_opt_ JsThreadServiceCallback threadService,
            _Out_ JsRuntimeHandle *runtime);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Creates a new runtime in Debug Mode.
    /// </summary>
    /// <param name="attributes">The attributes of the runtime to be created.</param>
    /// <param name="infoUri">The uri where the recorded Time-Travel data should be loaded from.</param>
    /// <param name="threadService">The thread service for the runtime. Can be null.</param>
    /// <param name="runtime">The runtime created.</param>
    /// <remarks>
    ///     <para>See <c>JsCreateRuntime</c> for additional information.</para>
    /// </remarks>
    /// <returns>
    ///     The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.
    /// </returns>
    CHAKRA_API
        JsTTDCreateDebugRuntime(
            _In_ JsRuntimeAttributes attributes,
            _In_reads_(infoUriCount) const byte* infoUri,
            _In_ size_t infoUriCount,
            _In_opt_ JsThreadServiceCallback threadService,
            _Out_ JsRuntimeHandle *runtime);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Creates a script context in Time Travel mode for running scripts.
    /// </summary>
    /// <remarks>
    ///     <para>See <c>JsCreateContext</c> for more information.</para>
    /// </remarks>
    /// <param name="runtime">The runtime the script context is being created in.</param>
    /// <param name="newContext">The created script context.</param>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDCreateContext(
            _In_ JsRuntimeHandle runtime,
            _Out_ JsContextRef *newContext);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Initialize functions that the TTD system uses to write info out of main memory when needed.
    /// </summary>
    /// <param name="runtime">The runtime to set the functions for (must be created in debug mode).</param>
    /// <param name="ttdInitializeTTDUriFunction">The <c>JsTTDInitializeUriCallback</c> function for converting the user provided location into an absolute location for reading/writing time travel recording data.</param>
    /// <param name="writeInitializeFunction">The <c>JsTTDInitializeForWriteLogStreamCallback</c> function for performing any initializtion needed prepare uri for storing time travel recording data.</param>
    /// <param name="getLogStreamInfo">The <c>JsTTDGetLogStreamCallback</c> function for generating a HANDLE to read/write time travel recording log data from.</param>
    /// <param name="getSnapshotStreamInfo">The <c>JsTTDGetSnapshotStreamCallback</c> function for generating a HANDLE to read/write snapshot data from.</param>
    /// <param name="getSrcCodeStreamInfo">The <c>JsTTDGetSrcCodeStreamCallback</c> function for generating a HANDLE to read/write source code data.</param>
    /// <param name="readBytesFromStream">The <c>JsTTDReadBytesFromStreamCallback</c> function for reading bytes from a HANDLE.</param>
    /// <param name="writeBytesToStream">The <c>JsTTDWriteBytesToStreamCallback</c> function for writing bytes to a HANDLE.</param>
    /// <param name="flushAndCloseStream">The <c>JsTTDFlushAndCloseStreamCallback</c> function for flushing and closing a HANDLE as needed.</param>
    CHAKRA_API
        JsTTDSetIOCallbacks(
            _In_ JsRuntimeHandle runtime,
            _In_ JsTTDInitializeForWriteLogStreamCallback writeInitializeFunction,
            _In_ TTDOpenResourceStreamCallback openResourceStream,
            _In_ JsTTDReadBytesFromStreamCallback readBytesFromStream,
            _In_ JsTTDWriteBytesToStreamCallback writeBytesToStream,
            _In_ JsTTDFlushAndCloseStreamCallback flushAndCloseStream);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Start Time-Travel Recording.
    /// </summary>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDStartTimeTravelRecording();

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Stop Time-Travel Recording.
    /// </summary>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDStopTimeTravelRecording();

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Emit Time-Travel Recording.
    /// </summary>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDEmitTimeTravelRecording();

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Start Time-Travel Debugging.
    /// </summary>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDStartTimeTravelDebugging();

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Pause Time-Travel recording before executing code on behalf of debugger or other diagnostic/telemetry.
    /// </summary>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDPauseTimeTravelBeforeRuntimeOperation();

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     ReStart Time-Travel recording after executing code on behalf of debugger or other diagnostic/telemetry.
    /// </summary>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDReStartTimeTravelAfterRuntimeOperation();

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Notify the Js runtime we are at a safe yield point in the event loop (i.e. no locals on the stack and we can proccess as desired).
    /// </summary>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDNotifyYield();

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Notify the event log that the contents of one buffer have been copied to a second buffer.
    /// </summary>
    /// <param name="dst">The buffer that was written into.</param>
    /// <param name="dstIndex">The first index modified.</param>
    /// <param name="src">The buffer that was copied from.</param>
    /// <param name="srcIndex">The first index copied.</param>
    /// <param name="count">The number of bytes copied.</param>
    CHAKRA_API
        JsTTDRawBufferCopySyncIndirect(
            _In_ JsValueRef dst,
            _In_ size_t dstIndex,
            _In_ JsValueRef src,
            _In_ size_t srcIndex,
            _In_ size_t count);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Notify the event log that the contents of a naked byte* buffer passed to the host have been modified synchronously.
    /// </summary>
    /// <param name="buffer">The buffer that was modified.</param>
    /// <param name="index">The first index modified.</param>
    /// <param name="count">The number of bytes written.</param>
    CHAKRA_API
        JsTTDRawBufferModifySyncIndirect(
            _In_ JsValueRef buffer,
            _In_ size_t index,
            _In_ size_t count);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Get info for notifying the TTD system that a raw buffer it shares with the host has been modified.
    /// </summary>
    /// <param name="instance">The array buffer we want to monitor for contents modification.</param>
    /// <param name="initialModPos">The first position in the buffer that may be modified.</param>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDRawBufferAsyncModificationRegister(
            _In_ JsValueRef instance,
            _In_ byte* initialModPos);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Notify the event log that the contents of a naked byte* buffer passed to the host have been modified asynchronously.
    /// </summary>
    /// <param name="finalModPos">One past the last modified position in the buffer.</param>
    CHAKRA_API
        JsTTDRawBufferAsyncModifyComplete(
            _In_ byte* finalModPos);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Before calling JsTTDMoveToTopLevelEvent (which inflates a snapshot and replays) check to see if we want to reset the script context.
    ///     We reset the script context if the move will require inflating from a different snapshot that the last one.
    /// </summary>
    /// <param name="runtimeHandle">The runtime handle that the script is executing in.</param>
    /// <param name="moveMode">Flags controlling the way the move it performed and how other parameters are interpreted.</param>
    /// <param name="targetEventTime">The event time we want to move to or -1 if not relevant.</param>
    /// <param name="createFreshCxts">Out parameter that indicates if new script contexts need to be created for this move.</param>
    /// <param name="targetStartSnapTime">Out parameter with the event time of the snapshot that we should inflate from.</param>
    /// <param name="targetEndSnapTime">Optional Out parameter with the snapshot time following the event.</param>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API JsTTDGetSnapTimeTopLevelEventMove(
        _In_ JsRuntimeHandle runtimeHandle,
        _In_ JsTTDMoveMode moveMode,
        _Inout_ int64_t* targetEventTime,
        _Out_ bool* createFreshCxts,
        _Out_ int64_t* targetStartSnapTime,
        _Out_opt_ int64_t* targetEndSnapTime);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Before calling JsTTDMoveToTopLevelEvent (which inflates a snapshot and replays) check to see if we want to reset the script context.
    ///     We reset the script context if the move will require inflating from a different snapshot that the last one.
    /// </summary>
    /// <param name="runtimeHandle">The runtime handle that the script is executing in.</param>
    /// <param name="createFreshCtxs">Indicates if new script contexts need to be created for this move.</param>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDPrepContextsForTopLevelEventMove(
            _In_ JsRuntimeHandle runtimeHandle,
            _In_ bool createFreshCtxs);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     During debug operations some additional information is populated during replay. This runs the code between the given 
    ///     snapshots to poulate this information which may be needed by the debugger to determine time-travel jump targets.
    /// </summary>
    ///<param name = "startSnapTime">The snapshot time that we will start executing from.< / param>
    ///<param name = "endSnapTime">The snapshot time that we will stop at (or -1 if we want to run to the end).< / param>
    /// <param name="moveMode">Additional flags for controling how the move is done.</param>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API JsTTDPreExecuteSnapShotInterval(
        _In_ int64_t startSnapTime,
        _In_ int64_t endSnapTime,
        _In_ JsTTDMoveMode moveMode);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Move to the given top-level call event time (assuming JsTTDPrepContextsForTopLevelEventMove) was called previously to reset any script contexts.
    ///     This also computes the ready-to-run snapshot if needed.
    /// </summary>
    /// <param name="moveMode">Additional flags for controling how the move is done.</param>
    /// <param name="snapshotTime">The event time that we will start executing from to move to the given target time.</param>
    /// <param name="eventTime">The event that we want to move to.</param>
    /// <returns>The code <c>JsNoError</c> if the operation succeeded, a failure code otherwise.</returns>
    CHAKRA_API
        JsTTDMoveToTopLevelEvent(
            _In_ JsTTDMoveMode moveMode,
            _In_ int64_t snapshotTime,
            _In_ int64_t eventTime);

    /// <summary>
    ///     TTD API -- may change in future versions:
    ///     Execute from the current point in the log to the end returning the error code.
    /// </summary>
    /// <param name="moveMode">Additional flags for controling how the move is done.</param>
    /// <param name="rootEventTime">The event time that we should move to next or notification (-1) that replay has ended.</param>
    /// <returns>
    ///     If the debugger requested an abort the code is JsNoError -- rootEventTime is the target event time we need to move to and re - execute from.
    ///     If we aborted at the end of the replay log the code is JsNoError -- rootEventTime is -1.
    ///     If there was an unhandled script exception the code is JsErrorCategoryScript.
    /// </returns>
    CHAKRA_API
        JsTTDReplayExecution(
            _Inout_ JsTTDMoveMode* moveMode,
            _Inout_ int64_t* rootEventTime);

#endif // _CHAKRADEBUG_H_
