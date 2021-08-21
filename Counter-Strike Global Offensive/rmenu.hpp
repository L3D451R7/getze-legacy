#include "sdk.hpp"
//class Editor :
//	public CBaseControl
//{
//public:
//	struct instruction_t
//	{
//		std::string sinstruction;
//		std::vector<std::string> param;
//	};
//	Editor();
//	void UpdateShit();
//	void Draw(bool hover);
//	void OnUpdate();
//	void OnClick();
//	Vector2D GetClickArea() { return Vector2D(m_w, m_h); };
//	Vector2D GetPush() { return Vector2D(m_w, m_h + 5); };
//	float GetValue() { return 1.f; };
//	void SetValue(float val) {};
//	std::vector<std::vector<std::string>> Text; // vector of lines of words inside the line
//	std::vector<instruction_t> instructions;
//	struct variable_t
//	{
//		std::string name;
//		std::string value;
//		std::string type;
//	};
//	std::vector<variable_t> variables;
//	void ReplaceStringInPlace(std::string& subject, const std::string& search,
//		const std::string& replace) {
//		size_t pos = 0;
//		while ((pos = subject.find(search, pos)) != std::string::npos) {
//			subject.replace(pos, search.length(), replace);
//			pos += replace.length();
//		}
//	}
//
//	std::string conditionalstring(int var, std::string op, int immediate, int destination)
//	{
//		if (op == "+") (destination = (var + immediate));
//		else if (op == "-") (destination = (var - immediate));
//		else if (op == "=") (destination = immediate);
//
//		return (std::to_string(destination));
//		//else if ((op == "-") && (var != immediate))  cur_line = destination;
//		//else if ((op == "<") && (var < immediate))   cur_line = destination;
//		//else if ((op == ">") && (var > immediate))   cur_line = destination;
//		//else if ((op == "<=") && (var <= immediate))  cur_line = destination;
//		//else if ((op == ">=") && (var >= immediate))  cur_line = destination;
//	}
//	struct validinstructions
//	{
//		int instructionid;
//		std::string sinstruction;
//		std::string desc;
//		int params;
//	};
//	std::vector < validinstructions > valdinstructions;
//private:
//	int line = -1;
//	double fasterase;
//	bool inloop = false;
//	double delayerase;
//	bool IsTyping = false;
//	std::string sstring;
//
//
//	void DoInstructions();
//	std::vector<std::string> validvariables
//	{
//		"var"
//	};
//	std::vector<std::string> validoperators
//	{
//		"+"
//	};
//	void AddInstruction(std::string name, std::string param, int params) { valdinstructions.push_back({ static_cast<int>(valdinstructions.size()), name, param, params }); };
//};

class CBaseControl;
class CGroupBox;
class CWindow;
class CMenu;
class CColorPicker;
class CCheckBox;
class CSlider;
class CComboBox;
class CTab;
class CListBox;
class CTextBox;
class CRadar;
class CBaseList;

extern std::vector<CWindow*> windows;
extern float window_alpha;
extern bool fully_update;

enum Drawstate {
	// Default Draw by boolean bMenu
	Default,
	// Draw by state && boolean bMenu
	MainWindowExtra,
	// Draw by state
	PureState
};

enum UIFlags
{
	UI_None = 0x00,
	UI_Drawable = 0x1,
	UI_Clickable = 0x02,
	UI_Focusable = 0x04,
	UI_RenderFirst = 0x08,
	UI_SaveFile = 0x10
};

enum UIControlTypes
{
	UIC_CheckBox = 1,
	UIC_Slider,
	UIC_KeyBind,
	UIC_ComboBox,
	UIC_TextBox,
	UIC_GroupBox,
	UIC_ListBox,
	UIC_Colorpicker,
	UIC_Button,
	UIC_MultiComboBox
};

struct SaveItem
{
	std::string name;
	CBaseControl* control;
};

extern std::vector<SaveItem> AllControls;
extern std::string user;

class CBaseControl
{
	friend class CWindow;
	friend class CTab;
public:
	std::string label = "nigger";
	std::string desc = "";
	int ControlType = 0;
	bool Flag(int f) { return m_Flags & f; }
	int m_Flags;
	void SetPos(int x, int y)
	{
		m_x = x;
		m_y = y;
	}
	void SetSize(int w, int h)
	{
		m_w = w;
		m_h = h;
	}
	void SetLabel(std::string text)
	{
		label = text;
	}
	void SetDesk(std::string text)
	{
		desc = text;
	}
	virtual float GetValue() = 0;
	virtual void SetValue(float val) = 0;
	virtual Vector2D GetPush() = 0;
	float GetPosY() { return m_y; };
	Vector2D GetSize() { return Vector2D(m_w, m_h); };
protected:
	int m_x = 0, m_y = 0, m_w = 0, m_h = 0;
	bool FinishedFocus = false;
	CWindow* parent;
	CGroupBox* parentgroup;
	std::string m_SaveName;
	Vector2D GetPos();

	virtual void Draw(bool) = 0;
	virtual void OnUpdate() = 0;
	virtual void OnClick() = 0;
	virtual Vector2D GetClickArea() = 0;

	int scroll = 0;
	int GetScroll() { if (scroll == 0) return 0; else return scroll > 0 ? 1 : -1; };

};

class CCheckBox : public CBaseControl
{
public:
	CCheckBox();
	float GetValue() { return Checked; };
	void SetValue(float val) { Checked = val; };
protected:
	bool Checked;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
	Vector2D GetPush();
	Vector2D GetClickArea();
};

class knife_t;

class CComboBox : public CBaseControl
{
public:
	CComboBox();

	float GetValue() { return SelectedIndex; };
	std::string GetName() {
		if (!Items.empty())
			return Items[SelectedIndex];

		return "error";
	};
	void AddItems(std::vector < std::string > items);
	void AddKnifes(std::vector < knife_t > items);
	void SetValue(float val) { SelectedIndex = val; };
protected:
	std::vector<std::string> Items;
	float SelectedIndex = 1;
	bool m_bIsOpen = false;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
	Vector2D GetClickArea();
	//14
	Vector2D GetPush() { return Vector2D(m_w, m_h + 8); };
};

class CMultiComboBox : public CBaseControl
{
public:
	CMultiComboBox();

	float GetValue() { return selected; };
	// use this for ur checks
	bool Has(int indx) { indx = Math::clamp(indx, 0, (int)Selected.size()); return Selected[indx];/*std::find(Selected.begin(), Selected.end(), indx) != Selected.end();*/ }
	std::string GetName(int indx) {
		if (!Items.empty())
			return Items[indx];

		return "error";
	};
	void AddItems(std::vector < std::string > items);
	void SetValue(float val) { Selected[0] = static_cast<int>(val); if (val) selected++;};
	void SetValue(int index, int value) { Selected[index] = value; if (value) selected++; };
	std::vector<bool> Selected;
	int selected;
protected:
	std::vector<std::string> Items;
	bool m_bIsOpen = false;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
	Vector2D GetClickArea();
	Vector2D GetPush() { return Vector2D(m_w, m_h + 8); };
};


class CKeyBind : public CBaseControl
{
public:
	CKeyBind();

	float GetValue() { return key; };
	bool Holding();
	bool Toggle();
	void SetValue(float val) { key = static_cast<int>(val); };
protected:
	int key = 0;
	bool m_bToggle = false;
	bool m_bIsOpen = false;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
	Vector2D GetClickArea();
	Vector2D GetPush() { return Vector2D(m_w, m_h + 8); };
};

class CSlider : public CBaseControl
{
public:
	CSlider();

	float GetValue() { return Value; };
	void Setup(int min, int max, const char* formatz = "%.2f")
	{
		Min = min;
		Max = max;
		format = formatz;

	}
	void ChangeMinMax(int min, int max)
	{
		if (min != 0)
			Min = min;
		if (max != 0)
			Max = max;

		Value = Math::clamp(Value, Min, Max);
	}
	void SetValue(float val) { Value = (val); };
protected:
	float Value = 0.0f;
	float Location = 0.0f;
	float Min = 0.0f, Max = 0.0f;
	bool DoDrag = false;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
	Vector2D GetClickArea();
	Vector2D GetPush() { return Vector2D(m_w, m_h + 8); };
	bool IsTyping = false; // 4 new shits v
	float fasterase = 0;
	float delayerase = 0;
	std::string Text = "";
private:
	const char* format = "%f";
};

class CTextBox : public CBaseControl
{
public:
	CTextBox();

	float GetValue() { return std::stof(Text); };
	std::string GetText() { return Text; };
	void SetValue(float val) { };
	void SetValue(std::string val) { Text = val; };
protected:
	std::string Text = "";
	bool IsTyping = false;
	bool AllowSpaces = true;
	bool OnlyDigits = false;
	double fasterase = 0;
	double delayerase = 0;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
	Vector2D GetClickArea();
	Vector2D GetPush() { return Vector2D(m_w, m_h + 8); };
};

class CListBox : public CBaseControl
{
public:
	struct Listboxitem
	{
		std::string name;
		int value;
		Color clr;
	};
	CListBox();

	void ResetItems()
	{
		Items.clear();
	};

	float GetValue()
	{
		if (!Items.empty() && selectedindex < Items.size())
			return Items[selectedindex].value;

		return 0;
	};
	void SetIdx(int i)
	{
		selectedindex = Math::clamp(i, 0, (int)Items.size());
	};
	int GetIdx()
	{
		return selectedindex;
	};
	std::string GetName()
	{
		if (!Items.empty())
			return Items[selectedindex].name;

		return "error";
	};
	void AddItem(std::string text, int value, Color clr)
	{
		Items.push_back({ text,value,clr });
	};
	void Setup(int w, int h)
	{
		SetSize(w, h);
	}
	int GetSkinId()
	{
		if (!Items.empty())
			return Items[selectedindex].value;

		return 0;
	}
	void SetValue(float val) { SetIdx(val); };
	int size() { return  Items.size(); }
protected:
	std::vector<Listboxitem> Items;
	bool DoDrag = false;
	int selectedindex = 0;
	int ScrollTop = 0;
	int SliderPos = 0;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
	Vector2D GetClickArea() { return Vector2D(m_w, m_h); };
	Vector2D GetPush() { return Vector2D(m_w, m_h + 8); };

};

class CColorPicker : public CBaseControl
{
public:
	CColorPicker();

	CColorPicker(std::string name)
	{
		label = name;
	};

	float GetValue() { return 0.0f; };
	Color GetColor() { return color; };
	void SetColor(Color col);
	void SetValue(float val) { };
protected:
	Color color = Color::White();
	Vector2D ColorPos;
	Vector2D m_hue_pos;
	bool m_bIsOpen = false;
	float m_hue = 1.f;
	float m_saturation = 1.f;
	float m_value = 1.f;
	bool DoDragHue = false;
	bool DoDragCol = false;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
	Vector2D GetClickArea();
	Color GetColorFromPenPosition(int x, int y, int width, int height);
	Vector2D GetPush() { return Vector2D(m_w, m_h + 17); };
};

class CButton : public CBaseControl
{
public:
	CButton();
	float GetValue() { return 0; };
	void SetFunc(void* func) { if (func) this->func = func; };
	void SetValue(float val) { };
protected:
	PVOID func = nullptr;
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
	Vector2D GetClickArea();
	Vector2D GetPush() { return Vector2D(m_w, m_h + 8); };
private:
	bool clicked = false;
};

class CTab
{
	friend class CBaseControl;
	friend class CWindow;
	friend class CMenu;
	friend class CGroupBox;
public:
	void SetTitle(std::string name)
	{
		Title = name;
	}

	void SetIcon(std::string icon)
	{
		Icon = icon;
	}

	void RegisterControl(CBaseControl* control, std::string label, int tab = 0)
	{
		control->label = label;
		control->parent = parent;
		Controls[tab].Controls.push_back({ control });
	}

	void RegisterControl(CBaseControl* control, std::string label, CGroupBox* parentgroup, int tab = 0)
	{
		control->label = label;
		control->parent = parent;
		control->parentgroup = parentgroup;
		Controls[tab].Controls.push_back({ control });
	}
	float alpha = 35;
	//float startfade = 0;
	//bool startanim = false;
	//double fadetime = 0;
	int h = 0, w = 0;
private:
	int tab = 0;
	std::string Title = "Unknown";
	std::string Icon = "Unknown";
	struct tabz
	{
		std::string title;
		std::vector<CBaseControl*> Controls;
	};
	std::vector<tabz> Controls;

	CWindow* parent;
	Vector2D pos = Vector2D(0, 0);
};

class CRadar :
	public CBaseControl
{
public:
	CRadar();
	struct radar_s
	{
		C_BasePlayer* Ent;
		Color Col;
		int size;
	};
	std::vector<radar_s> EntList;
	float zoom = 16;
protected:
	void Draw(bool hover);
	Vector2D WorldToRadar(const Vector location, const Vector origin, const Vector angles, float scale);
	void OnUpdate();
	void OnClick();
	float GetValue() { return 0.0f; };
	void SetValue(float val) { };
	Vector2D GetClickArea() { return Vector2D(0, 0); };
	Vector2D GetPush() { return Vector2D(m_w + 5, m_h + 5); };
};

class CBaseList :
	public CBaseControl
{
public:
	CBaseList();
	struct liststruct
	{
		std::string string;
		Color color;
	};
	std::vector<liststruct> list;
	void AddToList(std::string string, Color color = Color::White()) { list.push_back({ string, color }); };
protected:
	void Draw(bool hover);
	void OnUpdate();
	void OnClick();
	float GetValue() { return 0.0f; };
	void SetValue(float val) { };
	Vector2D GetClickArea() { return Vector2D(0, 0); };
	Vector2D GetPush() { return Vector2D(m_w + 5, m_h + 12); };
};

class CWindow
{
	friend class CBaseControl;
	friend class CMenu;
	friend class CGroupBox;
public:
	CWindow();
	bool ShouldDraw();
	void Draw();
	int m_x = 0, m_y = 0, m_w = 0, m_h = 0, finalh = 0, finalw = 0, starth = 0, startw = 0, o_w = 0, o_h = 0;
	float fadetime = 0;
	int m_DrawState = 0;
	void SetupWindow(int x, int y, int w, int h, std::string title, int iDrawState = 0);
	void SetTitle(std::string name)
	{
		Title = name;
		if (Tabs.size() > 1)
			SelectedTab = nullptr;
	}
	void SetDrawState(int iDrawState)
	{
		m_DrawState = iDrawState;
	}
	void SetDrawBool(bool bDrawState)
	{
		m_bIsOpen = bDrawState;
	}
	void SetPosition(int x, int y)
	{
		m_x = x; m_y = y;
	}
	void SetSize(int w, int h)
	{
		m_w = w; m_h = h;
		finalh = h;
		finalw = w;
		o_w = w;
		o_h = h;
	}
	std::string GetLabel()
	{
		return Title;
	}

	void RegisterTab(CTab* Tab, std::string title, std::string Icon = "Unknown", float h = 0, float w = 0, std::vector<std::string> tabs = { "" })
	{
		if (Tabs.size() == 0)
			SelectedTab = Tab;
		if (Tabs.size() > 0)
			SelectedTab = nullptr;

		Tab->h = h;
		Tab->w = w;

		Tab->SetTitle(title);
		Tab->SetIcon(Icon);
		Tab->parent = this;
		Tab->Controls.resize(tabs.size());
		for (int i = 0; i < tabs.size(); ++i)
			Tab->Controls[i].title = tabs[i];

		Tabs.push_back(Tab);
	}
	CTab* SelectedTab = nullptr;
	int grabheight = 20;

private:
	bool m_bIsOpen = false;
	bool m_bUseIcon = false;
	int m_bDrawTitle = 1;
	bool IsDraggingWindow;
	int DragOffsetX; int DragOffsetY;
	int tabsize = 45;
	std::vector<CTab*> Tabs;

	bool IsFocusingControl;
	CBaseControl* FocusedControl;

	std::string Title;
	RECT GetClientArea();
};

class CMenu
{
public:
	void StartRender();
	void FinishRender();
	//void HandleScroll(WPARAM in);
	void UpdateMenu();
	void UpdateFrame();
	//void CompileCode();
	void Setup();
	void HandleInput();
	void HandleTopMost();
	void PlaceControls();
	bool DrawMains = false;
	CRadar radar;
	CBaseList CSpecList;
	CBaseList CTList;
	HWND WND = 0;
	WNDPROC oldWindowProc = 0;
	//	Editor ESPEdit;
};



class CGroupBox : public CBaseControl
{
public:
	CGroupBox();
	void ResetItems()
	{
		PreviousControl = nullptr;
		subtabs.clear();
	}
	void PlaceControl(std::string Label, CBaseControl* control, int tab = 0);
	void SetupGroupbox(int x, int y, int w, int h, CTab * tab, std::string label, int tabid = 0)
	{
		SetPos(x, y);
		SetSize(w, h);
		ParentTab = tab;
		tab->RegisterControl(this, label, tabid);
	}

	float GetValue() { return SelectedSubTab; };
	void SetValue(float val) { SelectedSubTab = val; };
protected:
	std::vector<std::string> subtabs;
	void Draw(bool hover);
	int SelectedSubTab = 0;
	void OnUpdate();
	void OnClick();
	Vector2D GetPush() { return Vector2D(m_w, m_h + 2); };
	Vector2D GetClickArea() {
		return Vector2D(m_w, 30);
	};
private:
	CBaseControl * PreviousControl = nullptr;
	CTab* ParentTab = nullptr;
	int lasttab = 0;
};

enum Icons
{
	eagle,
	target,
	eye,
	cogwheel,
	gloves,
	arrow,
	checkmark,
	upNdownarrow,
	nut,
	targetlegit,
};

//class CPlayer;
//class CDrawings
//{
//public:
//	CDrawings();
//	~CDrawings();
//	LPVOID myResourceData;
//	//	void	DrawString(int x, int y, Color clrColor, /*bool bCenter,*/ wchar_t* szText);
//	void	DrawString(int x, int y, Color clrColor, bool bCenter, const char* szText, ...);
//	void	DrawStringFont(int x, int y, Color clrColor, bool bCenter, unsigned long font, const char * szText, ...);
//	void	DrawRect(int x, int y, int w, int h, Color clrColor);
//	void	DrawIcon(Vector2D pos, int icon, unsigned long font, bool center, Color clr = Color::White());
//
//	void	DrawOutlineRect(int x, int y, int w, int h, Color clrColor);
//	void	DrawRoundedRect(int x, int y, int w, int h, float smooth, Color col);
//	void	DrawRoundedGradientV(int x, int y, int w, int h, float smooth, Color col, Color col2);
//	void	DrawInlineRect(int X, int Y, int W, int H, Color clrColor);
//	void	DrawLine(int x0, int y0, int x1, int y1, Color clrColor);
//	void	DrawLine3Dwidth(int x0, int y0, int x1, int y1, int w, int height, Color clrColor);
//	void	DrawCircle(int x, int y, int r, int seg, Color clrColor);
//	void	TexturedPolygon(int n, Vertex_t * vertice, Color col);
//	void	FilledCircle(Vector position, float points, float radius, Color color);
//	void	DrawCornerBox(int x, int y, int w, int h, int cx, int cy, Color clrColor);
//	void	LogoDraw();
//
//	// menu stoff?
//	void DrawButtonSave(int x, int y, int w, int h);
//	void DrawButtonLoad(int x, int y, int w, int h);
//	void DrawButtonUnloadcheat(int x, int y, int w, int h, bool &unload);
//	void GradientV(int x, int y, int w, int h, Color c1, Color c2);
//	void DrawErrorMessage(char* message, bool &actioncanel);
//	void updatekeys();
//	void Draw3dbox(Vector mins, Vector maxs, Vector origin, Color color);
//	//void DrawSlider(float x, float y, float w, int min, int max, float & SmoothValue, int & sliderx, char * option);
//	bool Mousein(int x, int y, int w, int h);
//	float fade(float start, float end, double curtime, double endtime);
//	void DrawMouse(Color color);
//	void GradientH(int x, int y, int w, int h, Color c1, Color c2);
//	void InitColorpicker(int w, int h);
//	void DrawColorPicker(int x, int y);
//	void ClickHandler(HWND wnd);
//	void Polygon(int count, Vertex_t* Vertexs, Color color);
//	bool MakeBox(CPlayer * ent, int & x0, int & y0, int & x1, int & y1);
//	void DynamicBox(int index, CPlayer* pPlayer, Color clrColor);
//	void DynamicBoxStandAlone(CPlayer* pPlayer, Color clrColor, std::string);
//	bool IsMouseInRegion(int x, int y, int x2, int y2);
//	bool IsMouseInRegion(RECT region);
//	RECT GetTextSize(DWORD font, const char* text);
//	std::string LowerCase(std::string in);
//
//	std::vector<CWindow*> windows;
//
//	bool bMouse1pressed, bMouse2pressed, bMouse1released, bMouse2released;
//	vgui::HFont	hFont;
//	vgui::HFont	ESPFont;
//	vgui::HFont	Eagle;
//	vgui::HFont	mideagle;
//	vgui::HFont	Tabfont;
//	vgui::HFont	Checkmark;
//	vgui::HFont	SegoeUI;
//	vgui::HFont	MenuText;
//	vgui::HFont	BackArrow;
//	//vgui::HFont	LegitTabs;
//	//vgui::HFont	LastTabsText;
//	void InitFonts();
//	int DragOffsetX; int DragOffsetY;
//	bool GetKeyPress(unsigned int key);
//	int scroll = 0;
//	int GetScroll() { if (scroll == 0) return 0; else return scroll > 0 ? 1 : -1; };
//	void UpdateScroll(WPARAM wParam) { scroll = wParam; };
//	POINT Mouse;
//};

class CVars
{
public:
	CVars();
	~CVars();

	void Load(std::string name = "hake.ini");
	void Save();

	std::string GetClipBoardText();

	void Checkdir();

public:
	CGroupBox LegitAim;

	CGroupBox Visuals;
	CGroupBox Visuals_chams;
	CGroupBox Visuals_localp;

	CGroupBox Visuals_o_world;
	CGroupBox Visuals_o_removals;
	CGroupBox Visuals_o_misc;

	CCheckBox SearchAll;
	CTextBox SearchEnts;
	CTextBox SearchWhites;
	CTextBox SearchBlack;
	CGroupBox Lists;
	CGroupBox LuaList;
	CGroupBox EntLists;
	CGroupBox WhiteLists;
	CButton WhiteList;
	CButton RemoveWhiteListd;
	CButton WhiteListAll;
	CButton ClearWhitelist;
	CButton LoadLuaFile;
	CButton RefreshLuaList;
	//CGroupBox MiscGroup;
	CGroupBox movement;
	// legit shit
	CGroupBox Legit_Accuracy;
	CGroupBox Legit_Aimbot;
	CGroupBox Legit_Filter;
	CCheckBox Legit_Enable;
	CCheckBox Legit_PosAjs;
	CSlider Legit_fov[3];
	CSlider Legit_smooth[3];
	CComboBox Legit_hbotpriority[3];
	CComboBox Legit_hboxselection[3];
	CCheckBox Legit_rcs[3];
	// filters
	CCheckBox Legit_head[3];
	CCheckBox Legit_chest[3];
	CCheckBox Legit_stomach[3];
	CCheckBox Legit_legs[3];
	CCheckBox Legit_arms[3];
	CKeyBind Legit_Key;
	CCheckBox Legit_ff;

	CGroupBox RageBot;
	CGroupBox Ragebot_Accuracy;
	CGroupBox Ragebot_BAim;
	CGroupBox Ragebot_Hitscan;

	CCheckBox Antiaim_enable;
	CCheckBox Antiaim_attargets;
#pragma region aimbot
	CCheckBox RageBot_Enable;
	CCheckBox RageBot_NoRecoil;
	CCheckBox RageBot_NoSpread;
	CCheckBox RageBot_autor8;
	CComboBox RageBot_SilentAim;
	CComboBox RageBot_TargetSelection;
	CCheckBox RageBot_AutoFire;
	CKeyBind RageBot_Key;
	CSlider RageBot_MaximumFov;
	CSlider RageBot_MinDamage;
	CCheckBox RageBot_ScaledmgOnHp;
#pragma endregion

#pragma region accuracy
	CCheckBox	RageBot_AutoScope;
	CCheckBox/*CComboBox*/	RageBot_Resolver;
	CCheckBox	RageBot_FixLag;
	CCheckBox	RageBot_AdjustPositions;
	CComboBox	RageBot_DelayShot;
	CSlider		RageBot_HitChance;
	CComboBox	RageBot_AutoStop;
	CComboBox	RageBot_AutoStopMethod;
#pragma endregion

#pragma region body_aim
	CComboBox	RageBot_Baim_PreferBaim;
	CCheckBox	RageBot_Baim_fake;
	CCheckBox	RageBot_Baim_air;
	CCheckBox	RageBot_Baim_accurate;
	CSlider		RageBot_Baim_AfterXshots;
#pragma endregion

#pragma region hitscan
	CSlider		RageBot_PointScale_Head;
	CSlider		RageBot_PointScale_Body;
	CSlider		RageBot_PointScale_Foot;
	CMultiComboBox RageBot_Hitboxes;
	CMultiComboBox RageBot_MultipointHBoxes;
#pragma endregion

#pragma region anti_aim
	CCheckBox	anti_aim_enabled;
	CCheckBox	anti_aim_attarg;
	CComboBox	anti_aim_pitch;
	CCheckBox	anti_aim_freestand;
	CKeyBind	anti_aim_slowwalk_key;
	CSlider		anti_aim_slowwalk_speed;
#pragma endregion

#pragma region stand_yaw
	CComboBox	anti_aim_s_yaw;
	CSlider		anti_aim_s_addyaw;
	CComboBox	anti_aim_s_switchmode;
	CSlider		anti_aim_s_switchspeed;
	CSlider		anti_aim_s_switchangle;
	//CComboBox	RageBot_AutoStop;
	//CComboBox	RageBot_AutoStopMethod;
#pragma endregion

#pragma region move_yaw
	CComboBox	anti_aim_m_yaw;
	CSlider		anti_aim_m_addyaw;

	CComboBox	anti_aim_m_switchmode;
	CSlider		anti_aim_m_switchspeed;
	CSlider		anti_aim_m_switchangle;
	//CComboBox	RageBot_AutoStop;
	//CComboBox	RageBot_AutoStopMethod;
#pragma endregion

#pragma region fake_yaw
	CComboBox	anti_aim_desync;
	CSlider		anti_aim_desync_range;
	CKeyBind	anti_aim_desync_side_switch;
	//CComboBox	RageBot_AutoStop;
	//CComboBox	RageBot_AutoStopMethod;
#pragma endregion

	CGroupBox AntiAim;
	CGroupBox AntiAim_fake;
	CGroupBox AntiAim_standair;
	CGroupBox AntiAim_move;

	CGroupBox menutheme;
	CGroupBox fakelag;
	CGroupBox restrictions;
	CGroupBox exploits;
	CGroupBox configs;

	CCheckBox Visuals_Enable;
	CCheckBox Visuals_teammates;
	CComboBox Visuals_Box;
	CCheckBox Visuals_Name;

	CCheckBox Visuals_AimPoint;
	CCheckBox Visuals_Health;
	CCheckBox Visuals_Armor;
	CMultiComboBox Visuals_Weapon;
	CCheckBox Visuals_Snaplines;
	CColorPicker Visuals_SnaplinesColor;
	CColorPicker Visuals_skeletonColor;
	CCheckBox Visuals_fov_arrows;
	CComboBox Visuals_glow;
	CColorPicker Visuals_glow_color;

	CCheckBox Visuals_chams_teammates;
	CComboBox Visuals_chams_type;
	CColorPicker Visuals_chams_color;
	CCheckBox Visuals_chams_hidden;
	CColorPicker Visuals_chams_hidden_color;
	CColorPicker Visuals_chams_history_color;

	CCheckBox Visuals_lp_forcetp;
	CCheckBox Visuals_lp_forcetpnade;
	CSlider Visuals_lp_tpdist;
	CKeyBind Visuals_lp_toggletp;

	CCheckBox Visuals_wrld_postprocess;
	CCheckBox Visuals_wrld_nightmode;
	CSlider Visuals_wrld_prop_alpha;
	CMultiComboBox Visuals_wrld_entities;
	CColorPicker Visuals_wrld_entities_color;

	CCheckBox Visuals_rem_smoke;
	CCheckBox Visuals_rem_flash;
	CCheckBox Visuals_rem_scope;
	CCheckBox Visuals_rem_punch;
	CCheckBox Visuals_rem_teammate_model;

	CComboBox Visuals_misc_crosshair;
	CCheckBox Visuals_misc_preserve_kills;
	CCheckBox Visuals_misc_hitmarker;
	CComboBox Visuals_misc_wpn_spread;
	CCheckBox Visuals_misc_nade_tracer;
	CCheckBox Visuals_misc_tag;
	CCheckBox Visuals_misc_manual_indicator;
	CCheckBox Visuals_misc_flag_indicator;
	CCheckBox Visuals_misc_desync_indicator;
	CCheckBox Visuals_misc_event_log;
	CSlider Visuals_misc_fov;
	CSlider Visuals_misc_screen_aspr;
	CSlider Visuals_misc_off_x;
	CSlider Visuals_misc_off_y;
	CSlider Visuals_misc_off_z;

	CCheckBox Misc_AntiUT;
	CCheckBox Misc_AntiKICK;

	CKeyBind Exploits_fakeduck;
	CCheckBox Exploits_air_desync;
	CCheckBox Exploits_lc_break;

	CComboBox Visuals_HitBoxes;
	CComboBox Visuals_ChamsHistory;
	CCheckBox Visuals_UseTeamCol;
	CCheckBox Visuals_showAdm;
	CComboBox Visuals_Skeleton;
	CCheckBox Visuals_DroppedWeapons;
	CCheckBox Visuals_Players;
	CCheckBox Visuals_Whitelistents;
	CCheckBox Visuals_Friends;
	CCheckBox Visuals_Radar;
	CComboBox Visuals_SpecList;
	CComboBox Visuals_BadGuyList;
	CComboBox Visuals_HitMarker;
	CCheckBox Visuals_Antiscreengrab;
	CSlider Visuals_Radar_Size;
	CSlider Visuals_Radar_Zoom;
	CCheckBox Misc_AutoStrafeWASD;
	CCheckBox Misc_AutoStrafe;
	CCheckBox Misc_AutoJump;
	CCheckBox Misc_strafe_modifier;
	CSlider Visuals_FOV;
	CSlider Visuals_NightMode;
	CCheckBox Visuals_NoVisRecoil;
	CComboBox Visuals_HandChams;
	CComboBox Visuals_WeaponChams;
	CKeyBind Misc_CircleStrafer;
	CKeyBind Misc_Lag;
	CSlider Misc_lagval;
	CCheckBox Misc_WalkBot;
	//CCheckBox Misc_FakeLag;
	CCheckBox Friendly_Player;
	CCheckBox Spectate_Player;
	CCheckBox Visuals_AutoSpectate;
	CSlider Visuals_AutoSpectate_Distance;
	CKeyBind Misc_AirStuck_Key;

	CKeyBind Misc_aaleft;
	CKeyBind Misc_aaright;
	CKeyBind Misc_aaback;

	CCheckBox Misc_FakeLag;
	CComboBox Misc_fakelag_baseFactor;
	CMultiComboBox Misc_fakelag_triggers;
	CSlider Misc_fakelag_variance;
	CSlider Misc_fakelag_value;

	CGroupBox		Weapons;
	CGroupBox		Skins;
	CListBox		Skins_Weapons;
	CListBox		Skins_Paintkits;
	CTextBox		Skins_Search;
	CComboBox		Skins_Knife;

	CColorPicker EnemyBoxCol;
	CColorPicker GradientFrom;
	CColorPicker GradientTo;
	CColorPicker MenuTheme;
	CColorPicker Chams_HColor;
	/*CColorPicker Player_Visible;
	CColorPicker Player_Invisible;
	CColorPicker Enemy_Invisible;
	CColorPicker Friendly;
	CColorPicker Weapons_Dropped;
	CColorPicker Weapons_Chams;
	CColorPicker arms_chams;
	CColorPicker HitMarkerCol;*/

	CTextBox CreateConfig;
	CListBox PlayerList;
	CListBox Lua_List;
	CListBox EntList;
	CListBox WhiteEntList;
	CListBox Configs;
	CButton SaveButton;
	CButton LoadButton;
	CButton UpdateButton;
	CButton RefreshButton;
	CButton Import;
	CButton ClearKeypadCodesbtn;
	CButton CreateNewConfig;
	CButton deleteConfig;
	CCheckBox Misc_logkeypads;
	CMultiComboBox Misc_NameSteal;
	//CComboBox Misc_KillSay;
	//CCheckBox Thirdperson;
	//CCheckBox Visuals_Local;

	CComboBox Visuals_lglow;
	CColorPicker Visuals_lglow_color;

	CCheckBox Ragebot_headaim_only_on_shot;

	CCheckBox Visuals_lchams_enabled;
	CColorPicker Visuals_lchams_color;
	CCheckBox Visuals_ammo_bar;

	CCheckBox	Visuals_spectators;
	CSlider		Visuals_spectators_alpha;

	CComboBox Visuals_world_sky;
	CMultiComboBox Visuals_enemies_flags;

	CCheckBox Visuals_wrld_impact;
	CSlider Visuals_wrld_impact_duration;

	CCheckBox	anti_aim_slow_walk_accurate;
	CMultiComboBox RageBot_OptimalShot;
	CComboBox		RageBot_exploits;

	CMultiComboBox RageBot_allow_head;
	CMultiComboBox RageBot_bodyaim;

	CMultiComboBox Visuals_Bullet_Tracers;

	CGroupBox AntiAim_shift;

	CComboBox	anti_aim_ex_pitch;
	CComboBox	anti_aim_ex_yaw;
	CSlider		anti_aim_ex_addyaw;
	CCheckBox	Antiaim_flick_exploit;
	CCheckBox	Visuals_unlock_invertory;

	CColorPicker Visuals_spread_crosshair_color;

	CColorPicker Visuals_tracer_local_color;
	CColorPicker Visuals_tracer_enemy_color;
	CColorPicker Visuals_tracer_teammate_color;

	bool bMenu = false;
	bool unload = false;
	bool unloaderror = false;
private:
	//HMODULE m_hModule;
};

extern std::string GetGameDirectory();