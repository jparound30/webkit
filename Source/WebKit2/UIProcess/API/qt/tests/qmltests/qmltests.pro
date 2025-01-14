include(../tests.pri)

CONFIG += warn_on testcase

QT -= testlib
QT += qmltest

# QML files tested are the ones in WebKit source repository.
DEFINES += QUICK_TEST_SOURCE_DIR=\"\\\"$$PWD\\\"\"

OTHER_FILES += \
    DesktopWebView/tst_properties.qml \
    DesktopWebView/tst_navigationPolicyForUrl.qml \
    DesktopWebView/tst_loadZeroSizeView.qml \
    DesktopWebView/tst_loadProgress.qml \
    DesktopWebView/tst_loadProgressSignal.qml \
    DesktopWebView/tst_linkHovered.qml \
    DesktopWebView/tst_messaging.qml \
    TouchWebView/tst_properties.qml \
    TouchWebView/tst_load.qml \
    TouchWebView/tst_loadZeroSizeView.qml \
    TouchWebView/tst_loadNegativeSizeView.qml \
    TouchWebView/tst_loadProgress.qml \
    TouchWebView/tst_loadProgressSignal.qml


