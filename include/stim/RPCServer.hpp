#ifndef RPC_SERVER_HPP
#define RPC_SERVER_HPP

#include <iostream>
#include "Buffer.hpp"

struct RPCServerHelper {
    void SetFrequency(double freq){
        std::cout << "Here is SetFrequency register R/W: "<< freq << std::endl;
    }
    void SetPower(double power){
        std::cout << "Here is SetPower register R/W: "<< power << std::endl;
    }
};

struct SerializeBody{
    std::map<double, unsigned int> freqSet;
    std::map<double, unsigned int> powerSet;
};


CHK_RET CWRPC(STREAM_IN& inStream, STREAM_OUT& outStream){
    CHK_RET ret = 0;
    SerializeBody body;
    inStream >> body.freqSet >> body.powerSet;
    std::cout << "here is right: "<< body.freqSet.size() << "--" << body.powerSet.size() << std::endl;
    
    RPCServerHelper helper;
    if(body.freqSet.begin()->second){
        helper.SetFrequency(body.freqSet.begin()->first);
    }
    if(body.powerSet.begin()->second){
        helper.SetPower(body.powerSet.begin()->first);
    }

    return ret;
}

CHK_RET DTRPC(STREAM_IN& inStream, STREAM_OUT& outStream){
    CHK_RET ret = 0;
    return ret;
}

CHK_RET MODRPC(STREAM_IN& inStream, STREAM_OUT& outStream){
    CHK_RET ret = 0;
    return ret;
}

#endif