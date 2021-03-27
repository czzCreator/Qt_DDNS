#include "qhttprpc_simpleserver.h"

#include <QThread>

namespace rzwl_client{

QHttpRPC_SimpleServer *QHttpRPC_SimpleServer::m_pSingletonServer = Q_NULLPTR;

QHttpRPC_SimpleServer::QHttpRPC_SimpleServer(QObject *parent)
{
    m_server_port = m_default_srv_port;
    m_entire_package_cache.clear();

    m_data_recv_func = [this](QByteArray data)->void{

        qDebug() << "simple http server(" << this <<") recving data.....:  " << data;
        m_entire_package_cache.append(data);

    };

    connect(this,SIGNAL(sig_start(quint16)),this,SLOT(start(quint16)));

    m_pWorker_thread = new QThread(this);
    m_pWorker_thread->start();    //默认虚函数 启动事件循环
    this->moveToThread(m_pWorker_thread);

    qDebug() << QString("~~~ ~~~ singleton  QHttpRPC_SimpleServer  obj  create......inner thread runningStatus(%1) ~~~ ~~~")
                .arg(is_worker_thread_running());
}


QHttpRPC_SimpleServer::~QHttpRPC_SimpleServer()
{
    qDebug() << QString("~~~ ~~~ QHttpRPC_SimpleServer ready to destruction...... ~~~ ~~~");
}


void QHttpRPC_SimpleServer::wait_inner_thread_totally_quit()
{
    if(m_pWorker_thread->isRunning())
    {
        m_pWorker_thread->quit();
        m_pWorker_thread->wait();
    }

    qDebug() << QString("~~~ ~~~ QHttpRPC_SimpleServer ready to stop inner thread(%1)...... ~~~ ~~~").arg(is_worker_thread_running());
}

void QHttpRPC_SimpleServer::process(QHttpRequest* req, QHttpResponse* res)
{
    if(this->m_entire_package_cache.isEmpty())
    {
        qDebug() << QString("Oops,recv  m_entire_package_cache  is empty  inner  http server,check please .......");
        //为下次接收做准备
        this->m_entire_package_cache.clear();

        return;
    }

    QJsonParseError err_detector;
    QJsonDocument doc_obj = QJsonDocument::fromJson(this->m_entire_package_cache,&err_detector);
    if(doc_obj.isNull())
    {
        qCritical() << QString("recv  invalid json  format  content----->reason:") << err_detector.errorString();
        //为下次接收做准备
        this->m_entire_package_cache.clear();

        return;
    }

    QJsonObject js_obj = doc_obj.object();
    emit sig_recv_js_obj(js_obj);

    //反馈收到正常 200 ok
    const static char KMessage[] = "json format parsed Ok!";
    res->setStatusCode(qhttp::ESTATUS_OK);
    res->addHeaderValue("content-length", strlen(KMessage));
    res->end(KMessage);

    //为下次接收做准备
    this->m_entire_package_cache.clear();

    return;

//    if ( root.isEmpty()  ||  root.value("name").toString() != QLatin1Literal("add") ) {
//        const static char KMessage[] = "Invalid json format!";
//        res->setStatusCode(qhttp::ESTATUS_BAD_REQUEST);
//        res->addHeader("connection", "close");
//        res->addHeaderValue("content-length", strlen(KMessage));
//        res->end(KMessage);
//        return;
//    }

//    int total = 0;
//    auto args = root.value("args").toArray();
//    for ( const auto jv : args ) {
//        total += jv.toInt();
//    }
//    root["args"] = total;

//    QByteArray body = QJsonDocument(root).toJson();
//    res->addHeader("connection", "keep-alive");
//    res->addHeaderValue("content-length", body.length());
//    res->setStatusCode(qhttp::ESTATUS_OK);
//    res->end(body);

}

bool QHttpRPC_SimpleServer::is_worker_thread_running()
{
    if(m_pWorker_thread != Q_NULLPTR)
    {
        return m_pWorker_thread->isRunning();
    }
    else
        return false;
}



}
