/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SENDER_H_

#define SENDER_H_

#include <media/stagefright/foundation/AHandler.h>

namespace android {

#define LOG_TRANSPORT_STREAM            0
#define ENABLE_RETRANSMISSION           0
#define TRACK_BANDWIDTH                 0

struct ABuffer;
struct ANetworkSession;

struct Sender : public AHandler {
    Sender(const sp<ANetworkSession> &netSession, const sp<AMessage> &notify);

    enum {
        kWhatInitDone,
        kWhatSessionDead,
        kWhatBinaryData,
    };

    enum TransportMode {
        TRANSPORT_UDP,
        TRANSPORT_TCP_INTERLEAVED,
        TRANSPORT_TCP,
    };
    status_t init(
            const char *clientIP, int32_t clientRtp, int32_t clientRtcp,
            TransportMode transportMode);

    status_t finishInit();

    int32_t getRTPPort() const;

    void queuePackets(int64_t timeUs, const sp<ABuffer> &packets);
    void scheduleSendSR();

protected:
    virtual ~Sender();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatQueuePackets,
        kWhatSendSR,
        kWhatRTPNotify,
        kWhatRTCPNotify,
#if ENABLE_RETRANSMISSION
        kWhatRTPRetransmissionNotify,
        kWhatRTCPRetransmissionNotify
#endif
    };

    static const int64_t kSendSRIntervalUs = 10000000ll;

    static const uint32_t kSourceID = 0xdeadbeef;
    static const size_t kMaxHistoryLength = 128;

#if ENABLE_RETRANSMISSION
    static const size_t kRetransmissionPortOffset = 120;
#endif

    sp<ANetworkSession> mNetSession;
    sp<AMessage> mNotify;

    sp<ABuffer> mTSQueue;

    TransportMode mTransportMode;
    AString mClientIP;

    // in TCP mode
    int32_t mRTPChannel;
    int32_t mRTCPChannel;

    // in UDP mode
    int32_t mRTPPort;
    int32_t mRTPSessionID;
    int32_t mRTCPSessionID;

#if ENABLE_RETRANSMISSION
    int32_t mRTPRetransmissionSessionID;
    int32_t mRTCPRetransmissionSessionID;
#endif

    int32_t mClientRTPPort;
    int32_t mClientRTCPPort;
    bool mRTPConnected;
    bool mRTCPConnected;


    int64_t mFirstOutputBufferReadyTimeUs;
    int64_t mFirstOutputBufferSentTimeUs;

    uint32_t mRTPSeqNo;
#if ENABLE_RETRANSMISSION
    uint32_t mRTPRetransmissionSeqNo;
#endif

    uint64_t mLastNTPTime;
    uint32_t mLastRTPTime;
    uint32_t mNumRTPSent;
    uint32_t mNumRTPOctetsSent;
    uint32_t mNumSRsSent;

    bool mSendSRPending;

#if ENABLE_RETRANSMISSION
    List<sp<ABuffer> > mHistory;
    size_t mHistoryLength;
#endif

#if TRACK_BANDWIDTH
    int64_t mFirstPacketTimeUs;
    uint64_t mTotalBytesSent;
#endif

    void onSendSR();
    void addSR(const sp<ABuffer> &buffer);
    void addSDES(const sp<ABuffer> &buffer);
    static uint64_t GetNowNTP();

#if LOG_TRANSPORT_STREAM
    FILE *mLogFile;
#endif

    ssize_t appendTSData(
            const void *data, size_t size, bool timeDiscontinuity, bool flush);

    void onQueuePackets(const sp<ABuffer> &packets);

#if ENABLE_RETRANSMISSION
    status_t parseTSFB(const uint8_t *data, size_t size);
#endif

    status_t parseRTCP(const sp<ABuffer> &buffer);

    status_t sendPacket(int32_t sessionID, const void *data, size_t size);

    void notifyInitDone();
    void notifySessionDead();

    DISALLOW_EVIL_CONSTRUCTORS(Sender);
};

}  // namespace android

#endif  // SENDER_H_
