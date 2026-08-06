# 分布式相机组件指引

## 项目定位

本仓库对应 OpenHarmony `foundation/distributedhardware/distributed_camera`。优先按这些目录定位问题：

- `services/cameraservice/sourceservice/`：主控端（source）服务、状态机、设备管理、HDF 集成。
- `services/cameraservice/sinkservice/`：被控端（sink）服务、本地相机操作、授权管理。
- `services/channel/`：SoftBus 通道适配、数据收发、会话管理。
- `services/data_process/`：编解码、分辨率缩放、格式转换、EIS、帧率控制。
- `services/cameraservice/base/`：source/sink 公共协议和基础定义。
- `services/cameraservice/cameraoperator/`：本地相机操作封装。
- `interfaces/inner_kits/native_cpp/`：source SDK 和 sink SDK 公共接口。
- `common/`：常量、错误码、工具类。
- `sa_profile/`：SA 配置（4803=source, 4804=sink）。

## Where to look

| 任务 | 路径 |
| --- | --- |
| 修改 source 状态机、注册/去注册 | `services/cameraservice/sourceservice/src/distributedcameramgr/` |
| 修改 sink 捕获流程、授权 | `services/cameraservice/sinkservice/src/distributedcameramgr/` |
| 修改通道建立、数据分片 | `services/channel/src/` |
| 修改编解码节点、分辨率缩放 | `services/data_process/src/pipeline_node/`、`services/data_process/src/pipeline/` |
| 修改协议命令格式 | `services/cameraservice/base/include/dcamera_protocol.h` |
| 修改公共 SDK 接口 | `interfaces/inner_kits/native_cpp/` |
| 修改常量/错误码 | `common/include/constants/` |
| 修改本地相机操作封装 | `services/cameraservice/cameraoperator/` |

## 构建和验证

构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行。

```sh
./build.sh --product-name rk3568 --build-target distributed_camera --ccache
```

涉及真实设备组网、SoftBus 通道、编解码或相机框架集成的行为，需要双设备组网证据。提交使用 `git commit -s`。

## 知识索引

稳定背景知识放在 `docs/knowledge/`。改动前按场景读取对应文件：

| 场景 | 先读 |
| --- | --- |
| 数据流方向、source/sink 角色、编解码归属、pipeline 阶段、端到端链路 | `docs/knowledge/dcamera-data-pipeline.md` |
| source 状态机、enable/disable 流程、注册/去注册、HDF 加载、会话建立 | `docs/knowledge/dcamera-source-lifecycle.md` |
| SoftBus 通道类型、会话创建、数据分片、协议命令、通道超时和重连 | `docs/knowledge/dcamera-channel-softbus.md` |
| sink 相机操作、捕获流程、授权安全、token 校验、账户匹配 | `docs/knowledge/dcamera-sink-operations.md` |
| 编解码节点、分辨率缩放、格式转换、EIS、帧率控制、buffer 管理 | `docs/knowledge/dcamera-data-process.md` |

## 改动前声明

改动代码前必须声明以下三项，未声明则不应动手：

1. **任务类别**：本任务属于哪个场景（见知识索引表）。
2. **已读文档**：已阅读哪些知识文档（至少阅读对应场景的文档）。
3. **发现的约束**：从文档中发现了哪些与本次改动相关的约束。

## 项目约束

- source 是主控端（发起操作），sink 是被控端（执行本地相机操作）。注意区分两端的数据流方向：sink 侧编码后发送，source 侧接收后解码。
- 状态机有严格转换规则，不能跳过状态执行操作。设备注册必须经过 INIT → REGIST → OPENED → CONFIG_STREAM → CAPTURE 的顺序。
- 通道分三种用途（控制/视频/JPEG），应在控制通道上传输命令协议，在视频/JPEG 通道上传输帧数据，不可混用。
- 编解码使用 AVCodec 框架，buffer 回调在 codec 工作线程，回调中不应执行耗时操作或直接调用 pipeline 的同步接口。
- IPC 参数有大小限制（单次数据最大 50MB，设备 ID 最长 256 字符），新增参数传递时必须在 stub 层做校验。
- SoftBus 数据发送支持分片（最大 100MB），但分片协议头有版本和序列号，新增通道类型或修改分片逻辑时要保持协议兼容。
- DFX 日志使用 `DHLOGD`/`DHLOGI`/`DHLOGW`/`DHLOGE` 四级（对应 `HILOG_DEBUG`/`INFO`/`WARN`/`ERROR`）。每帧回调路径（编码输出回调、Surface buffer 回调、解码输出回调）中建议使用 `DHLOGD`，避免使用 `DHLOGI` 及以上级别，以免影响实时性能。

## 不可变更

- `interfaces/inner_kits/native_cpp/` 下的公共接口签名和错误码，修改前必须确认向后兼容。

## 修改前需确认

- 修改授权/安全/token 校验逻辑：需确认不影响设备信任等级判断。
- 修改 SoftBus 分片协议头：需确认与旧版本兼容。
- 修改会话名称拼接规则：需确认 source 和 sink 两端一致。
- 修改状态机新增事件：需确认所有相关状态都有处理。

## Done 定义

- 构建通过（`./build.sh --product-name rk3568 --build-target distributed_camera --ccache`）。
- 相关单元测试通过。
- 未引入公共接口签名变更（或已说明兼容性）。
- 无法运行验证时：在回复中说明"未执行编译验证，需在源码根目录执行构建"。

## 常见反模式

- 在 codec 回调中调用 pipeline 同步接口 → 死锁。应通过 EventHandler 转发。
- 在 `CAPTURE_STARTING` 状态重入 StartCapture → 状态混乱。应等 `CAPTURE_RUNNING` 或 `CAPTURE_IDLE`。
- 在控制通道上发视频帧 / 在视频通道上发命令 → 数据错乱。三种通道类型应各司其职。
- 在 sink 管线配置超过 1920×1920 的分辨率 → 编码失败。sink 最大分辨率 1920×1920。
- 在 `OnShutDown` 回调中阻塞等待 → SoftBus 线程池耗尽。应异步触发清理。
- 在清理未完成时重新初始化设备 → 资源泄漏。应等清理完成后再初始化。
- 跳过授权校验直接执行相机操作 → 安全漏洞。必须通过 `WaitForAuthorization` 等待授权结果。
