#ifndef QHTTPRPC_PKGDEALER_H
#define QHTTPRPC_PKGDEALER_H

#include <QTimer>
#include <QThread>
#include <QObject>
#include <QUrlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"
#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"


class QHttpRPC_PkgDealer;


///////////////////////////////////////////////////////////////////////////////
/// \brief The client::Client class
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
namespace client {
using namespace qhttp::client;
///////////////////////////////////////////////////////////////////////////////
class Client:public QObject
{
    Q_OBJECT
public:
    Client(QHttpRPC_PkgDealer *parent = Q_NULLPTR)
        :m_pParent_dataDealer(parent)
    {
        iclient.setParent(this);
        QObject::connect(&iclient, &QHttpClient::disconnected, [this]() {

            //断开连接后应该做些什么(通知外部 停止)
            finalize();
        });

        iquery_raw_headers.clear();
        iquery_body_bytes.clear();

        QObject * obj_p = (QObject *)(m_pParent_dataDealer);

        if(parent)
            this->setParent(obj_p);
    };
    ~Client()
    {
        qDebug() << QString("inner  client::Client   destructor ......");
    };

    void set_url_file_path(const QString &file_path){ iurl_file_path = file_path;}

    void set_url_query(const QUrlQuery& query){ iquery_get_para = query;}

    void set_req_raw_headers(const QMap<QByteArray,QByteArray> &raw_headers)
    {
        iquery_raw_headers.clear();
        iquery_raw_headers = raw_headers;
    }

    void set_req_url_method(const qhttp::THttpMethod method) { irequest_method = method;}
    qhttp::THttpMethod get_req_url_method() { return irequest_method;}

    void set_http_request_body(const QByteArray &byteArrays)
    {
        iquery_body_bytes.clear();    //清掉原来的
        iquery_body_bytes.append(byteArrays);
    }

    void send();

    //内部接收程序范例
    void onIncomingData(const QByteArray& data)
    {
        auto root = QJsonDocument::fromJson(data).object();

        if ( root.isEmpty() )
        {
            qDebug() << QString("the result is an invalid json, ready to finalize,send signal<finalize> \n");
            finalize();
            return;
        }

//        if ( istan >= icount )
//            finalize();
//        else
//            send();
    }

    void finalize()
    {
        emit sig_finalize();
    }

public slots:
    int http_start(QString host_name , quint16 port = 80);


signals:
    void sig_sending_request();                 //通知外部 准备发起 http 请求
    void sig_finalize();                        //通知外部 结束

    void sig_response_status_string(int code,QString st_qstring);    //通知外部 应答的状态

public:
    int          icount = 0;

    QString      iurl_file_path = "";
    QUrl         iurl;

    qhttp::THttpMethod irequest_method = qhttp::EHTTP_GET;          //默认使用 http get
    //The QUrlQuery class provides a way to manipulate a key-value pairs in a URL's query.
    //一般在 http get 的时候可能会用到
    QUrlQuery    iquery_get_para;
    //http bodys
    QByteArray   iquery_body_bytes;

    //this time query , raw header info added the request package
    QMap<QByteArray,QByteArray> iquery_raw_headers;

    QHttpClient  iclient;

    QHttpRPC_PkgDealer *m_pParent_dataDealer = Q_NULLPTR;
}; // struct client

} // namespace client





///////////////////////////////////////////////////////////////////////////////
/// \brief The QHttpRPC_PkgDealer class
///////////////////////////////////////////////////////////////////////////////

class QHttpRPC_PkgDealer:public QObject
{
    Q_OBJECT
public:
    enum HTTP_PRC_PKG_TYPE
    {
        HTTP_NORMAL_LOGIN = 0,      //需要验证码方式的登录
        HTTP_SERVE_LOGIN,           //不需要验证码方式的登录
        HTTP_GET_RANDOM_IMAGE,       //获取验证码
        HTTP_GET_INTERVAL,           //获取间隔数据
        HTTP_GET_YCGENERAL,          //获取一次设备数据
        HTTP_GET_OLTYPE,             //获取在线监测装置类型
        HTTP_GET_OLGENERAL,          //获取在线监测装置数据
        HTTP_GET_OLCANSHU,           //获取在线装置参数
        HTTP_GET_REALDATA,           //获取实时数据信息
        HTTP_GET_HISDATA,            //获取历史数据信息

        HTTP_GET_IPINFO_FRM_INTERNET,   //获取对外的ip信息
        UNKNOWN_HTTP_PKG_TYPE = -1
    };

public:
    QHttpRPC_PkgDealer(QObject *parent = Q_NULLPTR);
    virtual ~QHttpRPC_PkgDealer();

    static void set_package_body_encrypt_enable(bool bEnable);
    static bool get_package_body_encrypt_status();

    void init_http_client();
    void wait_inner_thread_totally_quit();

    //获取请求类型 对应 的url 路径
    QString get_req_mapped_urlPath(HTTP_PRC_PKG_TYPE pack_t);

    //子类实现该函数 完成不同应答不同包的解析
    virtual void special_package_dealer(const QJsonObject &objInfo);
    //子类实现该函数 完成视频POST请求的应答包
    virtual void special_package_liveVideo(const QString &strStreamInfo);

    //如果需要定制 http 头部信息的话 先得执行该函数对 头部信息进行添加
    void prepare_httpPkg_headers(QMap<QString, QString> &key_values);
    //如果需要在 http body 中添加數據 用下面這個方法
    void prepare_httpPkg_body(const QByteArray &byteArray)
    {
        m_pHttp_client->set_http_request_body(byteArray);
    }

    //本次请求的 http 包的业务类型 便于后面解析分类
    void prepare_httpPkg_type(HTTP_PRC_PKG_TYPE pack_t) { m_package_type = pack_t;}
    HTTP_PRC_PKG_TYPE get_httpPkg_type() { return m_package_type;}


    void start_http_get(const QString url_addrconst,const QString& url_file_path = "",const quint16 port = 80);
    void start_http_get_with_paraMap(QMap<QString, QString> &key_values,
                                     const QString url_addrconst, const QString& url_file_path = "", const quint16 port = 80);

    void start_http_post(const QString url_addrconst,const QString& url_file_path = "",const quint16 port = 80);
    void start_http_post_with_bodyPara(const QJsonObject &bodyPara,
                                       const QString url_addrconst,const QString& url_file_path = "",const quint16 port = 80);

    //存储本次 http应答 解析出的http 头部键值对信息
    void set_current_response_headers_info(const qhttp::THeaderHash &headers);
    const qhttp::THeaderHash &get_current_response_headers_info();

    bool is_worker_thread_running()
    {
        return m_Worker_thread.isRunning();
    }

public:
    std::function<void()> m_parser_function;                 //解析函数保存处
    std::function<void(QByteArray data)> m_data_recv_func;                  //缓存器专门用于接收字节流 直到收完本次通信内容

public slots:
    //继承后重写该函数 实现特定数据解析
    void init_parser_data_func();

signals:
    void sig_parsed_done();


private:
    QThread m_Worker_thread;

    qhttp::THeaderHash m_curr_response_headers;

    QByteArray m_entire_package_cache;
    HTTP_PRC_PKG_TYPE m_package_type = UNKNOWN_HTTP_PKG_TYPE;

    client::Client *m_pHttp_client;

    static bool m_bPkg_body_need_encrypt;
};


#endif // QHTTPRPC_PKGDEALER_H
