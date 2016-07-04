proto_library(
    name = 'paxos_lease_proto',
    srcs = [
        'paxos_lease.proto'
    ],
    deps = [
        '//poppy:rpc_option_proto',
        '//poppy:rpc_message_proto',
    ],
)

cc_binary(
    name = 'swim_server',
    srcs = [
        'swim_server.cc',
        'swim_server_impl.cc',
    ], 
    deps = [
        ':paxos_lease_proto',
        '#pthread',
        '//boost:boost_regex',
        '//boost:boost_thread',
        '//boost:boost_system',
        '//boost:boost_date_time',
        '//common/base/string:string',
        '//poppy:poppy',
        '//thirdparty/gflags:gflags',
        '//thirdparty/glog:glog',
        '//common/system/concurrency:concurrency',
        '//common/system/concurrency:sync_object',
        '//common/system/net:ip_address',
        '//common/system/time:time',
        '//common/system/timer:timer',
    ],
)

