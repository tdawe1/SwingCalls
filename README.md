# Swing Calls

Sierra Chart ACSIL port of the TradingView **SWING CALLS** / **SMA call buy/sale** study (Pine v4, nicks1008).

Overlay study: colored SMA, B/S swing entries on the SMA/EMA cross, and RSI reversal triangles.

## Install

1. Copy `SwingCalls.cpp` into your Sierra Chart `ACS_Source` folder, e.g.  
   `C:\SierraChart\ACS_Source\SwingCalls.cpp`
2. In Sierra Chart: **Analysis → Build Custom Studies DLL**
3. **Analysis → Studies → Add Custom Study → Swing Calls**
4. It overlays on the main price graph automatically.

## Signals

| Marker | Rule |
|---|---|
| **B** (below bar) | SMA crosses under the EMA **and** `high > SMA` |
| **S** (above bar) | SMA crosses over the EMA **and** `open > close` (down bar) |
| Triangle down | RSI crosses under the overbought level (default 80) |
| Triangle up | RSI crosses over the oversold level (default 20) |

SMA color:

- **Yellow** — RSI is extreme (≥ 85 or ≤ 15), or the bar overlaps the SMA
- **Lime** — whole bar is above the SMA (`low > SMA`)
- **Red** — whole bar is below the SMA (`high < SMA`)

Defaults match the original Pine: EMA 5, SMA 50, RSI 14 (Wilders).

## Alerts

- **Alert 1** — Possible Reversal on Swing Signal Alert (RSI cross)
- **Alert 2** — Swing Signal Entry Alert (B / S)

Set sounds on the study **Alerts** tab.

Formula alerts (replace `ID1` with this study's ID):

```
ID1.SG4 <> 0    // Buy call
ID1.SG5 <> 0    // Sell call
ID1.SG6 <> 0    // RSI bearish
ID1.SG7 <> 0    // RSI bullish
```

RSI is calculated but not drawn on the overlay so it does not distort the price scale. Read it in **Window → Chart Values**.

## License

Mozilla Public License 2.0. Original TradingView study © nicks1008.
