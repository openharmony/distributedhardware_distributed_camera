# SoftBus 通道与协议知识

本文只记录通道架构、SoftBus 会话管理、数据分片和协议命令格式。数据流阶段见 `dcamera-data-pipeline.md`，编解码见 `dcamera-data-process.md`。

## 通道角色

| 角色 | 创建方式 | 代码锚点 |
|------|----------|----------|
| sink（server） | `CreatSoftBusSinkSocketServer` 等待连接 | `channel/src/dcamera_channel_sink_impl.cpp` |
| source（client） | `CreateSoftBusSourceSocketClient` 主动连接 | `channel/src/dcamera_channel_source_impl.cpp` |

适配器单例：`DCameraSoftbusAdapter`，在 `channel/include/dcamera_softbus_adapter.h`。

## 通道类型与会话模式

每对 source-sink 维护多条通道，对应不同 SoftBus 会话模式：

| 通道 | 会话模式 | 传输方式 | 用途 |
|------|----------|----------|------|
| 控制通道 | `DCAMERA_SESSION_MODE_CTRL` | bytes | 协议命令（GET_INFO、CAPTURE 等） |
| 视频通道 | `DCAMERA_SESSION_MODE_VIDEO` | stream | 连续视频帧（CONTINUOUS_FRAME） |
| JPEG 通道 | bytes | 单帧拍照数据（SNAPSHOT_FRAME） |

不要在控制通道上传输视频帧，也不要在视频通道上传输命令协议。新增通道类型时必须在 `ICameraChannel` 接口和 `DCameraSoftbusAdapter` 中同步注册。

## 会话名称

会话名称由设备 ID、dhId 和用途标志拼接而成，格式：`"ohos.dhardware.dcamera" + <devId> + <sessionFlag>`。

- 会话名称最大长度：128 字节（`DCAMERA_SESSION_NAME_MAX_LEN`）。
- 修改会话名称拼接规则时必须确保 source 和 sink 两端一致。

## 数据分片

SoftBus 单次传输有大小限制，大数据（视频帧）需要分片：

- 最大支持总数据量：100MB（`DATABUFF_MAX_SIZE`）。
- 分片协议头包含：版本号、标志位、序列号、数据总长度。
- 接收端按序列号重组，收齐后交给上层。
- 分片逻辑在 `DCameraSoftbusSession` 中实现（`channel/include/dcamera_softbus_session.h`）。

修改分片逻辑时注意：协议头格式变更会导致两端不兼容，新增字段应追加在尾部。

## 数据收发回调

SoftBus 适配器注册的回调：

| 回调 | 触发时机 | 要点 |
|------|----------|------|
| `SourceOnBind` / `SinkOnBind` | socket 连接建立 | 校验 peerDevId 和 sessionName |
| `SourceOnBytes` / `SinkOnBytes` | 收到 bytes 数据 | 控制命令通道使用 |
| `SourceOnStream` / `SinkOnStream` | 收到 stream 数据 | 视频流通道使用 |
| `SourceOnShutDown` / `SinkOnShutDown` | 连接断开 | 触发重连或状态清理 |

回调在 SoftBus 线程池中执行，不要在回调中执行耗时操作。需要转发到 EventHandler 处理。

## 通道超时和重连

两种通道的超时机制不同：

| 通道类型 | 超时值 | 代码锚点 | 说明 |
|----------|--------|----------|------|
| 控制通道 | 4 秒 | `DCAMERA_QOS_TYPE_MAX_LATENCY`（`distributed_camera_constants.h`），通过 `g_qosInfo` 传给 SoftBus `Bind()` | SoftBus QoS 最大连接延迟，超过则连接失败 |
| 数据通道（视频/JPEG） | 3 秒 | `TIMEOUT_3_SEC`（`dcamera_source_input.h`），`WaitForOpenChannelCompletion` 中 `wait_for` | 等待视频帧/拍照通道建立完成 |

- 连接断开时通过 `OnShutDown` 回调通知上层，上层根据业务状态决定是否重连。
- sink 侧最大重试次数：3 次（`MAX_RETRY_TIMES_`）。
- source 侧有 death recipient 机制监控远端服务死亡。

## 协议命令

协议定义在 `services/cameraservice/base/include/dcamera_protocol.h`：

| 协议类型 | 命令 | 载体 |
|----------|------|------|
| `OPERATION` | `GET_INFO` | 查询 sink 相机能力 |
| `OPERATION` | `CHANNEL_NEG` | 通道参数协商 |
| `OPERATION` | `OPEN_CHANNEL` / `CLOSE_CHANNEL` | 通道开关 |
| `OPERATION` | `CAPTURE` / `STOP_CAPTURE` | 采集控制 |
| `OPERATION` | `UPDATE_METADATA` | 更新相机参数 |
| `MESSAGE` | `STATE_NOTIFY` | 状态通知 |
| `MESSAGE` | `METADATA_RESULT` | 参数查询结果 |

命令通过 JSON 格式封装，包含 `TYPE` 和 `CMD` 字段。新增命令时必须保持 JSON 格式兼容。

## 修改前检查

- 新增通道类型是否在 source 和 sink 两端同步实现？
- 会话名称拼接是否两端一致？
- 分片协议头变更是否向前兼容？
- 回调中是否有阻塞操作？
- 通道断开时的状态清理是否完整？

## 测试指引

通道传输测试在 `services/channel/test/`。SoftBus 适配器测试使用 mock 的 socket 接口。双设备组网测试验证真实通道建立和数据传输。
