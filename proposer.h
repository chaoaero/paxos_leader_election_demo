/*==================================================================
*   Copyright (C) 2016 All rights reserved.
*   
*   filename:     proposer.h
*   author:       Meng Weichao
*   created:      2016/07/03
*   description:  
*
================================================================*/
#ifndef __PROPOSER_H__
#define __PROPOSER_H__
#include <iostream>
#include <vector>
#include "paxos_leader_election/commom/rpc_client.h"
#include "paxos_leader_election/paxos_lease.pb.h"
#include "common/system/timer/timer_manager.h" 
#include "common/base/stdext/shared_ptr.h"
#include "common/base/function.h"
#include "common/base/closure.h"
#include "common/system/concurrency/atomic/atomic.h"
#include "common/system/concurrency/mutex.h"

class Proposer {
public:
    Proposer(chaoaero::RpcClient& client, uint64_t acquire_lease_timeout, uint64_t max_lease_time, uint64_t node_id);
    ~Proposer(); 
    bool start_acquire_lease();
    void start_preparing();
    void start_proposing();
    void set_highest_proposal_id(uint64_t highest_proposal_id);
    void get_highest_proposal_id() {
        return highest_proposal_id_;
    }
    void prepare_callback(poppy::RpcController* controller, PrepareRequest* request, PrepareResponse* response, int *succ_count);
    void propose_callback(poppy::RpcController* controller, ProposeRequest* request, ProposeResponse* response, int *succ_count);
    

private:
    TimerManager timer_manager_;
    bool learned_;
    bool preparing_;
    bool proposing_;
    vector<string> cluster_members_;
    Atomic<uint64_t> highest_proposal_id_;
    uint64_t acquire_lease_timeout_;
    uint64_t node_id_;
    uint64_t lease_owner_;
    uint64_t max_lease_time_;
    uint64_t duration_;
    uint64_t expire_time_;
    Atomic<uint64_t> acquire_lease_timer_id_;
    Atomic<uint64_t> extend_lease_timer_id_;
    chaoaero::RpcClient rpc_client_;
    Mutex mu_;    
};

#endif //__PROPOSER_H__
