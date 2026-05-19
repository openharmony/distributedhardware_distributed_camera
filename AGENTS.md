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

## 项目约束

- source 是主控端（发起操作），sink 是被控端（执行本地相机操作）。不要混淆两端的数据流方向：sink 侧编码后发送，source 侧接收后解码。
- 状态机有严格转换规则，不能跳过状态执行操作。设备注册必须经过 INIT → REGIST → OPENED → CONFIG_STREAM → CAPTURE 的顺序。
- 通道分三种用途（控制/视频/JPEG），不要在控制通道上传输视频帧数据，也不要在视频通道上传输命令协议。
- 编解码使用 AVCodec 框架，buffer 回调在 codec 工作线程，不要在回调中执行耗时操作或直接调用 pipeline 的同步接口。
- IPC 参数有大小限制（单次数据最大 50MB，设备 ID 最长 256 字符），新增参数传递时必须在 stub 层做校验。
- SoftBus 数据发送支持分片（最大 100MB），但分片协议头有版本和序列号，新增通道类型或修改分片逻辑时要保持协议兼容。
