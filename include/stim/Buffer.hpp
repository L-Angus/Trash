#ifndef SERIALIZER_BUFFER_HPP
#define SERIALIZER_BUFFER_HPP

#include <functional>
#include <string.h>
#include "./Serializer.hpp"


typedef int CHK_RET;

class At_VLBuf {
public:
    virtual ~At_VLBuf() = default;

    // 虚函数，读写接口需要子类实现
    virtual void write(const void *data, size_t size) = 0;
    virtual void read(void *data, size_t size) = 0;
};

class IoBuf : public At_VLBuf {
private:
    std::vector<char> buffer;  // 用于模拟数据流的缓冲区
    size_t read_pos = 0;

public:
    // 写入数据到缓冲区
    void write(const void *data, size_t size) override {
        const char *byte_data = static_cast<const char *>(data);
        buffer.insert(buffer.end(), byte_data, byte_data + size);
    }

    // 从缓冲区读取数据
    void read(void *data, size_t size) override {
        if (read_pos + size > buffer.size()) {
            throw std::out_of_range("Read beyond buffer size");
        }
        memcpy(data, buffer.data() + read_pos, size);
        read_pos += size;
    }

    // 获取缓冲区内容
    const std::vector<char>& getBuffer() const {
        return buffer;
    }
};

/* 数据流序列化接口 */
typedef SerializerBase<IoBuf> STREAM_IN;

/* 数据流反序列化接口 */
typedef SerializerBase<IoBuf> STREAM_OUT;

typedef std::function<CHK_RET(STREAM_OUT &, STREAM_IN &)> RpcMessageCall;
void RpcCmdRegister(std::string name, RpcMessageCall call);

#endif