#ifndef KEEP_ALIVE_DECLARE_H
#define KEEP_ALIVE_DECLARE_H

#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"
#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QUrlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

extern int keep_alive_main(int argc, char ** argv);

#endif // KEEP_ALIVE_DECLARE_H
