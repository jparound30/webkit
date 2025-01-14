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

#include "cc/CCThreadProxy.h"

#include "GraphicsContext3D.h"
#include "TraceEvent.h"
#include "cc/CCDelayBasedTimeSource.h"
#include "cc/CCFrameRateController.h"
#include "cc/CCInputHandler.h"
#include "cc/CCLayerTreeHost.h"
#include "cc/CCMainThreadTask.h"
#include "cc/CCScheduler.h"
#include "cc/CCScopedMainThreadProxy.h"
#include "cc/CCScrollController.h"
#include "cc/CCTextureUpdater.h"
#include "cc/CCThreadTask.h"
#include <wtf/CurrentTime.h>
#include <wtf/MainThread.h>

using namespace WTF;

namespace WebCore {

CCThread* CCThreadProxy::s_ccThread = 0;

void CCThreadProxy::setThread(CCThread* ccThread)
{
    s_ccThread = ccThread;
#ifndef NDEBUG
    CCProxy::setImplThread(s_ccThread->threadID());
#endif
}

PassOwnPtr<CCProxy> CCThreadProxy::create(CCLayerTreeHost* layerTreeHost)
{
    return adoptPtr(new CCThreadProxy(layerTreeHost));
}

CCThreadProxy::CCThreadProxy(CCLayerTreeHost* layerTreeHost)
    : m_commitRequested(false)
    , m_layerTreeHost(layerTreeHost)
    , m_compositorIdentifier(-1)
    , m_started(false)
    , m_lastExecutedBeginFrameAndCommitSequenceNumber(-1)
    , m_numBeginFrameAndCommitsIssuedOnImplThread(0)
    , m_mainThreadProxy(CCScopedMainThreadProxy::create())
    , m_readbackRequestOnImplThread(0)
    , m_finishAllRenderingCompletionEventOnImplThread(0)
    , m_commitCompletionEventOnImplThread(0)
    , m_nextFrameIsNewlyCommittedFrameOnImplThread(false)
{
    TRACE_EVENT("CCThreadProxy::CCThreadProxy", this, 0);
    ASSERT(isMainThread());
}

CCThreadProxy::~CCThreadProxy()
{
    TRACE_EVENT("CCThreadProxy::~CCThreadProxy", this, 0);
    ASSERT(isMainThread());
    ASSERT(!m_started);
}

bool CCThreadProxy::compositeAndReadback(void *pixels, const IntRect& rect)
{
    TRACE_EVENT("CCThreadPRoxy::compositeAndReadback", this, 0);
    ASSERT(isMainThread());
    ASSERT(m_layerTreeHost);

    // If a commit is pending, perform the commit first.
    if (m_commitRequested)  {
        // This bit of code is uglier than it should be because returning
        // pointers via the CCThread task model is really messy. Effectively, we
        // are making a blocking call to createBeginFrameAndCommitTaskOnImplThread,
        // and trying to get the CCMainThread::Task it returns so we can run it.
        OwnPtr<CCMainThread::Task> beginFrameAndCommitTask;
        {
            CCMainThread::Task* taskPtr = 0;
            CCCompletionEvent completion;
            s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::obtainBeginFrameAndCommitTaskFromCCThread, AllowCrossThreadAccess(&completion), AllowCrossThreadAccess(&taskPtr)));
            completion.wait();
            beginFrameAndCommitTask = adoptPtr(taskPtr);
        }

        beginFrameAndCommitTask->performTask();
    }

    // Draw using the new tree and read back the results.
    ReadbackRequest request;
    request.rect = rect;
    request.pixels = pixels;
    s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::requestReadbackOnImplThread, AllowCrossThreadAccess(&request)));
    request.completion.wait();
    return request.success;
}

void CCThreadProxy::requestReadbackOnImplThread(ReadbackRequest* request)
{
    ASSERT(CCProxy::isImplThread());
    ASSERT(!m_readbackRequestOnImplThread);
    if (!m_layerTreeHostImpl) {
        request->success = false;
        request->completion.signal();
        return;
    }
    m_readbackRequestOnImplThread = request;
    m_schedulerOnImplThread->setNeedsRedraw();
}

GraphicsContext3D* CCThreadProxy::context()
{
    return 0;
}

void CCThreadProxy::finishAllRendering()
{
    ASSERT(CCProxy::isMainThread());

    // Make sure all GL drawing is finished on the impl thread.
    CCCompletionEvent completion;
    s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::finishAllRenderingOnImplThread, AllowCrossThreadAccess(&completion)));
    completion.wait();
}

bool CCThreadProxy::isStarted() const
{
    ASSERT(CCProxy::isMainThread());
    return m_started;
}

bool CCThreadProxy::initializeLayerRenderer()
{
    TRACE_EVENT("CCThreadProxy::initializeLayerRenderer", this, 0);
    RefPtr<GraphicsContext3D> context = m_layerTreeHost->createLayerTreeHostContext3D();
    if (!context)
        return false;
    ASSERT(context->hasOneRef());

    // Leak the context pointer so we can transfer ownership of it to the other side...
    GraphicsContext3D* contextPtr = context.release().leakRef();
    ASSERT(contextPtr->hasOneRef());

    // Make a blocking call to initializeLayerRendererOnImplThread. The results of that call
    // are pushed into the initializeSucceeded and capabilities local variables.
    CCCompletionEvent completion;
    bool initializeSucceeded = false;
    LayerRendererCapabilities capabilities;
    s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::initializeLayerRendererOnImplThread,
                                          AllowCrossThreadAccess(contextPtr), AllowCrossThreadAccess(&completion),
                                          AllowCrossThreadAccess(&initializeSucceeded), AllowCrossThreadAccess(&capabilities),
                                          AllowCrossThreadAccess(&m_compositorIdentifier)));
    completion.wait();

    if (initializeSucceeded)
        m_layerRendererCapabilitiesMainThreadCopy = capabilities;
    return initializeSucceeded;
}

int CCThreadProxy::compositorIdentifier() const
{
    ASSERT(isMainThread());
    return m_compositorIdentifier;
}

const LayerRendererCapabilities& CCThreadProxy::layerRendererCapabilities() const
{
    return m_layerRendererCapabilitiesMainThreadCopy;
}

void CCThreadProxy::loseCompositorContext(int numTimes)
{
    ASSERT_NOT_REACHED();
}

void CCThreadProxy::setNeedsAnimate()
{
    ASSERT(isMainThread());
    if (m_commitRequested)
        return;

    TRACE_EVENT("CCThreadProxy::setNeedsAnimation", this, 0);
    m_commitRequested = true;
    s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::setNeedsAnimateOnImplThread));
}

void CCThreadProxy::setNeedsCommit()
{
    ASSERT(isMainThread());
    if (m_commitRequested)
        return;

    TRACE_EVENT("CCThreadProxy::setNeedsCommit", this, 0);
    m_commitRequested = true;
    s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::setNeedsCommitOnImplThread));
}

void CCThreadProxy::setNeedsAnimateOnImplThread()
{
    ASSERT(isImplThread());
    TRACE_EVENT("CCThreadProxy::setNeedsCommitOnImplThread", this, 0);
    m_schedulerOnImplThread->setNeedsAnimate();
}

void CCThreadProxy::onSwapBuffersCompleteOnImplThread()
{
    ASSERT(isImplThread());
    TRACE_EVENT("CCThreadProxy::onSwapBuffersCompleteOnImplThread", this, 0);
    m_schedulerOnImplThread->didSwapBuffersComplete();
}

void CCThreadProxy::setNeedsCommitOnImplThread()
{
    ASSERT(isImplThread());
    TRACE_EVENT("CCThreadProxy::setNeedsCommitOnImplThread", this, 0);
    m_schedulerOnImplThread->setNeedsCommit();
}

void CCThreadProxy::setNeedsRedraw()
{
    ASSERT(isMainThread());
    TRACE_EVENT("CCThreadProxy::setNeedsRedraw", this, 0);
    s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::setNeedsRedrawOnImplThread));
}

void CCThreadProxy::setVisible(bool visible)
{
    ASSERT(isMainThread());
    if (!visible) {
        CCCompletionEvent completion;
        s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::didBecomeInvisibleOnImplThread, AllowCrossThreadAccess(&completion)));
        return;
    }
    setNeedsRedraw();
}

void CCThreadProxy::didBecomeInvisibleOnImplThread(CCCompletionEvent* completion)
{
    ASSERT(isImplThread());
    m_layerTreeHost->didBecomeInvisibleOnImplThread(m_layerTreeHostImpl.get());
    m_schedulerOnImplThread->setVisible(false);
    m_layerTreeHostImpl->setVisible(false);
    completion->signal();
}

void CCThreadProxy::setNeedsRedrawOnImplThread()
{
    ASSERT(isImplThread());
    TRACE_EVENT("CCThreadProxy::setNeedsRedrawOnImplThread", this, 0);
    m_schedulerOnImplThread->setNeedsRedraw();
}

void CCThreadProxy::start()
{
    ASSERT(isMainThread());
    ASSERT(s_ccThread);
    // Create LayerTreeHostImpl.
    CCCompletionEvent completion;
    s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::initializeImplOnImplThread, AllowCrossThreadAccess(&completion)));
    completion.wait();

    m_started = true;
}

void CCThreadProxy::stop()
{
    TRACE_EVENT("CCThreadProxy::stop", this, 0);
    ASSERT(isMainThread());
    ASSERT(m_started);

    // Synchronously deletes the impl.
    CCCompletionEvent completion;
    s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::layerTreeHostClosedOnImplThread, AllowCrossThreadAccess(&completion)));
    completion.wait();

    m_mainThreadProxy->shutdown(); // Stop running tasks posted to us.

    ASSERT(!m_layerTreeHostImpl); // verify that the impl deleted.
    m_layerTreeHost = 0;
    m_started = false;
}

void CCThreadProxy::finishAllRenderingOnImplThread(CCCompletionEvent* completion)
{
    TRACE_EVENT("CCThreadProxy::finishAllRenderingOnImplThread", this, 0);
    ASSERT(isImplThread());
    ASSERT(!m_finishAllRenderingCompletionEventOnImplThread);
    m_schedulerOnImplThread->setNeedsRedraw();
    m_finishAllRenderingCompletionEventOnImplThread = completion;
}

void CCThreadProxy::scheduledActionBeginFrame()
{
    TRACE_EVENT("CCThreadProxy::scheduledActionBeginFrame", this, 0);
    m_mainThreadProxy->postTask(createBeginFrameAndCommitTaskOnImplThread());
}

void CCThreadProxy::obtainBeginFrameAndCommitTaskFromCCThread(CCCompletionEvent* completion, CCMainThread::Task** taskPtr)
{
    OwnPtr<CCMainThread::Task> task = createBeginFrameAndCommitTaskOnImplThread();
    *taskPtr = task.leakPtr();
    completion->signal();
}

PassOwnPtr<CCMainThread::Task> CCThreadProxy::createBeginFrameAndCommitTaskOnImplThread()
{
    TRACE_EVENT("CCThreadProxy::createBeginFrameAndCommitTaskOnImplThread", this, 0);
    ASSERT(isImplThread());
    double frameBeginTime = currentTime();

    // NOTE, it is possible to receieve a request for a
    // beginFrameAndCommitOnImplThread from finishAllRendering while a
    // beginFrameAndCommitOnImplThread is enqueued. Since CCMainThread doesn't
    // provide a threadsafe way to cancel tasks, it is important that
    // beginFrameAndCommit be structured to understand that it may get called at
    // a point that it shouldn't. We do this by assigning a sequence number to
    // every new beginFrameAndCommit task. Then, beginFrameAndCommit tracks the
    // last executed sequence number, dropping beginFrameAndCommit with sequence
    // numbers below the last executed one.
    int thisTaskSequenceNumber = m_numBeginFrameAndCommitsIssuedOnImplThread;
    m_numBeginFrameAndCommitsIssuedOnImplThread++;
    OwnPtr<CCScrollUpdateSet> scrollInfo = m_layerTreeHostImpl->processScrollDeltas();
    return createMainThreadTask(this, &CCThreadProxy::beginFrameAndCommit, thisTaskSequenceNumber, frameBeginTime, scrollInfo.release());
}

void CCThreadProxy::beginFrameAndCommit(int sequenceNumber, double frameBeginTime, PassOwnPtr<CCScrollUpdateSet> scrollInfo)
{
    TRACE_EVENT("CCThreadProxy::beginFrameAndCommit", this, 0);
    ASSERT(isMainThread());
    if (!m_layerTreeHost)
        return;

    // Scroll deltas need to be applied even if the commit will be dropped.
    m_layerTreeHost->applyScrollDeltas(*scrollInfo.get());

    // Drop beginFrameAndCommit calls that occur out of sequence. See createBeginFrameAndCommitTaskOnImplThread for
    // an explanation of how out-of-sequence beginFrameAndCommit tasks can occur.
    if (sequenceNumber < m_lastExecutedBeginFrameAndCommitSequenceNumber) {
        TRACE_EVENT("EarlyOut_StaleBeginFrameAndCommit", this, 0);
        return;
    }
    m_lastExecutedBeginFrameAndCommitSequenceNumber = sequenceNumber;

    // FIXME: recreate the context if it was requested by the impl thread
    {
        TRACE_EVENT("CCLayerTreeHost::animateAndLayout", this, 0);
        m_layerTreeHost->animateAndLayout(frameBeginTime);
    }

    ASSERT(m_lastExecutedBeginFrameAndCommitSequenceNumber == sequenceNumber);

    // Clear the commit flag after animateAndLayout here --- objects that only
    // layout when painted will trigger another setNeedsCommit inside
    // updateLayers.
    m_commitRequested = false;

    m_layerTreeHost->updateLayers();

    {
        // Notify the impl thread that the beginFrame has completed. This will
        // begin the commit process, which is blocking from the main thread's
        // point of view, but asynchronously performed on the impl thread,
        // coordinated by the CCScheduler.
        TRACE_EVENT("commit", this, 0);
        CCCompletionEvent completion;
        s_ccThread->postTask(createCCThreadTask(this, &CCThreadProxy::beginFrameCompleteOnImplThread, AllowCrossThreadAccess(&completion)));
        completion.wait();
    }

    m_layerTreeHost->commitComplete();

    ASSERT(m_lastExecutedBeginFrameAndCommitSequenceNumber == sequenceNumber);
}

void CCThreadProxy::beginFrameCompleteOnImplThread(CCCompletionEvent* completion)
{
    TRACE_EVENT("CCThreadProxy::beginFrameCompleteOnImplThread", this, 0);
    ASSERT(!m_commitCompletionEventOnImplThread);
    ASSERT(isImplThread());
    ASSERT(m_schedulerOnImplThread);
    ASSERT(m_schedulerOnImplThread->commitPending());
    if (!m_layerTreeHostImpl) {
        completion->signal();
        return;
    }

    m_commitCompletionEventOnImplThread = completion;

    ASSERT(!m_currentTextureUpdaterOnImplThread);
    m_currentTextureUpdaterOnImplThread = adoptPtr(new CCTextureUpdater(m_layerTreeHostImpl->contentsTextureAllocator()));
    m_layerTreeHost->updateCompositorResources(m_layerTreeHostImpl->context(), *m_currentTextureUpdaterOnImplThread);

    m_schedulerOnImplThread->beginFrameComplete();
}

bool CCThreadProxy::hasMoreResourceUpdates() const
{
    if (!m_currentTextureUpdaterOnImplThread)
        return false;
    return m_currentTextureUpdaterOnImplThread->hasMoreUpdates();
}

void CCThreadProxy::scheduledActionUpdateMoreResources()
{
    TRACE_EVENT("CCThreadProxy::scheduledActionUpdateMoreResources", this, 0);
    ASSERT(m_currentTextureUpdaterOnImplThread);
    static const int UpdatesPerFrame = 16;
    m_currentTextureUpdaterOnImplThread->update(m_layerTreeHostImpl->context(), UpdatesPerFrame);
}

void CCThreadProxy::scheduledActionCommit()
{
    TRACE_EVENT("CCThreadProxy::scheduledActionCommit", this, 0);
    ASSERT(m_currentTextureUpdaterOnImplThread);
    ASSERT(!m_currentTextureUpdaterOnImplThread->hasMoreUpdates());
    ASSERT(m_commitCompletionEventOnImplThread);

    m_currentTextureUpdaterOnImplThread.clear();


    m_layerTreeHostImpl->beginCommit();

    m_layerTreeHost->beginCommitOnImplThread(m_layerTreeHostImpl.get());
    CCTextureUpdater updater(m_layerTreeHostImpl->contentsTextureAllocator());
    m_layerTreeHostImpl->setVisible(m_layerTreeHost->visible());
    m_schedulerOnImplThread->setVisible(m_layerTreeHostImpl->visible());
    m_layerTreeHost->finishCommitOnImplThread(m_layerTreeHostImpl.get());

    m_layerTreeHostImpl->commitComplete();

    m_nextFrameIsNewlyCommittedFrameOnImplThread = true;

    m_commitCompletionEventOnImplThread->signal();
    m_commitCompletionEventOnImplThread = 0;
}

void CCThreadProxy::drawLayersAndSwapOnImplThread()
{
    TRACE_EVENT("CCThreadProxy::drawLayersAndSwapOnImplThread", this, 0);
    ASSERT(isImplThread());
    if (!m_layerTreeHostImpl)
        return;

    m_layerTreeHostImpl->drawLayers();

    // Check for a pending compositeAndReadback.
    if (m_readbackRequestOnImplThread) {
      m_layerTreeHostImpl->readback(m_readbackRequestOnImplThread->pixels, m_readbackRequestOnImplThread->rect);
      m_readbackRequestOnImplThread->success = !m_layerTreeHostImpl->isContextLost();
      m_readbackRequestOnImplThread->completion.signal();
      m_readbackRequestOnImplThread = 0;
    }

    m_layerTreeHostImpl->swapBuffers();

    // FIXME: handle case where m_layerTreeHostImpl->isContextLost.
    // FIXME: pass didSwapBuffersAbort if m_layerTreeHostImpl->isContextLost.
    ASSERT(!m_layerTreeHostImpl->isContextLost());

    // Process any finish request
    if (m_finishAllRenderingCompletionEventOnImplThread) {
        m_layerTreeHostImpl->finishAllRendering();
        m_finishAllRenderingCompletionEventOnImplThread->signal();
        m_finishAllRenderingCompletionEventOnImplThread = 0;
    }

    // Tell the main thread that the the newly-commited frame was drawn.
    if (m_nextFrameIsNewlyCommittedFrameOnImplThread) {
        m_nextFrameIsNewlyCommittedFrameOnImplThread = false;
        m_mainThreadProxy->postTask(createMainThreadTask(this, &CCThreadProxy::didCommitAndDrawFrame, m_layerTreeHostImpl->sourceFrameNumber()));
    }
}

void CCThreadProxy::didCommitAndDrawFrame(int frameNumber)
{
    ASSERT(isMainThread());
    if (!m_layerTreeHost)
        return;
    m_layerTreeHost->didCommitAndDrawFrame(frameNumber);
}

void CCThreadProxy::initializeImplOnImplThread(CCCompletionEvent* completion)
{
    TRACE_EVENT("CCThreadProxy::initializeImplOnImplThread", this, 0);
    ASSERT(isImplThread());
    m_layerTreeHostImpl = m_layerTreeHost->createLayerTreeHostImpl(this);
    const double displayRefreshIntervalMs = 1000.0 / 60.0;
    OwnPtr<CCFrameRateController> frameRateController = adoptPtr(new CCFrameRateController(CCDelayBasedTimeSource::create(displayRefreshIntervalMs, s_ccThread)));
    m_schedulerOnImplThread = CCScheduler::create(this, frameRateController.release());
    m_schedulerOnImplThread->setVisible(m_layerTreeHostImpl->visible());
    completion->signal();
}

void CCThreadProxy::initializeLayerRendererOnImplThread(GraphicsContext3D* contextPtr, CCCompletionEvent* completion, bool* initializeSucceeded, LayerRendererCapabilities* capabilities, int* compositorIdentifier)
{
    TRACE_EVENT("CCThreadProxy::initializeLayerRendererOnImplThread", this, 0);
    ASSERT(isImplThread());
    RefPtr<GraphicsContext3D> context(adoptRef(contextPtr));
    *initializeSucceeded = m_layerTreeHostImpl->initializeLayerRenderer(context);
    if (*initializeSucceeded)
        *capabilities = m_layerTreeHostImpl->layerRendererCapabilities();

    m_inputHandlerOnImplThread = CCInputHandler::create(m_layerTreeHostImpl.get());
    *compositorIdentifier = m_inputHandlerOnImplThread->identifier();

    completion->signal();
}

void CCThreadProxy::layerTreeHostClosedOnImplThread(CCCompletionEvent* completion)
{
    TRACE_EVENT("CCThreadProxy::layerTreeHostClosedOnImplThread", this, 0);
    ASSERT(isImplThread());
    m_layerTreeHost->deleteContentsTexturesOnImplThread(m_layerTreeHostImpl->contentsTextureAllocator());
    m_layerTreeHostImpl.clear();
    m_inputHandlerOnImplThread.clear();
    m_schedulerOnImplThread.clear();
    completion->signal();
}

void CCThreadProxy::scheduledActionDrawAndSwap()
{
    TRACE_EVENT("CCThreadProxy::scheduledActionDrawAndSwap", this, 0);
    drawLayersAndSwapOnImplThread();
}

} // namespace WebCore
