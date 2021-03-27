#include "qhttprpc_outeraddrrequester.h"

#include "qappglobalref.h"
#include "qddns_service.h"

QHttpRPC_OuterAddrRequester::QHttpRPC_OuterAddrRequester()
{

}

QHttpRPC_OuterAddrRequester::~QHttpRPC_OuterAddrRequester()
{
    qDebug() << QString("~~ QHttpRPC_OuterAddrRequester  ready  to  destruct ~~......");
}

void QHttpRPC_OuterAddrRequester::create_IPaddrInfoGetMethod(const QString &hostName,const int iPort,QObject *caller)
{
    QHttpRPC_OuterAddrRequester *new_nvr_rpc = new QHttpRPC_OuterAddrRequester;

    connect(new_nvr_rpc,&QHttpRPC_OuterAddrRequester::sig_parsed_done,caller,[=](){

        new_nvr_rpc->wait_inner_thread_totally_quit();
        new_nvr_rpc->deleteLater();
        //delete new_nvr_rpc;
    });

    connect(new_nvr_rpc,&QHttpRPC_OuterAddrRequester::sig_getIPaddr_success,caller,[=](QString ipInfo)->void{

                if(caller == Q_NULLPTR)
                    return;

                //与当前得到的ip信息进行比对
                qDebug() << QString("this time  get outter  ip Info--->") << ipInfo;
                QDDNS_Service * getTheService = dynamic_cast<QDDNS_Service *>(caller);
                getTheService->compareCachedIpAddr(ipInfo);


                //push to destruct array ......
                emit new_nvr_rpc->sig_parsed_done();
    });

    QMap <QString,QString> headKeys;
    headKeys.insert("content-type","application/json;charset=UTF-8");

    new_nvr_rpc->prepare_getIPaddrInfo_package();
    //new_nvr_rpc->prepare_httpPkg_headers(headKeys);

    new_nvr_rpc->start_http_get(hostName,"/ip",iPort);
}

void QHttpRPC_OuterAddrRequester::prepare_getIPaddrInfo_package()
{
    prepare_httpPkg_type(HTTP_GET_IPINFO_FRM_INTERNET);
}

void QHttpRPC_OuterAddrRequester::special_package_dealer(const QJsonObject &objInfo)
{
    if(get_httpPkg_type() == HTTP_GET_IPINFO_FRM_INTERNET)
    {
        if(objInfo.isEmpty())
        {
            qDebug() << QString("<class QHttpRPC_OuterAddrRequester> get an empty QJsonObject......");
            emit sig_parsed_done();
            return;
        }

        //收到结果样式 : {"origin":"199.193.127.102"}
        if(objInfo.value("origin") == QJsonValue::Undefined)
        {
            qCritical() << QString("can't   find  'origin'  key  in   QJsonObject......");
            emit sig_parsed_done();
            return;
        }

        emit sig_getIPaddr_success(objInfo.value("origin").toString());
    }
}
