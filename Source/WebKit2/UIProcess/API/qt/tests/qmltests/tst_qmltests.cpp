/*
    Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "../util.h"
#include <QtQuickTest/quicktest.h>
#include <QtWidgets/QApplication>

int main(int argc, char** argv)
{
    suppressDebugOutput();
    // Instantiate QApplication to prevent quick_test_main to instantiate a QGuiApplication.
    // This can be removed as soon as we do not use QtWidgets any more.
    QApplication app(argc, argv);
    return quick_test_main(argc, argv, "qmltests", 0, QUICK_TEST_SOURCE_DIR);
}
