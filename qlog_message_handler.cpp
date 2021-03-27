#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QFile>
#include <QMutex>
#include <QTextCodec>


#include "qlog_message_handler.h"

using  namespace  message_handler;

QString QLog_MessageHandler::m_logDir = DEFAULT_QtDDNS_LOG_DIR;
int QLog_MessageHandler::m_nDaysOfLog = DEFAULT_LOG_RESERVE_DAY;
QMutex QLog_MessageHandler::m_mutex;

QLog_MessageHandler::QLog_MessageHandler()
{
}



QLog_MessageHandler::~QLog_MessageHandler()
{

}

void QLog_MessageHandler::setLogDir(QString fileDir)
{
    m_mutex.lock();
    m_logDir = fileDir;
    m_mutex.unlock();
}


void QLog_MessageHandler::setLogReserve_day(int reserve_days)
{
    m_mutex.lock();
    m_nDaysOfLog = reserve_days;
    m_mutex.unlock();
}



#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
void QLog_MessageHandler::FormatMessage_myown(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    m_mutex.lock();


    QString type_name_str;
    QString strDir;
    QString no_prefix_log_type_name;
    switch (type)
    {
    case QtDebugMsg:
        type_name_str = "DEBUG";
        strDir = m_logDir + DEBUG_LOG_DIR_MACRO;
        no_prefix_log_type_name = DEBUG_LOG_NAME;
        break;
    case QtWarningMsg:
        type_name_str = "WARNING";
        strDir = m_logDir + WARNING_LOG_DIR_MACRO;
        no_prefix_log_type_name = WARNING_LOG_NAME;
        break;
    case QtCriticalMsg:
        type_name_str = "CRITICAL";
        strDir = m_logDir + CRITICAL_LOG_DIR_MACRO;
        no_prefix_log_type_name = CRITICAL_LOG_NAME;
        break;
    case QtInfoMsg:
        type_name_str = "INFO";
        strDir = m_logDir + INFO_LOG_DIR_MACRO;
        no_prefix_log_type_name = INFO_LOG_NAME;
        break;
    default:
        type_name_str = "UNKNOWN";
        strDir = m_logDir + "/unknown";    //未知类型日志，默认放到程序工作目录
        no_prefix_log_type_name = APP_LOG_PREFIX + QString("unknown.log");
        break;
    }

    QDir _dir;

    if(!_dir.exists(strDir))
        _dir.mkpath(strDir);


    //保留 m_nDaysOfLog 天的日志
    QDir fileDir(strDir);
    QStringList fileList = fileDir.entryList(QDir::Files, QDir::Name);


    //循环删除超出天数的文件
    for (int j = 0; j < fileList.size() - m_nDaysOfLog; j++)
    {
        QFile file(fileDir.absolutePath() + "/" + fileList.at(j));
        bool b = file.remove();
        if (!b)
        {
            qCritical() << file.fileName() << ":" << file.errorString();
        }
    }

    //日志内容控制
    QFile file_detail(QString("%1/%2_%3").arg(strDir).arg(QDate::currentDate().toString("yyyy_MM_dd")).arg(no_prefix_log_type_name));
    file_detail.open(QFile::WriteOnly | QFile::Append);

    //防止单个文件过大 >10M 的 情况 ，过大则 resize 一般不可能
    if(file_detail.size()>10*1024*1024)
    {
        file_detail.resize(0);
    }

    QTextStream out(&file_detail);
    out.setCodec(QTextCodec::codecForName("utf-8"));
    QString strTime = QString("[%1][%2]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz")).arg(type_name_str);
    QString strLine = QString("[%1:%2]").arg(context.file).arg(context.line);

    QString strMsg = QString("%1%2 %3\n").arg(strTime).arg(strLine).arg(msg);
    printf("%s", strMsg.toLocal8Bit().data());
    fflush(stdout);

    out << strTime << strLine;
#ifdef Q_OS_WIN
    out << "\r\n";
#else
    out << "\n";
#endif

    out << "\t" << msg;


#ifdef Q_OS_WIN
    out << "\r\n";
#else
    out << "\n";
#endif
    out << "------------------------------------------------------------------";

#ifdef Q_OS_WIN
    out << "\r\n";
#else
    out << "\n";
#endif

    file_detail.flush();
    file_detail.close();

    m_mutex.unlock();
}


//void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
//{
//    QByteArray localMsg = msg.toLocal8Bit();
//    switch (type) {
//    case QtDebugMsg:
//        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtInfoMsg:
//        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtWarningMsg:
//        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtCriticalMsg:
//        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        break;
//    case QtFatalMsg:
//        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//        abort();
//    }
//}

#else

void QLog_MessageHandler::FormatMessage_myown(QtMsgType type,
                                                                                                QString context_file,int  context_line,QString context_func,
                                                                                                const QString &msg)
{
    m_mutex.lock();

    //在开始记录之前都获取一下要保留的日志天数
    m_nDaysOfLog = g_Log_reserve_day;


    QString type_name_str;
    QString strDir;
    QString no_prefix_log_type_name;
    switch (type)
    {
    case QtDebugMsg:
        type_name_str = "DEBUG";
        strDir = m_logDir + DEBUG_LOG_DIR_MACRO;
        no_prefix_log_type_name = DEBUG_LOG_NAME;
        break;
    case QtWarningMsg:
        type_name_str = "WARNING";
        strDir = m_logDir + WARNING_LOG_DIR_MACRO;
        no_prefix_log_type_name = WARNING_LOG_NAME;
        break;
    case QtCriticalMsg:
        type_name_str = "CRITICAL";
        strDir = m_logDir + CRITICAL_LOG_DIR_MACRO;
        no_prefix_log_type_name = CRITICAL_LOG_NAME;
        break;
    case QtFatalMsg:
        type_name_str = "FATAL";
        strDir = m_logDir + FATAL_LOG_DIR_MACRO;
        no_prefix_log_type_name = FATAL_LOG_NAME;
        break;
    default:
        type_name_str = "UNKNOWN";
        strDir = "./";     //未知类型日志，默认放到程序工作目录
        no_prefix_log_type_name = APP_LOG_PREFIX + QString("unknown.log");
        break;
    }

    QDir _dir;

    if(!_dir.exists(strDir))
        _dir.mkpath(strDir);


    //保留 m_nDaysOfLog 天的日志
    QDir fileDir(strDir);
    QStringList fileList = fileDir.entryList(QDir::Files, QDir::Name);


    //循环删除超出天数的文件
    for (int j = 0; j < fileList.size() - m_nDaysOfLog; j++)
    {
        QFile file(fileDir.absolutePath() + "/" + fileList.at(j));
        bool b = file.remove();
        if (!b)
        {
            qCritical() << file.fileName() << ":" << file.errorString();
        }
    }


    //日志内容控制
    QFile file_detail(QString("%1/%2_%3").arg(strDir).arg(QDate::currentDate().toString("yyyy_MM_dd")).arg(no_prefix_log_type_name));
    file_detail.open(QFile::WriteOnly | QFile::Append);

    //防止单个文件过大 >10M 的 情况 ，过大则 resize 一般不可能
    if(file_detail.size()>g_LogSize)
    {
        file_detail.resize(0);
    }


    QTextStream out(&file_detail);
    out.setCodec(QTextCodec::codecForName("utf-8"));
    QString strTime = QString("[%1][%2]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz")).arg(type_name_str);
    QString strLine = QString("[%1:%2]").arg(context.file).arg(context.line);

    QString strMsg = QString("%1%2 %3\n").arg(strTime).arg(strLine).arg(msg);
    printf("%s", strMsg.toLocal8Bit().data());
    fflush(stdout);

    out << strTime << strLine;
#ifdef Q_OS_WIN
    out << "\r\n";
#else
    out << "\n";
#endif

    out << "\t" << msg;

#ifdef Q_OS_WIN
    out << "\r\n";
#else
    out << "\n";
#endif
    out << "------------------------------------------------------------------";

#ifdef Q_OS_WIN
    out << "\r\n";
#else
    out << "\n";
#endif

    file_detail.flush();
    file_detail.close();

    m_mutex.unlock();
}



void QLog_MessageHandler::FormatMessage_myown(QtMsgType type,const char *msg)
{
    m_mutex.lock();

    //在开始记录之前都获取一下要保留的日志天数
    m_nDaysOfLog = g_Log_reserve_day;


    QString type_name_str;
    QString strDir;
    QString no_prefix_log_type_name;
    switch (type)
    {
    case QtDebugMsg:
        type_name_str = "DEBUG";
        strDir = m_logDir + DEBUG_LOG_DIR_MACRO;
        no_prefix_log_type_name = DEBUG_LOG_NAME;
        break;
    case QtWarningMsg:
        type_name_str = "WARNING";
        strDir = m_logDir + WARNING_LOG_DIR_MACRO;
        no_prefix_log_type_name = WARNING_LOG_NAME;
        break;
    case QtCriticalMsg:
        type_name_str = "CRITICAL";
        strDir = m_logDir + CRITICAL_LOG_DIR_MACRO;
        no_prefix_log_type_name = CRITICAL_LOG_NAME;
        break;
    case QtFatalMsg:
        type_name_str = "FATAL";
        strDir = m_logDir + FATAL_LOG_DIR_MACRO;
        no_prefix_log_type_name = FATAL_LOG_NAME;
        break;
    default:
        type_name_str = "UNKNOWN";
        strDir = "./";     //未知类型日志，默认放到程序工作目录
        no_prefix_log_type_name = APP_LOG_PREFIX + QString("unknown.log");
        break;
    }

    QDir _dir;

    if(!_dir.exists(strDir))
        _dir.mkpath(strDir);


    //保留 m_nDaysOfLog 天的日志
    QDir fileDir(strDir);
    QStringList fileList = fileDir.entryList(QDir::Files, QDir::Name);


    //循环删除超出天数的文件
    for (int j = 0; j < fileList.size() - m_nDaysOfLog; j++)
    {
        QFile file(fileDir.absolutePath() + "/" + fileList.at(j));
        bool b = file.remove();
        if (!b)
        {
            qCritical() << file.fileName() << ":" << file.errorString();
        }
    }


    //日志内容控制
    QFile file_detail(QString("%1/%2_%3").arg(strDir).arg(QDate::currentDate().toString("yyyy_MM_dd")).arg(no_prefix_log_type_name));
    file_detail.open(QFile::WriteOnly | QFile::Append);

    //防止单个文件过大 >10M 的 情况 ，过大则 resize 一般不可能
    if(file_detail.size()>g_LogSize)
    {
        file_detail.resize(0);
    }


    QTextStream out(&file_detail);
    out.setCodec(QTextCodec::codecForName("utf-8"));
    QString strTime = QString("[%1][%2]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz")).arg(type_name_str);
    QString strLine = QString("[%1:%2]").arg(context.file).arg(context.line);

    QString strMsg = QString("%1%2 %3\n").arg(strTime).arg(strLine).arg(msg);
    printf("%s", strMsg.toLocal8Bit().data());
    fflush(stdout);


    out << strTime << strLine;
#ifdef Q_OS_WIN
    out << "\r\n";
#else
    out << "\n";
#endif

    out << "\t" << QString(msg);

#ifdef Q_OS_WIN
    out << "\r\n";
#else
    out << "\n";
#endif
    out << "------------------------------------------------------------------";

#ifdef Q_OS_WIN
    out << "\r\n";
#else
    out << "\n";
#endif

    file_detail.flush();
    file_detail.close();

    m_mutex.unlock();
}



#endif
