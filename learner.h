/*==================================================================
*   Copyright (C) 2016 All rights reserved.
*   
*   filename:     learner.h
*   author:       Meng Weichao
*   created:      2016/07/03
*   description:  
*
================================================================*/
#ifndef __LEARNER_H__
#define __LEARNER_H__
#include "paxos_leader_election/commom/rpc_client.h"
#include "paxos_leader_election/paxos_lease.pb.h"
#include "common/system/timer/timer_manager.h" 
#include "common/base/closure.h"
#include "common/system/concurrency/atomic/atomic.h"


#include <iostream>
class Learner {

public:
    Learner(chaoaero::RpcClient& client);
    ~Learner();
    bool is_lease_owner() {
        check_lease();
        return is_lease_owner_;
    }
    uint64_t get_lease_owner() {
        check_lease();
        return lease_owner_;
    }
    void set_learn_lease_cb(Closure<void> *cb);
    void set_lease_timeout_cb(Closure<void> *cb);
    void on_learn_request(const LearnRequest* request, poppy::EmptyResponse* response);
    void check_lease();
    void on_lease_timeout();

private:
    bool learned_;
    uint64_t lease_owner_;
    uint64_t expire_time_;
    TimerManager timer_manager_; 
    chaoaero::RpcClient rpc_client_;
    Closure<void>* on_learn_lease_cb_;
    Closure<void>* on_lease_timeout_cb_;
    Atomic<uint64_t> lease_timeout_timer_id_;


};
#endif //__LEARNER_H__
