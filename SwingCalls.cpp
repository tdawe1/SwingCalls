#include "sierrachart.h"

/*==========================================================================
	SWING CALLS

	Sierra Chart ACSIL port of TradingView "SWING CALLS" / "SMA call buy/sale"
	(Pine v4, nicks1008). Mozilla Public License 2.0.

	Opening this .cpp file does nothing on the chart. Sierra Chart will not
	run source code. You have to compile it, then add the study.

	INSTALL (do every step)
	-----------------------
	1. Copy this file to the ACS_Source folder of the MAIN Sierra Chart
	   install (not a sub instance), for example:
	     C:\SierraChart\ACS_Source\SwingCalls.cpp
	   The filename must stay SwingCalls.cpp with no spaces.

	2. In Sierra Chart, open a price chart.

	3. Analysis >> Build Custom Studies DLL
	     File >> Select Files  ->  choose SwingCalls.cpp  ->  Open
	     Build >> Remote Build
	   Wait until the output says the remote build succeeded.
	   If it failed, the errors are in that same output window.

	4. Analysis >> Studies >> Add Custom Study
	     open "Swing Calls"
	     select "Swing Calls"
	     Add  ->  OK

	5. You should see a yellow/lime/red 50 SMA on the price graph.
	   Window >> Message Log will also show "Swing Calls is running".

	Do not use File >> Open on this .cpp. That is for chartbooks.
	Do not use Analysis >> New/Open Custom Studies File and expect
	the study to appear. That only opens a text editor.

	Signals
	  B  SMA crosses under the 5 EMA and high > SMA
	  S  SMA crosses over  the 5 EMA and open > close
	  triangle down  RSI crosses under 80
	  triangle up    RSI crosses over  20

	SMA color
	  yellow  RSI >= 85 or RSI <= 15, or the bar overlaps the SMA
	  lime    whole bar above the SMA (low > SMA)
	  red     whole bar below the SMA (high < SMA)

	Alerts (study Alerts tab)
	  1  RSI reversal
	  2  B or S entry

	Formula alerts (replace ID1 with this study ID):
	  ID1.SG4 <> 0   buy
	  ID1.SG5 <> 0   sell
	  ID1.SG6 <> 0   RSI down
	  ID1.SG7 <> 0   RSI up
==========================================================================*/

SCDLLName("Swing Calls")

SCSFExport scsf_SwingCalls(SCStudyInterfaceRef sc)
{
	SCSubgraphRef LongSMA    = sc.Subgraph[0];
	SCSubgraphRef ShortEMA   = sc.Subgraph[1];
	SCSubgraphRef RSILine    = sc.Subgraph[2];
	SCSubgraphRef BuyCall    = sc.Subgraph[3];
	SCSubgraphRef SellCall   = sc.Subgraph[4];
	SCSubgraphRef RSIBearish = sc.Subgraph[5];
	SCSubgraphRef RSIBullish = sc.Subgraph[6];

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
		sc.GraphName = "Swing Calls";
		sc.StudyDescription =
			"Colored 50 SMA, B/S on SMA vs 5 EMA cross, RSI reversal triangles. "
			"Port of TradingView SWING CALLS (nicks1008).";
		sc.AutoLoop = 1;
		sc.GraphRegion = 0;
		sc.ValueFormat = VALUEFORMAT_INHERITED;
		sc.ScaleRangeType = SCALE_SAMEASREGION;
		sc.DrawZeros = 0;

		InEMALength.Name = "EMA Length";
		InEMALength.SetInt(5);
		InEMALength.SetIntLimits(1, 10000);

		InSMALength.Name = "SMA Length";
		InSMALength.SetInt(50);
		InSMALength.SetIntLimits(1, 10000);

		InRSILength.Name = "RSI Length";
		InRSILength.SetInt(14);
		InRSILength.SetIntLimits(1, 10000);

		InRSIAvgType.Name = "RSI Moving Average Type";
		InRSIAvgType.SetMovAvgType(MOVAVGTYPE_WILDERS);

		InRSIOverbought.Name = "RSI Overbought Limit";
		InRSIOverbought.SetFloat(80.0f);
		InRSIOverbought.SetFloatLimits(0.0f, 100.0f);

		InRSIOversold.Name = "RSI Oversold Limit";
		InRSIOversold.SetFloat(20.0f);
		InRSIOversold.SetFloatLimits(0.0f, 100.0f);

		InRSIExtremeHigh.Name = "RSI Extreme High (SMA yellow)";
		InRSIExtremeHigh.SetFloat(85.0f);
		InRSIExtremeHigh.SetFloatLimits(0.0f, 100.0f);

		InRSIExtremeLow.Name = "RSI Extreme Low (SMA yellow)";
		InRSIExtremeLow.SetFloat(15.0f);
		InRSIExtremeLow.SetFloatLimits(0.0f, 100.0f);

		InMarkerOffsetTicks.Name = "Marker Offset (ticks)";
		InMarkerOffsetTicks.SetInt(4);
		InMarkerOffsetTicks.SetIntLimits(0, 1000);

		InSignalsOnClose.Name = "Signals On Bar Close Only";
		InSignalsOnClose.SetYesNo(0);

		InEnableAlerts.Name = "Enable Alerts";
		InEnableAlerts.SetYesNo(1);

		InColorExtreme.Name = "SMA Color: RSI Extreme";
		InColorExtreme.SetColor(RGB(255, 255, 0));

		InColorAboveSMA.Name = "SMA Color: Bar Fully Above";
		InColorAboveSMA.SetColor(RGB(0, 255, 0));

		InColorBelowSMA.Name = "SMA Color: Bar Fully Below";
		InColorBelowSMA.SetColor(RGB(255, 0, 0));

		InColorOverlap.Name = "SMA Color: Price Overlapping";
		InColorOverlap.SetColor(RGB(255, 255, 0));

		LongSMA.Name = "Long SMA";
		LongSMA.DrawStyle = DRAWSTYLE_LINE;
		LongSMA.PrimaryColor = RGB(255, 255, 0);
		LongSMA.LineWidth = 3;
		LongSMA.DrawZeros = false;

		// Hidden. Original Pine does not plot the EMA.
		ShortEMA.Name = "Short EMA";
		ShortEMA.DrawStyle = DRAWSTYLE_IGNORE;
		ShortEMA.PrimaryColor = RGB(0, 160, 255);
		ShortEMA.LineWidth = 1;
		ShortEMA.DrawZeros = false;

		// Hidden. RSI is 0-100 and would wreck the price scale if drawn.
		RSILine.Name = "RSI";
		RSILine.DrawStyle = DRAWSTYLE_IGNORE;
		RSILine.PrimaryColor = RGB(180, 180, 180);
		RSILine.DrawZeros = false;
		RSILine.ValueFormat = 1;

		BuyCall.Name = "Buy Call";
		BuyCall.DrawStyle = DRAWSTYLE_TEXT;
		BuyCall.PrimaryColor = RGB(0, 255, 255);
		BuyCall.LineWidth = 12;
		BuyCall.DrawZeros = false;
		BuyCall.TextDrawStyleText = "B";

		SellCall.Name = "Sell Call";
		SellCall.DrawStyle = DRAWSTYLE_TEXT;
		SellCall.PrimaryColor = RGB(255, 0, 0);
		SellCall.LineWidth = 12;
		SellCall.DrawZeros = false;
		SellCall.TextDrawStyleText = "S";

		RSIBearish.Name = "RSI Alert Bearish";
		RSIBearish.DrawStyle = DRAWSTYLE_TRIANGLE_DOWN;
		RSIBearish.PrimaryColor = RGB(0, 128, 128);
		RSIBearish.LineWidth = 8;
		RSIBearish.DrawZeros = false;

		RSIBullish.Name = "RSI Alert Bullish";
		RSIBullish.DrawStyle = DRAWSTYLE_TRIANGLE_UP;
		RSIBullish.PrimaryColor = RGB(0, 128, 128);
		RSIBullish.LineWidth = 8;
		RSIBullish.DrawZeros = false;

		return;
	}

	const int EMALength = InEMALength.GetInt();
	const int SMALength = InSMALength.GetInt();
	const int RSILength = InRSILength.GetInt();

	int StartIndex = SMALength;
	if (EMALength > StartIndex)
		StartIndex = EMALength;
	if (RSILength > StartIndex)
		StartIndex = RSILength;
	sc.DataStartIndex = StartIndex;

	if (sc.Index == 0)
		sc.AddMessageToLog("Swing Calls is running on this chart.", 1);

	sc.ExponentialMovAvg(sc.Close, ShortEMA, EMALength);
	sc.SimpleMovAvg(sc.Close, LongSMA, SMALength);
	sc.RSI(sc.Close, RSILine, InRSIAvgType.GetMovAvgType(), RSILength);

	BuyCall[sc.Index] = 0.0f;
	SellCall[sc.Index] = 0.0f;
	RSIBearish[sc.Index] = 0.0f;
	RSIBullish[sc.Index] = 0.0f;

	if (sc.Index < StartIndex)
		return;

	const float SMA = LongSMA[sc.Index];
	const float RSINow = RSILine[sc.Index];

	COLORREF SMAColor;
	if (RSINow >= InRSIExtremeHigh.GetFloat() || RSINow <= InRSIExtremeLow.GetFloat())
		SMAColor = InColorExtreme.GetColor();
	else if (sc.Low[sc.Index] > SMA)
		SMAColor = InColorAboveSMA.GetColor();
	else if (sc.High[sc.Index] < SMA)
		SMAColor = InColorBelowSMA.GetColor();
	else
		SMAColor = InColorOverlap.GetColor();

	LongSMA.DataColor[sc.Index] = SMAColor;

	if (sc.Index < 1)
		return;

	if (InSignalsOnClose.GetYesNo()
		&& sc.GetBarHasClosedStatus() == BHCS_BAR_HAS_NOT_CLOSED)
	{
		return;
	}

	const float RSIPrev = RSILine[sc.Index - 1];
	const float OB = InRSIOverbought.GetFloat();
	const float OS = InRSIOversold.GetFloat();
	const int RSICrossUnderOB = (RSIPrev >= OB && RSINow < OB);
	const int RSICrossOverOS = (RSIPrev <= OS && RSINow > OS);

	// crossover(SMA, EMA)  => CROSS_FROM_BOTTOM
	// crossunder(SMA, EMA) => CROSS_FROM_TOP
	const int SMAEMACross = sc.CrossOver(LongSMA, ShortEMA);
	const int SellCallCond = (SMAEMACross == CROSS_FROM_BOTTOM)
		&& (sc.Open[sc.Index] > sc.Close[sc.Index]);
	const int BuyCallCond = (SMAEMACross == CROSS_FROM_TOP)
		&& (sc.High[sc.Index] > SMA);

	float Offset = InMarkerOffsetTicks.GetInt() * sc.TickSize;
	if (Offset <= 0.0f)
		Offset = sc.TickSize;

	if (BuyCallCond)
		BuyCall[sc.Index] = sc.Low[sc.Index] - Offset;

	if (SellCallCond)
		SellCall[sc.Index] = sc.High[sc.Index] + Offset;

	if (RSICrossUnderOB)
		RSIBearish[sc.Index] = sc.High[sc.Index] + Offset;

	if (RSICrossOverOS)
		RSIBullish[sc.Index] = sc.Low[sc.Index] - Offset;

	int& LastReversalAlertBar = sc.GetPersistentInt(1);
	int& LastEntryAlertBar = sc.GetPersistentInt(2);

	if (!InEnableAlerts.GetYesNo() || sc.Index != sc.ArraySize - 1)
		return;

	if ((RSICrossUnderOB || RSICrossOverOS) && LastReversalAlertBar != sc.Index)
	{
		sc.SetAlert(1, "Possible Reversal on Swing Signal Alert");
		LastReversalAlertBar = sc.Index;
	}

	if ((BuyCallCond || SellCallCond) && LastEntryAlertBar != sc.Index)
	{
		sc.SetAlert(2, "Swing Signal Entry Alert");
		LastEntryAlertBar = sc.Index;
	}
}
