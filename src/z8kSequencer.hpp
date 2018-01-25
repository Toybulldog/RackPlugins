#pragma once
struct z8kSequencer
{
	public:		
		void Init(Input *pRst, Input *pDir, Input *pClk, Output *pOut, std::vector<Param> &params, std::vector<int> steps)
		{
			curStep = 0;
			pReset = pRst;
			pDirection = pDir;
			pClock = pClk;
			pOutput = pOut;
			numSteps = steps.size();
			for(int k = 0; k < numSteps; k++)
				sequence.push_back(&params[steps[k]]);
		}
		
		int Step()
		{
			if(resetTrigger.process(pReset->value))
				curStep = 0;
			else if(clockTrigger.process(pClock->value))
			{
				if(directionTrigger.process(pDirection->value))
				{
					if(--curStep < 0)
						curStep = numSteps-1;
				} else
				{
					if(++curStep >= numSteps)
						curStep = 0;
				}
			}

			pOutput->value = sequence[curStep]->value;				
			return curStep;
		}
		
	private:
		SchmittTrigger clockTrigger;
		SchmittTrigger resetTrigger;
		SchmittTrigger directionTrigger;
		Input *pReset;
		Input *pDirection;
		Input *pClock;
		Output *pOutput;
		std::vector<Param *> sequence;
		int curStep;
		int numSteps;
};
