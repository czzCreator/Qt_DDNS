#include "qddns_service.h"

#include <QSettings>
#include <QCoreApplication>
#include <QProcess>

#include "qhttprpc_outeraddrrequester.h"


QDDNS_Service::QDDNS_Service()
{
    m_reserve_ip_addr_string.clear();
    m_pGetAddrInterval = new QTimer(this);

    //5s 获取一次IP信息
    m_pGetAddrInterval->setInterval(60*1000);
    //m_pGetAddrInterval->setSingleShot(true);

    connect(m_pGetAddrInterval,SIGNAL(timeout()),this,SLOT(slot_outer_ip_check_interval()));
}


QDDNS_Service::~QDDNS_Service()
{
    qDebug() << QString("~~ QDDNS_Service  object  ready  to  destruct...... ~~");
}


void QDDNS_Service::initSettingForAddrInfo()
{
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());

    QString original_ip_string = settings.value("IP_ADDR").toString();
    m_reserve_ip_addr_string = original_ip_string;

    qDebug() << QString("we get  original  ip  info  from   QSetting---->(%1)").arg(m_reserve_ip_addr_string);
}


void QDDNS_Service::compareCachedIpAddr(QString outerIpInfo)
{
    if(outerIpInfo.isEmpty())
    {
        qDebug() << QString("be compared Ip addr is empty,may be get new IP failed,check......");
        return;
    }

    QHostAddress test;
    if (!test.setAddress(outerIpInfo))
    {
        qDebug() << QString("ERROR : Invalid ip address from outer caller");
        return;
    }

    //本次得到的最新的 外部IP信息 写入注册表
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("IP_ADDR",outerIpInfo);

    //与原先的IP 信息进行比对 看是否有必要执行外部命令
    if(m_reserve_ip_addr_string != outerIpInfo)
    {
        //调用外部命令进行 远程执行ddns
        doDDNS_operate_Detached();

        m_reserve_ip_addr_string = outerIpInfo;
    }
    else
        qDebug() << QString("IP not changed......continue......");
}


//其他线程调用
void QDDNS_Service::startTimerCheck()
{
    if(!m_pGetAddrInterval->isActive())
        m_pGetAddrInterval->start();
}

//其他线程调用
void QDDNS_Service::stopService()
{
    if(m_pGetAddrInterval->isActive())
        m_pGetAddrInterval->stop();
}


void QDDNS_Service::slot_outer_ip_check_interval()
{
    //qDebug() << QString("~~~~~~~~~~~~~~~~~~~ sad interval test ~~~~~~~~~~~~~~~~~~~~thread:") << QThread::currentThread();

    QHttpRPC_OuterAddrRequester::create_IPaddrInfoGetMethod("httpbin.org",80,this);

}


void QDDNS_Service::doDDNS_operate_Detached()
{
    //停止计时器 去不停请求新IP信息
    m_pGetAddrInterval->stop();

    //QProcess 在终端中执行的结果进行返回
    // instantiate dynamically to avoid stack unwinding before the process terminates
    QProcess* process = new QProcess();

    // catch data output
    QObject::connect(process, &QProcess::readyRead, [process] () {
        QByteArray a = process->readAll();
        qDebug() << "{Attention!!!!!--->we recv  StandardOutPut(or ERROR!!!!!)  from  ddns:--->}"
                 << a;
    });

    // delete process instance when done, and get the exit status to handle errors.
    QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus /*exitStatus*/){
        qDebug()<< "process exited with code " << exitCode;
        process->deleteLater();
    });

    // start the process after making signal/slots connections
    process->start("./ddns.exe");



    //重新开启计时器 去不停请求新IP信息
    m_pGetAddrInterval->start();
}














