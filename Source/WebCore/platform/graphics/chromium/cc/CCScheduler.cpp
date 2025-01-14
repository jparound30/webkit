/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "cc/CCScheduler.h"
#include "TraceEvent.h"

namespace WebCore {

CCScheduler::CCScheduler(CCSchedulerClient* client, PassOwnPtr<CCFrameRateController> frameRateController)
    : m_client(client)
    , m_frameRateController(frameRateController)
    , m_updateMoreResourcesPending(false)
{
    ASSERT(m_client);
    m_frameRateController->setClient(this);

    // FIXME: make the CCSchedulerStateMachine turn off FrameRateController it isn't needed.
    m_frameRateController->setActive(true);
}

CCScheduler::~CCScheduler()
{
    m_frameRateController->setActive(false);
}

void CCScheduler::setVisible(bool visible)
{
    m_stateMachine.setVisible(visible);
}
void CCScheduler::setNeedsAnimate()
{
    // Stub through to requestCommit for now.
    setNeedsCommit();
}

void CCScheduler::setNeedsCommit()
{
    m_stateMachine.setNeedsCommit();
    processScheduledActions();
}

void CCScheduler::setNeedsRedraw()
{
    m_stateMachine.setNeedsRedraw();
    processScheduledActions();
}

void CCScheduler::beginFrameComplete()
{
    TRACE_EVENT("CCScheduler::beginFrameComplete", this, 0);
    m_stateMachine.beginFrameComplete();
    processScheduledActions();
}

void CCScheduler::didSwapBuffersComplete()
{
    TRACE_EVENT("CCScheduler::didSwapBuffersComplete", this, 0);
    m_frameRateController->didFinishFrame();
}

void CCScheduler::didSwapBuffersAbort()
{
    TRACE_EVENT("CCScheduler::didSwapBuffersAbort", this, 0);
    m_frameRateController->didAbortAllPendingFrames();
}

void CCScheduler::beginFrame()
{
    if (m_updateMoreResourcesPending) {
        m_updateMoreResourcesPending = false;
        m_stateMachine.beginUpdateMoreResourcesComplete(m_client->hasMoreResourceUpdates());
    }
    TRACE_EVENT("CCScheduler::beginFrame", this, 0);

    m_stateMachine.setInsideVSync(true);
    processScheduledActions();
    m_stateMachine.setInsideVSync(false);
}

void CCScheduler::processScheduledActions()
{
    // Early out so we don't spam TRACE_EVENTS with useless processScheduledActions.
    if (m_stateMachine.nextAction() == CCSchedulerStateMachine::ACTION_NONE)
        return;

    CCSchedulerStateMachine::Action action;
    do {
        action = m_stateMachine.nextAction();
        switch (action) {
        case CCSchedulerStateMachine::ACTION_NONE:
            return;
        case CCSchedulerStateMachine::ACTION_BEGIN_FRAME:
            m_client->scheduledActionBeginFrame();
            break;
        case CCSchedulerStateMachine::ACTION_BEGIN_UPDATE_MORE_RESOURCES:
            m_client->scheduledActionUpdateMoreResources();
            m_updateMoreResourcesPending = true;
            break;
        case CCSchedulerStateMachine::ACTION_COMMIT:
            m_client->scheduledActionCommit();
            break;
        case CCSchedulerStateMachine::ACTION_DRAW:
            m_client->scheduledActionDrawAndSwap();
            break;
        }
        m_stateMachine.updateState(action);

        // If we were just told to update resources, but there are no more
        // pending, then tell the state machine that the
        // beginUpdateMoreResources completed.  If more are pending, then we
        // will ack the update at the next draw.
        if (action == CCSchedulerStateMachine::ACTION_BEGIN_UPDATE_MORE_RESOURCES
            && !m_client->hasMoreResourceUpdates()) {
            m_updateMoreResourcesPending = false;
            m_stateMachine.beginUpdateMoreResourcesComplete(false);
        }

    } while (action != CCSchedulerStateMachine::ACTION_NONE);
}

}
