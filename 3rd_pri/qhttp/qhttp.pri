#
# came from ---> https://github.com/azadkuh/qhttp.git
#
#TOC
# QHttp is a lightweight, asynchronous and fast HTTP library in c++14 / Qt5,
# containing both server and client side classes for managing connections, parsing and building HTTP requests and responses.
# the objective of QHttp is being light weight with a simple API for Qt developers to implement RESTful web services
# in private (internal) zones.
# more by using std::function and c++14 generic lambda, the API is intentionally similar to the Node.js' http module.
# Asynchronous and non-blocking HTTP programming is quite easy with QHttp. have a look at sample codes.
# the fantastic nodejs/http-parser (which is a single pair of *.h/*.c files) is the only dependency of the QHttp.
# attention: c++14 is the minimum requirement for version 3.0+ please see releases

# This project was inspired by nikhilm/qhttpserver effort to implement a Qt HTTP server.
# QHttp pushes the idea further by implementing client side classes, better memory management, a lot more Node.js-like API, ...

DEFINES *= QHTTP_HAS_CLIENT
#用的时静态库
#win32:DEFINES *= QHTTP_STATIC_LIB_USE


INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/include



CONFIG(debug, debug|release) {

LIBS += -L$$PWD/xbin_debug  \
        -lqhttpd

} else {

LIBS += -L$$PWD/xbin_release  \
        -lqhttp

}


SOURCES += $$PWD/keep_alive_example.cpp \
    $$PWD/basic_server_example.cpp \
    $$PWD/hello_world_example.cpp \
    $$PWD/postcollector_example.cpp \
    $$PWD/qhttprpc_pkgdealer.cpp \
    $$PWD/qhttprpc_simpleserver.cpp

HEADERS += \
    $$PWD/basic_server_declare.h \
    $$PWD/hello_world_declare.h \
    $$PWD/keep_alive_declare.h \
    $$PWD/postcollector_declare.h \
    $$PWD/qhttprpc_pkgdealer.h \
    $$PWD/qhttprpc_simpleserver.h

FORMS +=

