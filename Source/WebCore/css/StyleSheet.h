/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2006, 2008 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef StyleSheet_h
#define StyleSheet_h

#include "KURLHash.h"
#include "PlatformString.h"
#include <wtf/ListHashSet.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class CSSRule;
class CachedCSSStyleSheet;
class MediaList;
class Node;

class StyleSheet : public RefCounted<StyleSheet> {
public:
    virtual ~StyleSheet();

    bool disabled() const { return m_disabled; }
    void setDisabled(bool disabled) { m_disabled = disabled; styleSheetChanged(); }

    Node* ownerNode() const { return m_parentNode; }
    void clearOwnerNode() { m_parentNode = 0; }

    StyleSheet* parentStyleSheet() const;

    CSSRule* parentRule() const { return m_parentRule; }
    void setParentRule(CSSRule* rule) { m_parentRule = rule; }

    // Note that href is the URL that started the redirect chain that led to
    // this style sheet. This property probably isn't useful for much except
    // the JavaScript binding (which needs to use this value for security).
    const String& href() const { return m_originalURL; }

    void setFinalURL(const KURL& finalURL) { m_finalURL = finalURL; }
    const KURL& finalURL() const { return m_finalURL; }

    const String& title() const { return m_strTitle; }
    void setTitle(const String& s) { m_strTitle = s; }
    MediaList* media() const { return m_media.get(); }
    void setMedia(PassRefPtr<MediaList>);

    virtual String type() const = 0;
    virtual bool isLoading() = 0;
    virtual void styleSheetChanged() { }

    virtual KURL completeURL(const String& url) const;
    virtual bool parseString(const String&, bool strict = true) = 0;

    virtual bool isCSSStyleSheet() const { return false; }
    virtual bool isXSLStyleSheet() const { return false; }

    KURL baseURL() const;

protected:
    StyleSheet(Node* ownerNode, const String& href, const KURL& finalURL);
    StyleSheet(CSSRule* parentRule, const String& href, const KURL& finalURL);

private:
    CSSRule* m_parentRule;
    Node* m_parentNode;
    String m_originalURL;
    KURL m_finalURL;
    String m_strTitle;
    RefPtr<MediaList> m_media;
    bool m_disabled;
};

} // namespace

#endif
