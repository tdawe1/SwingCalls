# Swing Calls

50 SMA (colored), B/S on the SMA vs 5 EMA cross, triangles when RSI leaves 80/20.

## Build

Copy `SwingCalls.cpp` into `ACS_Source` and run **Analysis → Build Custom Studies DLL**.  
Then **Add Custom Study → Swing Calls**. It sits on the price graph.

## Signals

- **B** — SMA crosses under the EMA and the bar high is still above the SMA
- **S** — SMA crosses over the EMA on a down bar (`open > close`)
- **▼** — RSI crosses under 80
- **▲** — RSI crosses over 20

SMA color: lime if the whole bar is above it, red if the whole bar is below, yellow if they overlap or RSI is ≥85 / ≤15.

## Alerts

1 = RSI reversal  
2 = B or S

Study formulas: `SG4 <> 0` buy, `SG5 <> 0` sell, `SG6 <> 0` RSI down, `SG7 <> 0` RSI up.
