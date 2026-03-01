#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

static const int W = 240;
static const int H = 135;

// ---------- PRNG (xorshift32) ----------
uint32_t rngState = 1;

uint32_t xrnd() {
  uint32_t x = rngState;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rngState = x;
  return rngState;
}

int irnd(int a, int b) { // inclusive
  return a + (int)(xrnd() % (uint32_t)(b - a + 1));
}

// ---------- Generative State ----------
uint32_t seed = 1;
float rage = 0.0f;

// ---------- Reset World ----------
void reseed(uint32_t newSeed) {
  seed = (newSeed != 0) ? newSeed : 1;
  rngState = seed;
  rage = 0.0f;
}

// ---------- SCREAM GENERATOR ----------
String makeScream(float r) {
  // Uppercase becomes more likely as rage rises
  bool caps = (irnd(0, 100) < (int)(r * 90.0f));

  char A  = caps ? 'A' : 'a';
  char Hc = caps ? 'H' : 'h';

  // More A's with rage (but capped)
  int aCount = 1 + (int)(r * 14.0f);
  if (aCount > 18) aCount = 18;

  // Occasional burst
  if (irnd(0, 100) < (int)(r * 50.0f)) {
    aCount += irnd(2, 5);
  }

  // H's scale with rage but are capped
  int hCount = 3 + (int)(r * 20.0f);
  if (hCount > 28) hCount = 28;

  String s = "";

  for (int i = 0; i < aCount; i++) {
    s += A;
    if (irnd(0, 100) < (int)(r * 15.0f)) s += ' ';
  }

  for (int i = 0; i < hCount; i++) {
    s += Hc;
    if (irnd(0, 100) < (int)(r * 20.0f)) s += ' ';
  }

  // Exclamation burst
  if (irnd(0, 100) < (int)(r * 75.0f)) {
    int bangs = 1 + irnd(0, 5);
    for (int i = 0; i < bangs; i++) s += '!';
  }

  return s;
}

void setup() {
  tft.init();
  tft.setRotation(1); // landscape 240x135
  tft.fillScreen(TFT_BLACK);

  // New “world” each boot
  reseed((uint32_t)esp_random());

  // Text setup
  tft.setTextDatum(MC_DATUM);  // center align
  tft.setTextSize(2);
}

void loop() {
  // -------- Rage Dynamics (slower + breathing) --------
  rage += 0.0008f; 

  if (irnd(0, 1000) < 5) {
    rage += 0.15f; // occasional spike
  }

  rage *= 0.992f;  // stronger decay (breathing)

  // Overload flash + collapse
  if (rage > 1.0f) {
    tft.fillScreen(TFT_WHITE);
    delay(60);
    rage *= 0.35f;
  }

  float intensity = rage;
  if (intensity > 1.0f) intensity = 1.0f;
  if (intensity < 0.0f) intensity = 0.0f;

  // Background
  tft.fillScreen(TFT_BLACK);

  // -------- Noise speckles increase with rage --------
  int specks = (int)(20 + intensity * 400.0f);
  for (int i = 0; i < specks; i++) {
    int x = irnd(0, W - 1);
    int y = irnd(0, H - 1);
    tft.drawPixel(x, y, TFT_DARKGREY);
  }

  // shake
  int shake = (int)(1 + intensity * 10.0f);
  int ox = irnd(-shake, shake);
  int oy = irnd(-shake, shake);

  // text
  String scream = makeScream(intensity);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(scream, W / 2 + ox, H / 2 + oy);

  // echo when rage at peak
  if (irnd(0, 100) < (int)(intensity * 40.0f)) {
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString(scream, W / 2 + ox + irnd(-4, 4), H / 2 + oy + irnd(-2, 2));
  }

  // Show seed (small)
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("seed " + String(seed), W / 2, H - 10);
  tft.setTextSize(2);

  // Frame pacing: p5 frameRate(10) ≈ 100ms per frame
  delay(100);
}

/*
void setup() {
  tft.init();
}

void loop() {
  tft.fillScreen(TFT_BLACK);
  delay(500);
  tft.fillScreen(TFT_WHITE);
  delay(500);
}
  */
