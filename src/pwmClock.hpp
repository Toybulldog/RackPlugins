#include "common.hpp"

struct PwmClockWidget : SequencerWidget
{
	PwmClockWidget();
	void SetBpm(float bpmint);
};


struct SA_TIMER	//sample accurate version
{
	float Reset()
	{
		prevTime = curTime = engineGetSampleTime();
		return Begin();
	}

	void RestartStopWatch() { stopwatch = 0; }
	float Begin()
	{
		RestartStopWatch();
		return totalPulseTime = 0;
	}
	float Elapsed() { return totalPulseTime; }
	float StopWatch() { return stopwatch; }

	float Step()
	{
		curTime += engineGetSampleTime();
		float deltaTime = curTime - prevTime;
		prevTime = curTime;
		totalPulseTime += deltaTime;
		stopwatch += deltaTime;
		return deltaTime;
	}

private:
	float curTime;
	float prevTime;
	float totalPulseTime;
	float stopwatch;
};