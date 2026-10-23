#pragma once
// Inverse and forward kinematics for a 4-joint writing arm.
//
// Arm frame (look from behind the base, facing the paper):
//   X = right, Y = forward (away from base), Z = up from the table.
//   Origin = base rotation axis on the table surface.
// Joint angles:
//   base     : 0 = forward, + = turn right
//   shoulder : upper arm angle from horizontal, + = up (90 = vertical)
//   elbow    : bend from straight, + = forearm bends down
//   pitch    : bend of the pen from the forearm line, + = down
// Absolute pen angle = shoulder - elbow - pitch  (-90 = straight down).
//
// Auto tilt: the pen may lean up to S.maxTilt deg from straight down, either way.
//   Tip leaning back toward the base  -> wrist moves out and down -> shoulder much lower.
//   Tip leaning away from the base    -> wrist moves in -> more reach at the far edge.
// Tilts are tried from "most back" to "most away"; the first one that fits all
// joint limits wins. That is the lowest shoulder the arm can use at that point.
//
// Side offset: the pen tip sits S.sideOff mm to the right of the arm's plane.

static inline float d2r(float d) { return d * 0.01745329252f; }
static inline float r2d(float r) { return r * 57.2957795f; }

static char ikMsg[80];

// Checks soft limits and the real servo pulse range (500 to 2500 us).
static const char* checkLim(int j, float a) {
  float lo, hi;
  effRange(j, lo, hi);
  if (a < lo - 0.01f || a > hi + 0.01f) {
    snprintf(ikMsg, sizeof(ikMsg), "%s limit (needs %.0f deg, range %.0f to %.0f)", JNAME[j], a, lo, hi);
    return ikMsg;
  }
  return nullptr;
}

// One pen angle. r = distance from base axis in the arm plane.
static const char* ikPlane(float r, float z, float penAng, float a[NJ]) {
  float pa = d2r(penAng);
  float wr = r - S.d0 - S.L3 * cosf(pa);      // wrist pivot, radial
  float wz = z - S.H - S.L3 * sinf(pa);       // wrist pivot, height above shoulder
  float D2 = wr * wr + wz * wz, D = sqrtf(D2);
  if (D > S.L1 + S.L2 - 0.5f) return "too far to reach";
  if (D < fabsf(S.L1 - S.L2) + 0.5f || D < 1) return "too close to reach";

  float ci = (S.L1 * S.L1 + S.L2 * S.L2 - D2) / (2 * S.L1 * S.L2);
  float cs = (S.L1 * S.L1 + D2 - S.L2 * S.L2) / (2 * S.L1 * D);
  float e = 180.0f - r2d(acosf(constrain(ci, -1.0f, 1.0f)));            // elbow-up solution
  float s = r2d(atan2f(wz, wr) + acosf(constrain(cs, -1.0f, 1.0f)));
  a[J_SHOULDER] = s;
  a[J_ELBOW] = e;
  a[J_PITCH] = s - e - penAng;

  const char* m;
  if ((m = checkLim(J_SHOULDER, s))) return m;
  if ((m = checkLim(J_ELBOW, e))) return m;
  if ((m = checkLim(J_PITCH, a[J_PITCH]))) return m;
  return nullptr;
}

// Pen tip at (x, y, z) mm. Fills a[] with joint angles.
// Returns nullptr if fine, or a short reason if the point cannot be reached.
const char* ik(float x, float y, float z, float a[NJ]) {
  float R2 = x * x + y * y, off = S.sideOff;
  if (R2 < 400 || R2 <= off * off + 100) return "too close to base";
  float r = sqrtf(R2 - off * off);                        // distance in the arm plane
  a[J_BASE] = r2d(atan2f(x, y) - asinf(off / sqrtf(R2)));
  a[J_ROLL] = S.rollDraw;
  a[J_CLAW] = 0;
  const char* m = checkLim(J_BASE, a[J_BASE]);
  if (m) return m;

  // Pen angle = -90 + t. Negative t = tip toward the base (wrist further out, arm lower).
  for (float t = -S.maxTilt;; t += TILT_STEP) {
    if (t > S.maxTilt) t = S.maxTilt;
    m = ikPlane(r, z, -90.0f + t, a);
    if (!m || t >= S.maxTilt) return m;
  }
}

void fk(const float a[NJ], float& x, float& y, float& z) {
  float s = d2r(a[J_SHOULDER]);
  float f = d2r(a[J_SHOULDER] - a[J_ELBOW]);
  float p = d2r(a[J_SHOULDER] - a[J_ELBOW] - a[J_PITCH]);
  float r = S.d0 + S.L1 * cosf(s) + S.L2 * cosf(f) + S.L3 * cosf(p);
  z = S.H + S.L1 * sinf(s) + S.L2 * sinf(f) + S.L3 * sinf(p);
  float b = d2r(a[J_BASE]);
  x = r * sinf(b) + S.sideOff * cosf(b);
  y = r * cosf(b) - S.sideOff * sinf(b);
}
