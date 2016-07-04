/*==================================================================
*   Copyright (C) 2016 All rights reserved.
*   
*   filename:     learner.cc
*   author:       Meng Weichao
*   created:      2016/07/04
*   description:  
*
================================================================*/
#include "learner.h"
void Learner::Learner(chaoaero::RpcClient& rpc_client): rpc_client_(rpc_client), learned_(false),  lease_owner_(0), expire_time_(0), lease_timeout_timer_id_(0){

} 

void Learner::set_lease_timeout_cb(Closure<void> *cb) {
    on_lease_timeout_cb_ = cb;
}

void Learner::set_learn_lease_cb(Closure<void> *cb) {
    on_learn_lease_cb_ = cb;
}

Learner::~Learner() {
    delete on_learn_lease_cb_;
    delete on_lease_timeout_cb_;
    timer_manager_.Stop();
}


void Learner::on_learn_request(const LearnRequest* request, poppy::EmptyResponse* response) {
    if(learned_ && expire_time_ < get_micros() ) {
        on_lease_timeout();
    }

    uint64_t e_time = get_micros() + request->duration() - 500;
    if(e_time < get_micros() )
        return;
    learned_ = true;
    lease_owner_ = request->leader_id();
    expire_time_ = e_time;
    timer_manager_.ModifyTimer(lease_timeout_timer_id_, expire_time_);
    on_learn_lease_cb_->Run();
}

void Learner::check_lease() {
    if(learned_ && expire_time_ < get_micros() )
        on_lease_timeout();
    
}

void Learner::on_lease_timeout() {
    timer_manager_.RemoveTimer(lease_timeout_timer_id_);
    learned_ = false;
    lease_owner_ = 0;
    expire_time_ = 0;
    lease_timeout_timer_id_ = 0;
    on_lease_timeout_cb_->Run();
}
