#ifndef QHTTPRPC_OUTERADDRREQUESTER_H
#define QHTTPRPC_OUTERADDRREQUESTER_H

#include "qhttprpc_pkgdealer.h"

class QHttpRPC_OuterAddrRequester: public QHttpRPC_PkgDealer
{
    Q_OBJECT
public:
    QHttpRPC_OuterAddrRequester();
    ~QHttpRPC_OuterAddrRequester();

    static void create_IPaddrInfoGetMethod(const QString &hostName,const int iPort,QObject *caller);

    void prepare_getIPaddrInfo_package();

signals:
    void sig_getIPaddr_success(QString outerAddrInfo);

public:
    void special_package_dealer(const QJsonObject &objInfo) override;

};

#endif // QHTTPRPC_OUTERADDRREQUESTER_H





