// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

/** 
 * ChartCreation
 *
 */

#ifndef _CHART_CREATION_INC_
#define _CHART_CREATION_INC_

#define DO_CHARTING 1
#if DO_CHARTING

/*-----------------------------------------------------------------------------
	FPS chart support.
-----------------------------------------------------------------------------*/

/** Helper structure for FPS chart entries. */
struct FFPSChartEntry
{
	/** Number of frames in bucket. */
	int32		Count;
	/** Cumulative time spent in bucket. */
	double	CummulativeTime;
};

//@todo this is a leftover from the old stats system, which used enums
enum FPSChartStats
{
	STAT_FPSChartFirstStat,
	STAT_FPSChart_0_5 = STAT_FPSChartFirstStat,
	STAT_FPSChart_5_10,
	STAT_FPSChart_10_15,
	STAT_FPSChart_15_20,
	STAT_FPSChart_20_25,
	STAT_FPSChart_25_30,
	STAT_FPSChart_30_35,
	STAT_FPSChart_35_40,
	STAT_FPSChart_40_45,
	STAT_FPSChart_45_50,
	STAT_FPSChart_50_55,
	STAT_FPSChart_55_60,
	STAT_FPSChart_60_65,
	STAT_FPSChart_65_70,
	STAT_FPSChart_70_75,
	STAT_FPSChart_75_80,
	STAT_FPSChart_80_85,
	STAT_FPSChart_85_90,
	STAT_FPSChart_90_95,
	STAT_FPSChart_95_100,
	STAT_FPSChart_100_105,
	STAT_FPSChart_105_110,
	STAT_FPSChart_110_115,
	STAT_FPSChart_115_120,
	STAT_FPSChart_120_INF,
	STAT_FPSChartLastBucketStat,
	STAT_FPSChart_30Plus,
	STAT_FPSChart_UnaccountedTime,
	STAT_FPSChart_FrameCount,

	/** Hitch stats */
	STAT_FPSChart_FirstHitchStat,
	STAT_FPSChart_Hitch_5000_Plus = STAT_FPSChart_FirstHitchStat,
	STAT_FPSChart_Hitch_2500_5000,
	STAT_FPSChart_Hitch_2000_2500,
	STAT_FPSChart_Hitch_1500_2000,
	STAT_FPSChart_Hitch_1000_1500,
	STAT_FPSChart_Hitch_750_1000,
	STAT_FPSChart_Hitch_500_750,
	STAT_FPSChart_Hitch_300_500,
	STAT_FPSChart_Hitch_200_300,
	STAT_FPSChart_Hitch_150_200,
	STAT_FPSChart_Hitch_100_150,
	STAT_FPSChart_Hitch_60_100,
	STAT_FPSChart_LastHitchBucketStat,
	STAT_FPSChart_TotalHitchCount,

	/** Unit time stats */
	STAT_FPSChart_UnitFrame,
	STAT_FPSChart_UnitRender,
	STAT_FPSChart_UnitGame,
	STAT_FPSChart_UnitGPU,

	STAT_FPSChart_TotalFrameCount,
	STAT_FPSChart_TotalUnitFrame,
	STAT_FPSChart_TotalUnitRender,
	STAT_FPSChart_TotalUnitGame,
	STAT_FPSChart_TotalUnitGPU,
};
/** Start time of current FPS chart.										*/
extern double			GFPSChartStartTime;
/** FPS chart information. Time spent for each bucket and count.			*/
extern FFPSChartEntry	GFPSChart[STAT_FPSChartLastBucketStat - STAT_FPSChartFirstStat];


/** Helper structure for hitch entries. */
struct FHitchChartEntry
{
	/** Number of hitches */
	int32 HitchCount;

	/** How many hitches were bound by the game thread? */
	int32 GameThreadBoundHitchCount;

	/** How many hitches were bound by the render thread? */
	int32 RenderThreadBoundHitchCount;

	/** How many hitches were bound by the GPU? */
	int32 GPUBoundHitchCount;
};


/** Hitch histogram.  How many times the game experienced hitches of various lengths. */
extern FHitchChartEntry GHitchChart[ STAT_FPSChart_LastHitchBucketStat - STAT_FPSChart_FirstHitchStat ];

#endif // DO_CHARTING

#endif // _CHART_CREATION_INC_