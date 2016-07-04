/*==================================================================
*   Copyright (C) 2016 All rights reserved.
*   
*   filename:     leader_election_server.cc
*   author:       Meng Weichao
*   created:      2016/06/30
*   description:  
*
================================================================*/
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include "poppy/rpc_server.h"
#include "common/base/string/string_number.h"
#include "common/base/string/algorithm.h"

// includes from thirdparty
#include "gflags/gflags.h"
#include "glog/logging.h"

#include "paxos_leader_election/paxos_lease.pb.h"

#include "paxoslease_server_impl.h"

//using namespace common;
using namespace std;

DEFINE_string(cluster_members, "10.136.127.187:9996,10.136.127.187:9997,10.136.127.187:9998", "cluster member list");
DEFINE_string(server_address, "0.0.0.0:9996","server address " );
DEFINE_int32(acquire_lease_timeout, 2000, "default acquire lease timeout");
DEFINE_int32(max_lease_time, 7000, "the default max lease time");

int main(int argc, char** argv) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    //google::InitGoogleLogging(argv[0]);

    // Define an rpc server.
    poppy::RpcServerOptions options;
    poppy::RpcServer rpc_server(0, NULL, options);

    vector<string> cluster_members;
    SplitString(FLAGS_cluster_members, ",", &cluster_members);

    PaxosLeaseServerImpl* service = new PaxosLeaseServerImpl(cluster_members, FLAGS_server_address, FLAGS_acquire_lease_timeout, FLAGS_max_lease_time);

    service->Init();

    rpc_server.RegisterService(service);

    if(!rpc_server.Start(FLAGS_server_address)) {
        LOG(WARNING) << "Failed to start server.";
        return EXIT_FAILURE;
    }

    return rpc_server.Run();


}

