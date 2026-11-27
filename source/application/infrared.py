#!/usr/bin/env python3

import numpy as np
import struct
from datetime import datetime

# 与设备端一致的 256 级 classic 伪彩查找表 (RGB888)
_CLASSIC_STOPS = np.array([
    [0.00, 0x35, 0x34, 0x3D],
    [0.10, 0x04, 0x05, 0x59],
    [0.25, 0x00, 0xDB, 0xFF],
    [0.45, 0x00, 0xB4, 0x3C],
    [0.65, 0xFD, 0xFC, 0x00],
    [0.80, 0xFF, 0x5D, 0x00],
    [0.90, 0xFF, 0x00, 0x00],
    [1.00, 0xFF, 0xED, 0xED],
], dtype=np.float32)


def _build_classic_lut() -> np.ndarray:
    t = np.linspace(0.0, 1.0, 256, dtype=np.float32)
    lut = np.empty((256, 3), dtype=np.uint8)
    for c in range(3):
        lut[:, c] = np.clip(np.interp(t, _CLASSIC_STOPS[:, 0], _CLASSIC_STOPS[:, c + 1]), 0, 255).astype(np.uint8)
    return lut


_CLASSIC_LUT = _build_classic_lut()


def _bspline_matrix(src_len: int, dst_len: int) -> np.ndarray:
    mat = np.zeros((dst_len, src_len), dtype=np.float32)
    if src_len <= 1 or dst_len <= 1:
        mat[:, 0] = 1.0
        return mat
    s = np.linspace(0.0, src_len - 1, dst_len, dtype=np.float32)
    i0 = np.floor(s).astype(np.int32)
    t = s - i0
    t2 = t * t
    t3 = t2 * t
    u = 1.0 - t
    w0 = (u * u * u) / 6.0
    w1 = (3.0 * t3 - 6.0 * t2 + 4.0) / 6.0
    w2 = (-3.0 * t3 + 3.0 * t2 + 3.0 * t + 1.0) / 6.0
    w3 = t3 / 6.0
    rows = np.arange(dst_len)
    for k, w in enumerate((w0, w1, w2, w3)):
        idx = np.clip(i0 + k - 1, 0, src_len - 1)
        np.add.at(mat, (rows, idx), w)
    return mat


class Infrared:
    def __init__(self):
        self.cache = bytearray()
        self._bspline_cache = {}

    def _get_bspline_mats(self, h: int, w: int, scale: int):
        key = (h, w, scale)
        if key not in self._bspline_cache:
            self._bspline_cache[key] = (_bspline_matrix(h, h * scale), _bspline_matrix(w, w * scale))
        return self._bspline_cache[key]

    # 纯 NumPy 三次 B 样条插值放大并映射伪彩，返回 (H*scale, W*scale, 3) uint8 RGB
    def render_rgb(self, matrix: np.ndarray, scale: int = 10) -> np.ndarray:
        h, w = matrix.shape
        by, bx = self._get_bspline_mats(h, w, scale)
        up = by @ matrix.astype(np.float32) @ bx.T
        v_min = float(matrix.min())
        v_max = float(matrix.max())
        span = v_max - v_min
        if span <= 0.0:
            idx = np.zeros(up.shape, dtype=np.uint8)
        else:
            idx = np.clip(((up - v_min) / span) * 255.0, 0, 255).astype(np.uint8)
        return _CLASSIC_LUT[idx]

    def printTemp(self, tempMatrix):
        if tempMatrix is None:
            return None
        print("\n\n")
        print(datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3])
        print('\n'.join([' '.join(['{:.1f}'.format(item) for item in row]) for row in tempMatrix]))

    def parse(self, buffer):
        if buffer is None:
            return None
        self.cache.extend(buffer)
        while True:
            frameIndex = self.cache.find(b'\xAA\x55\x5A\xA5')
            if frameIndex < 0:
                if len(self.cache) > 3:
                    self.cache = self.cache[-3:]
                return None
            if len(self.cache) - frameIndex < 8:
                if frameIndex > 0:
                    self.cache = self.cache[frameIndex:]
                return None
            width, height = struct.unpack('<HH', self.cache[frameIndex + 4:frameIndex + 8])
            if width == 0 or height == 0 or width * height > 65536:
                self.cache = self.cache[frameIndex + 4:]
                continue
            payloadLen = width * height * 2
            totalLen = 8 + payloadLen + 2
            if len(self.cache) - frameIndex < totalLen:
                if frameIndex > 0:
                    self.cache = self.cache[frameIndex:]
                return None
            payload = self.cache[frameIndex + 8:frameIndex + 8 + payloadLen]
            checksumRecv = struct.unpack('<H', self.cache[frameIndex + 8 + payloadLen:frameIndex + totalLen])[0]
            checksumCalc = sum(payload) & 0xFFFF
            if checksumCalc != checksumRecv:
                self.cache = self.cache[frameIndex + 4:]
                continue
            self.cache = self.cache[frameIndex + totalLen:]
            raw = np.frombuffer(payload, dtype='<u2').reshape((height, width))
            # 原始数据为开尔文温度，量纲为 0.1，转换为摄氏度: / 10.0 - 273.15
            tempMatrix = raw.astype(float) / 10.0 - 273.15
            self.printTemp(tempMatrix)
            return tempMatrix
