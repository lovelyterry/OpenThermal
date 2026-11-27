#!/usr/bin/env python3

import importlib.util
import subprocess
import sys
import tkinter as tk
from tkinter import ttk, messagebox


def _ensure_dependencies():
    required = (("numpy", "numpy"), ("serial", "pyserial"))
    missing = [pip_name for mod_name, pip_name in required if importlib.util.find_spec(mod_name) is None]
    if not missing:
        return
    print(f"[INFO] 检测到缺少运行依赖 {missing}，正在自动安装...")
    try:
        subprocess.check_call([sys.executable, "-m", "pip", "install", *missing])
    except subprocess.CalledProcessError:
        print("[WARN] 默认源安装失败，尝试使用清华镜像源重试...")
        subprocess.check_call([
            sys.executable, "-m", "pip", "install",
            "-i", "https://pypi.tuna.tsinghua.edu.cn/simple",
            *missing,
        ])


_ensure_dependencies()

from link import Serial, Tcp
from infrared import Infrared

class App:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("OpenThermal 上位机")
        self.root.resizable(False, False)

        self.infrared = Infrared()
        self.port = None
        self.connected = False
        self.after_id = None

        self.mode_var = tk.StringVar(value="serial")
        self.com_var = tk.StringVar(value="自动识别 (VID:303A PID:1001)")
        self.ip_var = tk.StringVar(value="192.168.4.1")
        self.tcp_port_var = tk.StringVar(value="6666")
        self.status_var = tk.StringVar(value="未连接")
        self.temp_var = tk.StringVar(value="MAX: --   MIN: --   CTR: --")
        self._photo = None

        self._build_ui()
        self._refresh_ports()
        self._on_mode_change()
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

    def _build_ui(self):
        pad = {"padx": 10, "pady": 6}
        frame = ttk.Frame(self.root, padding=12)
        frame.grid(row=0, column=0, sticky="nsew")

        ttk.Label(frame, text="连接方式:").grid(row=0, column=0, sticky="w", **pad)
        mode_box = ttk.Frame(frame)
        mode_box.grid(row=0, column=1, columnspan=2, sticky="w", **pad)
        ttk.Radiobutton(mode_box, text="USB 串口", value="serial", variable=self.mode_var, command=self._on_mode_change).pack(side="left", padx=(0, 12))
        ttk.Radiobutton(mode_box, text="WiFi TCP", value="tcp", variable=self.mode_var, command=self._on_mode_change).pack(side="left")

        ttk.Label(frame, text="串口端口:").grid(row=1, column=0, sticky="w", **pad)
        self.com_combo = ttk.Combobox(frame, textvariable=self.com_var, width=28, state="readonly")
        self.com_combo.grid(row=1, column=1, sticky="we", **pad)
        self.refresh_btn = ttk.Button(frame, text="刷新", width=6, command=self._refresh_ports)
        self.refresh_btn.grid(row=1, column=2, **pad)

        ttk.Label(frame, text="TCP 地址:").grid(row=2, column=0, sticky="w", **pad)
        tcp_box = ttk.Frame(frame)
        tcp_box.grid(row=2, column=1, columnspan=2, sticky="we", **pad)
        self.ip_entry = ttk.Entry(tcp_box, textvariable=self.ip_var, width=20)
        self.ip_entry.pack(side="left")
        ttk.Label(tcp_box, text=" : ").pack(side="left")
        self.tcp_port_entry = ttk.Entry(tcp_box, textvariable=self.tcp_port_var, width=7)
        self.tcp_port_entry.pack(side="left")

        self.conn_btn = ttk.Button(frame, text="连接并开始显示", command=self._toggle_connect)
        self.conn_btn.grid(row=3, column=0, columnspan=3, sticky="we", padx=10, pady=(10, 4))

        ttk.Label(frame, textvariable=self.status_var, foreground="#555").grid(row=4, column=0, columnspan=3, sticky="w", padx=10, pady=(2, 6))

        self._blank = tk.PhotoImage(width=320, height=320)
        self.img_label = ttk.Label(frame, image=self._blank)
        self.img_label.grid(row=5, column=0, columnspan=3, padx=10, pady=(4, 4))

        ttk.Label(frame, textvariable=self.temp_var, font=("Consolas", 10, "bold")).grid(row=6, column=0, columnspan=3, pady=(2, 4))

    def _on_mode_change(self):
        is_serial = self.mode_var.get() == "serial"
        self.com_combo.config(state="readonly" if is_serial else "disabled")
        self.refresh_btn.config(state="normal" if is_serial else "disabled")
        self.ip_entry.config(state="disabled" if is_serial else "normal")
        self.tcp_port_entry.config(state="disabled" if is_serial else "normal")

    def _refresh_ports(self):
        items = ["自动识别 (VID:303A PID:1001)"] + Serial.list_available()
        self.com_combo["values"] = items
        if self.com_var.get() not in items:
            self.com_var.set(items[0])

    def _toggle_connect(self):
        if self.connected:
            self._disconnect()
        else:
            self._connect()

    def _connect(self):
        try:
            self.infrared = Infrared()
            if self.mode_var.get() == "serial":
                self.port = Serial(self.com_var.get())
                self.port.open()
                self.status_var.set(f"已连接串口 ({self.com_var.get()})")
            else:
                ip = self.ip_var.get().strip()
                tcp_port = int(self.tcp_port_var.get().strip())
                self.port = Tcp(ip, tcp_port)
                self.port.open()
                self.status_var.set(f"已连接 TCP {ip}:{tcp_port}")

            self.connected = True
            self.conn_btn.config(text="断开连接")
            self._poll()
        except Exception as e:
            self._disconnect()
            messagebox.showerror("连接失败", str(e))

    def _disconnect(self):
        self.connected = False
        if self.after_id is not None:
            self.root.after_cancel(self.after_id)
            self.after_id = None
        if self.port is not None:
            try:
                self.port.close()
            except Exception:
                pass
            self.port = None
        self.img_label.config(image=self._blank)
        self._photo = None
        self.temp_var.set("MAX: --   MIN: --   CTR: --")
        self.conn_btn.config(text="连接并开始显示")
        self.status_var.set("未连接")

    def _poll(self):
        if not self.connected:
            return
        try:
            data = self.port.recv()
            if data is not None:
                temp_matrix = self.infrared.parse(data)
                if temp_matrix is not None:
                    rgb = self.infrared.render_rgb(temp_matrix)
                    h, w, _ = rgb.shape
                    ppm = f"P6 {w} {h} 255\n".encode("ascii") + rgb.tobytes()
                    self._photo = tk.PhotoImage(data=ppm)
                    self.img_label.config(image=self._photo)
                    t_max = float(temp_matrix.max())
                    t_min = float(temp_matrix.min())
                    t_ctr = float(temp_matrix[temp_matrix.shape[0] // 2, temp_matrix.shape[1] // 2])
                    self.temp_var.set(f"MAX: {t_max:.1f}C   MIN: {t_min:.1f}C   CTR: {t_ctr:.1f}C")
        except Exception as e:
            self._disconnect()
            messagebox.showerror("通信中断", str(e))
            return
        self.after_id = self.root.after(10, self._poll)

    def _on_close(self):
        self._disconnect()
        self.root.destroy()


if __name__ == '__main__':
    root = tk.Tk()
    App(root)
    root.mainloop()
