/*==================================================================
*   Copyright (C) 2016 All rights reserved.
*   
*   filename:     PaxosLease.h
*   author:       Meng Weichao
*   created:      2016/06/29
*   description:  
*
================================================================*/
#ifndef __PAXOSLEASE_H__
#define __PAXOSLEASE_H__
#include <unistd.h>
#include <string>
#include <iostream>
#include "paxoslease_proposer.h"
#include "paxoslease_acceptor.h"
#include "paxoslease_learner.h"
#include "common/system/timer/timer_manager.h"
#include "common/system/concurrency/mutex.h"

#include "paxos_leader_election/commom/thread_pool.h"
#include "paxos_leader_election/paxos_lease.pb.h"
#include "paxos_leader_election/commom/rpc_client.h"

class PaxosLeaseServerImpl: public PAXOSLeaseServer {
public:
    PaxosLeaseServerImpl(vector<string>& cluster_members, 
            string& server_address, 
            uint32_t acquire_lease_timeout, 
            uint32_t max_lease_time ); 
    virtual void prepare(google::protobuf::RpcController* controller, const PrepareRequest* request, PrepareResponse* response, google::protobuf::Closure* done); 
    virtual void propose(google::protobuf::RpcController* controller, const ProposeRequest* request, ProposeResponse* response, google::protobuf::Closure* done); 
    virtual void learn(google::protobuf::RpcController* controller, const LearnRequest* request, poppy::EmptyResponse* response, google::protobuf::Closure* done); 
    virtual void master(google::protobuf::RpcController* controller, const poppy::EmptyRequest* request, LeaderResponse* response, google::protobuf::Closure* done); 

    Init();
    bool is_lease_owner();
    void on_learn_lease();
    void on_lease_timeout();


private:
    vector<string> cluster_members_;
    string server_address_;
    uint32_t acquire_lease_timeout_;
    uint32_t max_lease_time_;
    uint32_t node_id_;

    Proposer proposer_;
    Acceptor acceptor_;
    Learner learner_;

    string accepted_leaseowner_;
    bool is_leaseowner_;
    uint64_t accepted_duration_;
    uint64_t accepted_expiretime_;
    ThreadPool leader_election_;
    chaoaero::RpcClient rpc_client_;

};
#endif //__PAXOSLEASE_H__
