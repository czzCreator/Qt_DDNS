#encoding=utf-8

# 下面三个模块用于将 python 代码写成服务的形式
import win32serviceutil 
import win32service 
import win32event

import logging
#让日志滚动记录 时间超过则会自动删除
from logging.handlers import TimedRotatingFileHandler
import inspect 

import os,sys
import time    #引入时间模块
import requests
import subprocess
import schedule


# 将此模块改写为 windows 服务  7*24 小时运行
class PythonDDNS_detectService(win32serviceutil.ServiceFramework): 
 
    _svc_name_ = "PythonDDNS_detectService"
    _svc_display_name_ = "Python DDNS Service for rzwl"
    _svc_description_ = "This is a python service that dynamic changed relation between ip and domain......"

    #服务初始化时 执行这个构造函数
    def __init__(self, args): 
        win32serviceutil.ServiceFramework.__init__(self, args) 
        self.hWaitStop = win32event.CreateEvent(None, 0, 0, None)
        self.logger = self._getLogger()             # 日志记录使用这个
        self.run = True             #定义了一个运行标志位  控制服务运行与否
        
        self.outter_ipweb_url = ""
        self.ip_info = [0]

    # 日志记录使用这个 内部日志函数
    def _getLogger(self):
        
        logger = logging.getLogger('[PythonDDNS_detectService]')
        
        this_file = inspect.getfile(inspect.currentframe())
        dirpath = os.path.abspath(os.path.dirname(this_file))
        
        #handler = logging.FileHandler(os.path.join(dirpath, "PythonDDNS_detectService.log"))
        
        #https://blog.csdn.net/xj626852095/article/details/70045620
        #logging库提供了两个可以用于日志滚动的class（可以参考https://docs.python.org/2/library/logging.handlers.html），
        #一个是RotatingFileHandler，它主要是根据日志文件的大小进行滚动，另一个是TimeRotatingFileHandler，
        #它主要是根据时间进行滚动。在实际应用中，我们通常根据时间进行滚动，
        #因此，本文中主要介绍TimeRotaingFileHandler的使用方法（RotatingFileHandler一样）。代码示例如下：
        fileFullPathName = os.path.join(dirpath, "PythonDDNS_detect_log");
        handler = TimedRotatingFileHandler(filename=fileFullPathName, when="D", interval=1, backupCount=7)

        formatter = logging.Formatter('%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s')
        handler.setFormatter(formatter)
        
        logger.addHandler(handler)
        logger.setLevel(logging.INFO)
        
        return logger

    # 定义函数
    def getOuterIpInfo(self, inf , url):
        
        inf.clear();
        try:
            ip_addr_inf = requests.get(url);            
        except exceptions.ConnectTimeout as connTimeOutErr:
            self.logger.critical(str(connTimeOutErr));
            return False;
        except exceptions.ReadTimeout as readTimeoutErr:
            self.logger.critical(str(readTimeoutErr));
            return False;
        except exceptions.Timeout as timeout_Err:
            self.logger.critical(str(timeout_Err));
            return False;
        except:
            self.logger.critical("unknown http client error...");
            return False;
        
        inf.append(ip_addr_inf.text);
        return True;
    

    # 程序初始化环境 路径等信息
    def app_startup_init(self):
    
        retval = os.getcwd()

        #print ("当前工作目录为 %s" % retval)
        self.logger.info("当前工作目录为 %s" % retval)

        os.chdir(sys.path[0]);
        retval = os.getcwd()
        #print ("切换工作目录(脚本所在目录) %s" % retval)
        self.logger.info("切换工作目录(脚本所在目录) %s" % retval)


        # 格式化成2016-03-20 11:45:39形式
        #print ("脚本启动时间: %s" % time.strftime("%Y-%m-%d %H:%M:%S",time.localtime()))
        self.logger.info("脚本启动时间: %s" % time.strftime("%Y-%m-%d %H:%M:%S",time.localtime()))


        # 利用外部网站定位自己 外网的 IP信息
        # 下面的网站是 可用的IP 查询网站

        ##curl ipinfo.io
        ##curl myip.ipip.net
        ##curl http://members.3322.org/dyndns/getip    (直接给ip)
        ##curl https://ip.cn
        ##curl httpbin.org/ip
        ##curl ip.sb 
        ##curl whatismyip.akamai.com        (直接给ip)
        ##curl ipecho.net/plain        (直接给ip)
        ##curl icanhazip.com          (直接给ip)

        self.outter_ipweb_url = "http://whatismyip.akamai.com";

        # 先初始化下 最初的ip信息
        isSuccess = self.getOuterIpInfo(self.ip_info, self.outter_ipweb_url);

        if isSuccess == False or len(self.ip_info) == 0:
            #print("get outer ip info from website failed,check...exit script")
            self.logger.critical("get outer ip info from website failed,check...exit script")
            exit(-1)

        # 先初始化执行一次 15s超时
        #try:
        #    cmd_out_bytes = subprocess.check_output(['ddns.exe'], timeout=15)
        #except subprocess.TimeoutExpired as e:
        #    print("subprocess.check_output  FAILED!!!!! timeout,check......");
        #    exit(-1);

        #print("初始化检测 当前广域网 对外ip 为 <%s>" %ip_info);
        self.logger.info("初始化检测 当前广域网 对外ip 为 <%s>" %self.ip_info[0]);



    def check_ddns_interval(self):

        ip_temp_info = [0];
        bRet = self.getOuterIpInfo(ip_temp_info , self.outter_ipweb_url);

        if bRet == True and ip_temp_info[0] == self.ip_info[0]:
            #print("<log time-%s >outer ip the same with before status <%s>!!!" %(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime()),ip_info[0]))
            self.logger.info("outer ip the same with before status <%s>!!!" %(self.ip_info[0]))
        elif bRet == True and not(ip_temp_info[0] == self.ip_info[0]):
            self.ip_info.clear();
            self.ip_info.append(ip_temp_info[0]);

            ddns_exe_out_string = "";
            try:
                cmd_out_bytes = subprocess.check_output(['ddns.exe'], timeout=15)  #超时15s 执行限制
                ddns_exe_out_string = cmd_out_bytes.decode();
            except subprocess.TimeoutExpired as e:
                self.logger.critical("<check_ddns_interval>subprocess.check_output  FAILED!!!!! timeout,check......");
                exit(-1);

            import_log_infp = "<WARN !!-> ip changed outside,now ip==<%s>" %(self.ip_info[0]);
            import_log_infp += "\n";
            import_log_infp += "Now we have already do  ddns  once.....";
            import_log_infp += "\n";
            import_log_infp += "<ddns ret string:>---" + ddns_exe_out_string;
            import_log_infp += "##########################################";

            self.logger.warn(import_log_infp);

        else:
            self.logger.critical("in  fun:check_ddns_interval   getOuterIpInfo   failed!!,check.....");
        return
    

 
    def SvcDoRun(self):
        self.logger.info("service begin to run....")

        self.app_startup_init()

        self.logger.info("service app_startup_init() done....")

        #创建一个按秒间隔执行任务
        schedule.every(60).seconds.do(self.check_ddns_interval)
        
        while self.run:
            schedule.run_pending();

        self.logger.info("service ready to stop....") 

            
    def SvcStop(self): 
        self.logger.info("set service to stop flag....")
        self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING) 
        win32event.SetEvent(self.hWaitStop) 
        self.run = False





##########################################


if __name__=='__main__':
    win32serviceutil.HandleCommandLine(PythonDDNS_detectService)











    


