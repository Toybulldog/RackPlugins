#pragma once
struct z8kSequencer
{
public:
	void Init(Input *pRst, Input *pDir, Input *pClk, Output *pOut, Light *pLights, std::vector<Param> &params, std::vector<int> steps)
	{
		curStep = 0;
		pReset = pRst;
		pDirection = pDir;
		pClock = pClk;
		pOutput = pOut;
		numSteps = steps.size();
		for(int k = 0; k < numSteps; k++)
		{
			sequence.push_back(&params[steps[k]]);
			leds.push_back(&pLights[steps[k]]);
		}
	}

	void Step()
	{
		if(resetTrigger.process(pReset->value))
			curStep = 0;
		else if(clockTrigger.process(pClock->value))
		{
			if(pDirection->value > 5)
			{
				if(--curStep < 0)
					curStep = numSteps - 1;
			} else
			{
				if(++curStep >= numSteps)
					curStep = 0;
			}
		}

		pOutput->value = sequence[curStep]->value;
		for(int k = 0; k < numSteps; k++)
			leds[k]->value = k == curStep ? 10.0 : 0;
	}

private:
	SchmittTrigger clockTrigger;
	SchmittTrigger resetTrigger;
	Input *pReset;
	Input *pDirection;
	Input *pClock;
	Output *pOutput;
	std::vector<Param *> sequence;
	std::vector<Light *> leds;
	int curStep;
	int numSteps;
};
