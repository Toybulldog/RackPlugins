#pragma once
#ifdef LAUNCHPAD
#include "communicator.hpp"

// The interface for driving the Novation Launchpad Pro
class ILaunchpadPro
{
public:
	static LaunchpadKey RC2Key(int r, int c)    {return (LaunchpadKey)(R8C1 + c + 10 * (7-r)); }
	static bool IsPlayKey(LaunchpadKey key) // true if key is a squared key (not a function key!)
	{
		if(key >= R8C1 && key <= R1C8)
		{
			int v  = ((int)key-1) % 10;
			return v < 8;
		}

		return false;
	}

	static bool Key2RC(LaunchpadKey key, int *r, int *c)
	{
		if(IsPlayKey(key))
		{
			int v = (int)key-1;
			int n = v / 10;
			*c = v % 10;
			*r = 8 - n;

			return true;
		}

		return false;
	}
	static bool IsValidkey(LaunchpadKey key)
	{
		return key >= RECORD_ARM
			&& key != reserved_unused0
			&& key != reserved_unused1;
	}

	static LaunchpadMessage *Led(LaunchpadMessage *dest, LaunchpadKey key, LaunchpadLed led)
	{
		switch(led.status)
		{
		case ButtonColorType::Normal: LedColor(dest, key, led.r_color); break;
		case ButtonColorType::RGB: LedRGB(dest, key, led.r_color, led.g, led.b); break;
		case ButtonColorType::Flash: LedFlash(dest, key, led.r_color); break;
		case ButtonColorType::Pulse: LedPulse(dest, key, led.r_color); break;
		}
		return dest;
	}
	static LaunchpadMessage *Led(LaunchpadMessage *dest, int r, int c, LaunchpadLed led) {return Led(dest, RC2Key(r, c), led);}
	static LaunchpadMessage *LedOff(LaunchpadMessage *dest, int r, int c)           {return LedOff(dest, RC2Key(r, c));}
	static LaunchpadMessage *LedOff(LaunchpadMessage *dest, LaunchpadKey key)      {return LedColor(dest, key, 0); }
	static LaunchpadMessage *LedColor(LaunchpadMessage *dest, LaunchpadKey key, int color)
	{
		dest->key = key;
		dest->cmd = KEYON;
		dest->param0 = color & 0x3f;
		return dest;
	}
	static LaunchpadMessage *LedColor(LaunchpadMessage *dest, int r, int c, int color)     {return LedColor(dest, RC2Key(r, c), color); }

	static LaunchpadMessage *RegisterScene(LaunchpadMessage *dest, LaunchpadScene scene, bool registr)
	{
	    dest->cmd = REGISTERSCENE;
	    dest->currentScene = SceneAll;
		dest->param0 = registr ? 1 : 0;
		dest->param1 = scene;
		return dest;
	}
	static LaunchpadMessage *SetScene(LaunchpadMessage *dest, LaunchpadScene scene)
	{
	    dest->cmd = SETSCENE;
	    dest->currentScene = scene;
		return dest;
	}
	static LaunchpadMessage *SetStandaloneMode(LaunchpadMessage *dest, LaunchpadMode mode)
	{
		dest->cmd = SET_STANDALONE_MODE;
		dest->param0 = mode;
		return dest;
	}

	static LaunchpadMessage *SetLiveMode(LaunchpadMessage *dest, LaunchpadLiveMode mode)
	{
		dest->cmd = SET_LIVE_MODE;
		dest->param0 = mode;
		return dest;
	}

	static LaunchpadMessage *SetStatus(LaunchpadMessage *dest, LaunchpadStatus status)
	{
		dest->cmd = SETSTATUS;
		dest->param0 = status;
		return dest;
	}

	static LaunchpadMessage *SideLed(LaunchpadMessage *dest, int color)
	{
		dest->cmd = SIDE_LED;
		dest->param0 = color & 0x3f;
		return dest;
	}

	static LaunchpadMessage *LightAll(LaunchpadMessage *dest, int color)
	{
		dest->cmd = LED_ALL;
		dest->param0 = color & 0x3f;
		return dest;
	}

	static LaunchpadMessage *LedFlash(LaunchpadMessage *dest, LaunchpadKey key, int color)
	{
		dest->key = key;
		dest->cmd = FLASH_KEY;
		dest->param0 = color & 0x3f;
		return dest;
	}
	static LaunchpadMessage *LedFlash(LaunchpadMessage *dest, int r, int c, int color)     {return LedFlash(dest, RC2Key(r, c), color); }

	static LaunchpadMessage *LedPulse(LaunchpadMessage *dest, LaunchpadKey key, int color)
	{
		dest->key = key;
		dest->cmd = PULSE_KEY;
		dest->param0 = color & 0x3f;
		return dest;
	}
	static LaunchpadMessage *LedPulse(LaunchpadMessage *dest, int r, int c, int color)     {return LedPulse(dest, RC2Key(r, c), color); }

	static LaunchpadMessage *LedRGB(LaunchpadMessage *dest, LaunchpadKey key, int c_r, int c_g, int c_b)
	{
		dest->key = key;
		dest->cmd = LED_RGB;
		dest->param0 = ((c_r & 0x3f) << 8) | (c_g & 0x3f); // R,G
		dest->param1 = (c_b & 0x3f); // B
		return dest;
	}
	static LaunchpadMessage *LedRGB(LaunchpadMessage *dest, int r, int c, int c_r, int c_g, int c_b)     {return LedRGB(dest, RC2Key(r, c), c_r, c_g, c_b); }
};

class launchpadDriver
{
	// uses keys     SESSION,    NOTE,     DEVICE,     USER
	// as page keys

public:
	static const int ALL_PAGES = -1;
	launchpadDriver(LaunchpadScene scene, int maxPage)
	{
		comm = new communicator();
		numPages = maxPage;
		myScene = scene;
		Reset();
        lastCheck = 0;
		registerMyScene(true);
	}

	~launchpadDriver()
	{
	    registerMyScene(false);
		delete comm;
	}

	bool Connected()    {return comm->Connected();}

	void LedRGB(int page, int r, int c, int c_r, int c_g, int c_b)      {return LedRGB(page, ILaunchpadPro::RC2Key(r, c), c_r, c_g, c_b); }
	void LedPulse(int page, int r, int c, int color)                    {return LedPulse(page, ILaunchpadPro::RC2Key(r, c), color); }
	void LedColor(int page, int r, int c, int color)                    {return LedColor(page, ILaunchpadPro::RC2Key(r, c), color); }
	void LedFlash(int page, int r, int c, int color)                    {return LedFlash(page, ILaunchpadPro::RC2Key(r, c), color); }
	void Led(int page, int r, int c, LaunchpadLed led)                  {return Led(page, ILaunchpadPro::RC2Key(r, c), led); }
	void Led(int page, LaunchpadKey key, LaunchpadLed led)              {setValue(page, key, led);}
	void LedColor(int page, LaunchpadKey key, int colr)
	{
		LaunchpadLed led;
		led.status = ButtonColorType::Normal;
		led.r_color = colr;
		Led(page, key, led);
	}
	void LedFlash(int page, LaunchpadKey key, int colr)
	{
		LaunchpadLed led;
		led.status = ButtonColorType::Flash;
		led.r_color = colr;
		Led(page, key, led);
	}
	void LedPulse(int page, LaunchpadKey key, int colr)
	{
		LaunchpadLed led;
		led.status = ButtonColorType::Pulse;
		led.r_color = colr;
		Led(page, key, led);
	}
	void LedRGB(int page, LaunchpadKey key, int c_r, int c_g, int c_b)
	{
		LaunchpadLed led;
		led.status = ButtonColorType::RGB;
		led.r_color = c_r;
		led.g = c_g;
		led.b = c_b;
		Led(page, key, led);
	}

	int GetPage() {return currentPage;}

	void SetPage(int page)
	{
		if(page >= 0 && page < numPages && currentPage != page)
		{
			currentPage = page;
			ClearPage(page);
			drive_autopage();
			redrawCache();
		}
	}

	void ClearAllPages()
	{
		for(int k = 0; k <= numPages; k++)
			ClearPage(k);
	}

	void ClearPage(int page)
	{
		if(page >= 0 && page < numPages)
		{
			LaunchpadLed off;
			off.status = ButtonColorType::Normal;
			off.r_color = 0;

			for(int k = RECORD_ARM; k <= USER; k++)
			{
				LaunchpadKey key = (LaunchpadKey)k;
				if(ILaunchpadPro::IsValidkey(key))
					setValue(page, key, off);
			}
		}
	}

	void Reset()
	{
		SetPage(0);
		ClearAllPages();
		comm->clear();
	}

	void SetAutoPageKey(LaunchpadKey key, int page)
	{
		if(page >= 0 && page < numPages)
		{
			autoPages[key] = page;
		} else if(page == -1) // remove key?
		{
			auto it= autoPages.find(key);
			if(it != autoPages.end())
				autoPages.erase(it);
		}
	}

	void drive_led(LaunchpadKey key, LaunchpadLed led)
	{
		LaunchpadMessage dest;
		ILaunchpadPro::Led(&dest, key, led);
		dest.currentScene = myScene;
		comm->Write(dest);
	}

protected:
	int numPages;
	int currentPage;
	LaunchpadScene myScene;
	communicator *comm;
	virtual void redrawCache() = 0;
	virtual void setCache(int page, LaunchpadKey key, LaunchpadLed led) {}
	void setValue(int page, LaunchpadKey key, LaunchpadLed led)
	{
		setCache(page, key,led);
		if(page == currentPage || page == launchpadDriver::ALL_PAGES)
			drive_led(key, led);
	}
	int isAutoPageKey(LaunchpadMessage *msg)
	{
		if(msg->cmd == LaunchpadCommand::KEYON)
		{
			auto it= autoPages.find(msg->key);
			if(it != autoPages.end())
				return it->second;
		}
		return -1;
	}

   void registerMyScene(bool registr)
    {
        lastCheck = GetTickCount();
		LaunchpadMessage dest;
		ILaunchpadPro::RegisterScene(&dest, myScene, registr);
		if(comm->Open())
			comm->Write(dest);

    }
    uint32_t lastCheck;

private:

	void drive_autopage()
	{
		LaunchpadLed dim, on;
		on.status = dim.status = ButtonColorType::Normal;
		dim.r_color = 1;
		on.r_color = 2;

		for(std::map<LaunchpadKey, int>::iterator it=autoPages.begin(); it!=autoPages.end(); ++it)
		{
			for(int k = 0; k < numPages; k++)
				setValue(k, it->first, it->second == currentPage ? on : dim);
		}
	}

private:
	std::map<LaunchpadKey, int> autoPages;
};


struct launchpadControl
{
public:
	virtual ~launchpadControl() {};
	void Draw(launchpadDriver *drv, bool force = false)
	{
		if((force || m_dirty) && (drv->GetPage() == m_page || m_page == launchpadDriver::ALL_PAGES))
		{
			draw(drv);
		}
        m_dirty = false;
        m_lastDrawnValue = getValue();

	}
	bool Intersect(int page, LaunchpadKey key, bool shift)    {return m_shifted != shift || page != m_page ? false : intersect(key);}

	void ChangeFromGUI(launchpadDriver *drv)  // gui updated: the new value is already in the binded parameter
	{
	    m_dirty = true;
        Draw(drv);
	}

	virtual void onLaunchpadKey(LaunchpadMessage msg) = 0;
	bool DetectGUIChanges() {return getValue() != m_lastDrawnValue;}

    int ID() {return is_light ? pBindedLight->firstLightId : pBindedParam->paramId; }
	void bindWidget(ModuleLightWidget *p) {pBindedLight = p; is_light = true;}
    void bindWidget(ParamWidget *p) {pBindedParam = p; }

protected:
	virtual void draw(launchpadDriver *drv) = 0;
	virtual bool intersect(LaunchpadKey key) {return false;}

	launchpadControl(int lp, int page, LaunchpadKey key, bool shifted)
	{
	    m_lpNumber = lp;
		is_light= false;
		m_page = page;
		m_key = key;
		m_shifted = shifted;
		pBindedLight = NULL;
		pBindedParam = NULL;
		m_dirty = true;
		m_lastDrawnValue = -10202020;
	}

	float getValue() 	{return is_light ? pBindedLight->module->lights[pBindedLight->firstLightId].getBrightness() : pBindedParam->value; }

	void setValue(float v)
	{
	    if(v != getValue())
        {
            if(is_light)
                pBindedLight->module->lights[pBindedLight->firstLightId].value = v;
            else
                pBindedParam->setValue(v);

            m_dirty = true;
        }
	}

	int m_lpNumber;
	int m_page;
	bool is_light;
	LaunchpadKey m_key;
	bool m_shifted;
	ModuleLightWidget *pBindedLight;
	ParamWidget *pBindedParam;
	bool m_dirty;
	float m_lastDrawnValue;
};

struct LaunchpadBindingDriver : public launchpadDriver
{
public:
	LaunchpadBindingDriver(LaunchpadScene scene, int maxPage) : launchpadDriver(scene, maxPage)
	{
	}

	virtual ~LaunchpadBindingDriver()
	{
		for(std::map<int, launchpadControl *>::iterator it=m_bindings.begin(); it!=m_bindings.end(); ++it)
		{
			if(it->second != NULL)
				delete it->second;
		}

		m_bindings.clear();
	}

	void Add(launchpadControl *ctrl, ParamWidget *p)
	{
		ctrl->bindWidget(p);
		int id = ctrl->ID();
		m_bindings[id] = ctrl;
#ifdef DEBUG
				info("binded param %i",id);
#endif

	}

	void Add(launchpadControl *ctrl, ModuleLightWidget *p)
	{
		ctrl->bindWidget(p);
		int id = ctrl->ID();
		m_bindings[0x8000 | id] = ctrl;
	}

	void ProcessLaunchpad()
	{
	    processGUI();
        processLaunchpadKeys();
        if(!Connected() && (GetTickCount()-lastCheck) >= 2000)
        {
            registerMyScene(true);
        }
	}

protected:
	virtual void redrawCache() override
	{
		for(std::map<int, launchpadControl *>::iterator it=m_bindings.begin(); it!=m_bindings.end(); ++it)
		{
			it->second->Draw(this, true);
		}
	}

private:
	std::map<int, launchpadControl *>m_bindings;
	void processGUI()
	{
		for(std::map<int, launchpadControl *>::iterator it=m_bindings.begin(); it!=m_bindings.end(); ++it)
		{
			if(it->second->DetectGUIChanges())
			{
				it->second->ChangeFromGUI(this);
			}
		}
	}

	void processLaunchpadKeys()
	{
        LaunchpadMessage msg;
        do
        {
            msg = comm->Read();
            if(msg.status != LaunchpadKeyStatus::keyNone && (msg.currentScene == SceneAll || msg.currentScene == myScene))
            {
                #ifdef DEBUG
                info("MSG: %i scene=%i", msg.cmd, msg.currentScene);
                #endif

                int page = isAutoPageKey(&msg);
                if(page >= 0 && msg.status == LaunchpadKeyStatus::keyDown && !msg.shiftDown)
                {
                    SetPage(page);
                } else if(msg.cmd == LaunchpadCommand::RESET)
                {
                    SetPage(currentPage);
                } else if(msg.cmd == LaunchpadCommand::SETSCENE)
                {
                    #ifdef DEBUG
                    info("MSG: set scene=%i myscene=%i", msg.param1, myScene);
                    #endif
                    if(myScene == msg.param1)
                    {
                        redrawCache();
                    }
                } else
                {
                    for(std::map<int, launchpadControl *>::iterator it=m_bindings.begin(); it!=m_bindings.end(); ++it)
                    {
                         if(it->second->Intersect(GetPage(), msg.key, msg.shiftDown))
                         {
                            #ifdef DEBUG
                            info("MSG: page=%i, key=%i shift=%i detected: %i", GetPage(), msg.key, msg.shiftDown, it->first);
                            #endif
                            it->second->onLaunchpadKey(msg);
                         }
                    }
                }
            }
        } while(msg.status != LaunchpadKeyStatus::keyNone);
	}

};

#endif
