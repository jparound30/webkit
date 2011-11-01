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

#include "WebCLCommandQueue.h"

namespace WebCore {

WebCLCommandQueue::~WebCLCommandQueue()
{
}

PassRefPtr<WebCLCommandQueue> WebCLCommandQueue::create(WebCLComputeContext* compute_context, cl_command_queue command_queue)
{
	return adoptRef(new WebCLCommandQueue(compute_context, command_queue));
}

WebCLCommandQueue::WebCLCommandQueue(WebCLComputeContext* compute_context, cl_command_queue command_queue) : m_context(compute_context), m_cl_command_queue(command_queue)
{
}

cl_command_queue WebCLCommandQueue::getCLCommandQueue()
{
	return m_cl_command_queue;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
