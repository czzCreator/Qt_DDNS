#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QFile>
#include <QMutex>
#include <QtGlobal>


//注意:外部在调用这个类的时候 可以配合信号把 信息 messmage 通知出去

#define DEFAULT_LOG_RESERVE_DAY 20          //默认存储20天的日志
#define DEFAULT_QtDDNS_LOG_DIR "./log"


//似乎不关心具体的日志名称 用宏定义不同日志类别
#define APP_LOG_PREFIX  "QtDDNS_"

#define DEBUG_LOG_DIR_MACRO     "/debug"
#define INFO_LOG_DIR_MACRO     "/info"
#define WARNING_LOG_DIR_MACRO     "/warning"
#define CRITICAL_LOG_DIR_MACRO     "/critical"
#define FATAL_LOG_DIR_MACRO     "/fatal"

#define DEBUG_LOG_NAME  APP_LOG_PREFIX"debug.log"
#define INFO_LOG_NAME  APP_LOG_PREFIX"info.log"
#define WARNING_LOG_NAME  APP_LOG_PREFIX"warning.log"
#define CRITICAL_LOG_NAME  APP_LOG_PREFIX"critical.log"
#define FATAL_LOG_NAME  APP_LOG_PREFIX"fatal.log"



namespace message_handler {

class  QLog_MessageHandler: public QFile
{
private:
    QLog_MessageHandler();
    ~QLog_MessageHandler();

public:
    static void setLogReserve_day(int reserve_days = DEFAULT_LOG_RESERVE_DAY);
    static void setLogDir(QString fileDir = DEFAULT_QtDDNS_LOG_DIR);

    /*
     * Installs a Qt message handler which has been defined previously.
     * This method is deprecated, use qInstallMessageHandler instead.
     * 不同版本 要求实现的回调函数 参数不同，高版本比较好
    */
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    static void FormatMessage_myown(QtMsgType type, const QMessageLogContext &context, const QString &msg);
#else

    //显式调用这种 日志会带上 函数 行数 源码文件信息
    static void FormatMessage_myown(QtMsgType type,
                                                                   QString context_file,int  context_line,QString context_func,
                                                                   const QString &msg);

    //利用qDebug  qCritical  qWarning ...  隐式调用这种 没有  函数 行数 源码文件信息
    //但是会 将Qt 内部日志也 打印到 日志里
    static void FormatMessage_myown(QtMsgType type,const char *msg);
#endif

private:
    static QString m_logDir;
    static int m_nDaysOfLog; //日志保留天数

    static QMutex m_mutex;
};

}


#endif // MESSAGEHANDLER_H
