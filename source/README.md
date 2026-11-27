# OpenThermal 例程使用说明

本目录为 **OpenThermal（INFRARED-PRO / MINI）** 的二次开发例程，基于 PlatformIO + Arduino 框架开发，包含：
- 板端实时热成像采集、热成像图插值放大与推屏显示
- 开关机电源锁定、低电保护与睡眠按键唤醒
- 内置 HTTP 实时热成像网页与 TCP/USB 二进制温度矩阵输出
- 配套 Python 上位机

---

## 1. 目录结构

```text
例程/
├── boards/                 # 板级配置与分区表 (ESP32-S3-F4P2 / ESP32-C3-F4P0, qio)
├── data/                   # LittleFS 文件系统目录
├── lib/                    # 本地扩展库目录
├── src/                    # 固件源码
│   ├── Config.hpp          # 产品型号与硬件引脚宏定义 (默认 INFRARED_PRO)
│   ├── Main.cpp            # 系统初始化与电源/无线/温控主循环
│   ├── Sensor.cpp          # HTPA32 采样、镜像旋转与二进制温度帧发送
│   ├── Gui.cpp             # 内部 热成像图渲染与 ST7789 推屏
│   ├── Network.cpp         # HTTP 实时网页 (/temp, /reboot) 与 TCP 6666 数据流
│   ├── Serial.cpp          # USB CDC 串口温度帧发送
│   ├── Power.hpp           # 2 秒长按锁电、关机睡眠唤醒与电池电压采样
│   ├── Panel.hpp           # LovyanGFX ST7789 SPI + DMA 显示与 CST816S 触摸驱动配置
│   ├── Wireless.hpp        # WiFi AP/STA 管理
│   └── HTPA32.hpp          # HTPA32x32 红外传感器底层驱动
├── application/                    # Python 图形上位机
│   ├── main.py             # Tkinter 界面入口（支持自动安装依赖）
│   ├── link.py             # USB 串口 / WiFi TCP 连接封装
│   └── infrared.py         # 二进制帧解析与 NumPy 伪彩渲染
├── make.ps1                # 一键编译与自动串口烧录脚本
└── platformio.ini          # PlatformIO 工程配置
```

---

## 2. 编译与烧录

在 PowerShell 中进入 `例程` 目录执行 [make.ps1](file:///C:/Users/Administrator/Desktop/OpenThermal/release/%E4%BE%8B%E7%A8%8B/make.ps1)：

```powershell
# 1. 编译固件并询问是否自动识别串口烧录（直接按回车即可烧录）
.\make.ps1

# 2. 编译后不询问直接自动烧录
.\make.ps1 flash

# 3. 跳过编译，直接烧录已有固件
.\make.ps1 flashonly
```

> 首次编译时 PlatformIO 会自动从网络拉取 `uTimerLib` 与 `LovyanGFX@1.1.16`。

---

## 3. 设备按键与屏幕功能

* **开机**：长按电源键 **2 秒**，电源锁存后屏幕亮起，播放 2 秒 `Booting...` 启动动画后进入热成像主画面。
* **主画面布局**（`280×240` 横屏）：
  * 左侧 `240×240`：实时热成像画面（内部 DMA RAM 离屏插值放大 + `classic` 256 级伪彩，已完成水平镜像与横屏 270° 校准）。
  * 右侧栏：自上而下等距显示最高温 `MAX`、最低温 `MIN`、中心温度 `CTR`（单位 ℃，保留 1 位小数）与电池电压 `BAT`（单位 V，保留 1 位小数）。
* **暂停 / 恢复采样**：开机后短按一次电源键可暂停或恢复传感器采样。
* **关机与再次开机**：开机后松开过一次电源键即解锁关机；再次长按电源键 **2 秒**，屏幕熄灭、关闭 WiFi 并断电/进入低功耗唤醒睡眠。无论是否插着 USB，关机后再次长按电源键即可重新开机。
* **低电保护**：开机 `3 秒` 后若电池电压低于 `3.55V`，设备会自动关机保护电池。

---

## 4. 网页端实时查看

设备上电默认开启无密码 WiFi 热点：
* **热点名称**：`INFRARED-xxxxxx`（`xxxxxx` 为设备 MAC 后 6 位）
* **默认地址**：`http://192.168.4.1/`

用电脑或手机连接热点后，在浏览器打开上述地址即可查看实时热成像画面（前端 JS 每 `80ms` 轮询 `/temp`，本地平滑插值放大至 `320×320` 绘制，并显示 `MAX` / `MIN` / `CTR`）：
* `GET /`：实时热成像网页
* `GET /temp`：返回当前帧 `32×32` 摄氏度温度矩阵 JSON
* `GET /reboot`：重启设备

---

## 5. Python 上位机使用

进入 `application` 目录直接运行 `main.py`（无需手动 `pip install`，首次启动会自动检测并安装 `numpy` 和 `pyserial`）：

```powershell
cd application
python main.py
```

在界面中可自由切换连接方式：
* **USB 串口**：默认自动识别 `VID:303A PID:1001`（波特率 `921600`），也可点击 **刷新** 手动选择 COM 口。
* **WiFi TCP**：连接设备热点后，保持默认 `192.168.4.1 : 6666` 点击 **连接并开始显示**。

---

## 6. 二进制温度矩阵通信协议

设备通过 **USB 串口（921600）** 与 **TCP 端口 `6666`** 同步发送二进制温度帧，每帧固定 `2058` 字节（已在上/下位机统一为横屏校准后的数据，行优先排列）：

| 字段 | 长度 | 说明 |
| :--- | :--- | :--- |
| 帧头 | 4 字节 | 固定同步字 `0xAA 0x55 0x5A 0xA5` |
| 宽度 `width` | 2 字节 | 小端 `uint16`（`32`） |
| 高度 `height` | 2 字节 | 小端 `uint16`（`32`） |
| 温度矩阵 | 2048 字节 | `32×32` 个小端 `uint16`，单位 `0.1 K`（`℃ = val / 10.0 - 273.15`） |
| 校验和 `checksum` | 2 字节 | 小端 `uint16`，为 2048 字节温度矩阵所有字节累加和的低 16 位 |

---

## 7. 开源许可说明

* 本例程基于 **Arduino（arduino-esp32）** 框架开发；如果你基于本例程或使用 Arduino 框架进行二次开发与分发，请严格遵循 **Arduino 对应的开源许可协议（LGPL-2.1）** 及所引用第三方库（如 `LovyanGFX`、`uTimerLib` 等）的各自开源协议。
* 本开源项目仅供个人免费学习、技术研究、自制（DIY）及非商业性二次开发。不得用于商业盈利。
* **特别声明**：OpenThermal **官方出厂固件不基于 Arduino 开发**。
