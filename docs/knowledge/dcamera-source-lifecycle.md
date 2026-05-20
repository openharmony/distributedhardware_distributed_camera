# source 端状态机与生命周期知识

本文只记录 source 端状态机转换、enable/disable 流程和会话管理。端到端数据流见 `dcamera-data-pipeline.md`，sink 侧操作见 `dcamera-sink-operations.md`。

## 状态机

source 端使用 5 状态有限状态机，状态定义在 `dcamera_source_state.h`：

```
DCAMERA_STATE_INIT (0)
    ↓ DCAMERA_EVENT_REGIST
DCAMERA_STATE_REGIST (1)
    ↓ DCAMERA_EVENT_OPEN
DCAMERA_STATE_OPENED (2)
    ↓ DCAMERA_EVENT_CONFIG_STREAMS
DCAMERA_STATE_CONFIG_STREAM (3)
    ↓ DCAMERA_EVENT_START_CAPTURE
DCAMERA_STATE_CAPTURE (4)
```

状态机实现在 `sourceservice/include/distributedcameramgr/dcamerastate/dcamera_source_state_machine.h`，每个状态有独立的 Execute 实现。

## 状态转换规则

| 当前状态 | 事件 | 目标状态 | 要点 |
|----------|------|----------|------|
| INIT | REGIST | REGIST | 设备注册，加载 HDF 驱动 |
| REGIST | OPEN | OPENED | 打开相机会话，获取能力信息 |
| REGIST | UNREGIST | INIT | 去注册，卸载 HDF 驱动 |
| OPENED | CONFIG_STREAMS | CONFIG_STREAM | 配置流参数，创建通道 |
| OPENED | START_CAPTURE | CAPTURE | 跳过流配置直接开始采集（内部处理） |
| CONFIG_STREAM | START_CAPTURE | CAPTURE | 开始采集 |
| CONFIG_STREAM | RELEASE_STREAMS | OPENED | 释放流配置 |
| CAPTURE | STOP_CAPTURE | OPENED 或 CONFIG_STREAM | 停止采集，根据流状态决定回退 |
| 任意 | UNREGIST | INIT | 强制回到初始状态 |

不能跳过状态执行操作。例如不能在 INIT 状态直接发起 CAPTURE。

## Enable 流程

由分布式硬件框架调用 `IDistributedCameraSource::RegisterDistributedHardware` 触发：

1. 参数校验（devId、dhId、reqId 非空，长度 ≤ 256）。
2. 查找或创建 `DCameraSourceDev`。
3. 触发状态机 REGIST 事件。
4. 通过 `DistributedHardwareFwkKit` 加载 HDF 相机驱动。
5. 获取 sink 侧编解码能力（`GET_INFO` 命令）。
6. 状态机进入 REGIST，等待 OPEN 事件。

关键锚点：`sourceservice/src/distributedcameramgr/dcamera_source_dev.cpp`。

## Disable 流程

由分布式硬件框架调用 `UnregisterDistributedHardware` 触发：

1. 停止所有采集和流。
2. 关闭通道。
3. 卸载 HDF 驱动。
4. 状态机回到 INIT。
5. 从 device map 中移除。

## 会话建立

从 OPENED 到 CAPTURE 的完整链路：

1. **CONFIG_STREAMS**：与 sink 协商流参数（`CHANNEL_NEG`），创建 source/sink 双侧通道。
2. **通道建立**：source 创建 SoftBus client socket，sink 创建 server socket。超时 3 秒。
3. **OPEN_CHANNEL**：source 向 sink 发送打开通道命令。
4. **START_CAPTURE**：source 向 sink 发送采集命令，sink 开始本地相机采集。

会话管理锚点：`sourceservice/src/distributedcameramgr/dcameracontrol/dcamera_source_controller.cpp`。

## HDF 集成

source 通过 HDF 虚拟相机驱动与相机框架对接：

- `DCameraProviderCallbackImpl`：HDF 服务回调，实现在 `sourceservice/include/distributedcameramgr/dcamerahdf/dcamera_provider_callback_impl.h`。
- 使用 HDI 接口 `IDCameraProvider`（V1.1）管理相机会话。
- HDF 驱动使能后，相机框架可以发现并使用分布式相机，接口与本地相机一致。

## 修改前检查

- 状态转换是否遵循状态机规则？不能跳状态。
- Enable/Disable 是否有对应的资源创建/销毁？
- HDF 驱动的加载和卸载是否成对？
- 新增的事件类型是否在所有相关状态中都有处理？
- 设备 map 的并发访问是否加锁？

## 测试指引

状态机测试使用 `DCameraSourceState*Test`。设备生命周期测试使用 `DCameraSourceDev*Test`。HDF 集成需要板侧双设备组网验证。
