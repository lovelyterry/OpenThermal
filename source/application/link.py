#!/usr/bin/env python3

# 该文件用于和热成像模块建立连接，获取热成像模块的原始数据
import serial
import re
import select
import socket

from serial.tools import list_ports

# TCP连接方式
# TCP连接方式需要上位机和热成像模块处于同一局域网
# 如果是热成像模块开启热点，上位机去连接，那么默认的IP地址就是192.168.4.1
# 默认的端口为6666


class Tcp:
    def __init__(self, server="192.168.4.1", port=6666):
        self.fd = None
        self.server = server
        self.port = port

    def open(self):
        self.fd = socket.socket()
        self.fd.settimeout(3.0)
        self.fd.connect((self.server, self.port))
        self.fd.setblocking(0)
        return self.fd

    def send(self, buffer):
        result = self.fd.send(buffer)
        if result == len(buffer):
            return result
        else:
            return None

    def recv(self):
        if self.fd is None:
            return None
        readable, _, _ = select.select([self.fd], [], [], 0)
        if not readable:
            return None
        buffer = self.fd.recv(65536)
        if not buffer:
            raise ConnectionError("TCP 连接已断开")
        return buffer

    def close(self):
        if self.fd is not None:
            self.fd.close()
            self.fd = None


# 串口连接方式
# windows需要安装驱动，可以通过双击目录下的usbdriver.ps1完成安装
# 基于linux的系统自带驱动，不用安装
# 遍历USB设备，查找VID为303A，PID为1001的设备打开即可
# 波特率为921600，数据位8，无校验，停止位1
class Serial:
    def __init__(self, port=None):
        self.fd = None
        self.port = port

    @staticmethod
    def list_available():
        return [p.device for p in list_ports.comports()]

    def open(self):
        port = self.port
        if not port or port == "自动识别 (VID:303A PID:1001)":
            port = None
            for item in list_ports.comports():
                info = list(item)
                if re.match("USB VID:PID=303A:1001", info[2]):
                    port = info[0]
                    break
        if port is None:
            raise RuntimeError("未找到设备串口 (VID:303A PID:1001)")
        self.fd = serial.Serial(port, 921600, 8, 'N', 1, timeout=1)
        if self.fd.is_open:
            return self.fd
        raise RuntimeError(f"无法打开串口 {port}")

    def send(self, buffer):
        result = self.fd.write(buffer)
        if result == len(buffer):
            return result
        else:
            return None

    def recv(self):
        if self.fd is not None and self.fd.in_waiting:
            buffer = self.fd.read(self.fd.in_waiting)
            return buffer
        else:
            return None

    def close(self):
        if self.fd is not None and self.fd.is_open:
            self.fd.close()
            self.fd = None
