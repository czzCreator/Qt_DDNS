#ifndef QDDNS_SERVICE_H
#define QDDNS_SERVICE_H

#include <QObject>
#include <QQueue>

#include "qhttprpc_outeraddrrequester.h"



class QDDNS_Service:public QObject
{
    Q_OBJECT
public:
    QDDNS_Service();
    ~QDDNS_Service();

    void initSettingForAddrInfo();

    void compareCachedIpAddr(QString outerIpInfo);

    void startTimerCheck();
    void stopService();

private slots:
    void slot_outer_ip_check_interval();

private:
    void doDDNS_operate_Detached();

private:
    QTimer *m_pGetAddrInterval = Q_NULLPTR;

    QString m_reserve_ip_addr_string;
};

#endif // QDDNS_SERVICE_H
