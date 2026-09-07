#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"

// =================================================================
// Real-time colour Waterfall, built from genuine, measured RSSI
// sweeps (see Scan.cpp / scanContinuousTick()).
//
// IMPORTANT HARDWARE NOTE: the SI4735/SI4732 receiver is a fully
// integrated chip talking to the ESP32 only over I2C. There is no
// raw IQ/RF sample path into the ESP32, so a true simultaneous FFT
// spectrum is not physically possible on this hardware - the only
// real signal-strength data available is RSSI/SNR for whichever one
// frequency the chip is currently tuned to. This Waterfall is built
// honestly from that: it sweeps the band (re-tuning step by step,
// non-blocking - one step per main loop() iteration), and turns each
// completed sweep into one new, real, colour-mapped history line.
// A new line therefore appears roughly once per sweep (a few seconds),
// not many times a second like a true FFT waterfall would.
// =================================================================

#define WF_ROWS   152  // Sweep-history rows kept and displayed (= 170-WF_TOP, 1px per row)
#define WF_TOP    18   // Y offset where the waterfall image starts (below header)

static uint8_t  wfHistory[WF_ROWS][SCAN_POINTS]; // Circular buffer of raw RSSI per sweep
static uint16_t wfHead     = 0;      // Row index of the most recently completed sweep
static uint16_t wfFilled   = 0;      // How many rows are valid so far (<=WF_ROWS)
static uint16_t wfCenterFreq;
static uint16_t wfSavedFreq;
static bool     wfActive   = false;

//
// Classic SDR waterfall colour ramp: dark blue (weak) -> cyan -> green
// -> yellow -> red (strong). Input 0..255.
//
static uint16_t wfColor(uint8_t v)
{
  uint8_t r, g, b;

  if(v<64)       { r=0;            g=v*2;          b=128+v*2; }
  else if(v<128) { r=0;            g=128+(v-64)*2; b=255-(v-64)*4; }
  else if(v<192) { r=(v-128)*4;    g=255;           b=0; }
  else           { r=255;          g=255-(v-192)*4; b=0; }

  return(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

void waterfallEnter()
{
  wfSavedFreq  = rx.getFrequency();
  wfCenterFreq = currentFrequency;
  wfHead       = 0;
  wfFilled     = 0;
  wfActive     = true;
  memset(wfHistory, 0, sizeof(wfHistory));

  // Sweep with the same step convention already used by the Scan feature
  scanContinuousStart(wfCenterFreq, 10);
}

void waterfallExit()
{
  if(!wfActive) return;
  wfActive = false;
  scanContinuousStop();
  rx.setFrequency(wfSavedFreq);
}

//
// Call once per main loop() iteration while the Waterfall view is open.
// Returns true when a new sweep completed (screen needs a redraw).
//
bool waterfallTickTime()
{
  if(!wfActive) return(false);

  if(!scanContinuousTick()) return(false);

  // A sweep just completed - copy it into the next history row
  wfHead = (wfHead + 1) % WF_ROWS;
  for(int i=0 ; i<SCAN_POINTS ; i++)
    wfHistory[wfHead][i] = scanGetPointRSSI(i);
  if(wfFilled < WF_ROWS) wfFilled++;

  return(true);
}

//
// Full-screen draw, mirroring how drawAbout() takes over the screen.
//
void drawWaterfall()
{
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(TH.text, TH.bg);
  spr.drawString("WATERFALL", 4, 2, 2);

  char freqStr[32];
  snprintf(freqStr, sizeof(freqStr), "%u kHz  SWEEP", (unsigned)wfCenterFreq);
  spr.setTextDatum(TR_DATUM);
  spr.drawString(freqStr, 316, 2, 2);

  // Draw available history rows, oldest at top, newest at bottom
  int rowsToDraw = wfFilled;
  int rowHeight = (170 - WF_TOP) / (WF_ROWS>0 ? WF_ROWS : 1);
  if(rowHeight<1) rowHeight = 1;

  for(int r=0 ; r<rowsToDraw ; r++)
  {
    int idx  = (wfHead - rowsToDraw + 1 + r + WF_ROWS) % WF_ROWS;
    int rowY = 170 - (rowsToDraw - r) * rowHeight;
    if(rowY < WF_TOP) continue;

    // Map SCAN_POINTS samples across the available screen width, and
    // fill the full row height (rowHeight px tall) with each colour
    for(int x=0 ; x<320 ; x++)
    {
      int sampleIdx = x * SCAN_POINTS / 320;
      uint16_t color = wfColor(wfHistory[idx][sampleIdx]);
      spr.drawFastVLine(x, rowY, rowHeight, color);
    }
  }

  spr.pushSprite(0, 0);
}
