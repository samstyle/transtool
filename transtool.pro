######################################################################
# Automatically generated by qmake (3.1) Sun Oct 1 10:59:39 2023
######################################################################

TEMPLATE = app
TARGET = transtool
INCLUDEPATH += .
QT += widgets

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += main.h \
           jrdict/classes.h \
           jrdict/mtableview.h \
           jrdict/vars.h \
           transtext/base.h \
           transtext/filetypes.h \
           transtext/mainwin.h \
           transtext/models.h \
           transtext/replace.h
FORMS += jrdict/jrdwidget.ui \
         jrdict/wordwin.ui \
         transtext/bookmark.ui \
         transtext/bookmarks.ui \
         transtext/iconwindow.ui \
         transtext/imgviewer.ui \
         transtext/replacedialog.ui \
         transtext/transwidget.ui
SOURCES += main.cpp \
           jrdict/formwin.cpp \
           jrdict/jdict.cpp \
           jrdict/jdictwin.cpp \
           jrdict/wordwin.cpp \
           transtext/bmlmodel.cpp \
           transtext/eagls.cpp \
           transtext/enm.cpp \
           transtext/ks.cpp \
           transtext/replace.cpp \
           transtext/snx.cpp \
           transtext/srp.cpp \
           transtext/tbmodel.cpp \
           transtext/transtext.cpp \
           transtext/transwin.cpp \
           transtext/trb.cpp \
           transtext/xplayer.cpp
RESOURCES += jrdict/dict4.qrc transtext/resources.qrc
