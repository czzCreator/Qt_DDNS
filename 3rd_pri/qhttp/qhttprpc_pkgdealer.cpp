#include "qhttprpc_pkgdealer.h"
//#include "cglobaldatastore.h"


int client::Client::http_start(QString host_name , quint16 port)
{

    QUrl url;
    url.setScheme("http");
    url.setHost(host_name);
    url.setPath(iurl_file_path);
    url.setPort(port);

    if(!iquery_get_para.isEmpty())
        url.setQuery(iquery_get_para);

    iurl   = url;
    //icount = count;

//    qDebug() << QString("[thread obj-->] ") << QThread::currentThread()
//             << QString(" | qhttp ready ---request URL:---") << url.toString();

    //通知外部 准备发起http 连接了
    emit sig_sending_request();
    send();

    return 0;
}


void client::Client::send()
{
    iclient.request(
                irequest_method,
                iurl,
                [this](QHttpRequest* req)
    {
        // server computes sum of these values
        qDebug() << QString("http send request_method[%1] body message ------>{")
                    .arg(irequest_method) << QString(iquery_body_bytes) << QString("} \nThread obj --")<<QThread::currentThread();

        auto body = iquery_body_bytes;

        //添加本次请求要设置的 http 头部信息
        //req->addHeader("connection", "keep-alive");           不需要保活重复利用底层 socket
        req->addHeaderValue("content-length", body.length());
        QMap<QByteArray,QByteArray>::iterator iter_head_raw  = iquery_raw_headers.begin();
        while(iter_head_raw != iquery_raw_headers.end())
        {
            req->addHeader(iter_head_raw.key(),iter_head_raw.value());
            iter_head_raw++;
        }
        iquery_raw_headers.clear();       //本次请求已经用完,清空缓存队列

        req->end(body);
    },
    [this](QHttpResponse* res)
    {
//        qDebug() << QString("file: ")<< __FILE__ << QString("  (Response)line:") <<  __LINE__ << QString(" func:") << __FUNCTION__
//                 << QString("  Thread obj:") << QThread::currentThread();


        if(m_pParent_dataDealer != Q_NULLPTR)
        {
            res->onData(m_pParent_dataDealer->m_data_recv_func);
        }

        if(m_pParent_dataDealer != Q_NULLPTR)
        {
            //数据全部收取完毕 才开始解析
            res->onEnd([this, res](){

                //解析完毕 向外部对象通知最后一次 response 的状态以及 status_code
                qhttp::TStatusCode getStatus_code = res->status();
                QString responseStatusString = res->statusString();

                sig_response_status_string(getStatus_code,responseStatusString);

                //存储本次 http 应答头部信息
                qhttp::THeaderHash responseHeaders = res->headers();

                m_pParent_dataDealer->set_current_response_headers_info(responseHeaders);
                //解析 http body 数据
                m_pParent_dataDealer->m_parser_function();
            });
        }

//        res->collectData(512);

//        res->onEnd([this, res](){
//            onIncomingData(res->collectedData());
//        });
    });
}


///////////////////////////////////////////////////////////////////////////////
/// \brief The QHttpRPC_PkgDealer class
///////////////////////////////////////////////////////////////////////////////

bool QHttpRPC_PkgDealer::m_bPkg_body_need_encrypt = false;   //默认发送的 http 数据不加密
QHttpRPC_PkgDealer::QHttpRPC_PkgDealer(QObject *parent)
    :QObject(parent)
{
    m_entire_package_cache.clear();

    m_pHttp_client = new client::Client(this);

    init_parser_data_func();

    connect(&m_Worker_thread, &QThread::finished, m_pHttp_client, &QObject::deleteLater);
    m_Worker_thread.start();    //默认虚函数 启动事件循环

    m_pHttp_client->moveToThread(&m_Worker_thread);

    init_http_client();
}


QHttpRPC_PkgDealer::~QHttpRPC_PkgDealer()
{
    //由于 m_pWorker_thread  m_pHttp_client 都设置了 QHttpRPC_PkgDealer 为父类
    //释放 父类会对应 释放子类 这里不用显式的释放
//    if(m_pWorker_thread != Q_NULLPTR)
//    {
//        //这里必须是线程已经停止的状态
//        if(m_pWorker_thread->isRunning())
//        {
//            qDebug() << QString("Oops, it's illegal to get m_pWorker_thread running status  here <QHttpRPC_PkgDealer>");
//            m_pWorker_thread->quit();
//        }

//        qDebug() << QString("Ready to destruct  m_pWorker_thread  In  QHttpRPC_PkgDealer~~~~");
//        m_pWorker_thread->deleteLater();
//        m_pWorker_thread = Q_NULLPTR;
//    }
}


void QHttpRPC_PkgDealer::set_package_body_encrypt_enable(bool bEnable)
{
    m_bPkg_body_need_encrypt = bEnable;
}

bool QHttpRPC_PkgDealer::get_package_body_encrypt_status()
{
    return m_bPkg_body_need_encrypt;
}


void QHttpRPC_PkgDealer::init_http_client()
{    

    //有可能是其他线程发过来 的 sig_sending_request 信号
    connect(m_pHttp_client, &client::Client::sig_sending_request ,this ,[](){

        //        qDebug() << QString("file: ")<< __FILE__ << QString("  line:") <<  __LINE__ << QString(" func:") << __FUNCTION__
        //                 << QString("  Thread obj:") << QThread::currentThread();

    });

    //收到的应答 的结果状态码
    connect(m_pHttp_client, &client::Client::sig_response_status_string , this,[this](int ret_code,QString status_string){

//        qDebug() << QString("file: ")<< __FILE__ << QString("  line:") <<  __LINE__ << QString(" func:") << __FUNCTION__
//                 << QString("  Thread obj:") << QThread::currentThread();

        qDebug() << QString("get  http  response  code:%1  and  status_string:%2  pkg_type(%3)")
                    .arg(ret_code).arg(status_string).arg(m_package_type);
    });

    connect(m_pHttp_client, &client::Client::sig_finalize ,this ,[](){

        qDebug() << QString("[QHttpRPC_PkgDealer::init_http_client()] -- recv<sig_finalize> should  do  something(may be connection closed or delete)? ......");

    });
}


void QHttpRPC_PkgDealer::wait_inner_thread_totally_quit()
{
    if(m_Worker_thread.isRunning())
    {
        m_Worker_thread.quit();
        m_Worker_thread.wait();
    }
}

QString QHttpRPC_PkgDealer::get_req_mapped_urlPath(HTTP_PRC_PKG_TYPE pack_t)
{
    QString ret_url_path;
    switch(pack_t)
    {
    case HTTP_NORMAL_LOGIN:
        ret_url_path = "/zxjc/login";
        break;
    case HTTP_SERVE_LOGIN:
        ret_url_path = "/zxjc/login";
        break;
    case HTTP_GET_RANDOM_IMAGE:
        ret_url_path = "/jeecg-boot/sys/randomImage/{KEY}";
        break;
    default:
        ret_url_path = "";
        break;
    }

    return ret_url_path;
}

//数据解析接口
void QHttpRPC_PkgDealer::init_parser_data_func()
{
    m_parser_function = [this]()->void{

//        qDebug() << QString("<Thd obj:") << QThread::currentThread() << QString(">czz test recv the http data----------------->")
//                 << QString(this->m_entire_package_cache);

        QJsonParseError error_info;
        QJsonDocument json_result = QJsonDocument::fromJson(QString(this->m_entire_package_cache).toUtf8(),&error_info);
        if(json_result.isNull())
        {
            qDebug() << QString("Parsed http pkg <type:%1>, error---> %2").arg(this->m_package_type).arg(error_info.errorString());
        }
        else
        {
            QJsonObject objInfo = json_result.object();
            special_package_dealer(objInfo);
        }
        //为下次接收做准备
        this->m_entire_package_cache.clear();
    };

    m_data_recv_func = [this](QByteArray data)->void{

        m_entire_package_cache.append(data);
    };
}

void QHttpRPC_PkgDealer::special_package_dealer(const QJsonObject &objInfo)
{
    //do something for you own......
}

void QHttpRPC_PkgDealer::special_package_liveVideo(const QString &strStreamInfo)
{

}

void QHttpRPC_PkgDealer::prepare_httpPkg_headers(QMap<QString, QString> &key_values)
{
    QMap<QByteArray, QByteArray> tmp_array_map_container;
    QMap<QString, QString>::iterator iter_head_raw  = key_values.begin();
    while(iter_head_raw != key_values.end())
    {
        //针对每一个 头部键值对 进行检查 或者一些特殊处理....
        //do something......

        tmp_array_map_container.insert(iter_head_raw.key().toUtf8(),iter_head_raw.value().toUtf8());
        iter_head_raw++;
    }

    m_pHttp_client->set_req_raw_headers(tmp_array_map_container);
}

void QHttpRPC_PkgDealer::start_http_get(const QString url_addrconst, const QString& url_file_path, const quint16 port)
{
    m_pHttp_client->set_req_url_method(qhttp::EHTTP_GET);
    m_pHttp_client->set_url_file_path(url_file_path);

    QTimer::singleShot(1,this,[this,url_addrconst,url_file_path,port](){

//        qDebug() << QString("start http get current...... thread obj -->") << QThread::currentThread();
//        qDebug() << QString("this object url path---->") << url_file_path;
        m_pHttp_client->http_start(url_addrconst,port);
    });
}

void QHttpRPC_PkgDealer::start_http_get_with_paraMap(QMap<QString,QString> &key_values,
                                                     const QString url_addrconst,const QString& url_file_path, const quint16 port)
{
    QUrlQuery query_tmp;
    QMap<QString,QString>::iterator iter_map = key_values.begin();
    for( ; iter_map != key_values.end() ; iter_map++)
    {
        query_tmp.addQueryItem(iter_map.key(),iter_map.value());
    }

    m_pHttp_client->set_url_query(query_tmp);
    m_pHttp_client->set_req_url_method(qhttp::EHTTP_GET);
    m_pHttp_client->set_url_file_path(url_file_path);


    QTimer::singleShot(1,this,[this,url_addrconst,port](){

//        qDebug() << QString("start http get with_paraMap current...... thread obj -->") << QThread::currentThread();
//        qDebug() << QString("this object ---->") << this;
        m_pHttp_client->http_start(url_addrconst,port);

    });
}

void QHttpRPC_PkgDealer::start_http_post(const QString url_addrconst, const QString& url_file_path, const quint16 port)
{
    m_pHttp_client->set_req_url_method(qhttp::EHTTP_POST);
    m_pHttp_client->set_url_file_path(url_file_path);

    QTimer::singleShot(1,this,[this,url_addrconst,port](){

//        qDebug() << QString("start http post current...... thread obj -->") << QThread::currentThread();
//        qDebug() << QString("this object ---->") << this;
//        qDebug() << QString("m_pHttp_client's    parent-->") << m_pHttp_client->parent();
//        qDebug() << QString("m_pWorker_thread's    parent-->") << m_pWorker_thread->parent();

        m_pHttp_client->http_start(url_addrconst,port);

        qDebug() << "start http post" << url_addrconst << port;

    });
}


void QHttpRPC_PkgDealer::start_http_post_with_bodyPara(const QJsonObject &bodyPara,
                                                       const QString url_addrconst,const QString& url_file_path,const quint16 port)
{
    m_pHttp_client->set_req_url_method(qhttp::EHTTP_POST);

    QJsonDocument doc(bodyPara);
    QString strJson(doc.toJson(QJsonDocument::Indented));
    QByteArray bytes_body = strJson.toUtf8();


    QString strSendBuffer;
    std::string std_message;
    QString mess_body_js_qstr(bytes_body);
    QString get_json_str_fromqueue = mess_body_js_qstr;

    if(get_package_body_encrypt_status())
    {
        //需要进行加密发送
        //SM_X_Util::encrypt_qstring_with_sm4(get_json_str_fromqueue,strSendBuffer);
        std_message = strSendBuffer.toStdString();
    }
    else
    {//无需加密
        std_message = get_json_str_fromqueue.toStdString();
    }

    m_pHttp_client->set_http_request_body(std_message.data());
    //m_pHttp_client->set_http_request_body(bytes_body);
    m_pHttp_client->set_url_file_path(url_file_path);


    QTimer::singleShot(1,this,[this,url_addrconst,port](){

//        qDebug() << QString("start http post with_bodyPara current...... thread obj -->") << QThread::currentThread();
//        qDebug() << QString("this object ---->") << this;
        m_pHttp_client->http_start(url_addrconst,port);

    });

}

void QHttpRPC_PkgDealer::set_current_response_headers_info(const qhttp::THeaderHash &headers)
{
    //清理一下
    m_curr_response_headers.clear();
    m_curr_response_headers = headers;
}

const qhttp::THeaderHash &QHttpRPC_PkgDealer::get_current_response_headers_info()
{
    return m_curr_response_headers;
}















