#ifndef  RPC_CLIENT_H_
#define  RPC_CLIENT_H_

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "mutex.h"
#include "thread_pool.h"
#include "glog/logging.h"
#include "poppy/rpc_channel.h"
#include "poppy/rpc_client.h"


namespace chaoaero {
class RpcClient {
public:
    RpcClient() {
        // 定义 client 对象，一个 client 程序只需要一个 client 对象
        // 可以通过 client_options 指定一些配置参数，譬如线程数、流控等
        rpc_client_ = new poppy::RpcClient();
    }
    ~RpcClient() {
        delete rpc_client_;
    }
    template <class T>
    bool GetStub(const std::string server, T** stub) {
        MutexLock lock(&host_map_lock_);
        poppy::RpcChannel* channel = NULL;
        HostMap::iterator it = host_map_.find(server);
        if (it != host_map_.end()) {
            channel = it->second;
        } else {
            // 定义 channel，代表通讯通道，每个服务器地址对应一个 channel
            // 可以通过 channel_options 指定一些配置参数
            poppy::RpcChannelOptions channel_options;
            channel = new poppy::RpcChannel(rpc_client_, server, channel_options);
            host_map_[server] = channel;
        }
        *stub = new T(channel);
        return true;
    }
    template <class Stub, class Request, class Response, class Callback>
    bool SendRequest(Stub* stub, void(Stub::*func)(
                    poppy::RpcController*,
                    const Request*, Response*, Callback*),
                    const Request* request, Response* response,
                    int32_t rpc_timeout, int retry_times) {
        // 定义 controller 用于控制本次调用，并设定超时时间（也可以不设置，缺省为10s）
        poppy::RpcController controller;
        controller.SetTimeout(rpc_timeout * 1000L);
        for (int32_t retry = 0; retry < retry_times; ++retry) {
            (stub->*func)(&controller, request, response, NULL);
            if (controller.Failed()) {
                if (retry < retry_times - 1) {
                    LOG(WARNING, "Send failed, retry ...\n");
                    usleep(1000000);
                } else {
                    LOG(WARNING)<<  "SendRequest fail: %s"<< controller.ErrorText().c_str();
                }
            } else {
                return true;
            }
            controller.Reset();
        }
        return false;
    }
    template <class Stub, class Request, class Response, class Callback>
    void AsyncRequest(Stub* stub, void(Stub::*func)(
                    poppy::RpcController*,
                    const Request*, Response*, Callback*),
                    const Request* request, Response* response,
                    boost::function<void (const Request*, Response*, bool, int)> callback,
                    int32_t rpc_timeout, int retry_times) {
        (void)retry_times;
        poppy::RpcController* controller = new poppy::RpcController();
        controller->SetTimeout(rpc_timeout * 1000L);
        google::protobuf::Closure* done = 
            NewClosure(&RpcClient::template RpcCallback<Request, Response, Callback>,
                                          controller, request, response, callback);
        (stub->*func)(controller, request, response, done);
    }
    template <class Request, class Response, class Callback>
    static void RpcCallback(poppy::RpcController* rpc_controller,
                            const Request* request,
                            Response* response,
                            boost::function<void (const Request*, Response*, bool, int)> callback) {

        bool failed = rpc_controller->Failed();
        int error = rpc_controller->ErrorCode();
        if (failed || error) {
            if (error != poppy::RPC_ERROR_SEND_BUFFER_FULL) {
                LOG(WARNING, "RpcCallback: %s\n", rpc_controller->ErrorText().c_str());
            }
        }
        delete rpc_controller;
        callback(request, response, failed, error);
    }
private:
    poppy::RpcClient* rpc_client_;
    typedef std::map<std::string, poppy::RpcChannel*> HostMap;
    HostMap host_map_;
    Mutex host_map_lock_;
};

} // namespace chaoaero

#endif  // RPC_CLIENT_H_
