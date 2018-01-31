#pragma once
#ifdef LAUNCHPAD
#include "communicator.hpp"
#include "launchpad.hpp"

// off on switch, single key
struct LaunchpadSwitch : launchpadControl
{
	public:
		LaunchpadSwitch(int page, LaunchpadKey key, LaunchpadLed offColor, LaunchpadLed onColor, bool shifted = false) : launchpadControl(page, key, shifted)
		{
			m_offColor = offColor;
			m_onColor = onColor;
		}

	protected:
		virtual void draw(launchpadDriver *drv) override    {drv->drive_led(m_key, getValue() > 0.0 ? m_onColor : m_offColor);  }
		virtual bool intersect(LaunchpadKey key) override   {return key == m_key;}
		virtual void onLaunchpadKey(LaunchpadMessage msg) override  {if(msg.status == LaunchpadKeyStatus::keyDown) setValue(getValue() < 1);}

	private:
		LaunchpadLed m_offColor;
		LaunchpadLed m_onColor;
};

struct LaunchpadMomentary : launchpadControl
{
	public:
		LaunchpadMomentary(int page, LaunchpadKey key, LaunchpadLed offColor, LaunchpadLed onColor, bool shifted = false)
			: launchpadControl(page, key, shifted)
		{
			m_offColor = offColor;
			m_onColor = onColor;
		}

	protected:
		virtual void draw(launchpadDriver *drv) override    {drv->drive_led(m_key, getValue() > 0.0 ? m_onColor : m_offColor); }
		virtual bool intersect(LaunchpadKey key) override   {return key == m_key;}
		virtual void onLaunchpadKey(LaunchpadMessage msg) override  {setValue(msg.status == LaunchpadKeyStatus::keyDown ? 1.0 : 0.0);}

	private:
		LaunchpadLed m_offColor;
		LaunchpadLed m_onColor;
};

struct LaunchpadKnob : launchpadControl
{
	public:
		LaunchpadKnob(int page, LaunchpadKey key, LaunchpadLed rgbMin, LaunchpadLed rgbMax, bool horizontal = false, bool shifted = false) : launchpadControl(page, key, shifted)
		{
			m_offColor = rgbMin;
			m_onColor = rgbMax;
			int r, c;
			ILaunchpadPro::Key2RC(m_key, &r, &c);
			if(horizontal)
				m_secondKey = ILaunchpadPro::RC2Key(r, c+1);
			else
				m_secondKey = ILaunchpadPro::RC2Key(r+1, c);
		}

	protected:
		virtual void draw(launchpadDriver *drv) override
		{
			float v = getValue();
			float inv_v = pBindedParam->maxValue - v + pBindedParam->minValue;
			LaunchpadLed led;
			led.status = ButtonColorType::RGB;
			led.r_color = (int)roundf(rescalef(v, pBindedParam->minValue, pBindedParam->maxValue, m_offColor.r_color, m_onColor.r_color));
			led.g = (int)roundf(rescalef(v, pBindedParam->minValue, pBindedParam->maxValue, m_offColor.g, m_onColor.g));
			led.b = (int)roundf(rescalef(v, pBindedParam->minValue, pBindedParam->maxValue, m_offColor.b, m_onColor.b));
			drv->drive_led(m_key, led);

			led.r_color = (int)roundf(rescalef(inv_v, pBindedParam->minValue, pBindedParam->maxValue, m_offColor.r_color, m_onColor.r_color));
			led.g = (int)roundf(rescalef(inv_v, pBindedParam->minValue, pBindedParam->maxValue, m_offColor.g, m_onColor.g));
			led.b = (int)roundf(rescalef(inv_v, pBindedParam->minValue, pBindedParam->maxValue, m_offColor.b, m_onColor.b));
			drv->drive_led(m_secondKey, led);
		}
		virtual bool intersect(LaunchpadKey key) override {return key == m_key || key == m_secondKey; }
		virtual void onLaunchpadKey(LaunchpadMessage msg) override
		{
			switch(msg.status)
			{
			case LaunchpadKeyStatus::keyPressure:
				float delta = sensitivity * (pBindedParam->maxValue - pBindedParam->minValue);
				if(msg.param0 < 100)
					delta /= 16.0;

				float v = pBindedParam->value;
				if(msg.key == m_key)
					v += delta;
				else
					v -= delta;

				setValue(v);
				break;
			}
		}

	private:
		const float sensitivity = 0.0015;
		LaunchpadKey m_secondKey;
		LaunchpadLed m_offColor;
		LaunchpadLed m_onColor;
};

struct LaunchpadLight : launchpadControl
{
	public:
		LaunchpadLight(int page, LaunchpadKey key, LaunchpadLed offColor, LaunchpadLed onColor) : launchpadControl(page, key, false)
		{
			m_offColor = offColor;
			m_onColor = onColor;
			cached_value = -1020292929;
		}

	protected:
		virtual void draw(launchpadDriver *drv) override {
			float newValue = getValue();
			if(newValue != cached_value)
			{
				drv->drive_led(m_key, newValue > 0.0 ? m_onColor : m_offColor);
				cached_value = newValue;
			}
		}
		virtual void onLaunchpadKey(LaunchpadMessage msg) override    {}

	private:
		LaunchpadLed m_offColor;
		LaunchpadLed m_onColor;
		float cached_value;
};

// Radiobutton switch, multiple keys, horizontal or vertical
struct LaunchpadRadio : launchpadControl
{
	public:
		LaunchpadRadio(int page, LaunchpadKey firstKey, int numKeys, LaunchpadLed unselectedColor, LaunchpadLed selectedColor, bool horizontal = false, bool shifted = false)
		: launchpadControl(page, firstKey, shifted)
		{
			m_numKeys = numKeys;
			m_unselectedColor = unselectedColor;
			m_selectedColor = selectedColor;
			m_horizontal = horizontal;
		}

		virtual void onLaunchpadKey(LaunchpadMessage msg) override
		{
			if(msg.status == LaunchpadKeyStatus::keyDown)
			{
				int r, c,ir, ic;
				if(ILaunchpadPro::Key2RC(m_key, &r, &c) && ILaunchpadPro::Key2RC(msg.key, &ir, &ic))
				{
					int v = m_horizontal ? ic-c : (m_numKeys-1)-(ir - r);
					setValue(v);
				}
			}
		}

	protected:
		 virtual bool intersect(LaunchpadKey key) override
		 {
			int r, c,ir, ic;
			if(ILaunchpadPro::Key2RC(m_key, &r, &c) && ILaunchpadPro::Key2RC(key, &ir, &ic))
			{
				if(m_horizontal)
					return ir == r && ic >= c && ic <= c+m_numKeys;
				else
					return ic == c && ir >= r && ir <= r+m_numKeys;
			}

			return false;
		 }

		virtual void draw(launchpadDriver *drv) override
		{
			int r, c;
			if(ILaunchpadPro::Key2RC(m_key, &r, &c))
			{
				int n = (int)roundf(getValue());
				for(int k = 0; k < m_numKeys; k++)
				{
					if(m_horizontal)
					{
						drv->drive_led(ILaunchpadPro::RC2Key(r, c), k == n ? m_selectedColor : m_unselectedColor);
						c++;
					} else
					{
						drv->drive_led(ILaunchpadPro::RC2Key(r, c), m_numKeys-1-k == n ? m_selectedColor : m_unselectedColor);
						r++;
					}
				}
			}
		}

	private:
		LaunchpadLed m_unselectedColor;
		LaunchpadLed m_selectedColor;
		int m_numKeys;
		bool m_horizontal;
};
#endif
