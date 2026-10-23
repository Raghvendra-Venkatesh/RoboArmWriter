#pragma once
#include <vector>
// Single-stroke font on a 6 wide x 8 tall grid.
// Each point is two digits "xy" (x = 0..6, y = 0..8, y up).
// '|' lifts the pen and starts a new stroke.

static const char* const FONT_AZ[26] = {
  "003860|1353",                               // A
  "044455574808005061635444",                  // B
  "6758180701105061",                          // C
  "00084866624000",                            // D
  "60000868|0444",                             // E
  "000868|0444",                               // F
  "67581807011050616434",                      // G
  "0008|6068|0464",                            // H
  "1050|1858|3038",                            // I
  "2868|585140100102",                         // J
  "0008|6802|2460",                            // K
  "080060",                                    // L
  "0008336860",                                // M
  "00086068",                                  // N
  "180701105061675818",                        // O
  "00084857554404",                            // P
  "180701105061675818|4260",                   // Q
  "00084857554404|3460",                       // R
  "675818070514546361501001",                  // S
  "0868|3038",                                 // T
  "080110506168",                              // U
  "083068",                                    // V
  "0810345068",                                // W
  "0068|0860",                                 // X
  "083468|3430",                               // Y
  "08680060",                                  // Z
};

static const char* const FONT_09[10] = {
  "180701105061675818|1157",                   // 0
  "163830|1050",                               // 1
  "07185867650060",                            // 2
  "07185867655434|546361501001",               // 3
  "50580363",                                  // 4
  "680805456362401001",                        // 5
  "5828060110506163541403",                    // 6
  "086820",                                    // 7
  "14050718586765541403011050616354",          // 8
  "1040626758180705145465",                    // 9
};

// Returns nullptr for space and unknown characters (they become a blank).
const char* glyph(char c) {
  if (c >= 'a' && c <= 'z') c -= 32;
  if (c >= 'A' && c <= 'Z') return FONT_AZ[c - 'A'];
  if (c >= '0' && c <= '9') return FONT_09[c - '0'];
  switch (c) {
    case '-': return "1454";
    case '.': return "3031";
    case '!': return "3833|3031";
    case '?': return "0718586765543432|3031";
  }
  return nullptr;
}

// Splits text into lines that fit the writing area width. Words are kept whole
// unless one word alone is wider than the area.
std::vector<String> wrapText(const String& txt) {
  float adv = 6 * S.textH / 8.0f + S.gap;
  int maxCh = max(1, (int)floorf((S.areaW + S.gap) / adv));
  std::vector<String> lines;
  String cur;
  int i = 0, n = txt.length();
  while (i < n) {
    while (i < n && txt[i] == ' ') i++;
    int st = i;
    while (i < n && txt[i] != ' ') i++;
    String w = txt.substring(st, i);
    if (!w.length()) break;
    while ((int)w.length() > maxCh) {                 // very long word: cut it
      if (cur.length()) { lines.push_back(cur); cur = ""; }
      lines.push_back(w.substring(0, maxCh));
      w = w.substring(maxCh);
    }
    if (!cur.length()) cur = w;
    else if ((int)(cur.length() + 1 + w.length()) <= maxCh) cur += " " + w;
    else { lines.push_back(cur); cur = w; }
  }
  if (cur.length()) lines.push_back(cur);
  return lines;
}

// Lays out text in the writing area and calls emit(newStroke, x, y) for every point (mm).
// Lines are centered on (cx, cy). The first line is the one furthest from the base.
// Returns the number of lines.
template <typename F>
int layoutText(const String& txt, F emit) {
  std::vector<String> lines = wrapText(txt);
  int nl = lines.size();
  float sc = S.textH / 8.0f;
  float adv = 6 * sc + S.gap;
  float pitch = S.textH * 1.6f;                       // line spacing
  float blockH = S.textH + (nl - 1) * pitch;
  float yTop = S.cy + blockH / 2;
  for (int li = 0; li < nl; li++) {
    const String& L = lines[li];
    int n = L.length();
    float x0 = S.cx - (n * adv - S.gap) / 2;
    float y0 = yTop - S.textH - li * pitch;
    for (int k = 0; k < n; k++) {
      const char* g = glyph(L[k]);
      if (!g) continue;
      bool ns = true;
      for (const char* p = g; *p;) {
        if (*p == '|') { ns = true; p++; continue; }
        if (!p[1]) break;
        float gx = p[0] - '0', gy = p[1] - '0';
        p += 2;
        float X = x0 + k * adv + gx * sc, Y = y0 + gy * sc;
        if (S.rot180) { X = 2 * S.cx - X; Y = 2 * S.cy - Y; }
        emit(ns, X, Y);
        ns = false;
      }
    }
  }
  return nl;
}
