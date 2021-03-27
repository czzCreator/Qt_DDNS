#ifndef QHTTPRPC_SIMPLESERVER_H
#define QHTTPRPC_SIMPLESERVER_H

#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"
#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"


#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

//# 这个服务类应该是全局唯一的

namespace rzwl_client {

using namespace qhttp::server;

class QHttpRPC_SimpleServer : public QHttpServer
{
    Q_OBJECT

    using QHttpServer::QHttpServer;
public:

    //懒加载 第一次调用时才创建
    static QHttpRPC_SimpleServer* getInstance()
    {
        // Instantiated on first use.
        if(m_pSingletonServer == Q_NULLPTR)
        {
            m_pSingletonServer = new QHttpRPC_SimpleServer();
        }

        // Guaranteed to be destroyed.
        return m_pSingletonServer;
    }

    static void getInstanceDestruct()
    {
        if(m_pSingletonServer)
        {
            m_pSingletonServer->wait_inner_thread_totally_quit();

            delete m_pSingletonServer;
            m_pSingletonServer = Q_NULLPTR;
        }
    }

    // C++ 11
    // =======
    // We can use the better technique of deleting the methods
    // we don't want.
    QHttpRPC_SimpleServer(QHttpRPC_SimpleServer const&) = delete;           //Don't Implement
    void operator=(QHttpRPC_SimpleServer const&) = delete;                  //Don't implement

    // Note: Scott Meyers mentions in his Effective Modern
    //       C++ book, that deleted functions should generally
    //       be public as it results in better error messages
    //       due to the compilers behavior to check accessibility
    //       before deleted status


    void wait_inner_thread_totally_quit();

    void set_server_port(int portNum = 9119) { m_server_port = portNum;}
    int get_server_port() { return m_server_port; }

    void process(QHttpRequest* req, QHttpResponse* res);

    bool is_worker_thread_running();    

signals:
    void sig_recv_js_obj(QJsonObject obj);
    void sig_start(quint16 port);

private slots:
    int start(quint16 port = 9119)
    {
        connect(this, &QHttpServer::newConnection, [this](QHttpConnection*){
            qDebug() << QString("simple server (") << this << ")" << "recv  a new connection";
        });


        TServerHandler processHandler = [this](QHttpRequest* req, QHttpResponse* res)
        {
            //req->collectData(512);

            req->onData(m_data_recv_func);

            req->onEnd([this, req, res]()
            {
                this->process(req, res);
            });
        };

        set_server_port(port);
        bool isListening = listen(QString::number(port),processHandler);

        if ( !isListening )
        {
            qCritical("simple server(http) can not listen on %d!\n", port);
            return -1;
        }

        return 0;
    }

private:
    /** construct a new HTTP Server. */
    explicit QHttpRPC_SimpleServer(QObject *parent = nullptr);
    ~QHttpRPC_SimpleServer();

private:
    std::function<void(QByteArray data)> m_data_recv_func;                  //缓存器专门用于接收字节流 直到收完本次通信内容


private:
    //default port num = 9119
    int m_server_port = -1;
    const int m_default_srv_port = 9119;

    //每个请求一个缓存
    QByteArray m_entire_package_cache;

    //内部事件循环用
    QThread *m_pWorker_thread = Q_NULLPTR;

    //堆上唯一 服务器
    static QHttpRPC_SimpleServer *m_pSingletonServer;

}; // struct server

}

#endif // QHTTPRPC_SIMPLESERVER_H

