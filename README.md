# Swing Calls

Sierra Chart study: colored 50 SMA, B/S on the SMA vs 5 EMA cross, triangles when RSI leaves 80/20.

Use the compiled **`SwingCalls_64.dll`**. Opening the `.cpp` file does nothing on the chart.

## Install (DLL)

1. Copy `SwingCalls_64.dll` into your Sierra Chart **Data** folder, usually:

   `C:\SierraChart\Data\SwingCalls_64.dll`

   Path is shown in **Global Settings → General Settings → Paths → Data Files Folder**.

2. Restart Sierra Chart if it was already running.

3. Open a price chart. **Analysis → Studies → Add Custom Study**
   - Expand **Swing Calls**
   - Select **Swing Calls**
   - **Add** → **OK**

4. A yellow/lime/red SMA should sit on the price graph. **Window → Message Log** should show `Swing Calls is running on this chart.`

This DLL is 64-bit for current Sierra Chart. It will not show up in an old 32-bit install.

## Rebuild from source

Needs `x86_64-w64-mingw32-g++` and Sierra Chart's `ACS_Source` headers:

```
make
make install
```

That writes `SwingCalls_64.dll` here and copies it to `C:\SierraChart\Data`. Restart Sierra Chart (or **Chart → Recalculate**) and add the study as above.

Or in Sierra Chart: copy `SwingCalls.cpp` to `ACS_Source`, then **Analysis → Build Custom Studies DLL → File → Select Files → Build → Remote Build**.

## Signals

- **B** — SMA crosses under the EMA and the bar high is still above the SMA
- **S** — SMA crosses over the EMA on a down bar (`open > close`)
- Triangle down — RSI crosses under 80
- Triangle up — RSI crosses over 20

SMA color: lime if the whole bar is above it, red if the whole bar is below, yellow if they overlap or RSI is ≥85 / ≤15.

B and S marks are uncommon. The colored SMA should show immediately after you add the study.

## Alerts

1 = RSI reversal  
2 = B or S

Study formulas: `SG4 <> 0` buy, `SG5 <> 0` sell, `SG6 <> 0` RSI down, `SG7 <> 0` RSI up.
