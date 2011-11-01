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

#ifndef WebCLPlatformIDList_h
#define WebCLPlatformIDList_h

#include <Opencl/opencl.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

#include "WebCLPlatformID.h"

namespace WebCore {

class WebCLComputeContext;

class WebCLPlatformIDList : public RefCounted<WebCLPlatformIDList> {

public:
	virtual ~WebCLPlatformIDList();
	static PassRefPtr<WebCLPlatformIDList> create(WebCLComputeContext*);
	WebCLPlatformIDList();
	cl_platform_id getCLPlatformIDs();
	
	unsigned length() const;
	WebCLPlatformID* item(unsigned index);
	
private:
	WebCLPlatformIDList(WebCLComputeContext*);

	WebCLComputeContext* m_context;

	Vector<RefPtr<WebCLPlatformID> > m_platform_id_list;
	cl_platform_id* m_cl_platforms;
	cl_uint m_num_platforms;
	
};

} // namespace WebCore

#endif // WebCLPlatformIDList_h
