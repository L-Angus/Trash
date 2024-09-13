#ifndef RPC_HANDLERS_HPP
#define RPC_HANDLERS_HPP

#include <unordered_map>
#include <string>
#include <functional>

#include "RPCServer.hpp"

class RpcHandler {
public:
    static const std::unordered_map<std::string, RpcMessageCall>& GetHandlers() {
        static const std::unordered_map<std::string, RpcMessageCall> rpcHandlers{
            {"CW", CWRPC},
            {"DT", DTRPC},
            {"MOD", MODRPC}
        };
        return rpcHandlers;
    }

    static RpcMessageCall GetHandler(const std::string& type) {
        const auto& handlers = GetHandlers();
        auto it = handlers.find(type);
        if (it != handlers.end()) {
            return it->second;  // 返回匹配的 handler
        }
        return nullptr;  // 如果没有找到，返回空的 function
    }
};

#endif