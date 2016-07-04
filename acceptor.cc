/*==================================================================
*   Copyright (C) 2016 All rights reserved.
*   
*   filename:     acceptor.cc
*   author:       Meng Weichao
*   created:      2016/07/04
*   description:  
*
================================================================*/
#include "acceptor.h"
#include "paxos_leader_election/common/timer.h"

Acceptor::Acceptor(): promosied_proposal_id_(0), accepted_(false), accepted_proposal_id_(0), 
accepted_lease_owner_(0), accepted_duration_(0), accepted_expire_time_(0), lease_timeout_timer_id_(0){

} 

Acceptor::~Acceptor() {
    timer_manager_.Stop();
}

void Acceptor::on_prepare_request(cosnt PrepareRequest* request, PrepareResponse* response) {
    int64_t now = get_micros();
    if(accepted_ && accepted_expire_time_ < now) {
        if(lease_timeout_timer_id_)
            timer_manager_.RemoveTimer(lease_timeout_timer_id_);
        on_lease_timeout();
    }

    if(request->proposal_id() < promosied_proposal_id_) {
        response->set_accepted(false);    
        response->set_proposal_id(accepted_proposal_id_);
        response->set_accepted_leaseowner(accepted_lease_owner_);
        return;
    } else {
        promosied_proposal_id_ = request->proposal_id();
        if(!accepted_) {
            response->set_proposal_id(request->proposal_id());
        } else {
            response->set_proposal_id(request->proposal_id());
            response->set_accepted_proposal_id(accepted_proposal_id_);
            response->set_accepted_lease_owner(accepted_lease_owner_);
            response->set_accepted_duration(accepted_duration_);
        }

    }

}

void Acceptor::on_propose_request(const ProposeRequest* request, ProposeResponse* response) {
    int64_t now = get_micros();
    if(accepted_ && accepted_expire_time_ < now) {
        if(lease_timeout_timer_id_)
            timer_manager_.RemoveTimer(lease_timeout_timer_id_);
        on_lease_timeout();
    }
    if(request->proposal_id() < promosied_proposal_id_) {
        response->set_vote_for_granted(false); 
    } else {
        accepted_ = true;
        accepted_proposal_id_ = request->proposal_id();
        accepted_lease_owner_ = request->candidate_id();
        accepted_duration_ = request->duration();
        accepted_expire_time_ = accepted_duration_ + get_micros();
        lease_timeout_timer_id_ = timer_manager_.AddOneshotTimer(accepted_expire_time_, Bind(&Acceptor::on_lease_timeout, this));
    }
}

void Acceptor::on_lease_timeout() {
    accepted_ = false;
    accepted_proposal_id_ = 0;
    accepted_leaseowner_ = 0;
    accepted_duration_ = 0;
    accepted_expire_time_ = 0;
}
