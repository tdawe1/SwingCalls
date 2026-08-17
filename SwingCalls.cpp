#include "sierrachart.h"

/*==========================================================================
	SWING CALLS  —  Sierra Chart ACSIL port of the TradingView study

	Original:  "SWING CALLS" / "SMA call buy/sale"  (Pine v4)
	Author:    nicks1008
	License:   Mozilla Public License 2.0
	           https://mozilla.org/MPL/2.0/

	What it does (same rules as the Pine script)
	--------------------------------------------
	Plots a 50-period SMA of close, colored:
	  yellow  — RSI is extreme (>= 85 or <= 15), or price is overlapping the SMA
	  lime    — the whole bar is above the SMA (low > SMA)
	  red     — the whole bar is below the SMA (high < SMA)

	Entry markers (B / S)
	  B  — SMA crosses under the 5-period EMA  AND  high > SMA
	  S  — SMA crosses over  the 5-period EMA  AND  open > close  (down bar)

	RSI reversal markers
	  triangle down  — RSI crosses under the overbought level (default 80)
	  triangle up    — RSI crosses over  the oversold  level (default 20)

	Alerts (Study Settings >> Alerts, or the built-in SetAlert calls)
	  Alert 1  — Possible Reversal on Swing Signal Alert   (RSI cross)
	  Alert 2  — Swing Signal Entry Alert                  (B / S)

	How to install
	--------------
	1. Copy this file into your Sierra Chart ACS_Source folder, e.g.
	     C:\SierraChart\ACS_Source\SwingCalls.cpp
	2. In Sierra Chart:  Analysis >> Build Custom Studies DLL
	   (or Analysis >> Build >> Build Custom Studies Source Code)
	3. Analysis >> Studies >> Add Custom Study >> "Swing Calls"
	4. Apply it to the main price graph (it overlays automatically).

	Study/Chart Alert formulas (optional, if you prefer formula alerts)
	  Buy  call:   ID0.SG4 <> 0     (replace ID0 with this study's ID)
	  Sell call:   ID0.SG5 <> 0
	  RSI bearish: ID0.SG6 <> 0
	  RSI bullish: ID0.SG7 <> 0
==========================================================================*/

SCDLLName("Swing Calls")

SCSFExport scsf_SwingCalls(SCStudyInterfaceRef sc)
{
	// ---- Subgraphs -------------------------------------------------------
	SCSubgraphRef LongSMA          = sc.Subgraph[0];  // colored SMA (visible)
	SCSubgraphRef ShortEMA         = sc.Subgraph[1];  // EMA, hidden by default
	SCSubgraphRef RSILine          = sc.Subgraph[2];  // RSI, hidden (Chart Values)
	SCSubgraphRef BuyCall          = sc.Subgraph[3];  // "B" below bar
	SCSubgraphRef SellCall         = sc.Subgraph[4];  // "S" above bar
	SCSubgraphRef RSIBearish       = sc.Subgraph[5];  // triangle down (RSI exit)
	SCSubgraphRef RSIBullish       = sc.Subgraph[6];  // triangle up   (RSI exit)

	// ---- Inputs ----------------------------------------------------------
	SCInputRef InEMALength         = sc.Input[0];
	SCInputRef InSMALength         = sc.Input[1];
	SCInputRef InRSILength         = sc.Input[2];
	SCInputRef InRSIAvgType        = sc.Input[3];
	SCInputRef InRSIOverbought     = sc.Input[4];
	SCInputRef InRSIOversold       = sc.Input[5];
	SCInputRef InRSIExtremeHigh    = sc.Input[6];
	SCInputRef InRSIExtremeLow     = sc.Input[7];
	SCInputRef InMarkerOffsetTicks = sc.Input[8];
	SCInputRef InSignalsOnClose    = sc.Input[9];
	SCInputRef InEnableAlerts      = sc.Input[10];
	SCInputRef InColorExtreme      = sc.Input[11];
	SCInputRef InColorAboveSMA     = sc.Input[12];
	SCInputRef InColorBelowSMA     = sc.Input[13];
	SCInputRef InColorOverlap      = sc.Input[14];

	if (sc.SetDefaults)
	{
		sc.GraphName        = "Swing Calls";
		sc.StudyDescription =
			"Overlay study ported from TradingView SWING CALLS (nicks1008). "
			"Colored SMA, B/S swing entries on SMA/EMA cross, RSI reversal triangles.";
		sc.AutoLoop         = 1;
		sc.GraphRegion      = 0;
		sc.ValueFormat      = VALUEFORMAT_INHERITED;
		sc.ScaleRangeType   = SCALE_SAMEASREGION;
		sc.DrawZeros        = 0;
		sc.DrawStudyUnderneathMainPriceGraph = 0;

		// --- Inputs (defaults match the Pine script) ----------------------
		InEMALength.Name = "EMA Length";
		InEMALength.SetInt(5);
		InEMALength.SetIntLimits(1, 10000);
		InEMALength.SetDescription("Fast EMA of close. Default 5.");

		InSMALength.Name = "SMA Length";
		InSMALength.SetInt(50);
		InSMALength.SetIntLimits(1, 10000);
		InSMALength.SetDescription("Slow SMA of close (the colored line). Default 50.");

		InRSILength.Name = "RSI Length";
		InRSILength.SetInt(14);
		InRSILength.SetIntLimits(1, 10000);
		InRSILength.SetDescription("RSI period. Hardcoded to 14 in the original Pine.");

		InRSIAvgType.Name = "RSI Moving Average Type";
		InRSIAvgType.SetMovAvgType(MOVAVGTYPE_WILDERS);
		InRSIAvgType.SetDescription(
			"Wilders matches TradingView ta.rsi / rsi(). "
			"Change only if you want a different RSI flavor.");

		InRSIOverbought.Name = "RSI Overbought Limit";
		InRSIOverbought.SetFloat(80.0f);
		InRSIOverbought.SetFloatLimits(0.0f, 100.0f);
		InRSIOverbought.SetDescription(
			"Pine: hl. RSI crossing under this prints the bearish reversal triangle.");

		InRSIOversold.Name = "RSI Oversold Limit";
		InRSIOversold.SetFloat(20.0f);
		InRSIOversold.SetFloatLimits(0.0f, 100.0f);
		InRSIOversold.SetDescription(
			"Pine: ll. RSI crossing over this prints the bullish reversal triangle.");

		InRSIExtremeHigh.Name = "RSI Extreme High (SMA yellow)";
		InRSIExtremeHigh.SetFloat(85.0f);
		InRSIExtremeHigh.SetFloatLimits(0.0f, 100.0f);
		InRSIExtremeHigh.SetDescription(
			"Hardcoded 85 in Pine. SMA turns yellow when RSI is at or above this.");

		InRSIExtremeLow.Name = "RSI Extreme Low (SMA yellow)";
		InRSIExtremeLow.SetFloat(15.0f);
		InRSIExtremeLow.SetFloatLimits(0.0f, 100.0f);
		InRSIExtremeLow.SetDescription(
			"Hardcoded 15 in Pine. SMA turns yellow when RSI is at or below this.");

		InMarkerOffsetTicks.Name = "Marker Offset (ticks)";
		InMarkerOffsetTicks.SetInt(4);
		InMarkerOffsetTicks.SetIntLimits(0, 1000);
		InMarkerOffsetTicks.SetDescription(
			"How far above/below the bar to place B/S text and RSI triangles.");

		InSignalsOnClose.Name = "Signals On Bar Close Only";
		InSignalsOnClose.SetYesNo(0);
		InSignalsOnClose.SetDescription(
			"No (default) matches TradingView: signals can print on the forming bar. "
			"Yes waits until the bar is closed (less flicker, later signal).");

		InEnableAlerts.Name = "Enable Alerts";
		InEnableAlerts.SetYesNo(1);
		InEnableAlerts.SetDescription(
			"Alert 1 = RSI reversal. Alert 2 = B/S swing entry. "
			"Configure sounds on the Alerts tab of this study.");

		InColorExtreme.Name = "SMA Color: RSI Extreme";
		InColorExtreme.SetColor(RGB(255, 255, 0));   // yellow

		InColorAboveSMA.Name = "SMA Color: Bar Fully Above";
		InColorAboveSMA.SetColor(RGB(0, 255, 0));    // lime

		InColorBelowSMA.Name = "SMA Color: Bar Fully Below";
		InColorBelowSMA.SetColor(RGB(255, 0, 0));    // red

		InColorOverlap.Name = "SMA Color: Price Overlapping";
		InColorOverlap.SetColor(RGB(255, 255, 0));   // yellow

		// --- Subgraph appearance ------------------------------------------
		LongSMA.Name        = "Long SMA";
		LongSMA.DrawStyle   = DRAWSTYLE_LINE;
		LongSMA.PrimaryColor = RGB(255, 255, 0);
		LongSMA.LineWidth   = 2;
		LongSMA.DrawZeros   = false;
		LongSMA.LineLabel   = LL_DISPLAY_NAME | LL_DISPLAY_VALUE | LL_VALUE_ALIGN_VALUES_SCALE;

		// Hidden by default — original Pine does not plot the EMA.
		// Change Draw Style to Line on the Subgraphs tab if you want it visible.
		ShortEMA.Name        = "Short EMA";
		ShortEMA.DrawStyle   = DRAWSTYLE_IGNORE;
		ShortEMA.PrimaryColor = RGB(0, 160, 255);
		ShortEMA.LineWidth   = 1;
		ShortEMA.DrawZeros   = false;

		// Hidden — 0-100 scale would distort the price graph if drawn.
		RSILine.Name         = "RSI";
		RSILine.DrawStyle    = DRAWSTYLE_IGNORE;
		RSILine.PrimaryColor = RGB(180, 180, 180);
		RSILine.DrawZeros    = false;
		RSILine.ValueFormat  = 1;

		BuyCall.Name              = "Buy Call";
		BuyCall.DrawStyle         = DRAWSTYLE_TEXT_WITH_BACKGROUND;
		BuyCall.PrimaryColor      = RGB(255, 255, 255);  // text
		BuyCall.SecondaryColor    = RGB(0, 255, 255);    // aqua fill (Pine color.aqua)
		BuyCall.SecondaryColorUsed = 1;
		BuyCall.LineWidth         = 10;                  // font height
		BuyCall.DrawZeros         = false;
		BuyCall.TextDrawStyleText = "B";

		SellCall.Name              = "Sell Call";
		SellCall.DrawStyle         = DRAWSTYLE_TEXT_WITH_BACKGROUND;
		SellCall.PrimaryColor      = RGB(0, 0, 0);        // text
		SellCall.SecondaryColor    = RGB(255, 0, 0);      // red fill
		SellCall.SecondaryColorUsed = 1;
		SellCall.LineWidth         = 10;
		SellCall.DrawZeros         = false;
		SellCall.TextDrawStyleText = "S";

		RSIBearish.Name        = "RSI Alert Bearish";
		RSIBearish.DrawStyle   = DRAWSTYLE_TRIANGLE_DOWN;
		RSIBearish.PrimaryColor = RGB(0, 128, 128);      // teal
		RSIBearish.LineWidth   = 8;
		RSIBearish.DrawZeros   = false;

		RSIBullish.Name        = "RSI Alert Bullish";
		RSIBullish.DrawStyle   = DRAWSTYLE_TRIANGLE_UP;
		RSIBullish.PrimaryColor = RGB(0, 128, 128);      // teal
		RSIBullish.LineWidth   = 8;
		RSIBullish.DrawZeros   = false;

		return;
	}

	// Recalc start so early bars (before the longest lookback) are skipped.
	const int EMALength  = InEMALength.GetInt();
	const int SMALength  = InSMALength.GetInt();
	const int RSILength  = InRSILength.GetInt();
	int StartIndex = SMALength;
	if (EMALength > StartIndex) StartIndex = EMALength;
	if (RSILength > StartIndex) StartIndex = RSILength;
	sc.DataStartIndex = StartIndex;

	// ---- Indicators ------------------------------------------------------
	// EMA and SMA of close. RSI uses Wilders by default (TradingView rsi()).
	sc.ExponentialMovAvg(sc.BaseDataIn[SC_LAST], ShortEMA, EMALength);
	sc.SimpleMovAvg(sc.BaseDataIn[SC_LAST], LongSMA, SMALength);
	sc.RSI(sc.BaseDataIn[SC_LAST], RSILine, InRSIAvgType.GetMovAvgType(), RSILength);

	const int Index = sc.Index;

	// Always clear this bar's markers first. The last (forming) bar is
	// recalculated on every tick, so a signal that is no longer true must
	// disappear the same way plotshape does in Pine.
	BuyCall[Index]    = 0.0f;
	SellCall[Index]   = 0.0f;
	RSIBearish[Index] = 0.0f;
	RSIBullish[Index] = 0.0f;

	if (Index < StartIndex)
		return;

	const float SMA    = LongSMA[Index];
	const float RSINow = RSILine[Index];

	// ---- SMA color (Pine mycolor) ----------------------------------------
	// yellow if RSI extreme, else lime if low > SMA, else red if high < SMA,
	// else yellow (bar overlapping the SMA).
	COLORREF SMAColor;
	if (RSINow >= InRSIExtremeHigh.GetFloat() || RSINow <= InRSIExtremeLow.GetFloat())
		SMAColor = InColorExtreme.GetColor();
	else if (sc.Low[Index] > SMA)
		SMAColor = InColorAboveSMA.GetColor();
	else if (sc.High[Index] < SMA)
		SMAColor = InColorBelowSMA.GetColor();
	else
		SMAColor = InColorOverlap.GetColor();

	LongSMA.DataColor[Index] = SMAColor;

	// Need a prior bar for any crossover. Color is already set above.
	if (Index < 1)
		return;

	// Optional: hold B/S and RSI markers until the bar closes (Pine does not).
	if (InSignalsOnClose.GetYesNo()
		&& sc.GetBarHasClosedStatus(Index) == BHCS_BAR_HAS_NOT_CLOSED)
	{
		return;
	}

	const float RSIPrev = RSILine[Index - 1];

	// ---- Crossovers ------------------------------------------------------
	// Pine: buyexit  = crossunder(rs, hl)   RSI falls through overbought
	//       sellexit = crossover (rs, ll)   RSI rises through oversold
	const float OB = InRSIOverbought.GetFloat();
	const float OS = InRSIOversold.GetFloat();
	const bool RSICrossUnderOB = (RSIPrev >= OB && RSINow < OB);
	const bool RSICrossOverOS  = (RSIPrev <= OS && RSINow > OS);

	// Pine: sellcall = crossover (sma2, ema1) and open > close
	//       buycall  = crossunder(sma2, ema1) and high > sma2
	// sc.CrossOver(A, B) == CROSS_FROM_BOTTOM  =>  A crossed up through B
	// sc.CrossOver(A, B) == CROSS_FROM_TOP     =>  A crossed down through B
	const int SMAEMACross = sc.CrossOver(LongSMA, ShortEMA);
	const bool SellCallCond = (SMAEMACross == CROSS_FROM_BOTTOM)
		&& (sc.Open[Index] > sc.Close[Index]);
	const bool BuyCallCond  = (SMAEMACross == CROSS_FROM_TOP)
		&& (sc.High[Index] > SMA);

	// ---- Place markers ---------------------------------------------------
	float Offset = InMarkerOffsetTicks.GetInt() * sc.TickSize;
	if (Offset <= 0.0f)
		Offset = sc.TickSize;

	if (BuyCallCond)
		BuyCall[Index] = sc.Low[Index] - Offset;

	if (SellCallCond)
		SellCall[Index] = sc.High[Index] + Offset;

	if (RSICrossUnderOB)
		RSIBearish[Index] = sc.High[Index] + Offset;

	if (RSICrossOverOS)
		RSIBullish[Index] = sc.Low[Index] - Offset;

	// ---- Alerts (once per bar, last bar only) ----------------------------
	// Persistent ints remember the last bar we already alerted so a forming
	// bar that stays true across many ticks does not spam the Alert Log.
	int& LastReversalAlertBar = sc.GetPersistentInt(1);
	int& LastEntryAlertBar    = sc.GetPersistentInt(2);

	if (!InEnableAlerts.GetYesNo() || Index != sc.ArraySize - 1)
		return;

	if ((RSICrossUnderOB || RSICrossOverOS) && LastReversalAlertBar != Index)
	{
		sc.SetAlert(1, "Possible Reversal on Swing Signal Alert");
		LastReversalAlertBar = Index;
	}

	if ((BuyCallCond || SellCallCond) && LastEntryAlertBar != Index)
	{
		sc.SetAlert(2, "Swing Signal Entry Alert");
		LastEntryAlertBar = Index;
	}
}
