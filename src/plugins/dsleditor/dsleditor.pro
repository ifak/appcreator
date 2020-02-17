DEFINES += DSLEDITOR_LIBRARY

PROVIDER = ifak

include(dsleditor_dependencies.pri)
include($$PWD/../../appcreatorplugin.pri)

win32-msvc*:QMAKE_CXXFLAGS += -wd4251 -wd4290 -wd4250

#CONFIG(release):DEFINES += QT_NO_DEBUG_OUTPUT

DISTFILES += \
    dsleditor.json \
    dsleditor_dependencies.pri \
    dsleditor.pluginspec.in

SOURCES += \
    dsleditorwidget.cpp \
    dslhighlighter.cpp \
    dslcodeformatter.cpp \
    dslautocompleter.cpp \
    dsleditorplugin.cpp \
    dslindenter.cpp \
    dsloutlinemodel.cpp \
    dslgraphiceditor.cpp \
    dslproposalitem.cpp

HEADERS += \
    dsleditor_global.hpp \
    dsleditorwidget.hpp \
    dslhighlighter.hpp \
    dslcodeformatter.hpp \
    dslautocompleter.hpp \
    dsleditorplugin.hpp \
    dslindenter.hpp \
    dsloutlinemodel.hpp \
    dslgraphiceditor.hpp \
    dslproposalitem.hpp
