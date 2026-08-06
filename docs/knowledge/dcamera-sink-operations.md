# sink 端相机操作与授权知识

本文只记录 sink 端本地相机操作、捕获流程和授权安全。source 端状态机见 `dcamera-source-lifecycle.md`，端到端数据流见 `dcamera-data-pipeline.md`。

## 核心组件

| 组件 | 锚点 | 职责 |
|------|------|------|
| `DistributedCameraSinkService` | `sinkservice/src/distributedcamera/` | SA 入口（SA ID 4804），IPC stub |
| `DCameraSinkDev` | `sinkservice/include/distributedcameramgr/` | 封装单个 sink 相机设备 |
| `DCameraSinkController` | `sinkservice/include/distributedcameramgr/dcamera_sink_controller.h` | 核心控制器，管理捕获会话和授权 |
| `DCameraSinkOutput` | `sinkservice/include/distributedcameramgr/dcamera_sink_output.h` | 输出流管理，对接 camera framework |
| `DCameraSinkDataProcess` | `sinkservice/include/distributedcameramgr/` | 数据处理协调 |
| Camera operator | `cameraoperator/` | 本地相机操作封装 |

## 捕获流程

sink 端收到 source 的 `CAPTURE` 命令后的处理链路：

1. `DistributedCameraSinkService` 路由到对应 `DCameraSinkDev`。
2. `DCameraSinkDev` 调用 `DCameraSinkController::StartCapture`。
3. 安全校验（token、账户匹配）。
4. `DCameraSinkController::StartCaptureInner` 并行启动两个 ffrt 任务：
   - **编码器初始化任务**：`output_->StartCapture` 初始化编码流，获取 Surface。
   - **授权等待任务**：`WaitForAuthorization` 等待用户授权结果。
5. 授权等待完成后：
   - 授权失败：发送 `EVENT_AUTHORIZATION_PREPARED(0)`，`captureState_` 置为 `CAPTURE_IDLE`，编码器任务完成后在 `OnEncoderPrepared` 中调用 `output_->StopCapture` 清理资源。
   - 授权成功：发送 `EVENT_AUTHORIZATION_PREPARED(1)`，然后执行 `operator_->PrepareCapture`（相机准备），完成后发送 `EVENT_CAMERA_PREPARED`。
6. 编码器任务完成时发送 `EVENT_ENCODER_PREPARED`；若此时授权已失败，则清理编码器资源而非继续。
7. 编码器和相机均就绪后，`CheckAndCommitCapture` 调用 `operator_->CommitCapture(preparedSurface_)` 将编码器创建的 Surface 传给相机框架，建立 Surface 直连。
8. 视频流（CONTINUOUS_FRAME）：相机框架采集的视频帧直接通过 `previewSurface_`（即编码器的 `encodeProducerSurface_`）灌入编码器，不经过回调中转。编码器输出回调 → `EncodeDataProcess::OnProcessedVideoBuffer` → `DCameraSinkDataProcess::SendDataAsync` → 通过 channel 发送给 source。
9. 照片流（SNAPSHOT_FRAME）：相机框架通过 `photoSurface_` 回调 → `DCameraPhotoSurfaceListener::OnBufferAvailable` → `DCameraSinkOutput::OnPhotoResult` → `DCameraSinkDataProcess::FeedStream` → 不经过编码直接通过 channel 发送给 source。

关键锚点：`sinkservice/src/distributedcameramgr/dcamera_sink_controller.cpp`。

**时序要点**：授权等待和编码器初始化并行执行，`PrepareCapture` 在授权通过后才启动。不应将 `PrepareCapture` 与编码器初始化并行，也不应在授权未决时调用 `PrepareCapture`。视频流使用 Surface 直连模式：编码器通过 `CreateInputSurface` 创建 Surface，经 `CommitCapture` 传给相机框架做 PreviewOutput，相机帧直接灌入编码器，不经过 `OnVideoResult` 回调中转。

## 业务状态

`DCameraSinkController` 维护两个独立状态：

```cpp
enum DcameraBusinessState : int32_t {
    UNKNOWN, IDLE, RUNNING, PAUSING
};

enum DcameraCaptureState : int32_t {
    CAPTURE_IDLE, CAPTURE_STARTING, CAPTURE_RUNNING,
};
```

- `BusinessState` 跟踪整体业务运行状态。
- `CaptureState` 跟踪当前捕获状态。
- 两个状态独立更新，`CAPTURE_STARTING` → `CAPTURE_RUNNING` 是异步过程，需要通过 EventHandler 完成状态切换。

不应在 `CAPTURE_STARTING` 状态再次发起 StartCapture，需要等到 `CAPTURE_RUNNING` 或回到 `CAPTURE_IDLE`。

## 授权与安全

sink 端在执行相机操作前必须完成授权校验：

| 校验项 | 说明 |
|--------|------|
| 设备 token | source 设备必须持有有效 token |
| 账户匹配 | 两端设备必须在同一账户下 |
| 敏感设备标记 | `isSensitive` 标记决定是否需要用户授权弹窗 |
| 授权结果 | 通过 `SetAuthorizationResult` 设置，`granted=true` 才允许操作 |
| 访问监听器 | 通过 `SetAccessListener` 注册超时和授权回调 |

授权流程：
1. sink 收到 source 操作请求。
2. 检查 `isSensitive` 和 `isSameAccount`。
3. 如果需要授权，通过 `IAccessListener` 通知上层弹窗。
4. 用户确认后调用 `SetAuthorizationResult`。
5. 授权通过后继续执行操作。

修改授权逻辑时注意：跳过授权校验会导致安全漏洞，不同安全级别的设备有不同的授权策略。

## 设备生命周期

sink 端相机设备在以下场景需要重新校验或清理：

- 设备上线/下线（`SubscribeLocalHardware` / `UnsubscribeLocalHardware`）。
- 通道断开（`CloseChannel`）。
- 暂停/恢复（`PauseDistributedHardware` / `ResumeDistributedHardware`）。
- 强制停止（`StopDistributedHardware`）。
- 远端服务死亡（death recipient 触发）。

清理应释放相机流、停止编码、关闭通道。不应在清理未完成时重新初始化设备。

## Surface buffer 管理

sink 通过 Surface 与 camera framework 交互：

- `DCameraSurfaceHolder` 管理 Surface 生命周期。
- buffer 回调在 camera framework 线程，回调中不应执行耗时处理。
- buffer 使用引用计数管理，使用完毕后必须释放。
- 连续帧流和拍照流使用不同的 Surface 实例。

## 关联阅读

修改本文涉及内容时，可能还需要阅读：
- 修改授权/安全逻辑 → `dcamera-sink-operations.md`（本文）+ `dcamera-channel-softbus.md`（授权结果通过通道发送）
- 修改编码管线 → `dcamera-data-process.md`（编解码节点）+ `dcamera-data-pipeline.md`（数据流阶段位置）

### 触发条件

遇到以下术语时必须阅读本文：授权、token、ACL、账户匹配、`isSensitive`、`isSameAccount`、`SetAuthorizationResult`、`WaitForAuthorization`、`CAPTURE_STARTING`、`CAPTURE_IDLE`。

修改以下路径时必须阅读本文：`sinkservice/src/distributedcameramgr/dcamera_sink_controller.cpp`、`sinkservice/src/distributedcameramgr/dcamera_sink_output.cpp`、`cameraoperator/`。

## 常见故障排查

| 症状 | 排查方向 | 关键检查点 |
|------|----------|------------|
| StartCapture 无响应 | 授权流程 | `WaitForAuthorization` 是否超时？`SetAuthorizationResult` 是否被调用？`IAccessListener` 是否已注册？ |
| 授权通过但无视频帧 | 编码器初始化 + Surface 直连 | `EVENT_ENCODER_PREPARED` 是否已发送？`CommitCapture(preparedSurface_)` 是否执行？`previewSurface_` 是否有效？ |
| 捕获状态卡在 CAPTURE_STARTING | EventHandler 事件丢失 | `CheckAndCommitCapture` 是否等到 `isEncoderReady_` + `isCameraReady_` + `isAuthorizationReady_` 三者都为 true？ |
| 照片流收不到 | photoSurface 回调 | `DCameraPhotoSurfaceListener::OnBufferAvailable` 是否触发？`OnPhotoResult` 是否被调用？ |
| 授权失败后资源泄漏 | 清理路径 | `OnEncoderPrepared` 中是否调用了 `output_->StopCapture`？`captureState_` 是否回到 `CAPTURE_IDLE`？ |

## 修改前检查

- StartCapture 前是否校验了授权状态？
- 捕获状态是否正确切换（不能跳过 STARTING 直接到 RUNNING）？
- Surface buffer 是否在所有路径上正确释放？
- 设备清理是否覆盖所有触发场景？
- 新增的 IPC 接口是否在 stub 层做了参数校验？

## 测试指引

| 测试目标 | 构建目标 | 测试文件 |
|----------|---------|---------|
| sink 控制器（含授权） | `DCameraSinkMgrTest` | `sinkservice/test/unittest/common/distributedcameramgr/dcamera_sink_controller_test.cpp` |
| sink 输出流 | `DCameraSinkMgrTest` | `sinkservice/test/unittest/common/distributedcameramgr/dcamera_sink_output_test.cpp` |
| 授权控制 | `DCameraSinkMgrTest` | `sinkservice/test/unittest/common/distributedcameramgr/dcamera_sink_access_control_test.cpp` |
| sink 设备生命周期 | `DCameraSinkMgrTest` | `sinkservice/test/unittest/common/distributedcameramgr/dcamera_sink_dev_test.cpp` |

完整捕获流程需要板侧双设备验证。
