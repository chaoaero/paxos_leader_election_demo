/*==================================================================
*   Copyright (C) 2016 All rights reserved.
*   
*   filename:     paxoslease_server_impl.cc
*   author:       Meng Weichao
*   created:      2016/06/30
*   description:  
*
================================================================*/
#include "paxoslease_server_impl.h"
#include "common/base/string/string_number.h"
#include "common/base/closure.h"
#include <string>

PaxosLeaseServerImpl::PaxosLeaseServerImpl(vector<string>& cluster_members, 
            string& server_address, 
            uint32_t acquire_lease_timeout, 
            uint32_t max_lease_time ): cluster_members_(cluster_members), server_address_(server_address), acquire_lease_timeout_(acquire_lease_timeout), max_lease_time_(max_lease_time)  {
    uint32_t pos = server_address.find(":");
    string str_port = server_address.substr(pos);
    StringToNumber(str_port, &node_id_); // we simply use port as node id for instance
    
}

void PaxosLeaseServerImpl::Init() {
    proposer_.Init(rpc_client_, acquire_lease_timeout_, max_lease_time_, node_id_);
    //acceptor_.Init();
    learner_.Init(rpc_client_);
    learner_.set_lease_timeout_cb(NewPermanentClosure(on_lease_timeout));
    learner_.set_learn_lease_cb(NewPermanentClosure(on_learn_lease));
    proposer_.start_acquire_lease();
}

void PaxosLeaseServerImpl::prepare(google::protobuf::RpcController* controller, const PrepareRequest* request, PrepareResponse* response, google::protobuf::Closure* done) {
    acceptor_.on_prepare_request(request, response);
    done->Run();
}

void PaxosLeaseServerImpl::propose(google::protobuf::RpcController* controller, const ProposeRequest* request, ProposeResponse* response, google::protobuf::Closure* done) {
    acceptor_.on_propose_request(request, response);
    done->Run();
} 

void PaxosLeaseServerImpl::learn(google::protobuf::RpcController* controller, const LearnRequest* request, poppy::EmptyResponse* response, google::protobuf::Closure* done) {
    learner_.on_learn_request(request, response);
    done->Run();
}

void PaxosLeaseServerImpl::master(google::protobuf::RpcController* controller, const poppy::EmptyRequest* request, LeaderResponse* response, google::protobuf::Closure* done) {
    uint64_t leader_id = learner_.get_lease_owner(); 
    response->set_leader_id(leader_id);
    done->Run();
}


bool PaxosLeaseServerImpl::is_lease_owner() {
    return learner_.is_lease_owner();
}

void PaxosLeaseServerImpl::on_learn_lease() {
    if(!is_lease_owner()) {
        proposer_.stop_acquire_lease();
    }

}

void PaxosLeaseServerImpl::on_lease_timeout() {
    proposer_.start_acquire_lease();

}

//transtoleader need addtask
