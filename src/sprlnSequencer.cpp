#include "Spiralone.hpp"
#include "sprlnSequencer.hpp"
#include "SpiraloneModule.hpp"

extern float AccessParam(Spiralone *p, int seq, int id);
extern Input *AccessInput(Spiralone *p, int seq, int id);
extern float *AccessOutput(Spiralone *p, int seq, int id);
extern float *AccessLight(Spiralone *p, int id);

void spiraloneSequencer::Step(int seq, Spiralone *pSpir)
{
	if (AccessInput(pSpir, seq, Spiralone::RESET_1)->value > LVL_ON / 2)
		Reset(seq, pSpir);
	else
	{
		int mode = (int)std::roundf(AccessParam(pSpir, seq, Spiralone::MODE_1));
		if (mode > 0)
		{
			int clk = clockTrig.process(AccessInput(pSpir, seq, Spiralone::CLOCK_1)->value); // 1=rise, -1=fall
			if (clk == 1)
			{
				int numSteps = getInput(seq, pSpir, Spiralone::INLENGHT_1, Spiralone::LENGHT_1, 1.0, TOTAL_STEPS);
				int stride = getInput(seq, pSpir, Spiralone::INSTRIDE_1, Spiralone::STRIDE_1, 1.0, 8.0);

				*AccessLight(pSpir, ledID(seq)) = 0.0;
				switch (mode)
				{
				case 1: // fwd:
					curPos += stride;
					break;

				case 2: // bwd
					curPos -= stride;
					break;

				case 3: //altd
					if (pp_rev)
					{
						curPos -= stride;
						if(curPos < 0)
						{
							pp_rev = !pp_rev;
						}
					} else
					{
						curPos += stride;
						if (curPos >= numSteps)
						{
							pp_rev = !pp_rev;
						}
					}
					break;
				}
				if (curPos < 0)
					curPos = numSteps + curPos;
				
				curPos %= numSteps;
				
				outputVoltage(seq, pSpir);
				gate(clk, seq, pSpir);
			} else if(clk == -1)
				gate(clk, seq, pSpir);
		}
	}
}

void spiraloneSequencer::Reset(int seq, Spiralone *pSpir)
{
	curPos = 0;
	pp_rev = false;
	for (int k = 0; k < TOTAL_STEPS; k++)
		*AccessLight(pSpir, ledID(seq, k)) = 0.0;
}

int spiraloneSequencer::getInput(int seq, Spiralone *pSpir, int input_id, int knob_id, float minValue, float maxValue)
{
	float normalized_in = AccessInput(pSpir, seq, input_id)->active ? rescalef(AccessInput(pSpir, seq, input_id)->value, 0.0, 5.0, 0.0, maxValue) : 0.0;
	float v = clampf(normalized_in + AccessParam(pSpir, seq, knob_id), minValue, maxValue);
	return (int)roundf(v);
}

void spiraloneSequencer::outputVoltage(int seq, Spiralone *pSpir)
{
	float v = AccessParam(pSpir, seq, Spiralone::XPOSE_1);
	if (AccessInput(pSpir, seq, Spiralone::INXPOSE_1)->active)
		v += AccessInput(pSpir, seq, Spiralone::INXPOSE_1)->value;
	*AccessOutput(pSpir, seq, Spiralone::CV_1) = clampf(v, 0.0, 10.0);
}

void spiraloneSequencer::gate(int clk, int seq, Spiralone *pSpir)
{
	if (clk == 1)
	{
		*AccessLight(pSpir, ledID(seq)) = 10.0;
		*AccessOutput(pSpir, seq, Spiralone::GATE_1) = LVL_ON;
	}
	else if (clk == -1) // fall
	{
		*AccessOutput(pSpir, seq, Spiralone::GATE_1) = LVL_OFF;
	}
}
