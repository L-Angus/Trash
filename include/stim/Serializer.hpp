#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <iostream>
#include <map>

#include "ByteCov.hpp"

typedef enum tagMsgType {
  MSG_PACK_INT = 0xA0,
  MSG_PACK_FLOAT,
  MSG_PACK_STRING,
  MSG_PACK_BOOL,
  MSG_PACK_BIN,
  MSG_PACK_ARRAY,
  MSG_PACK_MAP,
  MSG_PACK_UNSUPPORT
} MsgType;

typedef struct tagMsgInfo {
  MsgType type;
  uint8_t consum;
  uint32_t elements;
  tagMsgInfo(MsgType a, uint8_t b, uint32_t c)
      : type(a), consum(b), elements(c) {}
} MsgInfo;

template <class T, class B, class Enable = void> class Serializer {
public:
  // 改为非侵入式的序列化，反序列化调用
  static void serialize(B &buf, const T &a) { serialize(buf, a); }
  static void deserialize(B &buf, T &a) { deserialize(buf, a); }
};

/* 整数类型数据 */
template <class T, class B>
class Serializer<
    T, B,
    typename std::enable_if<
        std::is_integral<T>::value || std::is_enum<T>::value, void>::type> {
public:
  static void serialize(B &buf, const T &s) {
    uint8_t first;
    if (s >= 0) {
      if (s <= 0x7f) {
        uint8_t byte = (uint8_t)s;
        buf.write((void *)&byte, 1);
        return;
      }

      if (s <= 0xff) {
        uint8_t first = 0xCC;
        uint8_t byte = (uint8_t)s;
        buf.write((void *)&first, 1);
        buf.write((void *)&byte, sizeof(byte));
      }

      if (s <= 0xffff) {
        uint8_t first = 0xCD;
        uint16_t byte = ntohs((uint16_t)s);

        buf.write((void *)&first, 1);
        buf.write((void *)&byte, sizeof(byte));
      }

      if (s <= 0xffffffff) {
        uint8_t first = 0xCE;
        uint32_t byte = ntohl((uint32_t)s);
        buf.write((void *)&first, 1);
        buf.write((void *)&byte, sizeof(byte));
      }
      first = 0xCF;
      uint64_t byte = ntohll((uint64_t)s);
      buf.write((void *)&first, 1);
      buf.write((void *)&byte, sizeof(byte));

      return;
    }

    if (s >= -32) {
      int8_t byte = (int8_t)s;
      buf.write((void *)&byte, 1);
      return;
    }

    if (s >= -128) {
      int8_t first = 0xD0;
      int8_t byte = (int8_t)s;
      buf.write((void *)&first, 1);
      buf.write((void *)&byte, sizeof(byte));
      return;
    }

    if (s >= -32768) {
      int8_t first = 0xD1;
      int16_t byte = ntohs((uint16_t)s);
      buf.write((void *)&first, 1);
      buf.write((void *)&byte, sizeof(byte));
      return;
    }

    if (s >= -0x7fffffff) {
      uint8_t first = 0xD2;
      int32_t byte = ntohl((uint32_t)s);
      buf.write((void *)&first, 1);
      buf.write((void *)&byte, sizeof(byte));
      return;
    }

    first = 0xD3;
    int64_t byte = ntohll((uint64_t)s);
    buf.write((void *)&first, 1);
    buf.write((void *)&byte, sizeof(byte));
    return;
  }

  static void deserialize(B &buf, T &s) {
    uint8_t first = 0;
    buf.read(&first, 1);
    if (first <= 0x7f) {
      s = static_cast<T>(first);
      return;
    }
    if (first >= 0xE0) {
      s = static_cast<T>(first);
      return;
    }
    if (first == 0xCC) {
      uint8_t byte{0};
      buf.read((void *)&byte, sizeof(uint8_t));
      s = static_cast<T>(byte);
      return;
    }
    if (first == 0xCD) {
      uint16_t byte{0};
      buf.read((void *)&byte, sizeof(uint16_t));
      s = static_cast<T>(ntohs(byte));
      return;
    }
    if (first == 0xCE) {
      uint32_t byte{0};
      buf.read((void *)&byte, sizeof(uint32_t));
      s = static_cast<T>(ntohl(byte));
      return;
    }
    if (first == 0xCF) {
      uint64_t byte{0};
      buf.read((void *)&byte, sizeof(uint64_t));
      s = static_cast<T>(ntohll(byte));
      return;
    }
    if (first == 0xD0) {
      int8_t byte{0};
      buf.read((void *)&byte, sizeof(byte));
      s = static_cast<T>(byte);
      return;
    }
    if (first == 0xD1) {
      int16_t byte{0};
      buf.read((void *)&byte, sizeof(byte));
      s = static_cast<T>(ntohs((uint16_t)byte));
      return;
    }
    if (first == 0xD2) {
      int32_t byte{0};
      buf.read((void *)&byte, sizeof(byte));
      s = static_cast<T>(ntohl((uint32_t)byte));
      return;
    }
    if (first == 0xD3) {
      int64_t byte{0};
      buf.read((void *)&byte, sizeof(byte));
      s = static_cast<T>(ntohll((uint64_t)byte));
      return;
    }
  }
};

/* 浮点数据 */
template <typename T, typename B>
class Serializer<
    T, B,
    typename std::enable_if<std::is_floating_point<T>::value, void>::type> {
public:
  static void serialize(B &buf, const T &s) {
    uint8_t byte = 0xCB;
    buf.write((void *)&byte, 1);
    uint64_t byte64_t;
    memcpy(&byte64_t, (void *)&s, sizeof(T));
    byte64_t = ntohll((int64_t)byte64_t);
    buf.write((void *)&byte64_t, sizeof(T));
  }
  static void deserialize(B &buf, T &s) {
    uint8_t first = 0;
    buf.read(&first, 1);
    uint64_t byte64_t{0};
    buf.read(&byte64_t, sizeof(byte64_t));
    byte64_t = ntohll((int64_t)byte64_t);
    memcpy((void *)&s, (void *)&byte64_t, sizeof(T));
  }
};

/* 字符串特化接口 */
template <typename B> class Serializer<std::string, B> {
public:
  static void serialize(B &buf, const std::string &s) {
    int len = s.size();
    if (len <= 31) {
      uint8_t byte = 0xA0 + len;
      buf.write((void *)&byte, sizeof(byte));
      buf.write((void *)s.c_str(), len);
      return;
    }

    if (len <= 255) {
      uint8_t byte = 0xD9;
      uint8_t len8 = len;
      buf.write((void *)&byte, sizeof(byte));
      buf.write((void *)&len8, sizeof(len8));
      buf.write((void *)s.c_str(), len);
      return;
    }

    if (len <= 65535) {
      uint8_t byte = 0xDA;
      uint16_t len16 = len;
      len16 = ntohs(len16);
      buf.write((void *)&byte, sizeof(byte));
      buf.write((void *)&len16, sizeof(len16));
      buf.write((void *)s.c_str(), len);
      return;
    }

    uint8_t byte = 0xDB;
    uint32_t len32 = len;
    len32 = ntohl(len32);
    buf.write((void *)&byte, sizeof(byte));
    buf.write((void *)&len32, sizeof(len32));
    buf.write((void *)s.c_str(), len);
  }
  static void deserialize(B &buf, std::string &s) {
    uint32_t first = 0;
    buf.read(&first, 1);

    if ((first >= 0xA0) && (first <= 0xbf)) {
      uint8_t len = first - 0xA0;
      s.resize(len);
      buf.read((void *)&s[0], len);
      return;
    }
    if (first == 0xD9) {
      uint8_t len = 0;
      buf.read((void *)&len, sizeof(len));
      s.resize(len);
      buf.read((void *)&s[0], len);
      return;
    }
    if (first == 0xDA) {
      uint16_t len = 0;
      buf.read((void *)&len, sizeof(len));
      len = ntohs(len);
      s.resize(len);
      buf.read((void *)&s[0], len);
      return;
    }
    if (first == 0xDB) {
      uint32_t len = 0;
      buf.read((void *)&len, sizeof(len));
      len = ntohl(len);
      s.resize(len);
      buf.read((void *)&s[0], len);
      return;
    }
  }
};

/* vector容器 */
template <typename T, typename B> class Serializer<std::vector<T>, B> {
public:
  static void serialize(B &buf, const std::vector<T> &container) {
    int len = container.size();

    if (len <= 15) {
      uint8_t byte = 0x90 + len;
      buf.write((void *)&byte, sizeof(byte));
    } else if (len <= 65535) {
      uint8_t byte = 0xDC;
      uint16_t l = ntohs((uint16_t)len);
      buf.write((void *)&byte, sizeof(byte));
      buf.write((void *)&l, sizeof(l));
    } else {
      uint8_t byte = 0xDD;
      uint32_t l = ntohl((uint32_t)len);

      buf.write((void *)&byte, sizeof(byte));
      buf.write((void *)&l, sizeof(l));
    }
    for (const auto &i : container) {
      Serializer<T, B>::serialize(buf, i);
    }
  }

  static void deserialize(B &buf, std::vector<T> &container) {
    int len = 0;
    uint8_t first = 0;
    buf.read(&first, 1);
    if ((first >= 0x90) && (first <= 0x9f)) {
      len = first - 0x90;
    } else if (first == 0xdc) {
      uint16_t l = 0;
      buf.read((void *)&l, sizeof(l));
      len = ntohl(l);
    } else if (first == 0xdd) {
      uint32_t l = 0;
      buf.read((void *)&l, sizeof(l));
      len = ntohl(l);
    }
    if (len > 0) {
      container.resize(len);
      for (int i = 0; i < len; i++) {
        Serializer<T, B>::deserialize(buf, container[i]);
      }
    }
  }
};

template <typename T> constexpr int test_func(typename T::key_type * = nullptr);

/* map容器 */
template <typename K, typename B>
class Serializer<
    K, B,
    typename std::enable_if<std::is_same<decltype(test_func<K>(0)), int>::value,
                            void>::type> {
public:
  typedef typename K::key_type Key;
  typedef typename K::mapped_type Value;
  static void serialize(B &buf, const std::map<Key, Value> &container) {
    int size = container.size();
    if (size <= 15) {
      uint8_t byte = 0x80 + size;
      buf.write((void *)&byte, sizeof(byte));
    } else if (size <= 65535) {
      uint8_t byte = 0xDE;
      uint16_t l = ntohs((uint16_t)size);
      buf.write((void *)&byte, sizeof(byte));
      buf.write((void *)&l, sizeof(l));
    } else {
      uint8_t byte = 0xDF;
      uint32_t l = ntohl((uint32_t)size);
      buf.write((void *)&byte, sizeof(byte));
      buf.write((void *)&l, sizeof(l));
    }
    for (const auto &pair : container) {
      Serializer<Key, B>::serialize(buf, pair.first);
      Serializer<Value, B>::serialize(buf, pair.second);
    }
  }

  static void deserialize(B &buf, std::map<Key, Value> &container) {
    int len = 0;
    uint8_t first = 0;
    buf.read(&first, 1);
    if ((first >= 0x80) && (first <= 0x8f)) {
      len = first - 0x80;
    } else if (first == 0xde) {
      uint16_t l = 0;
      buf.read((void *)&l, sizeof(l));
      len = ntohl(l);
    }
    if (len) {
      for (int i = 0; i < len; i++) {
        Key key;
        Value value;
        Serializer<Key, B>::deserialize(buf, key);
        Serializer<Value, B>::deserialize(buf, value);
        container.insert(std::make_pair(std::move(key), value));
      }
    }
  }
};

/**
 * @brief
 * B为序列化空间
 */
template <typename B> class SerializerBase : public B {
public:
  /* 子类继承父类构造函数 */
  using B::B;
  virtual ~SerializerBase() = default;
  using SType = SerializerBase<B>;

public:
  MsgInfo CheckType() { return MagPackCheckType(this->data(), this->size()); }

  template <typename T> SerializerBase &operator<<(const T &v) {
    Serializer<T, SType>::serialize(*this, v);
    return *this;
  }
  template <typename T> SerializerBase &operator>>(T &v) {
    Serializer<T, SType>::deserialize(*this, v);
    return *this;
  }
};

#endif
