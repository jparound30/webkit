/*
 * Copyright (C) 2001, 2002 Apple Computer, Inc.  All rights reserved.
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


#import <qregexp.h>

#import <sys/types.h>
#import <regex.h>


class QRegExp::KWQRegExpPrivate
{
public:
    KWQRegExpPrivate();
    KWQRegExpPrivate(QString pattern, bool caseSensitive, bool glob);
    ~KWQRegExpPrivate();

    void compile(bool caseSensitive, bool glob);

    QString pattern;
    regex_t regex;

    uint refCount;
};

QRegExp::KWQRegExpPrivate::KWQRegExpPrivate() : pattern(""), refCount(0)
{
    compile(true, false);
}

QRegExp::KWQRegExpPrivate::KWQRegExpPrivate(QString p, bool caseSensitive, bool glob) : pattern(p), refCount(0)
{
    compile(caseSensitive, glob);
}

static QString RegExpFromGlob(QString glob)
{
    QString result = glob;

    // escape regexp metacharacters which are NOT glob metacharacters

    result.replace(QRegExp("\\\\"), "\\\\");
    result.replace(QRegExp("\\."), "\\.");
    result.replace(QRegExp("\\+"), "\\+");
    result.replace(QRegExp("\\$"), "\\$");
    // FIXME: incorrect for ^ inside bracket group
    result.replace(QRegExp("\\^"), "\\^");

    // translate glob metacharacters into regexp metacharacters
    result.replace(QRegExp("\\*"), ".*");
    result.replace(QRegExp("\\?"), ".");
   
    // Require the glob to match the whole string
    result = "^" + result + "$";

    return result;
}

void QRegExp::KWQRegExpPrivate::compile(bool caseSensitive, bool glob)
{
    QString p;

    if (glob) {
	p = RegExpFromGlob(pattern);
    } else {
	p = pattern;
    }

    const char *cpattern = p.latin1();

    regcomp(&regex, cpattern, REG_EXTENDED | (caseSensitive ? 0 : REG_ICASE));
}

QRegExp::KWQRegExpPrivate::~KWQRegExpPrivate()
{
    regfree(&regex);
}


QRegExp::QRegExp() : d(new QRegExp::KWQRegExpPrivate())
{
}

QRegExp::QRegExp(const QString &pattern, bool caseSensitive, bool glob) : d(new QRegExp::KWQRegExpPrivate(pattern, caseSensitive, glob))
{
}

QRegExp::QRegExp(const char *cpattern) : d(new QRegExp::KWQRegExpPrivate(cpattern, true, false))
{
}


QRegExp::QRegExp(const QRegExp &re) : d (re.d)
{
}

QRegExp::~QRegExp()
{
}

QRegExp &QRegExp::operator=(const QRegExp &re)
{
    QRegExp tmp(re);
    KWQRefPtr<QRegExp::KWQRegExpPrivate> tmpD = tmp.d;
    
    tmp.d = d;
    d = tmpD;

    return *this;
}

QString QRegExp::pattern() const
{
    return d->pattern;
}

int QRegExp::match(const QString &str, int startFrom, int *matchLength, bool treatStartAsStartOfInput) const
{
    const char *cstring = str.latin1() + startFrom;

    int flags = 0;

    if (startFrom != 0 && !treatStartAsStartOfInput) {
	flags |= REG_NOTBOL;
    }

    regmatch_t match[1];
    int result = regexec(&d->regex, cstring, 1, match, flags);

    if (result != 0) {
	return -1;
    } else {
	if (matchLength != NULL) {
	    *matchLength = match[0].rm_eo - match[0].rm_so;
	}
	return startFrom + match[0].rm_so;
    }
}
