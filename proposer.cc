/*==================================================================
*   Copyright (C) 2016 All rights reserved.
*   
*   filename:     proposer.cc
*   author:       Meng Weichao
*   created:      2016/07/04
*   description:  
*
================================================================*/
#include "proposer.h"
#include <boost/scoped_ptr.hpp>
#include "paxos_leader_election/common/timer.h"

Proposer::Proposer(chaoaero::RpcClient& rpc_client, uint64_t acquire_lease_timeout, uint64_t max_lease_time, uint64_t node_id, vector<string>& cluster_members) : rpc_client_(rpc_client), acquire_lease_timeout_(acquire_lease_timeout), max_lease_time_(max_lease_time_), node_id_(node_id), preparing_(false), proposing_(false), acquire_lease_timer_id_(0), extend_lease_timer_id_(0), cluster_members_(cluster_members)  {
    highest_proposal_id_ = node_id_ ;// simply use node id plus seq as ballot number 

}

Proposer::~Proposer() {
    timer_manager_.Stop();
}

// extend lease timer should also call this
Proposer::start_acquire_lease() {
    if(preparing_ || proposing_)
        return;
    start_preparing(); 
}

void Proposer::start_preparing() {
    preparing_ = true;
    proposing_ = false;
    timer_manager_.RemoveTimer(acquire_lease_timer_id_);
    timer_manager_.AddOneshotTimer(max_lease_time_, Bind(&Proposer::start_acquire_lease(), this));
    lease_owner_ = node_id_;
    // self improving highest proposal id
    highest_proposal_id_++;
    // TODO need add the proposing message
    int succ_count = 0; 
    for(vector<string>::const_iterator iter = cluster_members_.begin(); iter != cluster_members_.end(); iter++) {
        PaxosLeaseServerImpl::Stub *stub;
        rpc_client_.GetStub(*iter, &stub);
        boost::scoped_ptr<PaxosLeaseServerImpl::Stub> stub_guard(stub);
        poppy::RpcController* rpc_controller = new poppy::RpcController();
        PrepareRequest *request = new PrepareRequest();
        PrepareResponse *response = new PrepareResponse();
        google::protobuf::Closure* done =  NewClosure(&prepare_callback, rpc_controller, request, response, &succ_count);
        request->set_proposal_id(highest_proposal_id_);
        request->set_candidate_id(node_id_);
        stub->prepare(rpc_controller, request, response, done);
    }
    
    if(succ_count > cluster_members_.size() / 2 + 1) {
        start_proposing();
    } else {
        start_preparing();
    }

}

void Proposer::prepare_callback(poppy::RpcController* controller, PrepareRequest* request, PrepareResponse* response, int* succ_count) {
    MutexLocker lock(mu_); 
    boost::scoped_ptr<PrepareRequest> request_ptr(request);
    boost::scoped_ptr<PrepareResponse> response_ptr(response);
    if(response_ptr->accepted() == false) {
    } else if(response_ptr->has_accepted_proposal_id()) {
        highest_proposal_id_ = response_ptr->accepted_proposal_id();
        lease_owner_ = response_ptr->accepted_leaseowner();
        *succ_count += 1;
    } else {
        lease_owner_ = 0;
        *succ_count += 1;
    }
}

void Proposer::start_proposing() {

    preparing_ = false;
    proposing_ = true;
    duration_ = max_lease_time_;
    expire_time_ = get_micros() + duration_;
    int succ_count = 0;
    for(vector<string>::const_iterator iter = cluster_members_.begin(); iter != cluster_members_.end(); iter++) {
        PaxosLeaseServerImpl::Stub *stub;
        rpc_client_.GetStub(*iter, &stub);
        boost::scoped_ptr<PaxosLeaseServerImpl::Stub> stub_guard(stub);
        poppy::RpcController* rpc_controller = new poppy::RpcController();
        ProposeRequest *request = new ProposeRequest();
        ProposeResponse *response = new ProposeResponse();
        google::protobuf::Closure* done =  NewClosure(&propose_callback, rpc_controller, request, response, &succ_count);
        stub->propose(rpc_controller, request, response, done);
    }

    // send learn message
    if(succ_count > cluster_members_.size() / 2 + 1 && expire_time_ > get_micros() - 500) {
        for(vector<string>::const_iterator iter = cluster_members_.begin(); iter != cluster_members_.end(); iter++) {
            PaxosLeaseServerImpl::Stub *stub;
            rpc_client_.GetStub(*iter, &stub);
            boost::scoped_ptr<PaxosLeaseServerImpl::Stub> stub_guard(stub);
            poppy::RpcController* rpc_controller = new poppy::RpcController();
            LearnRequest* request = new LearnRequest();
            poppy::EmptyResponse() *response = new poppy::EmptyResponse();
            request->set_proposal_id(highest_proposal_id_);
            request->set_leader_id(lease_owner_);
            request->set_duration(expire_time_ - get_micros());
            request->set_expire_time(expire_time_);
            stub->propose(rpc_controller, request, response, NULL);
        }

        // reset timer
        timer_manager_.RemoveTimer(acquire_lease_timer_id_);
        timer_manager_.RemoveTimer(extend_lease_timer_id_);
        extend_lease_timer_id_ = timer_manager_.AddOneshotTimer(get_micros() + (expire_time_ - get_micros() )/ 7);
    } 

}

void Proposer:propose_callback(poppy::RpcController* controller, ProposeRequest* request, ProposeResponse* response, int *succ_count) {
    MutexLocker lock(mu_); 
    boost::scoped_ptr<PrepareRequest> request_ptr(request);
    boost::scoped_ptr<PrepareResponse> response_ptr(response);   
    if(response->vote_for_granted()) 
        *succ_count += 1;
}

