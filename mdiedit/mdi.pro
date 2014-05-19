QT += widgets

HEADERS       = mainwindow.h \
                mdichild.h \
                finddialog.h \
                document.h
FORMS =  find.ui
SOURCES       = main.cpp \
                mainwindow.cpp \
                mdichild.cpp \
                finddialog.cpp \
                document.cpp


# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/mainwindows/mdi
INSTALLS += target
