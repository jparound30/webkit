/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "Console.h"

#include "Chrome.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "Page.h"
#include "PlatformString.h"

namespace WebCore {

Console::Console(Frame* frame)
    : RefCounted<Console>(0)
    , m_frame(frame)
{
}

void Console::disconnectFrame()
{
    m_frame = 0;
}

void Console::error(const String& message)
{
    if (!m_frame)
        return;

    Page* page = m_frame->page();
    if (!page)
        return;

    page->chrome()->addMessageToConsole(JSMessageSource, ErrorMessageLevel, message, 0, m_frame->loader()->url().prettyURL());
}

void Console::info(const String& message)
{
    if (!m_frame)
        return;

    Page* page = m_frame->page();
    if (!page)
        return;

    page->chrome()->addMessageToConsole(JSMessageSource, LogMessageLevel, message, 0, m_frame->loader()->url().prettyURL());
}

void Console::log(const String& message)
{
    if (!m_frame)
        return;

    Page* page = m_frame->page();
    if (!page)
        return;

    page->chrome()->addMessageToConsole(JSMessageSource, LogMessageLevel, message, 0, m_frame->loader()->url().prettyURL());
}

void Console::warn(const String& message)
{
    if (!m_frame)
        return;

    Page* page = m_frame->page();
    if (!page)
        return;

    page->chrome()->addMessageToConsole(JSMessageSource, WarningMessageLevel, message, 0, m_frame->loader()->url().prettyURL());
}

} // namespace WebCore
