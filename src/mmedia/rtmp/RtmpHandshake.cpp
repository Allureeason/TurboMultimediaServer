#include "RtmpHandshake.h"
#include "base/TTime.h"
#include "mmedia/base/Log.h"

#include <cstdint>
#include <random>
#include <openssl/sha.h>
#include <openssl/hmac.h>

#if OPENSSL_VERSION_NUMBER > 0x10100000L
    #define HMAC_setup(ctx, key, len) ctx = HMAC_CTX_new();HMAC_Init_ex(ctx, key, len, EVP_sha256(), NULL);
    #define HMAC_crunch(ctx, buf, len) HMAC_Update(ctx, buf, len);
    #define HMAC_final(ctx, dig, dlen) HMAC_Final(ctx, dig, &dlen);HMAC_CTX_free(ctx);
#else
    #define HMAC_setup(ctx, key, len) HMAC_CTX_init(&ctx);HMAC_Init_ex(&ctx, key, len, EVP_sha256(), NULL);
    #define HMAC_crunch(ctx, buf, len) HMAC_Update(&ctx, buf, len);
    #define HMAC_final(ctx, dig, dlen) HMAC_Final(&ctx, dig, &dlen);HMAC_CTX_cleanup(&ctx);
#endif

// 简单握手
// 1. C0和S0 由一个字节组成，表示rtmp的版本号。目前使用的版本号为3
// 0 1 2 3 4 5 6 7 
// +-+-+-+-+-+-+-+-+
// |version(4bytes)|
// +-+-+-+-+-+-+-+-+
// C0 and S0 bits
// version: rtmp版本号。

// 2. C1 和 S1 包长度为 1536 字节，结构如下：
// +-+-+-+-+-+-+-+-+
// | time(4 bytes) |
// +-+-+-+-+-+-+-+-+
// | zero(4 bytes) |
// +-+-+-+-+-+-+-+-+
// | random bytes |
// +-+-+-+-+-+-+-+-+
// | random bytes |
// |    (cont)    |
// |    ....      |
// +-+-+-+-+-+-+-+-+
// C1 and S1 bits
// time（4 bytes）：时间戳，取值可以为零或其他任意值。
// zero（4 bytes）：本字段必须为零。
// random （1528 bytes）：本字段可以包含任意数据。由于握手的双方需要区分另一端，此字段填充的数据必须足够随机以防止与其他握手端混淆）

// 3. C2 和 S2 包长度为 1536 字节，分别回应S1和C1。结构如下：
// +-+-+-+-+-+-+-+-+
// | time (4bytes) |
// +-+-+-+-+-+-+-+-+
// | time2 (4bytes)|
// +-+-+-+-+-+-+-+-+
// | random echo   |
// +-+-+-+-+-+-+-+-+
// | random echo  |
// |    (cont)    |
// |    ....      |
// +-+-+-+-+-+-+-+-+
// C2 and S2 bits

// 复杂握手
// 1. C0和S0 由一个字节组成，表示rtmp的版本号。目前使用的版本号为3
// 0 1 2 3 4 5 6 7 
// +-+-+-+-+-+-+-+-+
// |version(4bytes)|
// +-+-+-+-+-+-+-+-+
// C0 and S0 bits
// version: rtmp版本号。

// 2. C1 和 S1 包长度为 1536 字节，除了4字节的时间戳和4字节版本号外，还有764字节的key和764字节的digest。有两种结构，如下：
// +-+-+-+-+-+-+-+-+
// | time (4bytes) |
// +-+-+-+-+-+-+-+-+
// |version(4bytes)|
// +-+-+-+-+-+-+-+-+
// | key(764bytes) |
// +-+-+-+-+-+-+-+-+
// |digest(764bytes)|
// +-+-+-+-+-+-+-+-+
// C1 and S1 bits
// 或者
// +-+-+-+-+-+-+-+-+
// | time (4 bytes) |
// +-+-+-+-+-+-+-+-+
// | version (4 bytes) |
// +-+-+-+-+-+-+-+-+
// | digest (764 bytes) |
// +-+-+-+-+-+-+-+-+
// | key (764 bytes) |
// +-+-+-+-+-+-+-+-+
// C1 and S1 bits
// time（4 bytes）：时间戳，取值可以为零或其他任意值。
// version（4 bytes）：客户端为0x0C, 0x00, 0x0D, 0x0E。服务端为0x0D, 0x0E, 0x0A, 0x0D。
// digest（764 bytes）：密文。
// key（764 bytes）：密钥。


// 3. C2 和 S2 包长度为 1536 字节。结构如下：
// +-+-+-+-+-+-+-+-+
// | random-data (1504 bytes) |
// +-+-+-+-+-+-+-+-+
// | digest-data (32 bytes) |
// +-+-+-+-+-+-+-+-+
// C2 and S2 bits
// random-data (1504 bytes)：随机数据
// digest-data (32 bytes)：random-data的摘要。
// digest-data计算方法：
// S2：先通过C1的digest，计算出key，再用这个key计算random-data的digest。
// C2：先通过S1的digest，计算出key，再用这个key计算random-data的digest


namespace {
    // rtmp服务端version
    static const uint8_t rtmp_server_ver[] = {
        0x0D, 0x0E, 0x0A, 0x0D
    };

    // rtmp客户端version
    static const uint8_t rtmp_client_ver[] = {
        0x0C, 0x00, 0x0D, 0x0E
    };

    // rtmp服务端key
    #define SERVER_KEY_OPEN_PART_LEN 36
    static const uint8_t rtmp_server_key[] = {
        'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
        'F', 'l', 'a', 's', 'h', ' ', 'M', 'e', 'd', 'i', 'a', ' ',
        'S', 'e', 'r', 'v', 'e', 'r', ' ', '0', '0', '1',
        0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1, 0x02,
        0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB, 0x93, 0xB8,
        0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
    };

    // rtmp客户端key
    #define PLAYER_KEY_OPEN_PART_LEN 30
    static const uint8_t rtmp_player_key[] = {
        'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
        'F', 'l', 'a', 's', 'h', ' ', 'P', 'l', 'a', 'y', 'e', 'r', ' ', '0', '0', '1',
        0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1, 0x02,
        0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB, 0x93, 0xB8,
        0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
    };

    // 计算摘要
    void CalculateDigest(const uint8_t *src, int len, int gap, const uint8_t *key, int keylen, uint8_t *dst) {
        uint32_t digest_len = 0;
        #if OPENSSL_VERSION_NUMBER > 0x10100000L
        HMAC_CTX *ctx;
        #else
        HMAC_CTX ctx;
        #endif

        HMAC_setup(ctx, key, keylen);  // 初始化HMAC上下文，传入密钥
        if(gap <= 0) {
            HMAC_crunch(ctx, src, len);  // 一次性处理全部数据
        } else {
            HMAC_crunch(ctx, src, gap);  // 第一段：处理前gap长度数据
            HMAC_crunch(ctx, src + gap + SHA256_DIGEST_LENGTH, 
                        len - gap - SHA256_DIGEST_LENGTH);  // 第二段：跳过gap和SHA256摘要长度后的数据
        }
        HMAC_final(ctx, dst, digest_len);  // 结束HMAC计算，输出摘要到dst
    }

    // 验证摘要
    bool VerifyDigest(uint8_t *buf, int digest_pos, const uint8_t *key, size_t key_len) {
        uint8_t digest[SHA256_DIGEST_LENGTH];
        CalculateDigest(buf, tmms::mm::kRtmpHandShakePacketSize, digest_pos, key, key_len, digest);
        return memcmp(buf + digest_pos, digest, SHA256_DIGEST_LENGTH) == 0; // 比较计算出的摘要和buf中的摘要是否一致
    }

    // 获取摘要偏移量
    int32_t GetDigestOffset(const uint8_t *buf, int off, int mod_val) {
        int32_t offset = 0;
        const uint8_t *ptr = reinterpret_cast<const uint8_t*>(buf + off);
        int32_t res = 0;

        offset = ptr[0] + ptr[1] + ptr[2] + ptr[3];
        res = (offset % mod_val) + (off + 4);
        return res;
    }
}


using namespace tmms::mm;
using namespace tmms::base;

RtmpHandShake::RtmpHandShake(const TcpConnectionPtr& conn, bool is_client) :
    connection_(conn), is_client_(is_client) {
}

void RtmpHandShake::start() {
    // 创建C1/S1
    createC1S1();

    if (is_client_) {
        state_ = kHandShakeWaitS0S1;
        sendC1S1();
    } else {
        state_ = kHandShakeWaitC0C1;
    }
}

uint8_t RtmpHandShake::genRandom() {
    std::mt19937 mt(std::random_device{}());
    std::uniform_int_distribution<> dist(0, 255);
    return dist(mt) % 256;
}


void RtmpHandShake::createC1S1() {
    for (int i = 0; i < kRtmpHandShakePacketSize + 1; i++) {
        C1S1_[i] = genRandom();
    }
    
    // C0 version
    C1S1_[0] = kRtmpVersion;

    // C1 timestamp（4 bytes）：时间戳，取值可以为零或其他任意值。
    memset(C1S1_ + 1, 0x00, 4);

    if (!is_complex_handshake_) {
        // 简单握手
        memset(C1S1_ + 5, 0x00, 4);
    } else {
        // 复杂握手
        // version（4 bytes）：客户端为0x0C, 0x00, 0x0D, 0x0E。服务端为0x0D, 0x0E, 0x0A, 0x0D
        // 计算digest偏移量
        auto offset = GetDigestOffset(C1S1_ + 1, 8, 728);
        // 获取digest数据
        auto digest_data = C1S1_ + 1 + offset;
        if (is_client_) {
            // 客户端version
            memcpy(C1S1_ + 5, rtmp_client_ver, 4);
            // 生成客户端摘要
            CalculateDigest(C1S1_ + 1, kRtmpHandShakePacketSize, offset, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN, digest_data);
        } else {
            // 服务端
            memcpy(C1S1_ + 5, rtmp_server_ver, 4);
            // 生成服务端摘要
            CalculateDigest(C1S1_ + 1, kRtmpHandShakePacketSize, offset, rtmp_server_key, SERVER_KEY_OPEN_PART_LEN, digest_data);
        }

        // 保存摘要digest, 用于后续的C2和S2握手
        memcpy(digest_, (void*)digest_data, SHA256_DIGEST_LENGTH);
    }
}

int32_t RtmpHandShake::checkC1S1(const char* data, int len) {
    // 检查数据长度是否正确
    if (len != kRtmpHandShakePacketSize + 1) {
        RTMPLOG_ERROR << "checkC1S1: data length error, len: " << len;
        return -1;
    }

    // 检查版本号是否正确
    if (data[0] != kRtmpVersion) {
        RTMPLOG_ERROR << "checkC1S1: version error, version: " << data[0];
        return -1;
    }

    // 判断是否是复杂握手
    uint32_t* version = (uint32_t*)(data + 5);
    if (*version == 0) {
        // 简单握手
        is_complex_handshake_ = false;
        return 0;
    }

    uint32_t offset = -1;
    // 复杂握手
    if (is_complex_handshake_) {
        // c1握手数据
        uint8_t* handshake = (uint8_t*)(data + 1);
        // 采用C1S1格式1， 获取digest偏移量
        offset = GetDigestOffset(handshake, 8, 728);
        
        if (is_client_) {
            // 客户端握手
            if (!VerifyDigest(handshake, offset, rtmp_server_key, SERVER_KEY_OPEN_PART_LEN)) {
                // 失败了，再尝试采用C1S1格式2， 获取digest偏移量
                offset = GetDigestOffset(handshake, 764 + 8, 728);
                if (!VerifyDigest(handshake, offset, rtmp_server_key, SERVER_KEY_OPEN_PART_LEN)) {
                    return -1;
                }
            }
        } else {
            // 服务端握手
            if (!VerifyDigest(handshake, offset, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN)) {
                // 失败了，再尝试采用C1S1格式2， 获取digest偏移量
                offset = GetDigestOffset(handshake, 764 + 8, 728);
                if (!VerifyDigest(handshake, offset, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN)) {
                    return -1;
                }
            }
        }
    }

    return offset;
}

void RtmpHandShake::sendC1S1() {
    auto conn = connection_.lock();
    if (!conn) {
        return;
    }
    conn->send(reinterpret_cast<const char*>(C1S1_), kRtmpHandShakePacketSize + 1);
}

void RtmpHandShake::createC2S2(const char* data, int bytes, int offset) {
    for (int i = 0; i < kRtmpHandShakePacketSize; i++) {
        C2S2_[i] = genRandom();
    }

    memcpy(C2S2_, data, 8);

    // 时间戳
    uint32_t timestamp = (uint32_t)TTime::now();
    uint8_t* t = (uint8_t*)&timestamp;
    C2S2_[0] = t[3];
    C2S2_[1] = t[2];
    C2S2_[2] = t[1];
    C2S2_[3] = t[0];

    // 复杂握手
    if (is_complex_handshake_) {
        uint8_t digest[32] = {0};
        if (is_client_) {
            CalculateDigest(digest_, 32, 0, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN, digest);
        } else {
            CalculateDigest(digest_, 32, 0, rtmp_server_key, SERVER_KEY_OPEN_PART_LEN, digest);
        }

        CalculateDigest(C2S2_, kRtmpHandShakePacketSize - 32, 0, digest, 32, &C2S2_[kRtmpHandShakePacketSize - 32]);
    }
}

bool RtmpHandShake::checkC2S2(const char* data, int len) {
    return true;
}

void RtmpHandShake::sendC2S2() {
    auto conn = connection_.lock();
    if (!conn) {
        return;
    }
    conn->send(reinterpret_cast<const char*>(C2S2_), kRtmpHandShakePacketSize);
}

int32_t RtmpHandShake::handshake(MsgBuffer& buf) {
    auto conn = connection_.lock();
    if (!conn) {
        return -1;
    }

    switch (state_) {
        case kHandShakeWaitC0C1: {
            if (buf.readableBytes() < kRtmpHandShakePacketSize + 1) {
                return 1;
            }
            // 收到C0C1

            RTMPLOG_TRACE << "host: " << conn->getPeerAddr().toIpPort() << " recv C0C1";
            // 校验C0C1，获取摘要digest偏移位置offset
            auto offset = checkC1S1(buf.peek(), kRtmpHandShakePacketSize + 1);
            if (offset >= 0) {
                // 校验通过,创建S2
                createC2S2(buf.peek() + 1, kRtmpHandShakePacketSize, offset);
                // 清除buf数据
                buf.retrieve(kRtmpHandShakePacketSize + 1);
                // 状态转换为 发送S0S1
                state_ = kHandShakePostS0S1;
                sendC1S1();
            } else {
                RTMPLOG_ERROR << "host: " << conn->getPeerAddr().toIpPort() << " check C0C1 failed";
                return -1;
            }
            break;
        }
        case kHandShakeWaitC2: {
            if (buf.readableBytes() < kRtmpHandShakePacketSize) {
                return 1;
            }
            // 收到C2
            RTMPLOG_TRACE << "host: " << conn->getPeerAddr().toIpPort() << " recv C2";
            // 校验C2
            if (checkC2S2(buf.peek(), kRtmpHandShakePacketSize)) {
                state_ = kHandShakeDone;
                buf.retrieve(kRtmpHandShakePacketSize);
                RTMPLOG_TRACE << "host: " << conn->getPeerAddr().toIpPort() << " handshake done";
            } else {
                RTMPLOG_ERROR << "host: " << conn->getPeerAddr().toIpPort() << " check C2 failed";
                return -1;
            }
            break;
        }
        case kHandShakeWaitS0S1: {
            if (buf.readableBytes() < kRtmpHandShakePacketSize + 1) {
                return 1;
            }
            // 收到S0S1
            RTMPLOG_TRACE << "host: " << conn->getPeerAddr().toIpPort() << " recv S0S1";
            // 校验S0S1，获取摘要digest偏移位置offset
            auto offset = checkC1S1(buf.peek(), kRtmpHandShakePacketSize + 1);
            if (offset >= 0) {
                // 校验通过,创建C2
                createC2S2(buf.peek() + 1, kRtmpHandShakePacketSize, offset);
                // 清除buf数据
                buf.retrieve(kRtmpHandShakePacketSize + 1);
                if (buf.readableBytes() >= kRtmpHandShakePacketSize) {
                    // 可能已经收到S2了
                    state_ = kHandShakeDoing;
                    sendC2S2();
                    return 2;
                } else {
                    state_ = kHandShakePostC2;
                    sendC2S2();
                }
                // 状态转换为 发送C2
                state_ = kHandShakePostS0S1;
                sendC1S1();
            } else {
                RTMPLOG_ERROR << "host: " << conn->getPeerAddr().toIpPort() << " check S0S1 failed";
                return -1;
            }
            break;
        }
    
    default:
        break;
    }
    return 0;
}

void RtmpHandShake::writeComplete() {
    auto conn = connection_.lock();
    if (!conn) {
        return;
    }
    switch (state_) {
        case kHandShakePostS0S1: {
            RTMPLOG_TRACE << "host: " << conn->getPeerAddr().toIpPort() << " post S0S1";
            // 发送完S0S1,可以直接发送S2
            state_ = kHandShakePostS2;
            sendC2S2();
            break;
        }
        case kHandShakePostS2: {
            RTMPLOG_TRACE << "host: " << conn->getPeerAddr().toIpPort() << " post S2";
            // 发送完S2, 等待C2
            state_ = kHandShakeWaitC2;
            break;
        }
        case kHandShakePostC0C1: {
            RTMPLOG_TRACE << "host: " << conn->getPeerAddr().toIpPort() << " post C0C1";
            state_ = kHandShakeWaitS0S1;
            break;
        }
        case kHandShakePostC2: {
            RTMPLOG_TRACE << "host: " << conn->getPeerAddr().toIpPort() << " post C2";
            state_ = kHandShakeDone;
            break;
        }
        case kHandShakeDoing: {
            RTMPLOG_TRACE << "host: " << conn->getPeerAddr().toIpPort() << " post C2";
            state_ = kHandShakeDone;
            break;
        }
    default:
        break;
    }
}