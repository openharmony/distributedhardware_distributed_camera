# 分布式相机端到端数据流知识

本文只记录端到端数据流的流向、角色归属和阶段划分。source 状态机见 `dcamera-source-lifecycle.md`，通道细节见 `dcamera-channel-softbus.md`，编解码节点见 `dcamera-data-process.md`。

## 角色与方向

- **source（主控端）**：发起操作、接收远端画面。SA ID = 4803。代码在 `services/cameraservice/sourceservice/`。
- **sink（被控端）**：执行本地相机操作、发送画面。SA ID = 4804。代码在 `services/cameraservice/sinkservice/`。

数据流方向：**本地相机 → sink 编码 → SoftBus 传输 → source 解码 → 显示/拍照输出**。

不要混淆：source 不做编码，sink 不做解码。source 的 pipeline 是解码管线，sink 的 pipeline 是编码管线。

## 端到端阶段

| 阶段 | 锚点 | 要点 |
|------|------|------|
| 本地相机采集 | `cameraoperator/`、`DCameraSinkOutput` | sink 通过 camera framework 获取 Surface buffer。 |
| sink 编码管线 | `data_process/pipeline/dcamera_pipeline_sink` → `EncodeDataProcess` | H.264/H.265 编码，动态码率，IDR 帧间隔 2000ms。 |
| 通道发送 | `channel/dcamera_channel_sink_impl` → `DCameraSoftbusAdapter::SendSofbusStream` | 视频流走 stream 模式，控制命令走 bytes 模式。 |
| source 通道接收 | `channel/dcamera_channel_source_impl` → SoftBus 回调 `SourceOnStream` | 分片重组后交给 pipeline。 |
| source 解码管线 | `data_process/pipeline/dcamera_pipeline_source` → `DecodeDataProcess` → `ScaleConvertProcess` → 可选 `EISDataProcess` | 解码 → 缩放/格式转换 → 可选 EIS。 |
| 输出到 consumer | `DCameraSourceDataProcess` → `DCameraStreamDataProcess` → `FeedingSmoother` → Surface | 帧率平滑后送给显示或拍照 consumer。 |

## 双路流

source 和 sink 之间维护两条数据通道：

| 流类型 | 用途 | sink 侧 | source 侧 |
|--------|------|----------|------------|
| CONTINUOUS_FRAME | 连续视频帧（预览/录像） | 相机持续采集 → 编码 → 发送 | 接收 → 解码 → 显示 |
| SNAPSHOT_FRAME | 单帧拍照 | 拍照回调 → JPEG 发送 | 接收 → 拍照输出 |

两条流使用独立的通道实例，通道状态独立管理。

## 控制流

控制命令（协议命令字符串）通过 bytes 类型通道传输，不经过编解码 pipeline：

| 命令 | 方向 | 作用 |
|------|------|------|
| `GET_INFO` | source → sink | 查询远端相机能力 |
| `CHANNEL_NEG` | source → sink | 协商通道参数 |
| `OPEN_CHANNEL` / `CLOSE_CHANNEL` | source → sink | 建立/断开数据通道 |
| `CAPTURE` / `STOP_CAPTURE` | source → sink | 开始/停止采集 |
| `UPDATE_METADATA` | source → sink | 更新相机参数 |
| `STATE_NOTIFY` | 双向 | 状态变更通知 |

## 修改前检查

- 修改的数据路径是 source 侧还是 sink 侧？编码在 sink，解码在 source。
- 修改是否影响 CONTINUOUS_FRAME 和 SNAPSHOT_FRAME 两条流的独立性？
- 控制命令和数据流是否在各自正确的通道类型上传输？
- 新增 pipeline 节点是否插在正确位置（编码前/解码后）？

## 测试指引

端到端集成使用双设备组网测试。编解码单元测试在 `services/data_process/test/`。通道传输测试在 `services/channel/test/`。sink 相机操作测试在 `services/cameraservice/sinkservice/test/`。
