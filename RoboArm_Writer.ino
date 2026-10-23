/*
  RoboArm Writer
  6 DOF arm (PCA9685 + ESP8266) that writes text with a marker.
  Aura Project Labs

  Board  : NodeMCU 1.0 (ESP-12E), Flash Size 4MB (FS:2MB OTA:~1019KB)
  Libs   : Adafruit PWM Servo Driver, ArduinoJson 7
  Wiring : PCA9685 SDA -> D2, SCL -> D1, VCC -> 3.3V, GND common.
           Servo power (V+) from a separate 5 to 6 V supply (5 A or more).

  Channels: CH0 base, CH1 shoulder, CH2 elbow, CH3 wrist rotate, CH4 wrist pitch, CH5 claw
  (claw can be turned off in tab 3 when the gripper is removed and the marker is taped on)
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "config.h"
#include "settings.h"
#include "kinematics.h"
#include "font.h"
#include "motion.h"
#include "paths.h"
#include "web_page.h"

ESP8266WebServer server(80);
bool apMode = false;

// ---------------- helpers ----------------
static float r1(float v) { return roundf(v * 10) / 10; }

void sendJson(JsonDocument& d) {
  String s;
  serializeJson(d, s);
  server.send(200, "application/json", s);
}

void reply(bool ok, const char* err = nullptr) {
  JsonDocument d;
  d["ok"] = ok;
  if (err) d["err"] = err;
  sendJson(d);
}

void goalPen(float& x, float& y, float& z) {
  float a[NJ];
  angles(goalUs, a);
  fk(a, x, y, z);
}

// ---------------- HTTP handlers ----------------
void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", PAGE);
}

void handleConfig() {
  JsonDocument d;
  toJson(d);
  sendJson(d);
}

void handleState() {
  JsonDocument d;
  d["busy"] = busy();
  d["job"] = job ? jobName : String("");
  d["prog"] = q.empty() ? 0.0f : (float)qi / q.size();
  d["err"] = lastErr;
  d["errId"] = errId;
  d["cal"] = allCalibrated();
  JsonArray us = d["us"].to<JsonArray>();
  JsonArray an = d["an"].to<JsonArray>();
  JsonArray lv = d["live"].to<JsonArray>();
  for (int j = 0; j < NJ; j++) {
    us.add(lroundf(goalUs[j]));
    an.add(r1(usToAng(j, goalUs[j])));
    lv.add(live[j]);
  }
  float x, y, z;
  goalPen(x, y, z);
  d["x"] = r1(x); d["y"] = r1(y); d["z"] = r1(z);
  d["ip"] = apMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
  d["ap"] = apMode;
  d["heap"] = ESP.getFreeHeap();
  sendJson(d);
}

// GET /api/preview?text=..&h=..&g=..&r=0|1
// JSON is built by hand: 40 letters would not fit in RAM as a JsonDocument.
void handlePreview() {
  String txt = server.arg("text");
  if (txt.length() > MAX_TEXT) txt = txt.substring(0, MAX_TEXT);
  float oh = S.textH, og = S.gap; bool orr = S.rot180;
  if (server.hasArg("h")) S.textH = constrain(server.arg("h").toFloat(), 3.0f, 200.0f);
  if (server.hasArg("g")) S.gap = server.arg("g").toFloat();
  if (server.hasArg("r")) S.rot180 = server.arg("r") == "1";

  String out, bad;
  out.reserve(6000);
  bad.reserve(600);
  out = "{\"strokes\":[";
  bool firstS = true, firstP = true;
  int nBad = 0;
  int nl = layoutText(txt, [&](bool ns, float x, float y) {
    if (ns) { out += firstS ? "[" : "],["; firstS = false; firstP = true; }
    if (!firstP) out += ",";
    firstP = false;
    out += "["; out += String(x, 1); out += ","; out += String(y, 1); out += "]";
    if (!reachOk(x, y) && nBad++ < 40) {
      if (bad.length()) bad += ",";
      bad += "["; bad += String(x, 1); bad += ","; bad += String(y, 1); bad += "]";
    }
  });
  if (!firstS) out += "]";
  S.textH = oh; S.gap = og; S.rot180 = orr;

  // Reach along the forward line at paper height
  float rMin = -1, rMax = -1, a[NJ], zc = zDown(S.cx, S.cy);
  for (float r = 20; r <= 500; r += 2) {
    if (!ik(0, r, zc, a)) { if (rMin < 0) rMin = r; rMax = r; }
  }
  out += "],\"bad\":[" + bad + "],\"nBad\":" + String(nBad) + ",\"lines\":" + String(nl);
  out += ",\"rMin\":" + String(rMin, 0) + ",\"rMax\":" + String(rMax, 0) + "}";
  server.send(200, "application/json", out);
}

// POST /api/cmd  {"cmd": "...", ...}
void handleCmd() {
  JsonDocument d;
  if (deserializeJson(d, server.arg("plain"))) return reply(false, "Bad request");
  String c = d["cmd"] | "";

  // These always work, even while drawing.
  if (c == "stop")  { stopAll(nullptr); return reply(true); }
  if (c == "relax") { stopAll(nullptr); relaxAll(); return reply(true); }
  if (job) return reply(false, "Busy drawing. Press STOP first.");

  int j = d["j"] | -1;
  bool jOk = j >= 0 && j < NJ;
  float x, y, z, a[NJ];

  if (c == "raw" || c == "nudge" || c == "joint" || c == "ref" || c == "limit") {
    if (!jOk) return reply(false, "Bad joint");
  }

  if (c == "raw") {
    runSingle(mk(C_RAW, j, d["us"] | goalUs[j]));
  } else if (c == "nudge") {
    runSingle(mk(C_RAW, j, goalUs[j] + (d["d"] | 0.0f)));
  } else if (c == "joint") {
    runSingle(mk(C_JOINT, j, d["a"] | 0.0f));
  } else if (c == "ref") {
    ServoCal& sc = S.sv[j];
    float us = roundf(goalUs[j]);
    if (String(d["which"] | "A") == "B") {
      if (fabsf(us - sc.pA) < 50 && sc.doneA) return reply(false, "Ref B is almost the same pulse as Ref A. Move the servo first.");
      sc.pB = us; sc.doneB = true;
    } else {
      if (fabsf(us - sc.pB) < 50 && sc.doneB) return reply(false, "Ref A is almost the same pulse as Ref B. Move the servo first.");
      sc.pA = us; sc.doneA = true;
    }
    saveSettings();
    // Both refs saved: check the scale looks like a real servo (about 11 us per deg).
    if (sc.doneA && sc.doneB && j != J_CLAW) {
      float k = fabsf(sc.pB - sc.pA) / fabsf(sc.aB - sc.aA);
      if (k < 5 || k > 20) {
        static char w[140];
        snprintf(w, sizeof(w), "Saved, but check it: Ref A and B give %.1f us per degree. A normal servo is about 11. Set the arm to the pictures again.", k);
        JsonDocument r;
        r["ok"] = true; r["warn"] = w;
        return sendJson(r);
      }
    }
  } else if (c == "limit") {
    float ang = usToAng(j, goalUs[j]);
    if (String(d["which"] | "min") == "max") S.sv[j].maxA = r1(ang); else S.sv[j].minA = r1(ang);
    saveSettings();
  } else if (c == "set") {
    applyJson(d.as<JsonVariantConst>());
    saveSettings();
  } else if (c == "defaults") {
    stopAll(nullptr);
    relaxAll();
    setDefaults();
    saveSettings();
  } else if (c == "park") {
    runSingle(mk(C_PARK));
  } else if (c == "savePark") {
    for (int k = 0; k < NJ; k++) if (k != J_CLAW) S.park[k] = r1(usToAng(k, goalUs[k]));
    saveSettings();
  } else if (c == "touchGo") {
    int i = d["i"] | -1;
    if (i < 0 || i >= NTOUCH) return reply(false, "Bad point");
    touchXY(i, x, y);
    z = zUp(x, y) + 5;
    const char* e = ik(x, y, z, a);
    if (e) return reply(false, e);
    runSingle(mk(C_JXYZ, x, y, z));
  } else if (c == "touchSave") {
    int i = d["i"] | -1;
    if (i < 0 || i >= NTOUCH) return reply(false, "Bad point");
    goalPen(x, y, z);
    float tx, ty;
    touchXY(i, tx, ty);
    if (fabsf(x - tx) > 5 || fabsf(y - ty) > 5) {
      static char m[140];
      snprintf(m, sizeof(m), "Not saved: the pen is at X%.0f Y%.0f, but point %d is at X%.0f Y%.0f. Press Go for this point first, then only use Arm down / up.", x, y, i + 1, tx, ty);
      return reply(false, m);
    }
    S.tp[i] = {r1(x), r1(y), r1(z), true};
    saveSettings();
    if (planeBad()) return reply(false, "Saved, but the touch points disagree by more than 5 mm per 100 mm. Check arm sizes and servo calibration, then clear and redo them.");
  } else if (c == "saveLevel") {
    // One point: paper height and text center = where the pen touches now.
    goalPen(x, y, z);
    S.zPaper = r1(z);
    S.cx = r1(x);
    S.cy = r1(y);
    if (S.areaW > 140) S.areaW = 140;
    for (int i = 0; i < NTOUCH; i++) S.tp[i].ok = false;
    S.tp[0] = {S.cx, S.cy, S.zPaper, true};                 // flat paper until a bottom point is saved
    saveSettings();
  } else if (c == "goBottom") {
    if (!S.tp[0].ok) return reply(false, "Save the writing point first.");
    x = S.cx; y = S.cy - 25; z = zUp(x, y) + 3;
    const char* e = ik(x, y, z, a);
    if (e) return reply(false, e);
    runSingle(mk(C_JXYZ, x, y, z));
  } else if (c == "saveBottom") {
    if (!S.tp[0].ok) return reply(false, "Save the writing point first.");
    goalPen(x, y, z);
    float dist = sqrtf(sq(x - S.cx) + sq(y - S.cy));
    if (dist < 10) return reply(false, "Too close to the writing point. Press Go to bottom point first.");
    S.tp[1] = {r1(x), r1(y), r1(z), true};
    saveSettings();
  } else if (c == "touchClear") {
    for (int i = 0; i < NTOUCH; i++) S.tp[i].ok = false;
    saveSettings();
  } else if (c == "fitArea") {
    if (!d["h"].isNull()) S.areaH = d["h"].as<float>();
    const char* e = fitArea();
    if (e) { loadSettings(); return reply(false, e); }
    saveSettings();
  } else if (c == "jog" || c == "goto" || c == "center") {
    if (c == "jog") {
      goalPen(x, y, z);
      x += d["dx"] | 0.0f; y += d["dy"] | 0.0f; z += d["dz"] | 0.0f;
    } else if (c == "goto") {
      x = d["x"] | S.cx; y = d["y"] | S.cy; z = d["z"] | zUp(x, y);
    } else {
      x = S.cx; y = S.cy; z = zUp(x, y);
    }
    const char* e = ik(x, y, z, a);
    if (e) return reply(false, e);
    runSingle(mk(C_JXYZ, x, y, z));
  } else if (c == "claw") {
    if (!S.clawOn) return reply(false, "Claw is turned off (gripper removed).");
    runSingle(mk(C_JOINT, J_CLAW, d["pct"] | 0.0f));
  } else if (c == "draw" || c == "square" || c == "corners") {
    if (!allCalibrated()) return reply(false, "Finish servo calibration first (all A and B).");
    if (planeBad()) return reply(false, "Touch points disagree (paper slope over 5 mm per 100 mm). Clear and redo them on tab 4.");
    const char* e;
    if (c == "draw") {
      JsonObject w = d["draw"].to<JsonObject>();
      w["textH"] = d["h"]; w["gap"] = d["g"]; w["rot180"] = d["r"];
      applyJson(d.as<JsonVariantConst>());
      saveSettings();
      String t = d["text"] | "";
      if (t.length() > MAX_TEXT) t = t.substring(0, MAX_TEXT);
      e = jobText(t);
    } else if (c == "square") {
      e = jobSquare(40);
    } else {
      e = jobCorners();
    }
    if (e) return reply(false, e);
  } else {
    return reply(false, "Unknown command");
  }
  reply(true);
}

// ---------------- setup / loop ----------------
void startWiFi() {
  WiFi.persistent(false);
#if WIFI_USE_HOME
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("Joining WiFi \"%s\"", WIFI_SSID);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) { delay(250); Serial.print('.'); }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    Serial.printf("Connected. Dashboard: http://%s  or  http://%s.local\n", WiFi.localIP().toString().c_str(), HOSTNAME);
  } else {
    Serial.println("Could not join home WiFi. Starting own hotspot.");
    apMode = true;
  }
#else
  apMode = true;
#endif
  if (apMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.printf("Hotspot \"%s\" started. Dashboard: http://%s\n", AP_SSID, WiFi.softAPIP().toString().c_str());
  }
  if (MDNS.begin(HOSTNAME)) MDNS.addService("http", "tcp", 80);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n\nRoboArm Writer starting");

  LittleFS.begin();
  setDefaults();
  bool had = loadSettings();
  Serial.println(had ? "Calibration loaded." : "No calibration saved yet. Using defaults.");

  Wire.begin(I2C_SDA, I2C_SCL);
  pwm.begin();
  pwm.setOscillatorFrequency(PCA_OSC);
  pwm.setPWMFreq(SERVO_FREQ);
  delay(10);

  // All servos start relaxed.
  for (int j = 0; j < NJ; j++) {
    float a = (j == J_CLAW) ? S.sv[J_CLAW].aA : S.park[j];
    curUs[j] = goalUs[j] = constrain(angToUs(j, clampAng(j, a)), (float)US_MIN_HARD, (float)US_MAX_HARD);
  }
  relaxAll();

  // Only drive the arm at power on if it is fully calibrated.
  // Wake one servo at a time so the power supply does not dip.
  if (allCalibrated()) {
    Serial.println("Moving to park pose.");
    const int order[NJ] = {J_SHOULDER, J_ELBOW, J_PITCH, J_ROLL, J_BASE, J_CLAW};
    for (int k = 0; k < NJ; k++) {
      if (order[k] == J_CLAW && !S.clawOn) continue;
      writeUs(order[k], curUs[order[k]]);
      delay(350);
    }
  } else {
    Serial.println("Servos left relaxed. Open the dashboard and calibrate.");
  }

  startWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/config", HTTP_GET, handleConfig);
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/preview", HTTP_GET, handlePreview);
  server.on("/api/cmd", HTTP_POST, handleCmd);
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();
}

void loop() {
  server.handleClient();
  MDNS.update();
  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last >= TICK_MS) {
    last = now;
    motionTick();
  }
}
