#pragma once

#include <Config.hpp>
#include <WiFi.h>

class Wireless {
    char macStr[18] = { 0 };
    bool wifiStatus = false;

    String softApName() {
        return String{ "INFRARED" } + "-" + String(macStr);
    }

public:
    String wifiSsid   = "";
    String wifiPasswd = "";

    static Wireless& inst() {
        static Wireless wireless{};
        return wireless;
    }

    const String netInfo(void) {
        if (WiFi.isConnected()) {
            return String{ "{\"SSID\":\"" } + WiFi.SSID() + "\",\"IP\":\"" + WiFi.localIP().toString() + "\", \"MAC\":\"" + WiFi.macAddress() + "\"}";
        } else {
            return String{ "{\"SSID\":\"" } + WiFi.softAPSSID() + "\",\"IP\":\"" + WiFi.softAPIP().toString() + "\", \"MAC\":\"" + WiFi.softAPmacAddress() + "\"}";
        }
    }

    virtual void init() {
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        sprintf(macStr, "%02X%02X%02X", mac[3], mac[4], mac[5]);

        control(true);
    }

    virtual void control(bool enable) {
        if (enable) {
            WiFi.setAutoReconnect(false);
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(softApName().c_str(), "");
            WiFi.scanNetworks(true, false, true);
            wifiStatus = true;
        } else {
            wifiStatus = false;
            WiFi.mode(WIFI_OFF);
        }
    }

    void switchMode() {
        if (WiFi.isConnected()) {
            auto apName = String{ "INFRARED" } + "-[" + WiFi.localIP().toString() + "]";
            if (WiFi.softAPSSID().compareTo(apName.c_str())) {
                WiFi.softAP(apName.c_str(), "Do not connect to this AP!");
            }
        } else {
            if (WiFi.softAPSSID().compareTo(softApName().c_str())) {
                WiFi.softAP(softApName().c_str(), "");
            }
            if (!WiFi.softAPgetStationNum() && !wifiSsid.isEmpty()) {
                WiFi.begin(wifiSsid.c_str(), wifiPasswd.c_str());
            }
        }
    }

    void handle(uint32_t count) {
        if (wifiStatus && count % 5 == 0) {
            switchMode();
        }
    }

    bool status() { return wifiStatus; }
};
