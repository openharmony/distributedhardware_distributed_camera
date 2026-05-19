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
3. 安全校验（授权、token、账户匹配）。
4. `DCameraSinkController` 协调 `DCameraSinkOutput` 初始化相机流。
5. `DCameraSinkOutput` 通过 camera framework 创建 capture session。
6. 相机数据通过 Surface buffer 回调到 `DCameraSinkOutput::OnVideoResult` / `OnPhotoResult`。
7. 数据交给 `DCameraSinkDataProcess` 进入编码 pipeline。
8. 编码后通过 channel 发送给 source。

关键锚点：`sinkservice/src/distributedcameramgr/dcamera_sink_controller.cpp`。

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

不要在 `CAPTURE_STARTING` 状态再次发起 StartCapture，需要等到 `CAPTURE_RUNNING` 或回到 `CAPTURE_IDLE`。

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

清理应释放相机流、停止编码、关闭通道。不要在清理未完成时重新初始化设备。

## Surface buffer 管理

sink 通过 Surface 与 camera framework 交互：

- `DCameraSurfaceHolder` 管理 Surface 生命周期。
- buffer 回调在 camera framework 线程，不要在回调中做耗时处理。
- buffer 使用引用计数管理，使用完毕后必须释放。
- 连续帧流和拍照流使用不同的 Surface 实例。

## 修改前检查

- StartCapture 前是否校验了授权状态？
- 捕获状态是否正确切换（不能跳过 STARTING 直接到 RUNNING）？
- Surface buffer 是否在所有路径上正确释放？
- 设备清理是否覆盖所有触发场景？
- 新增的 IPC 接口是否在 stub 层做了参数校验？

## 测试指引

sink 控制器测试在 `sinkservice/test/`。相机操作测试需要 mock camera framework。授权流程测试使用 `DCameraSinkController*Test`。完整捕获流程需要板侧双设备验证。
