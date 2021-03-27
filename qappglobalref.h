#ifndef QAPPGLOBALREF_H
#define QAPPGLOBALREF_H

#include <QDebug>
#include <QFileInfo>
#include <QThread>
#include <QProcess>
#include <QQueue>


#include "qlog_message_handler.h"
#include "qhttprpc_simpleserver.h"

namespace app_exit_code
{
    const int APP_SHUTDOWN_NORMALLY = 0x000001FF;
    const int APP_RESTART_CMD = 0x000002FF;
}


class QAppGlobalRef
{
public:
    // this is how clients can access the single instance
    static QAppGlobalRef* getInstance();     //单例(为了使其分配在堆上)

    void setAppDirPath(const QString& dir_path){ _appPath = dir_path; }
    QString getAppDirPath(){ return _appPath; }

private:
    QString _appPath;

public:
    int createtimes = 0;

private:
    static QAppGlobalRef * _inst;   //the one, single instance

    static QMutex m_mutex_pending;
    static QQueue<void *> _requestDestructPendingArray;

    QAppGlobalRef() { _requestDestructPendingArray.clear(); };             //private constructor
    QAppGlobalRef(const QAppGlobalRef&) = delete;
    QAppGlobalRef& operator=(const QAppGlobalRef &) = delete;
};


#endif // QAPPGLOBALREF_H
