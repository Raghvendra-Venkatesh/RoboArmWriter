#pragma once
// Calibration and settings. Stored in LittleFS as /cfg.json.
#include <LittleFS.h>
#include <ArduinoJson.h>

enum { J_BASE = 0, J_SHOULDER, J_ELBOW, J_ROLL, J_PITCH, J_CLAW, NJ };
static const char* const JNAME[NJ] = {"Base", "Shoulder", "Elbow", "Wrist rotate", "Wrist pitch", "Claw"};

// Two point calibration per servo:
// at joint angle aA the servo needs pulse pA, at aB it needs pB.
// Everything in between is a straight line. Direction comes for free.
struct ServoCal {
  uint8_t ch;
  float aA, aB;       // reference angles (joint degrees, claw in %)
  float pA, pB;       // pulse in microseconds at aA and aB
  float minA, maxA;   // soft limits in joint degrees
  bool doneA, doneB;  // has the user saved this reference?
};

#define NTOUCH 5
struct TouchPt { float x, y, z; bool ok; };

struct Settings {
  ServoCal sv[NJ];
  // Geometry (mm). See README for the picture.
  float H;        // table to shoulder pivot
  float d0;       // base axis to shoulder pivot, forward (+) / back (-)
  float L1;       // shoulder pivot to elbow pivot
  float L2;       // elbow pivot to wrist pitch pivot
  float L3;       // wrist pitch pivot to pen tip
  float maxTilt;  // pen may tilt up to this many deg from straight down, either way
  float sideOff;  // pen tip is this far to the RIGHT of the arm center line (mm)
  float rollDraw; // wrist rotate angle while drawing (holds the taped marker)
  bool clawOn;    // false = gripper removed, claw servo not used
  // Paper and pen
  float zPaper;   // paper height used when no touch points are saved
  float lift;     // pen up height above paper
  float press;    // how far below paper height the pen is pushed when drawing
  // Speeds
  float vDraw, vTravel, vZ;  // mm/s
  float slew;                // deg/s for joint moves
  // Writing area (arm frame: X right, Y forward, origin at base axis)
  float cx, cy, areaW, areaH;
  float textH, gap;
  bool rot180;
  float park[NJ];            // park pose, joint angles (claw not used)
  TouchPt tp[NTOUCH];        // saved paper touch points
};

Settings S;
float planeA, planeB, planeC;  // paper height z = A + B*x + C*y (from touch points)

void setDefaults() {
  //                 ch   aA   aB    pA    pB   min  max
  const float d[NJ][7] = {
    {0,   0,  90, 1500, 2500, -90,  90},   // base: 0 = forward, 90 = right
    {1,  90,   0, 1500,  500, -10, 180},   // shoulder: 90 = straight up, 0 = level forward
    {2,  90,   0, 1500,  500, -10, 170},   // elbow: 0 = straight, 90 = right angle
    {3,   0,  90, 1500, 2500, -90,  90},   // wrist rotate
    {4,   0,  90, 1500, 2500, -90, 120},   // wrist pitch: 0 = in line, 90 = bent down
    {5,   0, 100, 1500, 1000,   0, 100},   // claw: 0 % = closed on marker, 100 % = open
  };
  for (int j = 0; j < NJ; j++) {
    ServoCal& c = S.sv[j];
    c.ch = (uint8_t)d[j][0];
    c.aA = d[j][1]; c.aB = d[j][2];
    c.pA = d[j][3]; c.pB = d[j][4];
    c.minA = d[j][5]; c.maxA = d[j][6];
    c.doneA = c.doneB = false;
  }
  S.H = 70; S.d0 = 0; S.L1 = 105; S.L2 = 100; S.L3 = 150;
  S.maxTilt = 30; S.sideOff = 0; S.rollDraw = 0; S.clawOn = false;
  S.zPaper = 0; S.lift = 6; S.press = 1.0f;
  S.vDraw = 25; S.vTravel = 50; S.vZ = 20; S.slew = 45;
  S.cx = 0; S.cy = 150; S.areaW = 140; S.areaH = 60;
  S.textH = 25; S.gap = 6; S.rot180 = false;
  const float pk[NJ] = {0, 90, 90, 0, 90, 0};
  for (int j = 0; j < NJ; j++) S.park[j] = pk[j];
  for (int i = 0; i < NTOUCH; i++) S.tp[i] = {0, 0, 0, false};
}

// Fits a flat plane through the saved touch points (least squares).
// 0 points: flat at zPaper. 1 or 2 points: flat at their average.
void fitPlane() {
  int n = 0;
  float xm = 0, ym = 0, zm = 0;
  for (int i = 0; i < NTOUCH; i++) if (S.tp[i].ok) { xm += S.tp[i].x; ym += S.tp[i].y; zm += S.tp[i].z; n++; }
  planeB = planeC = 0;
  if (n == 0) { planeA = S.zPaper; return; }
  xm /= n; ym /= n; zm /= n;
  planeA = zm;
  if (n == 2) {                               // writing point + bottom point: slope along the line between them
    const TouchPt* p[2]; int k = 0;
    for (int i = 0; i < NTOUCH; i++) if (S.tp[i].ok) p[k++] = &S.tp[i];
    float dx = p[1]->x - p[0]->x, dy = p[1]->y - p[0]->y, dz = p[1]->z - p[0]->z, L2 = dx * dx + dy * dy;
    if (L2 < 25) return;
    planeB = dz * dx / L2;
    planeC = dz * dy / L2;
    planeA = p[0]->z - planeB * p[0]->x - planeC * p[0]->y;
    return;
  }
  if (n < 3) return;
  float sxx = 0, sxy = 0, syy = 0, sxz = 0, syz = 0;
  for (int i = 0; i < NTOUCH; i++) if (S.tp[i].ok) {
    float dx = S.tp[i].x - xm, dy = S.tp[i].y - ym, dz = S.tp[i].z - zm;
    sxx += dx * dx; sxy += dx * dy; syy += dy * dy; sxz += dx * dz; syz += dy * dz;
  }
  float det = sxx * syy - sxy * sxy;
  if (fabsf(det) < 1) return;                 // points in a line: stay flat
  planeB = (sxz * syy - syz * sxy) / det;
  planeC = (syz * sxx - sxz * sxy) / det;
  planeA = zm - planeB * xm - planeC * ym;
}

float paperZ(float x, float y) { return planeA + planeB * x + planeC * y; }

static void getF(JsonVariantConst v, float& f) { if (!v.isNull()) f = v.as<float>(); }
static void getB(JsonVariantConst v, bool& b)  { if (!v.isNull()) b = v.as<bool>(); }

void sanitize() {
  for (int j = 0; j < NJ; j++) {
    ServoCal& c = S.sv[j];
    if (c.ch > 15) c.ch = j;
    if (fabsf(c.aB - c.aA) < 1) c.aB = c.aA + 90;
    if (fabsf(c.pB - c.pA) < 5) c.pB = c.pA + 500;
    c.pA = constrain(c.pA, US_MIN_HARD, US_MAX_HARD);
    c.pB = constrain(c.pB, US_MIN_HARD, US_MAX_HARD);
    if (c.minA > c.maxA) { float t = c.minA; c.minA = c.maxA; c.maxA = t; }
  }
  if (S.L1 < 10) S.L1 = 10;
  if (S.L2 < 10) S.L2 = 10;
  if (S.L3 < 0)  S.L3 = 0;
  S.vDraw = constrain(S.vDraw, 1, 200);
  S.vTravel = constrain(S.vTravel, 1, 300);
  S.vZ = constrain(S.vZ, 1, 100);
  S.slew = constrain(S.slew, 5, 360);
  S.textH = constrain(S.textH, 3, 200);
  S.lift = constrain(S.lift, 2, 80);
  S.press = constrain(S.press, -5, 10);
  S.maxTilt = constrain(S.maxTilt, 0, 70);
  fitPlane();
}

void applyServo(int j, JsonObjectConst o) {
  if (j < 0 || j >= NJ || o.isNull()) return;
  ServoCal& c = S.sv[j];
  if (!o["ch"].isNull()) c.ch = o["ch"].as<int>();
  getF(o["aA"], c.aA); getF(o["aB"], c.aB);
  getF(o["pA"], c.pA); getF(o["pB"], c.pB);
  getF(o["minA"], c.minA); getF(o["maxA"], c.maxA);
  getB(o["doneA"], c.doneA); getB(o["doneB"], c.doneB);
}

// Works for both the saved file and the "set" command from the dashboard.
void applyJson(JsonVariantConst d) {
  JsonArrayConst sa = d["servos"];
  if (!sa.isNull()) { int j = 0; for (JsonObjectConst o : sa) applyServo(j++, o); }
  JsonObjectConst one = d["servo"];
  if (!one.isNull()) applyServo(one["j"] | -1, one);

  JsonObjectConst g = d["geo"];
  if (!g.isNull()) {
    getF(g["H"], S.H); getF(g["d0"], S.d0); getF(g["L1"], S.L1); getF(g["L2"], S.L2);
    getF(g["L3"], S.L3); getF(g["maxTilt"], S.maxTilt); getF(g["sideOff"], S.sideOff);
    getF(g["rollDraw"], S.rollDraw); getB(g["clawOn"], S.clawOn);
  }
  JsonObjectConst w = d["draw"];
  if (!w.isNull()) {
    getF(w["zPaper"], S.zPaper); getF(w["lift"], S.lift); getF(w["press"], S.press);
    getF(w["vDraw"], S.vDraw); getF(w["vTravel"], S.vTravel); getF(w["vZ"], S.vZ); getF(w["slew"], S.slew);
    getF(w["cx"], S.cx); getF(w["cy"], S.cy); getF(w["areaW"], S.areaW); getF(w["areaH"], S.areaH);
    getF(w["textH"], S.textH); getF(w["gap"], S.gap); getB(w["rot180"], S.rot180);
  }
  JsonArrayConst p = d["park"];
  if (!p.isNull()) { int j = 0; for (JsonVariantConst v : p) { if (j < NJ) S.park[j] = v.as<float>(); j++; } }
  JsonArrayConst t = d["touch"];
  if (!t.isNull()) {
    int i = 0;
    for (JsonArrayConst v : t) {
      if (i >= NTOUCH) break;
      S.tp[i] = {v[0].as<float>(), v[1].as<float>(), v[2].as<float>(), v[3].as<bool>()};
      i++;
    }
  }
}

void toJson(JsonDocument& d) {
  JsonArray sa = d["servos"].to<JsonArray>();
  for (int j = 0; j < NJ; j++) {
    const ServoCal& c = S.sv[j];
    JsonObject o = sa.add<JsonObject>();
    o["name"] = JNAME[j]; o["ch"] = c.ch;
    o["aA"] = c.aA; o["aB"] = c.aB; o["pA"] = c.pA; o["pB"] = c.pB;
    o["minA"] = c.minA; o["maxA"] = c.maxA; o["doneA"] = c.doneA; o["doneB"] = c.doneB;
  }
  JsonObject g = d["geo"].to<JsonObject>();
  g["H"] = S.H; g["d0"] = S.d0; g["L1"] = S.L1; g["L2"] = S.L2; g["L3"] = S.L3;
  g["maxTilt"] = S.maxTilt; g["sideOff"] = S.sideOff; g["rollDraw"] = S.rollDraw; g["clawOn"] = S.clawOn;
  JsonObject w = d["draw"].to<JsonObject>();
  w["zPaper"] = S.zPaper; w["lift"] = S.lift; w["press"] = S.press;
  w["vDraw"] = S.vDraw; w["vTravel"] = S.vTravel; w["vZ"] = S.vZ; w["slew"] = S.slew;
  w["cx"] = S.cx; w["cy"] = S.cy; w["areaW"] = S.areaW; w["areaH"] = S.areaH;
  w["textH"] = S.textH; w["gap"] = S.gap; w["rot180"] = S.rot180;
  JsonArray p = d["park"].to<JsonArray>();
  for (int j = 0; j < NJ; j++) p.add(S.park[j]);
  JsonArray t = d["touch"].to<JsonArray>();
  for (int i = 0; i < NTOUCH; i++) {
    JsonArray v = t.add<JsonArray>();
    v.add(S.tp[i].x); v.add(S.tp[i].y); v.add(S.tp[i].z); v.add(S.tp[i].ok);
  }
  JsonObject pl = d["plane"].to<JsonObject>();
  pl["a"] = planeA; pl["b"] = planeB; pl["c"] = planeC;
}

bool loadSettings() {
  File f = LittleFS.open("/cfg.json", "r");
  if (!f) return false;
  JsonDocument d;
  DeserializationError e = deserializeJson(d, f);
  f.close();
  if (e) return false;
  applyJson(d.as<JsonVariantConst>());
  // Settings saved by the first firmware (no tilt yet): lower the pen lift once.
  if (d["geo"]["maxTilt"].isNull() && S.lift > 6) S.lift = 6;
  sanitize();
  return true;
}

bool saveSettings() {
  sanitize();
  JsonDocument d;
  toJson(d);
  File f = LittleFS.open("/cfg.json", "w");
  if (!f) return false;
  serializeJson(d, f);
  f.close();
  return true;
}

bool allCalibrated() {
  for (int j = 0; j < NJ; j++) {
    if (j == J_CLAW && !S.clawOn) continue;
    if (!S.sv[j].doneA || !S.sv[j].doneB) return false;
  }
  return true;
}

// ---- angle <-> pulse ----
float usToAng(int j, float us) {
  const ServoCal& c = S.sv[j];
  return c.aA + (us - c.pA) * (c.aB - c.aA) / (c.pB - c.pA);
}
float angToUs(int j, float a) {
  const ServoCal& c = S.sv[j];
  return c.pA + (a - c.aA) * (c.pB - c.pA) / (c.aB - c.aA);
}
// Usable angle range of a joint: the soft limits AND what the servo can reach
// inside the hard pulse limits. An angle outside this is never sent.
void effRange(int j, float& lo, float& hi) {
  float a1 = usToAng(j, US_MIN_HARD), a2 = usToAng(j, US_MAX_HARD);
  lo = max(S.sv[j].minA, min(a1, a2));
  hi = min(S.sv[j].maxA, max(a1, a2));
}
float clampAng(int j, float a) {
  float lo, hi;
  effRange(j, lo, hi);
  return constrain(a, lo, hi);
}

// Touch points that disagree a lot mean they were saved at wrong poses.
#define MAX_SLOPE 0.05f   // 5 mm per 100 mm
int touchCount() { int n = 0; for (int i = 0; i < NTOUCH; i++) n += S.tp[i].ok; return n; }
bool planeBad() { return touchCount() >= 3 && (fabsf(planeB) > MAX_SLOPE || fabsf(planeC) > MAX_SLOPE); }
