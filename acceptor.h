/*==================================================================
*   Copyright (C) 2016 All rights reserved.
*   
*   filename:     acceptor.h
*   author:       Meng Weichao
*   created:      2016/07/03
*   description:  
*
================================================================*/
#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__
#include "paxos_leader_election/commom/rpc_client.h"
#include "paxos_leader_election/paxos_lease.pb.h"
#include "common/system/timer/timer_manager.h" 
#include "common/system/concurrency/atomic/atomic.h"
#include <iostream>
class Acceptor {
public:
    Acceptor();
    ~Acceptor();
    void on_prepare_request(const PrepareRequest* request, PrepareResponse* response);
    void on_propose_request(const ProposeRequest* request, ProposeResponse* response);
    void on_lease_timeout();

private:
    TimerManager timer_manager_; 
    uint64_t promosied_proposal_id_;
    bool accepted_;
    uint64_t accepted_proposal_id_;
    uint64_t accepted_lease_owner_;
    uint64_t accepted_duration_;
    uint64_t accepted_expire_time_;

    Atomic<uint64_t> lease_timeout_timer_id_;
    

    chaoaero::RpcClient rpc_client_;

};
#endif //__ACCEPTOR_H__
