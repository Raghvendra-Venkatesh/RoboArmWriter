#pragma once
// Servo output, smooth joint moves, straight Cartesian lines, and the job queue.
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <vector>

Adafruit_PWMServoDriver pwm(PCA_ADDR);

float curUs[NJ];    // pulse being sent now
float goalUs[NJ];   // pulse we are heading to (used for nudges and jogs)
bool  live[NJ];     // false = servo relaxed (no pulse)

void writeUs(int j, float us) {
  us = constrain(us, (float)US_MIN_HARD, (float)US_MAX_HARD);
  curUs[j] = us;
  pwm.writeMicroseconds(S.sv[j].ch, (uint16_t)lroundf(us));
  live[j] = true;
}

void relaxAll() {
  for (int j = 0; j < NJ; j++) { pwm.setPWM(S.sv[j].ch, 0, 4096); live[j] = false; }
}

void angles(const float us[NJ], float a[NJ]) { for (int j = 0; j < NJ; j++) a[j] = usToAng(j, us[j]); }

// ---------------- job queue ----------------
enum { C_JXYZ, C_LINE, C_PAUSE, C_PARK, C_RAW, C_JOINT };
// Packed to 10 bytes so a 40 letter job fits in RAM. Values are stored x10 (0.1 mm, 0.1 us, 0.1 deg).
struct __attribute__((packed)) Cmd {
  uint8_t t;
  int16_t a, b, c;
  uint16_t v;          // speed in 0.1 mm/s, or pause in ms
  float A() const { return a * 0.1f; }
  float B() const { return b * 0.1f; }
  float C() const { return c * 0.1f; }
  float V() const { return t == C_PAUSE ? (float)v : v * 0.1f; }
};
static int16_t pk10(float f) { return (int16_t)constrain(lroundf(f * 10), -32000L, 32000L); }
inline Cmd mk(uint8_t t, float a = 0, float b = 0, float c = 0, float v = 0) {
  Cmd m; m.t = t; m.a = pk10(a); m.b = pk10(b); m.c = pk10(c);
  m.v = (uint16_t)constrain(lroundf(t == C_PAUSE ? v : v * 10), 0L, 65000L);
  return m;
}

std::vector<Cmd> q;
size_t qi = 0;
bool job = false;
String jobName, lastErr;
uint32_t errId = 0;

// ---------------- active motion ----------------
enum { MV_NONE, MV_JOINT, MV_LINE, MV_PAUSE };
uint8_t mv = MV_NONE;
float mvFrom[NJ], mvTo[NJ];
bool  mvUse[NJ];
float mvT, mvStep;               // progress 0..1 and step per tick
float lnFrom[3], lnTo[3];

bool busy() { return mv != MV_NONE || qi < q.size(); }

void clearQ() { q.clear(); q.shrink_to_fit(); qi = 0; }
bool qFull = false;   // set when a job did not fit

void setErr(const char* e) { lastErr = e; errId++; }

void stopAll(const char* why) {
  clearQ();
  mv = MV_NONE;
  job = false;
  for (int j = 0; j < NJ; j++) goalUs[j] = curUs[j];
  if (why) setErr(why);
}

static char failBuf[120];
void fail(const char* e, float x, float y, float z) {
  snprintf(failBuf, sizeof(failBuf), "Stopped: %s at X%.0f Y%.0f Z%.0f", e, x, y, z);
  stopAll(failBuf);
}

void runSingle(const Cmd& c) { stopAll(nullptr); q.push_back(c); }

// Joint move: to[j] in us, NAN = leave that joint alone. All joints arrive together.
// Speed is in real servo degrees (fixed US_PER_SERVO_DEG), so it does not depend on
// calibration. A half-done calibration must never make a servo crawl in PCA steps.
void startJoint(const float to[NJ], float degPerSec) {
  float maxUs = 0;
  for (int j = 0; j < NJ; j++) {
    mvUse[j] = !isnan(to[j]);
    if (!mvUse[j]) continue;
    mvTo[j] = constrain(to[j], (float)US_MIN_HARD, (float)US_MAX_HARD);
    mvFrom[j] = live[j] ? curUs[j] : mvTo[j];      // relaxed servo: it will jump
    goalUs[j] = mvTo[j];
    float d = fabsf(mvTo[j] - mvFrom[j]);
    if (d > maxUs) maxUs = d;
  }
  float ms = maxUs / US_PER_SERVO_DEG / degPerSec * 1000.0f;
  mvStep = ms <= TICK_MS ? 1.0f : TICK_MS / ms;
  mvT = 0;
  mv = MV_JOINT;
}

void startCmd(const Cmd& c) {
  float to[NJ], a[NJ];
  for (int j = 0; j < NJ; j++) to[j] = NAN;
  switch (c.t) {
    case C_RAW:
      to[(int)c.A()] = c.B();
      startJoint(to, RAW_DEG_S);          // calibration nudges: fast and direct
      break;
    case C_JOINT: {
      int j = (int)c.A();
      to[j] = angToUs(j, clampAng(j, c.B()));
      startJoint(to, S.slew);
    } break;
    case C_PARK:
      for (int j = 0; j < NJ; j++) if (j != J_CLAW) to[j] = angToUs(j, clampAng(j, S.park[j]));
      startJoint(to, S.slew);
      break;
    case C_JXYZ: {
      const char* e = ik(c.A(), c.B(), c.C(), a);
      if (e) { fail(e, c.A(), c.B(), c.C()); return; }
      for (int j = 0; j < NJ; j++) if (j != J_CLAW) to[j] = angToUs(j, clampAng(j, a[j]));
      startJoint(to, S.slew);
    } break;
    case C_LINE: {
      if (!live[J_BASE] || !live[J_SHOULDER] || !live[J_ELBOW] || !live[J_PITCH]) {
        stopAll("Arm is relaxed. Press Park first.");
        return;
      }
      angles(curUs, a);
      fk(a, lnFrom[0], lnFrom[1], lnFrom[2]);
      lnTo[0] = c.A(); lnTo[1] = c.B(); lnTo[2] = c.C();
      float dx = lnTo[0] - lnFrom[0], dy = lnTo[1] - lnFrom[1], dz = lnTo[2] - lnFrom[2];
      float len = sqrtf(dx * dx + dy * dy + dz * dz);
      if (len < 0.05f) { mv = MV_NONE; return; }
      float ms = len / c.V() * 1000.0f;
      mvStep = ms <= TICK_MS ? 1.0f : TICK_MS / ms;
      mvT = 0;
      mv = MV_LINE;
    } break;
    case C_PAUSE:
      mvStep = c.V() <= TICK_MS ? 1.0f : TICK_MS / c.V();
      mvT = 0;
      mv = MV_PAUSE;
      break;
  }
}

// Called every TICK_MS. Progress moves by a fixed step per tick, so a slow
// web request only pauses the arm, it never makes it jump.
void motionTick() {
  if (mv != MV_NONE) {
    mvT += mvStep;
    if (mvT > 1) mvT = 1;
    if (mv == MV_JOINT) {
      float e = mvT * mvT * (3 - 2 * mvT);      // smooth start and stop
      for (int j = 0; j < NJ; j++)
        if (mvUse[j]) writeUs(j, mvFrom[j] + (mvTo[j] - mvFrom[j]) * e);
    } else if (mv == MV_LINE) {
      float p[3], a[NJ];
      for (int i = 0; i < 3; i++) p[i] = lnFrom[i] + (lnTo[i] - lnFrom[i]) * mvT;
      const char* e = ik(p[0], p[1], p[2], a);
      if (e) { fail(e, p[0], p[1], p[2]); return; }
      const int js[4] = {J_BASE, J_SHOULDER, J_ELBOW, J_PITCH};
      for (int k = 0; k < 4; k++) { int j = js[k]; writeUs(j, angToUs(j, a[j])); goalUs[j] = curUs[j]; }
    }
    if (mvT >= 1) mv = MV_NONE;
  }
  if (mv == MV_NONE) {
    if (qi < q.size()) startCmd(q[qi++]);
    else if (!q.empty()) { clearQ(); job = false; }
  }
}

// Check every point of the queued job before moving anything.
static char chkBuf[120];
const char* checkQ() {
  float a[NJ], px = NAN, py = 0, pz = 0;
  for (const Cmd& c : q) {
    yield();
    if (c.t == C_PARK) { px = NAN; continue; }
    if (c.t != C_LINE && c.t != C_JXYZ) continue;
    int n = 1;
    if (c.t == C_LINE && !isnan(px)) {
      float d = sqrtf(sq(c.A() - px) + sq(c.B() - py) + sq(c.C() - pz));
      n = max(1, (int)ceilf(d / 3.0f));
    }
    for (int i = 1; i <= n; i++) {
      float t = (float)i / n;
      float x = isnan(px) ? c.A() : px + (c.A() - px) * t;
      float y = isnan(px) ? c.B() : py + (c.B() - py) * t;
      float z = isnan(px) ? c.C() : pz + (c.C() - pz) * t;
      const char* e = ik(x, y, z, a);
      if (e) {
        snprintf(chkBuf, sizeof(chkBuf), "Cannot reach X%.0f Y%.0f Z%.0f: %s", x, y, z, e);
        return chkBuf;
      }
    }
    px = c.A(); py = c.B(); pz = c.C();
  }
  return nullptr;
}
