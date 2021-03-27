#include <QCoreApplication>

#include "qappglobalref.h"

#include "qddns_service.h"

extern void app_release_operation();
extern void app_action_before_exit(const int app_exitCode);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setOrganizationName("rzwl");
    a.setApplicationName("Qt_DDNS");


    message_handler::QLog_MessageHandler::setLogReserve_day(DEFAULT_LOG_RESERVE_DAY);
    message_handler::QLog_MessageHandler::setLogDir(DEFAULT_QtDDNS_LOG_DIR);
    qInstallMessageHandler(message_handler::QLog_MessageHandler::FormatMessage_myown);


    qDebug() << QString("<<=====================================================>>");
    qDebug() << QString("<< ~~~ QtDDNS begin to init(run) ~~~ >>");
    qDebug() << QString("main() begin ---- Thread obj:") << QThread::currentThread();
    qDebug() << QString("<<=====================================================>>");

    /************************************************************/
    //启动内部简易的 http server 服务器 默认端口 10086
    /************************************************************/
    rzwl_client::QHttpRPC_SimpleServer *pSingleton_httpSrv = rzwl_client::QHttpRPC_SimpleServer::getInstance();
    emit pSingleton_httpSrv->sig_start(10086);
    /************************************************************/

    QDDNS_Service oneService;
    oneService.initSettingForAddrInfo();
    oneService.startTimerCheck();

    int app_ret_result = a.exec();

    /************************************************************/
    /************************************************************/

    app_release_operation();

    /************************************************************/
    /************************************************************/

    app_action_before_exit(app_ret_result);

    /************************************************************/

    qDebug() << QString("<<=====================================================>>");
    qDebug() << QString("~~ QtDDNS App ready to quit, ret_result:%1 ~~").arg(app_ret_result);
    qDebug() << QString("<<=====================================================>>");

    return app_ret_result;
}



void app_release_operation()
{
    //do some release thing ......

    //db connect pool destroy......
    //CDataBaseOperator::GetDataProcess()->CleanUpPool();

    //simple singleton http server stop and destruct
    rzwl_client::QHttpRPC_SimpleServer::getInstanceDestruct();
}



void app_action_before_exit(const int app_exitCode)
{
    //结束前根据 主事件循环退出码决定要做的动作
    switch(app_exitCode)
    {
    case app_exit_code::APP_RESTART_CMD:
    {
        qInfo() << QString("[In main()]: the app now going to restart itself!!");

        //要启动当前程序的另一个进程，有要使二者没有"父子"关系
        bool restart_result = QProcess::startDetached(qApp->applicationFilePath(), QStringList());
        if(!restart_result)
            qInfo() << QString("[In main()]: QProcess restart app failed! check!!");

        break;
    }

    case app_exit_code::APP_SHUTDOWN_NORMALLY:
    {
        qInfo() << QString("[In main()]: the app now going to exit normally....");
        break;
    }

    default:
        break;
    }
}




//出了main() 函数有些全局库将开始析构......





















