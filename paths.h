#pragma once
// Turns strokes (lists of paper points) into motion commands.
// Pen heights follow the paper plane from the touch calibration.

static float zDown(float x, float y) { return paperZ(x, y) - S.press; }
static float zUp(float x, float y)   { return paperZ(x, y) + S.lift; }

struct PathBuilder {
  bool in = false, first = true;
  float lx = 0, ly = 0;
  PathBuilder(size_t reserveN) {
    clearQ();
    qFull = false;
    q.reserve(min(reserveN, (size_t)MAX_CMDS));
  }
  void add(const Cmd& c) {
    if (q.size() >= MAX_CMDS) { qFull = true; return; }
    q.push_back(c);
  }
  void penUp() {
    if (in) add(mk(C_LINE, lx, ly, zUp(lx, ly), S.vZ));
    in = false;
  }
  // Lift, travel, lower the pen at (x, y).
  void moveTo(float x, float y) {
    penUp();
    if (first) {
      add(mk(C_JXYZ, x, y, zUp(x, y) + SAFE_Z));
      add(mk(C_LINE, x, y, zUp(x, y), S.vZ));
      first = false;
    } else {
      add(mk(C_LINE, x, y, zUp(x, y), S.vTravel));
    }
    add(mk(C_LINE, x, y, zDown(x, y), S.vZ));
    in = true; lx = x; ly = y;
  }
  void lineTo(float x, float y) {
    add(mk(C_LINE, x, y, zDown(x, y), S.vDraw));
    lx = x; ly = y;
  }
  void dwell(float ms) { add(mk(C_PAUSE, 0, 0, 0, ms)); }
  void finish() {
    bool drew = !first;
    penUp();
    if (drew) add(mk(C_LINE, lx, ly, zUp(lx, ly) + SAFE_Z, S.vZ));
    add(mk(C_PARK));
  }
};

// Checks the queue and starts it. Returns an error text or nullptr.
const char* startJob(const char* name) {
  if (qFull) { clearQ(); return "Too long for memory. Use fewer letters."; }
  const char* e = checkQ();
  if (e) { clearQ(); return e; }
  lastErr = "";
  job = true;
  jobName = name;
  return nullptr;
}

const char* jobText(const String& txt) {
  if (txt.length() == 0) return "Type some text first.";
  size_t pts = 0, strokes = 0;                       // count first, so memory is reserved once
  layoutText(txt, [&](bool ns, float, float) { pts++; if (ns) strokes++; });
  if (!pts) return "Nothing to draw.";
  PathBuilder pb(pts + 3 * strokes + 6);
  layoutText(txt, [&](bool ns, float x, float y) {
    if (ns) pb.moveTo(x, y); else pb.lineTo(x, y);
  });
  pb.finish();
  return startJob("Text");
}

const char* jobSquare(float side) {
  PathBuilder pb(12);
  float h = side / 2;
  pb.moveTo(S.cx - h, S.cy - h);
  pb.lineTo(S.cx + h, S.cy - h);
  pb.lineTo(S.cx + h, S.cy + h);
  pb.lineTo(S.cx - h, S.cy + h);
  pb.lineTo(S.cx - h, S.cy - h);
  pb.finish();
  return startJob("Test square");
}

// Touch / corner points: 1 near-left, 2 near-right, 3 far-right, 4 far-left, 5 center.
void touchXY(int i, float& x, float& y) {
  const float f[NTOUCH][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}, {0, 0}};
  x = S.cx + f[i][0] * S.areaW / 2;
  y = S.cy + f[i][1] * S.areaH / 2;
}

// Pen touches the 4 corners and the center for 1.5 s each.
const char* jobCorners() {
  PathBuilder pb(30);
  for (int i = 0; i < NTOUCH; i++) {
    float x, y;
    touchXY(i, x, y);
    pb.moveTo(x, y);
    pb.dwell(1500);
  }
  pb.finish();
  return startJob("Corner check");
}

// ---------- Fit area to reach ----------
static bool reachOk(float x, float y) {
  float a[NJ];
  return !ik(x, y, zDown(x, y), a) && !ik(x, y, zUp(x, y), a);
}
static bool reachDown(float x, float y) {   // faster: pen down height only
  float a[NJ];
  return !ik(x, y, zDown(x, y), a);
}

// Fit area rules: the area stays in front of the arm.
#define FIT_MIN_Y     80.0f    // near edge at least this far from the base (mm)
#define FIT_MAX_ANG   55.0f    // corners within +-55 deg of base turn
#define FIT_MAX_W     280.0f   // A4 is 297 mm wide

// Is the whole rectangle (centered on x = 0) reachable? Checks the edges and the middle line.
static bool rectOk(float cy, float w, float h) {
  const float st = 15;
  int nx = max(1, (int)ceilf(w / st)), ny = max(1, (int)ceilf(h / st));
  for (int i = 0; i <= nx; i++) {
    float x = -w / 2 + w * i / nx;
    if (!reachDown(x, cy - h / 2) || !reachDown(x, cy + h / 2) || !reachDown(x, cy)) return false;
  }
  for (int i = 1; i < ny; i++) {
    float y = cy - h / 2 + h * i / ny;
    if (!reachDown(-w / 2, y) || !reachDown(w / 2, y)) return false;
  }
  // corners again with the pen up
  return reachOk(-w / 2, cy - h / 2) && reachOk(w / 2, cy - h / 2) && reachOk(-w / 2, cy + h / 2) && reachOk(w / 2, cy + h / 2);
}

// Finds the widest area (keeping the current depth) the arm can reach at paper height.
const char* fitArea() {
  float h = S.areaH, bestW = 0, bestCy = S.cy;
  float k = tanf(FIT_MAX_ANG * 0.01745329f);
  for (float cy = FIT_MIN_Y + h / 2; cy <= 450; cy += 8) {
    yield();
    float hi = min(FIT_MAX_W, 2 * k * (cy - h / 2));   // keep corners within the angle
    if (hi < 20 || !rectOk(cy, 20, h)) continue;
    float lo = 20;
    if (rectOk(cy, hi, h)) lo = hi;
    while (hi - lo > 6) {
      float mid = (lo + hi) / 2;
      yield();
      if (rectOk(cy, mid, h)) lo = mid; else hi = mid;
    }
    if (lo > bestW) { bestW = lo; bestCy = cy; }
  }
  if (bestW <= 0) return "No area of this depth can be reached. Make Area depth smaller, or check arm sizes.";
  S.cx = 0;
  S.cy = bestCy;
  S.areaW = floorf(bestW - (bestW >= FIT_MAX_W ? 0 : 6));   // small safety margin
  return nullptr;
}
