#ifndef MMEDIA_RTMP_HANDSHAKE_H
#define MMEDIA_RTMP_HANDSHAKE_H

#include <cstdint>
#include "network/net/TcpConnection.h"

namespace tmms {
    namespace mm {

        // 摘要长度
        const int32_t kDigestLength = 32;

        // rtmp握手包大小
        const int32_t kRtmpHandShakePacketSize = 1536;

        // rtmp版本
        const uint8_t kRtmpVersion = '\x03';

        enum RtmpHandshakeState {
            kHandShakeInit = 0,
            // client
            kHandShakePostC0C1,
            kHandShakeWaitS0S1,
            kHandShakePostC2,
            kHandShakeWaitS2,
            kHandShakeDoing,

            // server
            kHandShakeWaitC0C1,
            kHandShakePostS0S1,
            kHandShakePostS2,
            kHandShakeWaitC2,

            kHandShakeDone,
        };

        using namespace tmms::network;

        class RtmpHandShake {
            public:
                RtmpHandShake(const TcpConnectionPtr& conn, bool is_client = false);
                ~RtmpHandShake() = default;

                void start();
                int32_t handshake(MsgBuffer& buf);
                void writeComplete();

            private:
                uint8_t genRandom();
                void createC1S1();
                int32_t checkC1S1(const char* data, int len);
                void sendC1S1();

                void createC2S2(const char* data, int bytes, int offset);
                bool checkC2S2(const char* data, int len);
                void sendC2S2();

            private:
                std::weak_ptr<TcpConnection> connection_;
                bool is_client_ { false};
                bool is_complex_handshake_ { true };
                uint8_t digest_[kDigestLength];
                uint8_t C1S1_[kRtmpHandShakePacketSize + 1] = { 0};
                uint8_t C2S2_[kRtmpHandShakePacketSize] = { 0 }; 
                RtmpHandshakeState state_ { kHandShakeInit };
        };
    }
}


#endif