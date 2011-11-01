/*
 * Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY SAMSUNG ELECTRONICS CORPORATION AND ITS
 * CONTRIBUTORS "AS IS", AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG
 * ELECTRONICS CORPORATION OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS, OR BUSINESS INTERRUPTION), HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
 * NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(WEBCL)

#include "JSDOMGlobalObject.h"
#include "DOMWindow.h"
#include "JSDOMWindow.h"
#include <runtime/JSFunction.h>
#include "JSWebCLComputeContext.h"
#include "JSDOMBinding.h"
#include "WebCLComputeContext.h"
#include "JSImageData.h"
#include "JSOESStandardDerivatives.h"
#include "JSOESTextureFloat.h"
#include "JSFloat32Array.h"
#include "JSInt32Array.h"
#include "JSWebCLContext.h"
#include "JSUint8Array.h"
#include "JSWebKitCSSMatrix.h"
#include "JSCustomWebCLFinishCallback.h"
#include "NotImplemented.h"
#include "OESStandardDerivatives.h"
#include "OESTextureFloat.h"
#include "Float32Array.h"
#include "WebCLGetInfo.h"
#include "Int32Array.h"
#include <runtime/Error.h>
#include <runtime/JSArray.h>
#include <wtf/FastMalloc.h>
#include <wtf/OwnFastMallocPtr.h>
#include "JSWebCLPlatformID.h"
#include "JSWebCLDeviceID.h"
#include "JSWebCLKernel.h"
#include "JSWebCLProgram.h"
#include "JSWebCLCommandQueue.h"
#include "JSWebCLContext.h"
#include "JSWebCLMem.h"
#include "JSWebCLEvent.h"
#include "JSWebCLSampler.h"
#include "JSWebCLImage.h"
#include "WebCLPlatformID.h"
#include "WebCLKernel.h"
#include "WebCLProgram.h"
#include "WebCLDeviceID.h"
#include "WebCLCommandQueue.h"
#include "WebCLContext.h"
#include "WebCLMem.h"
#include "WebCLEvent.h"
#include "WebCLSampler.h"
#include "WebCLImage.h"
#include "WebCLBuffer.h"
#include "WebCLKernelTypes.h"
#include "ScriptValue.h"
#include <stdio.h>

using namespace JSC;
using namespace std;

namespace WebCore {

EncodedJSValue JSC_HOST_CALL JSWebCLComputeContextConstructor::constructJSWebCLComputeContext(ExecState* exec)
{
	JSWebCLComputeContextConstructor* jsConstructor = static_cast<JSWebCLComputeContextConstructor*>(exec->callee());
	ScriptExecutionContext* context = jsConstructor->scriptExecutionContext();
	RefPtr<WebCLComputeContext> webCLComputeContext = WebCLComputeContext::create(context);

	//DOMWindow* window = asJSDOMWindow(exec->lexicalGlobalObject())->impl();
	//RefPtr<WebCLComputeContext> webCLComputeContext = WebCLComputeContext::create(window->document());
	//return JSValue::encode(asObject(toJS(exec, jsConstructor->globalObject())));
	return JSValue::encode(CREATE_DOM_WRAPPER(exec, jsConstructor->globalObject(), WebCLComputeContext, webCLComputeContext.get()));
}


static PassRefPtr<WebCLFinishCallback> createFinishCallback(ExecState* exec, JSDOMGlobalObject* globalObject, JSValue value)
{
	if (value.isUndefinedOrNull())
	{
		setDOMException(exec, TYPE_MISMATCH_ERR);
		return 0;
	}
	JSObject* object = asObject(value);
	return JSCustomWebCLFinishCallback::create(object, globalObject);
}

static JSValue toJS(ExecState* exec, JSDOMGlobalObject* globalObject, const WebCLGetInfo& info)
{
	switch (info.getType()) {
		case WebCLGetInfo::kTypeBool:
			return jsBoolean(info.getBool());
		case WebCLGetInfo::kTypeBoolArray: {
											   MarkedArgumentBuffer list;
											   const Vector<bool>& value = info.getBoolArray();
											   for (size_t ii = 0; ii < value.size(); ++ii)
												   list.append(jsBoolean(value[ii]));
											   return constructArray(exec, list);
										   }
		case WebCLGetInfo::kTypeFloat:
										   return jsNumber(info.getFloat());
		case WebCLGetInfo::kTypeInt:
										   return jsNumber(info.getInt());
		case WebCLGetInfo::kTypeNull:
										   return jsNull();
		case WebCLGetInfo::kTypeString:
										   return jsString(exec, info.getString());
		case WebCLGetInfo::kTypeUnsignedInt:
										   return jsNumber(info.getUnsignedInt());
		case WebCLGetInfo::kTypeUnsignedLong:
										   return jsNumber(info.getUnsignedLong());
		case WebCLGetInfo::kTypeWebCLFloatArray:
										   return toJS(exec, globalObject, info.getWebCLFloatArray());
		case WebCLGetInfo::kTypeWebCLIntArray:
										   return toJS(exec, globalObject, info.getWebCLIntArray());
		case WebCLGetInfo::kTypeWebCLProgram:
										   return toJS(exec, globalObject, info.getWebCLProgram());
		case WebCLGetInfo::kTypeWebCLContext:
										   return toJS(exec, globalObject, info.getWebCLContext());
		case WebCLGetInfo::kTypeWebCLCommandQueue:
										   return toJS(exec, globalObject, info.getWebCLCommandQueue());
		default:
										   notImplemented();
										   return jsUndefined();
	}
}

JSValue JSWebCLComputeContext::getDeviceInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLDeviceID* device  = toWebCLDeviceID(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned device_type  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getDeviceInfo(device,device_type);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}

JSValue JSWebCLComputeContext::getPlatformInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLPlatformID* platform  = toWebCLPlatformID(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned platform_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getPlatformInfo(platform,platform_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLComputeContext::getKernelInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLKernel* kernel  = toWebCLKernel(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned kernel_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getKernelInfo(kernel,kernel_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLComputeContext::getProgramInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLProgram* program  = toWebCLProgram(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned programInfo  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getProgramInfo(program,programInfo);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLComputeContext::getCommandQueueInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLCommandQueue* queue  = toWebCLCommandQueue(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned queue_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getCommandQueueInfo(queue,queue_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLComputeContext::getProgramBuildInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 3)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLProgram* program  = toWebCLProgram(exec->argument(0));
	WebCLDeviceID* device  = toWebCLDeviceID(exec->argument(1));
	if (exec->hadException())
		return jsUndefined();
	unsigned build_info  = exec->argument(2).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getProgramBuildInfo(program,device,build_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLComputeContext::getContextInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLContext* wcontext  = toWebCLContext(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned context_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getContextInfo(wcontext,context_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}

JSValue JSWebCLComputeContext::getKernelWorkGroupInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 3)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLKernel* program  = toWebCLKernel(exec->argument(0));
	WebCLDeviceID* device  = toWebCLDeviceID(exec->argument(1));
	if (exec->hadException())
		return jsUndefined();
	unsigned work_info  = exec->argument(2).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getKernelWorkGroupInfo(program,device,work_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLComputeContext::getMemObjectInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLMem* memObj  = toWebCLMem(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned mem_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getMemObjectInfo(memObj,mem_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLComputeContext::getEventInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLEvent* eventObj  = toWebCLEvent(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned event_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getEventInfo(eventObj,event_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLComputeContext::getEventProfilingInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLEvent* eventObj  = toWebCLEvent(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned event_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getEventProfilingInfo(eventObj,event_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}

JSValue JSWebCLComputeContext::getSamplerInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLSampler* wsampler  = toWebCLSampler(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned sampler_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getSamplerInfo(wsampler,sampler_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLComputeContext::getImageInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLImage* wimage  = toWebCLImage(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned image_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getImageInfo(wimage,image_info);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLComputeContext::finish(ExecState* exec)
{
	if (exec->argumentCount() != 3)
		return throwSyntaxError(exec);

	WebCLCommandQueue* queue  = toWebCLCommandQueue(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();

	RefPtr<WebCLFinishCallback> finishCallback = createFinishCallback(exec, static_cast<JSDOMGlobalObject*>(exec->lexicalGlobalObject()), exec->argument(1));
	if (exec->hadException())
		return jsUndefined();
	ASSERT(finishCallback);

	unsigned userParam  = exec->argument(2).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();

	m_impl->finish(queue ,finishCallback.release(), userParam);
	return jsUndefined();

}

JSValue JSWebCLComputeContext::setKernelArg(ExecState* exec)
{
	if (exec->argumentCount() != 4)
		return throwSyntaxError(exec);

	WebCLComputeContext* context = static_cast<WebCLComputeContext*>(impl());
	WebCLKernel* kernel  = toWebCLKernel(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();

	unsigned argIndex  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();

	JSValue value = exec->argument(2);
	ScriptValue object(exec->globalData(), exec->argument(2));
	if (exec->hadException())
		return jsUndefined();

	unsigned argType  = exec->argument(3).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
   	context->setKernelArg(kernel ,argIndex, object.toWebCLKernelTypeValue(exec),argType);
	return jsUndefined();

}

} // namespace WebCore

#endif // ENABLE(WEBCL)
