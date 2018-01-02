struct CV_LINE
{
	void Reset()
	{
		slideNoteIncrement=0;
		startNoteValue = -1.0;
	}

	void Set(float newNote, float slideTime, bool slide_on)
	{
		if (startNoteValue < 0.0)
			startNoteValue = newNote;

        if(slide_on)
            slideNoteIncrement = (newNote - startNoteValue) / (10000.0*slideTime);
        else
        {
            startNoteValue = newNote;
            slideNoteIncrement = 0;
        }
	}

	float Play(float deltaTime, float curNote)
	{
		float cv = startNoteValue + slideNoteIncrement * deltaTime;
		if(slideNoteIncrement < 0 && curNote > cv)		// inc negativo ---> cv potrebbe essere INFERIORE a target
			cv = curNote;		// cv non puo' essere INFERIORE a curstep.value
		else if(slideNoteIncrement > 0 && curNote < cv) // inc positivo --> cv potrebbe sovraelongare
			cv = curNote;		// cv non puo' essere SUPERIORE a curstep.value
		return cv;
	}

private:
	float startNoteValue;
	float slideNoteIncrement;
};

struct GATE_LINE
{
private:
	int stepDivCounter;
	int stepDiv;
	float curGateTime;
	float totalPulseTime;
	void gate_len(float deltaTime, float *pOut)
	{
		totalPulseTime += deltaTime;
		if(totalPulseTime > curGateTime)
			*pOut = LVL_OFF;
	}

public:
	void Reset()
	{
		curGateTime = totalPulseTime = 0;
		stepDivCounter=stepDiv=0;
	}

	void Set(float gateTime, int step_div)
	{
		totalPulseTime = 0;
		stepDiv = 1 + step_div;
		curGateTime = gateTime;
		stepDivCounter=0;
	}

	void Play(bool play, int switchMode, float deltaTime, float *pOut)
	{
		switch (switchMode)
		{
			case 0:	// off
				if(play)
					*pOut = LVL_OFF;
				break;

			case 1:  //single pulse
				if(play)
					*pOut = LVL_ON;
				else
					gate_len(deltaTime, pOut);
				break;

			case 2: // multiple pulse
			{
				if((stepDivCounter++ % stepDiv) == 0)
				{
					*pOut = LVL_ON;
				} else
				{
					gate_len(deltaTime, pOut);
				}
			}
			break;

			case 3:	// continuo
				if(play)
					*pOut = LVL_ON;
				break;
		}
	}
};

struct TIMER
{
	void Reset()
	{
		prevTime = clock();
	}

	float Step()
	{
		clock_t curTime = clock();
		clock_t deltaTime = curTime - prevTime;
		prevTime = curTime;
		return _toSec(deltaTime);
	}

private:
	clock_t prevTime;
	float _toSec(clock_t t) {return float(t) / CLOCKS_PER_SEC;}
};

struct STEP_COUNTER
{
	void Reset()
	{
		pulseCounter=0;
		curStep=0;
		pp_rev=false;
	}

	void Set(int rm, int numstp)
	{
		runMode=rm;
		numSteps = numstp;
	}

	int *getAddress(int var)
	{
	    switch(var)
	    {
	        case 0: return &runMode;
	        case 1: return &numSteps;
	    }
	    return NULL;
	}

	int CurStep() {return curStep;}

	bool Play(int switchCounter, Param *pSteps)
	{
		bool play = ++pulseCounter > switchCounter;
		if(play)
		{
			pulseCounter = 0;
			curStep = get_next_step(curStep, pSteps);
		}
		return play;
	}

private:
	int pulseCounter;
	bool pp_rev;
	int curStep;
	int runMode;
    int numSteps;

	bool testaCroce() {return rand() >= RAND_MAX/2;}
	int getRand(int rndMax)
	{
		float f = rand()/float(RAND_MAX);
		return int(f * rndMax);
	}

	int get_next_step(int current, Param *pSteps)
	{
		switch(runMode)
		{
			case 0: // FWD
				return inc_step(current, pSteps);

			case 1: // BWD
				return dec_step(current, pSteps);

			case 2: // ping ed anche pong
				if(pp_rev)
				{
					int step = dec_step(current, pSteps);
					if( step <= current )
						return step;
					pp_rev = !pp_rev;
					return inc_step(current, pSteps);
				} else
				{
					int step = inc_step(current, pSteps);
					if(step >= current )
						return step;
					pp_rev = !pp_rev;
					return dec_step(current, pSteps);
				}
				break;

			case 3: // BROWNIAN
			{
				if(testaCroce())
				{
					return inc_step(current, pSteps);
				} else
				{
					return testaCroce() ? dec_step(current, pSteps) : current;
				}
			}
			break;

			case 4: // At casacc
				current = getRand(numSteps); // OCIO: step off NON funziona con random!
				break;
		}

		return current;
	}

	int inc_step(int step, Param *pSteps)
	{
		for(int k = 0; k < 8; k++)
		{
			if(++step >= numSteps)
				step = 0;
			if(pSteps[step].value > 0.0)  // step on?
				break;
		}

		return step;
	}

	int dec_step(int step, Param *pSteps)
	{
		for(int k = 0; k < 8; k++)
		{
			if(--step < 0)
				step =numSteps - 1;
			if(pSteps[step].value > 0.0)  // step on?
				break;
		}

		return step;
	}

};
