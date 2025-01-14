/*
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef qtouchwebpage_p_h
#define qtouchwebpage_p_h

#include "QtSGUpdateQueue.h"
#include "QtTouchWebPageProxy.h"
#include "qtouchwebpage.h"
#include "qwebnavigationcontroller.h"
#include <QMenu>

class QRectF;
class QSGNode;
class QString;
class QTouchWebPage;

class QTouchWebPagePrivate {
public:
    QTouchWebPagePrivate(QTouchWebPage* view);

    void setPageProxy(QtWebPageProxy*);

    void _q_onAfterSceneRender();
    void _q_onSceneGraphInitialized();
    void paintToCurrentGLContext();
    QtTouchWebPageProxy* touchPageProxy() { return static_cast<QtTouchWebPageProxy*>(pageProxy); }

    QTouchWebPage* const q;
    QtWebPageProxy* pageProxy;
    QWebNavigationController* navigationController;
    WebKit::QtSGUpdateQueue sgUpdateQueue;
    bool paintingIsInitialized;
};

#endif /* qtouchwebpage_p_h */
