# 编解码与数据处理管线知识

本文只记录编解码 pipeline 的节点编排、buffer 管理和线程模型。端到端数据流见 `dcamera-data-pipeline.md`，通道传输见 `dcamera-channel-softbus.md`。

## 管线架构

数据处理使用 pipeline 模式，source 和 sink 各有一条独立管线：

**sink 管线（编码方向）**：`DCameraPipelineSink`
```
相机原始帧 → EncodeDataProcess → 编码后数据 → 通道发送
```

**source 管线（解码方向）**：`DCameraPipelineSource`
```
通道接收数据 → DecodeDataProcess → ScaleConvertProcess → [可选] EISDataProcess → 输出
```

管线接口定义在 `data_process/include/interfaces/idata_process_pipeline.h`，每个节点继承 `AbstractDataProcess`。

## 分辨率限制

| 维度 | source pipeline | sink pipeline | 锚点 |
|------|-----------------|---------------|------|
| 最小分辨率 | 320×240 | 320×240 | `dcamera_pipeline_source.h` / `dcamera_pipeline_sink.h` |
| 最大分辨率 | 4160×3120 | 1920×1920 | 同上 |
| 帧率范围 | 0-30 fps | 0-30 fps | 同上 |

source 的最大分辨率（4160×3120）大于 sink（1920×1920），因为 source 需要支持高分辨率拍照输出。不要在 sink 管线中配置超过 1920×1920 的分辨率。

## 编码节点（EncodeDataProcess）

- 位置：`data_process/include/pipeline_node/multimedia_codec/encoder/`
- 使用 AVCodec 视频编码器。
- IDR 帧间隔：2000ms。
- 支持动态码率调整（根据网络状况）。
- 编码 buffer 使用循环队列管理，需要同步等待可用 buffer。

编码回调在 codec 工作线程中执行，不要在回调中调用 pipeline 的同步接口。

## 解码节点（DecodeDataProcess）

- 位置：`data_process/include/pipeline_node/multimedia_codec/decoder/`
- 使用 AVCodec 视频解码器。
- 支持格式：YUV420、NV12、RGBA32。
- 使用 Surface 渲染模式支持硬件加速。
- 输入/输出 buffer 队列有大小限制。

## 缩放与格式转换（ScaleConvertProcess）

- 位置：`data_process/include/pipeline_node/scale_conversion/`
- 使用 FFmpeg libswscale。
- 支持格式：YUV420、NV12、RGBA、P010。
- 支持裁剪和旋转。
- 临时 buffer 按 16 字节对齐。

修改格式转换逻辑时注意：新增像素格式需要在 `image_common_type.h` 中注册，并在 pipeline 创建时正确映射到 FFmpeg 格式常量。

## EIS 节点（EISDataProcess）

- 位置：`data_process/include/pipeline_node/eis/`
- 电子防抖，可选节点。
- 依赖 IMU 传感器数据（通过 `DCameraIMUSensor` 获取）。
- 仅在 source pipeline 中使用。

## FPS 控制节点（FpsControllerProcess）

- 位置：`data_process/include/pipeline_node/fpscontroller/`
- 控制输出帧率，避免超出目标帧率。
- 在 source pipeline 中使用，配合 `FeedingSmoother` 做帧率平滑。

## Buffer 管理

| buffer 类型 | 管理方式 | 要点 |
|-------------|----------|------|
| Surface buffer | camera framework / codec 框架 | 回调线程归还，不要跨线程持有 |
| 编码输入 buffer | 循环队列 + 条件变量 | 队列满时阻塞等待 |
| 编码输出 buffer | AVCodec 输出回调 | 异步获取，需要同步到 pipeline 线程 |
| 缩放临时 buffer | FFmpeg 对齐分配 | 16 字节对齐，使用后立即释放 |
| 通道发送 buffer | DataBuffer 引用计数 | 发送完成后释放引用 |

不要在 codec 回调线程中直接分配大内存或执行 pipeline 同步操作，应通过 EventHandler 转发。

## 线程模型

| 线程 | 职责 | 要点 |
|------|------|------|
| pipeline 事件线程 | source pipeline 事件处理 | `DCameraPipelineSource::StartEventHandler` 创建 |
| codec 工作线程 | 编解码回调 | AVCodec 框架管理 |
| SoftBus 线程池 | 数据收发回调 | 不要在回调中阻塞 |
| 主线程 | 操作入口 | 启停采集、配置参数 |
| IMU 传感器线程 | EIS 传感器数据 | 仅 EIS 启用时活跃 |

跨线程共享状态必须加锁。pipeline 的 `ProcessData` 和 `OnError` 可能从不同线程调用。

## 管线创建参数

管线创建时需要传入 `VideoConfigParams`（source 配置和 target 配置）：

- `sourceConfig`：远端或输入的编码参数。
- `targetConfig`：本地或输出的解码参数。
- 两端参数在 `CHANNEL_NEG` 阶段协商确定。

修改管线参数时，确保 source 和 sink 两端的协商结果一致。参数不匹配会导致编解码失败。

## 修改前检查

- 新增 pipeline 节点是否插在正确位置（sink 编码前 / source 解码后）？
- 分辨率参数是否在管线限制范围内？
- codec 回调中是否有阻塞操作？
- 跨线程状态访问是否加锁？
- 新增像素格式是否在 `image_common_type.h` 中注册？
- 管线参数是否与 CHANNEL_NEG 协商结果一致？

## 测试指引

编解码单元测试在 `data_process/test/`。缩放和格式转换使用 `ScaleConvertProcess*Test`。编解码使用 mock 的 AVCodec 接口。完整 pipeline 测试需要板侧双设备验证真实编解码路径。
