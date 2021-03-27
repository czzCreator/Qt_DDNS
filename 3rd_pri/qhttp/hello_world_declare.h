#ifndef HELLO_WORLD_DECLARE_H
#define HELLO_WORLD_DECLARE_H

#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"

//#include "../include/unixcatcher.hpp"

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QUrlQuery>

#include <QLocalServer>
#include <QTimer>
#include <QFile>

extern int hello_world_main(int argc, char ** argv);

#endif // HELLO_WORLD_DECLARE_H
