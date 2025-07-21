/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "dcamera_time_statistician.h"
#include "distributed_hardware_log.h"
#include "distributed_camera_constants.h"
#include <memory>

namespace OHOS {
namespace DistributedHardware {
void DCameraTimeStatistician::CalProcessTime(const std::shared_ptr<IFeedableData>& data)
{
    CHECK_AND_RETURN_LOG(data == nullptr, "data is nullptr");
    TimeStatistician::CalProcessTime(data);
    std::shared_ptr<DataBuffer> dataBuffer = std::reinterpret_pointer_cast<DataBuffer>(data);
    DCameraFrameInfo frameInfo = dataBuffer->frameInfo_;
    int64_t encode = frameInfo.timePonit.finishEncode - frameInfo.timePonit.startEncode;
    int64_t trans = frameInfo.timePonit.recv - frameInfo.timePonit.send + frameInfo.offset;
    int64_t decode = frameInfo.timePonit.finishDecode - frameInfo.timePonit.startDecode;
    int64_t decode2Scale = frameInfo.timePonit.startScale - frameInfo.timePonit.finishDecode;
    int64_t scale = frameInfo.timePonit.finishScale - frameInfo.timePonit.startScale;
    int64_t recv2Feed = frameInfo.timePonit.startSmooth - frameInfo.timePonit.recv;
    SetFrameIndex(frameInfo.index);
    averEncodeTime_ = CalAverValue(encode, encodeTimeSum_);
    averTransTime_ = CalAverValue(trans, transTimeSum_);
    averDecodeTime_ = CalAverValue(decode, decodeTimeSum_);
    averDecode2ScaleTime_ = CalAverValue(decode2Scale, decode2ScaleTimeSum_);
    averScaleTime_ = CalAverValue(scale, scaleTimeSum_);
    averRecv2FeedTime_ = CalAverValue(recv2Feed, recv2FeedTimeSum_);
    SetRecvTime(frameInfo.timePonit.recv);
    DHLOGD("Encode %{public}" PRId64", trans %{public}" PRId64", decode %{public}" PRId64", decode2Scale %{public}"
        PRId64", scale %{public}" PRId64", recv2Feed %{public}" PRId64", averEncode %{public}" PRId64
        " averTrans %{public}" PRId64", averDecode %{public}" PRId64", averDecode2Scale %{public}" PRId64
        ", averScale %{public}" PRId64", averRecv2Feed %{public}" PRId64, encode, trans,
        decode, decode2Scale, scale, recv2Feed, averEncodeTime_, averTransTime_, averDecodeTime_,
        averDecode2ScaleTime_, averScaleTime_, averRecv2FeedTime_);
}

void DCameraTimeStatistician::CalWholeProcessTime(const std::shared_ptr<DataBuffer>& data)
{
    CHECK_AND_RETURN_LOG(data == nullptr, "data is nullptr");
    DCameraFrameInfo frameInfo = data->frameInfo_;
    int64_t smooth = frameInfo.timePonit.finishSmooth - frameInfo.timePonit.startSmooth;
    int64_t sink = frameInfo.timePonit.send - frameInfo.timePonit.startEncode;
    int64_t source = frameInfo.timePonit.finishSmooth - frameInfo.timePonit.recv;
    int64_t whole = frameInfo.timePonit.finishSmooth - frameInfo.timePonit.startEncode + frameInfo.offset;
    int64_t self = frameInfo.timePonit.finishSmooth - frameInfo.timePonit.finishDecode;
    averSmoothTime_ = CalAverValue(smooth, smoothTimeSum_);
    averSourceTime_ = CalAverValue(source, sourceTimeSum_);
    averWholeTime_ = CalAverValue(whole, wholeTimeSum_);
    DHLOGD("Smooth %{public}" PRId64", sink %{public}" PRId64", source %{public}" PRId64", whole %{public}" PRId64
        ", self %{public}" PRId64", averSmooth %{public}" PRId64", averSource %{public}" PRId64", averWhole %{public}"
        PRId64, smooth, sink, source, whole, self, averSmoothTime_, averSourceTime_, averWholeTime_);
}

void DCameraTimeStatistician::SetFrameIndex(const int32_t index)
{
    frameIndex_ = index;
}

int64_t DCameraTimeStatistician::CalAverValue(int64_t& value, int64_t& valueSum)
{
    if (frameIndex_ == FRAME_HEAD) {
        return 0;
    }
    if (INT64_MAX - valueSum < value) {
        DHLOGD("CalAverValue addition overflow.");
        return 0;
    }
    valueSum += value;
    return valueSum / frameIndex_;
}

int64_t DCameraTimeStatistician::GetAverEncodeTime()
{
    return averEncodeTime_;
}

int64_t DCameraTimeStatistician::GetAverTransTime()
{
    return averTransTime_;
}

int64_t DCameraTimeStatistician::GetAverDecodeTime()
{
    return averDecodeTime_;
}

int64_t DCameraTimeStatistician::GetAverDecode2ScaleTime()
{
    return averDecode2ScaleTime_;
}

int64_t DCameraTimeStatistician::GetAverScaleTime()
{
    return averScaleTime_;
}

int64_t DCameraTimeStatistician::GetAverRecv2FeedTime()
{
    return averRecv2FeedTime_;
}

int64_t DCameraTimeStatistician::GetAverSmoothTime()
{
    return averSmoothTime_;
}

int64_t DCameraTimeStatistician::GetRecv2FeedTime()
{
    return averRecv2FeedTime_;
}

int64_t DCameraTimeStatistician::GetAverSourceTime()
{
    return averSourceTime_;
}

int64_t DCameraTimeStatistician::GetAverWholeTime()
{
    return averWholeTime_;
}

int64_t DCameraTimeStatistician::GetRecvTime()
{
    return recvTime_;
}

void DCameraTimeStatistician::SetRecvTime(const int64_t recvTime)
{
    recvTime_ = recvTime;
}
} // namespace DistributedHardware
} // namespace OHOS