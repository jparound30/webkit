/*
 * Copyright (C) 2002 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include <browserextension.h>

class QWidget;
class KHTMLPart;

class KHTMLPartBrowserExtension : public KParts::BrowserExtension {
public:
    KHTMLPartBrowserExtension(KHTMLPart *);
    void editableWidgetFocused(QWidget *) { }
    void editableWidgetBlurred(QWidget *) { }
    void setLocationBarURL(const QString &) { }
    
    virtual void openURLRequest(const KURL &, 
				const KParts::URLArgs &args = KParts::URLArgs());
     
    virtual void createNewWindow(const KURL &url, 
				 const KParts::URLArgs &urlArgs = KParts::URLArgs());
    virtual void createNewWindow(const KURL &url,
				 const KParts::URLArgs &urlArgs, 
				 const KParts::WindowArgs &winArgs, 
				 KParts::ReadOnlyPart *&part);
    
private:
     void createNewWindow(const KURL &url, 
			  const KParts::URLArgs &urlArgs, 
			  const KParts::WindowArgs &winArgs, 
			  KParts::ReadOnlyPart **part);

     KHTMLPart *m_part;
};

class KHTMLPartBrowserHostExtension {
public:
    KHTMLPartBrowserHostExtension(KHTMLPart *) { }
};

class KHTMLZoomFactorAction { };
