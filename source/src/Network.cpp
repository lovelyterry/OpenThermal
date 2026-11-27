#include <WiFi.h>
#include <WebServer.h>

#include <Config.hpp>
#include <Wireless.hpp>

extern uint16_t sensorRawData[IR_WIDTH * IR_HEIGHT];

static WebServer*  httpServer = nullptr;
static WiFiServer* tcpServer  = nullptr;
static WiFiClient  tcpClients[MAX_CLIENTS];

static void tcpServerLoop() {
    if (tcpServer->hasClient()) {
        for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
            if (!tcpClients[i] || !tcpClients[i].connected()) {
                tcpClients[i] = tcpServer->accept();
                break;
            }
        }
    }
    for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
        if (tcpClients[i] && tcpClients[i].connected()) {
            while (tcpClients[i].available() > 0) { tcpClients[i].read(); }
        }
    }
}

void networkSendFrame(const uint8_t* data, size_t len) {
    if (!tcpServer || !Wireless::inst().status()) { return; }
    for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
        if (tcpClients[i] && tcpClients[i].connected()) {
            tcpClients[i].write(data, len);
        }
    }
}

static const char INDEX_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>OpenThermal Viewer</title>
<style>
  body { margin:0; background:#141414; color:#eee; font-family:system-ui,sans-serif; display:flex; flex-direction:column; align-items:center; justify-content:center; min-height:100vh; padding:16px; box-sizing:border-box; }
  h2 { margin:0 0 12px; font-size:18px; font-weight:600; }
  .panel { background:#1f1f1f; border-radius:12px; padding:16px; box-shadow:0 4px 20px rgba(0,0,0,.5); display:flex; flex-direction:column; align-items:center; gap:12px; }
  canvas { border-radius:8px; background:#000; width:320px; height:320px; }
  .stats { display:flex; gap:18px; font-size:15px; font-variant-numeric:tabular-nums; }
  .stats span b { color:#ffb300; }
</style>
</head>
<body>
  <h2>OpenThermal 实时热成像</h2>
  <div class="panel">
    <canvas id="cv" width="320" height="320"></canvas>
    <div class="stats">
      <span>MAX: <b id="tmax">--</b> ℃</span>
      <span>MIN: <b id="tmin">--</b> ℃</span>
      <span>CTR: <b id="tctr">--</b> ℃</span>
    </div>
  </div>
<script>
const cv = document.getElementById('cv');
const ctx = cv.getContext('2d');
const img = ctx.createImageData(cv.width, cv.height);
const stops = [
  [0.00, 0x35,0x34,0x3D], [0.10, 0x04,0x05,0x59], [0.25, 0x00,0xDB,0xFF],
  [0.45, 0x00,0xB4,0x3C], [0.65, 0xFD,0xFC,0x00], [0.80, 0xFF,0x5D,0x00],
  [0.90, 0xFF,0x00,0x00], [1.00, 0xFF,0xED,0xED]
];
const lut = new Uint8ClampedArray(256 * 3);
for (let i = 0; i < 256; i++) {
  const t = i / 255;
  let k = 0;
  while (k < stops.length - 2 && t > stops[k + 1][0]) k++;
  const [t0,r0,g0,b0] = stops[k], [t1,r1,g1,b1] = stops[k + 1];
  const f = (t - t0) / (t1 - t0 || 1);
  lut[i*3]   = r0 + (r1 - r0) * f;
  lut[i*3+1] = g0 + (g1 - g0) * f;
  lut[i*3+2] = b0 + (b1 - b0) * f;
}

const bsCache = {};
function getBSplineTable(srcLen, dstLen) {
  const key = srcLen + 'x' + dstLen;
  if (bsCache[key]) return bsCache[key];
  const idx = new Int32Array(dstLen * 4);
  const w = new Float32Array(dstLen * 4);
  for (let d = 0; d < dstLen; d++) {
    const s = (srcLen > 1 && dstLen > 1) ? (d * (srcLen - 1)) / (dstLen - 1) : 0;
    const i0 = Math.floor(s);
    const t = s - i0, t2 = t * t, t3 = t2 * t, u = 1 - t;
    const base = d * 4;
    for (let k = 0; k < 4; k++) {
      let c = i0 + k - 1;
      if (c < 0) c = 0; else if (c >= srcLen) c = srcLen - 1;
      idx[base + k] = c;
    }
    w[base]     = (u * u * u) / 6;
    w[base + 1] = (3 * t3 - 6 * t2 + 4) / 6;
    w[base + 2] = (-3 * t3 + 3 * t2 + 3 * t + 1) / 6;
    w[base + 3] = t3 / 6;
  }
  return (bsCache[key] = { idx, w });
}
let tmpBuf = null;

function drawFrame(res) {
  const { width: w, height: h, min, max, ctr, data } = res;
  const dw = cv.width, dh = cv.height;
  const tx = getBSplineTable(w, dw), ty = getBSplineTable(h, dh);
  if (!tmpBuf || tmpBuf.length !== h * dw) tmpBuf = new Float32Array(h * dw);

  for (let sy = 0; sy < h; sy++) {
    const sOff = sy * w, tOff = sy * dw;
    for (let dx = 0; dx < dw; dx++) {
      const b = dx * 4;
      tmpBuf[tOff + dx] =
        data[sOff + tx.idx[b]]     * tx.w[b] +
        data[sOff + tx.idx[b + 1]] * tx.w[b + 1] +
        data[sOff + tx.idx[b + 2]] * tx.w[b + 2] +
        data[sOff + tx.idx[b + 3]] * tx.w[b + 3];
    }
  }

  const span = (max > min) ? (max - min) : 1;
  const invSpan = 255 / span;
  const px = img.data;
  for (let dy = 0; dy < dh; dy++) {
    const b = dy * 4;
    const r0 = ty.idx[b] * dw, r1 = ty.idx[b + 1] * dw, r2 = ty.idx[b + 2] * dw, r3 = ty.idx[b + 3] * dw;
    const wy0 = ty.w[b], wy1 = ty.w[b + 1], wy2 = ty.w[b + 2], wy3 = ty.w[b + 3];
    const rowOut = dy * dw * 4;
    for (let dx = 0; dx < dw; dx++) {
      const val = tmpBuf[r0 + dx] * wy0 + tmpBuf[r1 + dx] * wy1 + tmpBuf[r2 + dx] * wy2 + tmpBuf[r3 + dx] * wy3;
      let ci = Math.round((val - min) * invSpan);
      if (ci < 0) ci = 0; else if (ci > 255) ci = 255;
      const p = rowOut + dx * 4, c = ci * 3;
      px[p] = lut[c]; px[p + 1] = lut[c + 1]; px[p + 2] = lut[c + 2]; px[p + 3] = 255;
    }
  }
  ctx.putImageData(img, 0, 0);

  document.getElementById('tmax').textContent = max.toFixed(1);
  document.getElementById('tmin').textContent = min.toFixed(1);
  document.getElementById('tctr').textContent = ctr.toFixed(1);
}

async function poll() {
  try {
    const r = await fetch('/temp', { cache: 'no-store' });
    if (r.ok) drawFrame(await r.json());
  } catch (e) {}
  setTimeout(poll, 80);
}
poll();
</script>
</body>
</html>)HTML";

void networkTask(void* parameter) {
    httpServer = new WebServer(80);
    tcpServer  = new WiFiServer();

    httpServer->enableCORS(true);

    httpServer->on("/", HTTP_GET, []() {
        httpServer->send_P(200, "text/html", INDEX_HTML);
    });

    httpServer->on("/temp", HTTP_GET, []() {
        static uint16_t snap[IR_WIDTH * IR_HEIGHT];
        memcpy(snap, sensorRawData, sizeof(snap));

        uint16_t minRaw = snap[0], maxRaw = snap[0];
        for (int i = 1; i < IR_WIDTH * IR_HEIGHT; i++) {
            if (snap[i] < minRaw) { minRaw = snap[i]; }
            if (snap[i] > maxRaw) { maxRaw = snap[i]; }
        }
        uint16_t ctrRaw = snap[(IR_HEIGHT / 2) * IR_WIDTH + (IR_WIDTH / 2)];

        String json;
        json.reserve(IR_WIDTH * IR_HEIGHT * 6 + 128);
        json += "{\"width\":" + String(IR_WIDTH) + ",\"height\":" + String(IR_HEIGHT);
        json += ",\"min\":" + String((float)minRaw / 10.0f - 273.15f, 1);
        json += ",\"max\":" + String((float)maxRaw / 10.0f - 273.15f, 1);
        json += ",\"ctr\":" + String((float)ctrRaw / 10.0f - 273.15f, 1);
        json += ",\"data\":[";
        for (int i = 0; i < IR_WIDTH * IR_HEIGHT; i++) {
            if (i > 0) { json += ','; }
            json += String((float)snap[i] / 10.0f - 273.15f, 1);
        }
        json += "]}";
        httpServer->send(200, "application/json", json);
    });

    httpServer->on("/reboot", HTTP_GET, []() {
        httpServer->send(200, "text/plain", "Rebooting...");
        delay(200);
        ESP.restart();
    });

    httpServer->onNotFound([]() {
        httpServer->send(404, "text/plain", "Not Found");
    });

    httpServer->begin();

    tcpServer->begin(6666);

    for (;;) {
        if (Wireless::inst().status()) {
            httpServer->handleClient();
            tcpServerLoop();
        }
        delay(10);
    }
}
