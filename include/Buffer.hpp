#ifndef SERIALIZER_BUFFER_HPP
#define SERIALIZER_BUFFER_HPP

#include <functional>
#include "./Serializer.hpp"

typedef int CHK_RET;

class At_VLBuf {
public:
};

class IoBuf : public At_VLBuf {
public:
};

/* 数据流序列化接口 */
typedef SerializerBase<IoBuf> STREAM_IN;

/* 数据流反序列化接口 */
typedef SerializerBase<IoBuf> STREAM_OUT;

typedef std::function<CHK_RET(STREAM_OUT &, STREAM_IN &)> RpcMessageCall;
void RpcCmdRegister(std::string name, RpcMessageCall call);

#endif