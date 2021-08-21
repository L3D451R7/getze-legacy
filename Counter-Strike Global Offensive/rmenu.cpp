#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <sys\stat.h>
#include <algorithm>
#include "rmenu.hpp" 
#include "menu.h"
#include "source.hpp"
#include "hooked.hpp"
#include "simpleini.hpp"

char* KeyStrings[254] = { "none", "mouse1", "mouse2", "BRK", "mouse3", "mouse4", "mouse5",
nullptr, "backspace", "tab", nullptr, nullptr, nullptr, "enter", nullptr, nullptr, "shift",
"ctrl", "alt","pause","capslock", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
"NONE", nullptr, nullptr, nullptr, nullptr, "space","pageup", "pagedown", "end", "home", "left",
"uparrow", "rightarrow", "downarrow", nullptr, "PRNT", nullptr, "printscrn", "insert","delete", nullptr, "0", "1",
"2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u",
"v", "w", "x","y", "z", "LFTWIN", "RGHTWIN", nullptr, nullptr, nullptr, "num0", "num1",
"num2", "num3", "num4", "num5", "num6","num7", "num8", "num9", "*", "+", "_", "-", ".", "/", "F1", "F2", "F3",
"F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12","F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21",
"F22", "F23", "F24", nullptr, nullptr, nullptr, nullptr, nullptr,nullptr, nullptr, nullptr,
"numlock", "scroll lock", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr,nullptr, nullptr, nullptr, nullptr, nullptr, "leftshift", "rightshift", "leftcontrol",
"rightcontrol", "leftmenu", "rightmenu", nullptr,nullptr, nullptr,nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, "NTRK", "PTRK", "STOP", "PLAY", nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, ";", "+", ",", "-", ".", "/?", "~", nullptr, nullptr,
nullptr, nullptr,nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "[{", "\\|", "}]", "'\"", nullptr,
nullptr, nullptr, nullptr,nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, };

char* KeyStringsTextBox[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "",
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "", nullptr, nullptr, nullptr,
nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
"Y", "Z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

//std::string user = "nigger";
float window_alpha = 0.f;
bool fully_update = false;

FORCEINLINE float fade(float start, float end, double curtime, double endtime)
{
	return start + ((end - start) * (curtime / endtime));
}

std::string LowerCase(std::string in)
{
	int dif = 'a' - 'A';
	for (int i = 0; i<in.length(); i++)
	{
		if ((in[i] >= 'A') && (in[i] <= 'Z'))
			in[i] += dif;
	}
	return in;
}

std::vector<CWindow*> windows;

CWindow::CWindow()
{
	//windows.push_back(this);
}

bool CWindow::ShouldDraw()
{
	if (cheat::features::menu.menu_opened) {
		if (window_alpha < 255.f)
			window_alpha += min(255.f - window_alpha, 5.f);
	}
	else
	{
		window_alpha = 0.f;
	}

	window_alpha = Math::clamp(window_alpha, 0.f, 255.f);

	// *Default option* Draws when menu enabled
	if (m_DrawState == Default && cheat::features::menu.menu_opened)
		return true;

	// Draws when menu enabled and custom state is true
	if (m_DrawState == MainWindowExtra)
	{
		if (cheat::features::menu.menu_opened && m_bIsOpen)
			return true;
		else
			return false;
	}

	// Draws by state
	if (m_DrawState == PureState)
		return m_bIsOpen;

	return false;
}

void CWindow::Draw()
{
	// animate the poopoo
	auto animtime = (Source::m_pGlobalVars->realtime - fadetime)/* / CLOCKS_PER_SEC*/;
	animtime = Math::clamp(animtime, 0.f, 0.05f);
	// wait a frame after choosing a tab so u dont select an option by accident
	bool finishtabs = true;
	//if (abs(finalh - m_h) > 5)

	if (finalh != m_h)
		m_h = fade(starth, finalh, animtime, 0.05f);
	else if (animtime == 0.05f)
		m_h = finalh;

	//if (abs(finalw - m_w) > 5)
	if (finalw != m_w)
		m_w = fade(startw, finalw, animtime, 0.05f);
	else if (animtime == 0.05f)
		m_w = finalw;
	// process input

	//base
	Drawing::DrawRectGradientVertical(m_x, m_y, m_w, m_h, Color(33, 34, 36, window_alpha), Color(53, 54, 56, window_alpha));
	// lil tittle rect for tittle
	Drawing::DrawRect(m_x, m_y, m_w, 18, Color(43, 44, 46, window_alpha));
	// inside
	Drawing::DrawRect(m_x + 2, m_y + 18, m_w - 4, m_h - 20, Color(31, 33, 35, window_alpha));
	// the lil cute eagle
	Drawing::DrawStringFont(Drawing::Eagle, m_x + 2, m_y + 2, Color::White(window_alpha), false, "r");
	// the tittle
	if (m_DrawState)
		Drawing::DrawStringFont(Drawing::hFont, m_x + 20, m_y + 3, Color::White(window_alpha), false, Title.c_str());
	// x shit
	if (m_DrawState == Default)
	{
		if (cheat::features::menu.Mousein(Vector(m_x + m_w - 20, m_y,0), Vector(20, 18,0)))
		{
			Drawing::DrawRect(m_x + m_w - 20, m_y, 20, 18, Color(244, 66, 89, window_alpha));
			if (cheat::features::menu.mouse1_pressed && windows.at(windows.size() - 1) == this)
			{
				// handle input
				cheat::features::menu.menu_opened = false;
				Source::m_pInputSystem->EnableInput(!cheat::features::menu.menu_opened);
			}
		}
		// the lil x
		Drawing::DrawStringFont(Drawing::Eagle, m_x + m_w - 10, m_y + 2, Color::White(window_alpha), 1, "z");
	}

	//Drawing::DrawStringFont(Drawing::ESPFont, m_x + 2 + 16, m_y + 3, Color::White(window_alpha), 0, Title.c_str());

	if (Tabs.size() > 1 && SelectedTab == nullptr)
	{
		// Big Bird
		Drawing::DrawStringFont(Drawing::mideagle, m_x + m_w / 2, m_y + 30, Color::White(window_alpha), 1, "r");
		// Welcome, user
		Drawing::DrawStringFont(Drawing::SegoeUI, m_x + m_w / 2, m_y + 65, Color::White(window_alpha), 1, "getze.us");
		// Line?
		Drawing::DrawRectGradientHorizontal(m_x + 3, m_y + 85, m_w - 6, 1, cheat::Cvars.GradientFrom.GetColor().alpha(window_alpha), cheat::Cvars.GradientTo.GetColor().alpha(window_alpha));

		//Drawing::GradientH(m_x + 3, m_y + 85, m_w - 6, 1, cheat::Cvars.GradientFrom.GetColor(), cheat::Cvars.GradientTo.GetColor());
		constexpr float fadetime = 0.5f;
		constexpr int min_alpha = 35;
		const auto step = (255 - min_alpha) / fadetime;
		int row = 1;
		int position = 1;

		for (int i = 0; i < Tabs.size(); ++i)
		{
			auto tab = Tabs[i];
			//auto secondsPassed = (clock() - tab->fadetime) / CLOCKS_PER_SEC;
			//secondsPassed = Math::clamp(secondsPassed, 0.f, 0.5f);
			//Drawing::DrawStringFont(30, 30 + (i * 10), Color::White(), 0, "passed: %f, alpha: %f", secondsPassed, tab->alpha);
			tab->alpha = Math::clamp(tab->alpha, min_alpha, 255);
			//Color tabcolor = Color(cheat::Cvars.GradientFrom.GetColor().r()
			//	, cheat::Cvars.GradientFrom.GetColor().g(),
			//	cheat::Cvars.GradientFrom.GetColor().b(),
			//	tab->alpha);
			Color tabcolor = Color(255, 255, 255, min(window_alpha,tab->alpha));
			//if (tab->alpha > 105.f)
			//	tabcolor = Color(cheat::Cvars.GradientFrom.GetColor().r()
			//		, cheat::Cvars.GradientFrom.GetColor().g(),
			//		cheat::Cvars.GradientFrom.GetColor().b(),
			//		tab->alpha);

			if (!(i % 3) && i > 0)
			{
				position = 0;
				row++;
			}
			if (i) position++;
			// handle shit below i think i was drugged while doing this
			if (cheat::features::menu.Mousein(Vector(m_x + (position * 200) - 160 - 2, m_y + 20 + (row * 180) - 80 - 2,0), Vector(125 + 4, 150 + 4,0)))
			{
				if (tab->alpha < 255) {
					tab->alpha += step * Source::m_pGlobalVars->absoluteframetime;
				}

				if (cheat::features::menu.mouse1_pressed && windows.at(windows.size() - 1) == this)
				{
					SelectedTab = tab;
					finishtabs = false;
					// no break so we can get size if final height not set..
					//break;
				}
			}
			else
			{
				if (tab->alpha > min_alpha) {
					tab->alpha -= step * Source::m_pGlobalVars->absoluteframetime;
				}
			}

			Drawing::DrawOutlinedRect(m_x + (position * 200) - 160 - 2, m_y + 20 + (row * 180) - 80 - 2, 125 + 4, 150 + 4, Color(44, 47, 51, window_alpha));
			Drawing::DrawRect(m_x + (position * 200) - 160, m_y + 20 + (row * 180) - 80, 125, 150, Color(44, 47, 51, window_alpha));
			Drawing::DrawStringFont(Drawing::SegoeUI, m_x + (position * 200) - 100, m_y + 35 + (row * 180),
				tabcolor, 1, tab->Title.c_str());
			Drawing::DrawStringFont(Drawing::Tabfont, m_x + (position * 200) - 100, m_y - 20 + (row * 180),
				tabcolor, 1, tab->Icon.c_str());

		}
		//finalh = row * 200 + 47;
		//finalw = o_w;
	}


	// Back Arrow gotta do it b4 other shit or byebye
	if (m_DrawState == Default && this->SelectedTab != nullptr)
	{
		if (SelectedTab->h)
		{
			finalh = SelectedTab->h;
			starth = m_h;
			fadetime = Source::m_pGlobalVars->realtime;
		}
		if (SelectedTab->w)
		{
			finalw = SelectedTab->w;
			startw = m_w;
			fadetime = Source::m_pGlobalVars->realtime;
		}

		Color arrowcol(Color::White(window_alpha));
		//Drawing::DrawOutlineRect(m_x + 5, m_y + 20, m_w / 3, 55, Color::White());
		if (cheat::features::menu.Mousein(Vector(m_x + 5, m_y + 20,0), Vector(m_w / 3, 55,0)))
		{
			arrowcol = cheat::Cvars.MenuTheme.GetColor().alpha(window_alpha);
			//change arrow col
			Drawing::DrawStringFont(Drawing::hFont, m_x + 10, m_y + 60, arrowcol, false, "'ESC' or 'BCKSPC'");
			if (cheat::features::menu.mouse1_pressed && windows.at(windows.size() - 1) == this)
			{
				finalh = o_h;
				finalw = o_w;
				startw = m_w;
				starth = m_h;
				fadetime = Source::m_pGlobalVars->realtime;
				SelectedTab = nullptr;
				if (FocusedControl)
				{
					FocusedControl->OnClick();
					FocusedControl = nullptr;
				}
			}
		}

		Drawing::DrawStringFont(Drawing::BackArrow, m_x + 35, m_y + 42, arrowcol, 1, "g");

		// not waiting for input & pressed escape so go back nigga
		if (!this->FocusedControl && windows.at(windows.size() - 1) == this && (cheat::game::pressed_keys[VK_ESCAPE]
			|| cheat::game::pressed_keys[VK_BACK]))
		{
			finalh = o_h;
			finalw = o_w;
			startw = m_w;
			starth = m_h;
			fadetime = Source::m_pGlobalVars->realtime;
			SelectedTab = nullptr;
			if (FocusedControl)
			{
				FocusedControl->OnClick();
				FocusedControl = nullptr;
			}
		}
	}

	// draw the controls
	if (this->SelectedTab != nullptr)
	{
		if (this->SelectedTab->Title.find("Skins") != std::string::npos && parser::weapons.list.size() < 42)
			return;

		if (m_DrawState == Default)
		{
			// Big Bird
			Drawing::DrawStringFont(Drawing::mideagle, m_x + m_w / 2, m_y + 30, Color::White(window_alpha), 1, this->SelectedTab->Icon.c_str());
			// Tab Title
			Drawing::DrawStringFont(Drawing::SegoeUI, m_x + m_w / 2, m_y + 65, Color::White(window_alpha), 1, this->SelectedTab->Title.c_str());
			// Line?
			Drawing::DrawRectGradientHorizontal(m_x + 3, m_y + 85, m_w - 6, 1, cheat::Cvars.GradientFrom.GetColor().alpha(window_alpha), cheat::Cvars.GradientTo.GetColor().alpha(window_alpha));
			//Drawing::GradientH(m_x + 3, m_y + 85, m_w - 6, 1, cheat::Cvars.GradientFrom.GetColor(), cheat::Cvars.GradientTo.GetColor());
		}

		// Focused widget
		bool SkipWidget = false;
		CBaseControl* SkipMe = nullptr;

		if (this->FocusedControl != nullptr)
		{
			// We need to draw it last, so skip it in the regular loop
			SkipWidget = true;
			SkipMe = this->FocusedControl;
		}

		// draw all controls with a drawable flag
		if (this->SelectedTab->Controls.size() > 0)
		{
			auto tab = this->SelectedTab->tab;

			//draw sub tabs
			if (this->SelectedTab->Controls.size() > 1)
			{
				Drawing::DrawOutlinedRect(m_x + 3, m_y + 90, m_w - 6, 26, Color(43, 44, 46, window_alpha));
				Drawing::DrawRectGradientVertical(m_x + 4, m_y + 91, m_w - 8, 25, Color(43, 44, 46, window_alpha), Color(37, 38, 38, window_alpha));
				int TabSize = (m_w - 5) / this->SelectedTab->Controls.size();

				for (int i = 0; i < this->SelectedTab->Controls.size(); ++i)
				{
					if (cheat::features::menu.mouse1_pressed && cheat::features::menu.Mousein(Vector(m_x + 3 + (i*TabSize), m_y + 90,0), Vector(TabSize, 25,0)))
						this->SelectedTab->tab = i;

					if (i == tab)
						Drawing::DrawRectGradientVertical(m_x + 3 + (i*TabSize), m_y + 90, TabSize, 25, Color(37, 38, 38, window_alpha), Color(43, 44, 46, window_alpha));


					Drawing::DrawStringFont(Drawing::SegoeUI,m_x + 3 + (i*TabSize + 1) + TabSize / 2.f, m_y + 95, Color::White(window_alpha), 1, this->SelectedTab->Controls[i].title.c_str());
				}
			}

			for (int i = 0; i < this->SelectedTab->Controls[tab].Controls.size(); ++i)
			{
				auto control = this->SelectedTab->Controls[tab].Controls[i];

				if (SkipWidget && SkipMe == control /*|| control->Flag(UI_SkipRender)*/)
					continue;

				if (control && control->Flag(UI_Drawable))
				{
					auto positions = control->GetPos();
					bool hover = false;
					if (cheat::features::menu.Mousein(Vector(positions.x - 1, positions.y - 1,0), Vector(control->GetClickArea().x, control->GetClickArea().y,0))
						&& windows.at(windows.size() - 1) == this)
					{
						if (!SkipWidget)
						{
							hover = true;
							if (cheat::features::menu.mouse1_pressed)
							{
								if (finishtabs)
									control->OnClick();
								if (control->Flag(UI_Focusable) && finishtabs)
								{
									this->FocusedControl = control;
									control->FinishedFocus = false;
								}
							}
						}
					}

					control->Draw(hover);
					// debug
					//Drawing::DrawRect(positions.x - 1, positions.y - 1, control->GetClickArea().x, control->GetClickArea().y,Color(255,255,255,100));
				}
			}

			if (SkipWidget)
			{
				auto control = this->FocusedControl;

				if (control && control->Flag(UI_Drawable))
				{
					if (cheat::features::menu.mouse1_pressed)
						control->OnClick();

					// to let the control decide wether focus has been finished
					if (control->FinishedFocus)
						this->FocusedControl = nullptr;

					control->Draw(SkipWidget);
					// debug
					//Drawing::DrawRect(positions.x - 1, positions.y - 1, control->GetClickArea().x, control->GetClickArea().y,Color(255,255,255,100));
				}
			}
		}
	}

	if (cheat::features::menu.Mousein(Vector(m_x, m_y, 0), Vector(m_w, grabheight, 0)) && windows.at(windows.size() - 1) == this)
	{
		if (cheat::game::pressed_keys[1]) {

			if (!IsDraggingWindow) {
				DragOffsetX = cheat::features::menu._cursor_position.x - m_x;
				DragOffsetY = cheat::features::menu._cursor_position.y - m_y;
				IsDraggingWindow = true;
			}
		}
	}

	if (IsDraggingWindow && !cheat::game::pressed_keys[1])
		IsDraggingWindow = false;

	if (IsDraggingWindow)
	{
		m_x = cheat::features::menu._cursor_position.x;
		m_y = cheat::features::menu._cursor_position.y;
		m_x = m_x - DragOffsetX;
		m_y = m_y - DragOffsetY;
	}
}

void CWindow::SetupWindow(int x, int y, int w, int h, std::string title, int iDrawState)
{
	if (windows.size() == 0)
		m_bUseIcon = true;

	SetPosition(x, y);
	SetSize(w, h);
	SetTitle(title);
	SetDrawState(iDrawState);
	windows.push_back(this);
}

RECT CWindow::GetClientArea()
{
	RECT window;
	if (Tabs.size() > 1)
		window.top = m_y + 75 + tabsize;
	else
		window.top = m_y + 30;

	window.left = m_x + 5;
	window.right = m_x + m_w - 5;
	window.bottom = m_x + m_h - 5;
	return window;
}

std::vector<SaveItem> AllControls;

Vector2D CBaseControl::GetPos()
{
	if (parent)
		return Vector2D(parent->GetClientArea().left + m_x, parent->GetClientArea().top + m_y);

	return Vector2D(0, 0);
}

CCheckBox::CCheckBox()
{
	Checked = false;

	m_Flags = UI_Clickable | UI_Drawable | UI_SaveFile;
	m_w = 10;
	m_h = 10;
	m_SaveName = std::string("CheckBox" + std::to_string(AllControls.size()));
	AllControls.push_back({ m_SaveName, this });

	ControlType = UIC_CheckBox;
}

void CCheckBox::Draw(bool hover)
{
	auto pos = GetPos();
	Color Checkmark = Color(0, 0, 0, 0);
	Color Outline(Color(43, 44, 46, window_alpha));

	if (hover)
		Outline = Color(53, 54, 56, window_alpha);

	if (Checked)
		Checkmark = cheat::Cvars.MenuTheme.GetColor().alpha(window_alpha);// cheat::Cvars.GradientFrom.GetColor();
													 //-15
													 // outline
	Drawing::DrawRect(pos.x + ((GetClickArea().x * 2 - 50) / 2) - 1, pos.y - 1, m_w + 2, m_h + 2, Outline);
	// main inside
	Drawing::DrawRectGradientVertical(pos.x + ((GetClickArea().x * 2 - 50) / 2), pos.y, m_w, m_h, Color(43, 44, 46, window_alpha), Color(37, 38, 38, window_alpha));
	// CheckMark
	Drawing::DrawStringFont(Drawing::Checkmark, pos.x + ((GetClickArea().x * 2 - 50) / 2), pos.y, Checkmark, false, "h");
	// Option name
	Drawing::DrawStringFont(Drawing::SegoeUI, pos.x, pos.y - 3, Color(190, 190, 190, window_alpha), 0, label.c_str());

	if (hover && !desc.empty())
	{
		auto size = static_cast<int>(Drawing::GetTextSize(Drawing::SegoeUI, desc.c_str()).right);
		Drawing::DrawOutlinedRect(cheat::features::menu._cursor_position.x - size / 2 - 2, cheat::features::menu._cursor_position.y - 25, size + 4, 19, Outline);
		Drawing::DrawRect(cheat::features::menu._cursor_position.x - size / 2 - 1, cheat::features::menu._cursor_position.y - 24, size + 2, 17, Color(43, 44, 46, window_alpha));
		Drawing::DrawStringFont(Drawing::SegoeUI, cheat::features::menu._cursor_position.x, cheat::features::menu._cursor_position.y - 24, Color(250, 250, 250, window_alpha), 1, desc.c_str());
	}

}

void CCheckBox::OnUpdate()
{
}

void CCheckBox::OnClick()
{
	Checked = !Checked;
}

Vector2D CCheckBox::GetPush()
{
	return Vector2D(m_w, m_h + 8);
}

Vector2D CCheckBox::GetClickArea()
{
	if (parentgroup)
		return Vector2D(parentgroup->GetSize().x /** 2*/, m_h + 4);
	else
		return Vector2D(m_w + Drawing::GetTextSize(Drawing::hFont, label.c_str()).right + 5, m_h + 2);
}

//Vector2 CCheckBox::SetClickArea(int w, int h)
//{
//	Vector2(m_w + Drawing::GetTextSize(Drawing::hFont, label.c_str()).right + 5, m_h + 2);
//}


CGroupBox::CGroupBox()
{
	m_Flags = UI_Drawable | UI_RenderFirst;
	ControlType = UIC_GroupBox;
}

void CGroupBox::PlaceControl(std::string Label, CBaseControl * control, int tab)
{
	if (!ParentTab)
		return;

	// so you can have same comboboxes for different tabs
	if (tab != lasttab)
		PreviousControl = nullptr;
	// set control position
	Vector2D pos(0, 0);
	pos.x = m_x + 5;
	if (PreviousControl)
	{
		pos.y = PreviousControl->GetPosY() + PreviousControl->GetPush().y;
	}
	else
	{
		// give room for gay subtabs lol
		if (subtabs.size())
			pos.y = m_y + 35;
		else
			pos.y = m_y + 5;
	}

	control->SetPos(pos.x, pos.y);
	if (control->ControlType == UIC_Slider || control->ControlType == UIC_ComboBox || control->ControlType == UIC_MultiComboBox || control->ControlType == UIC_TextBox || control->ControlType == UIC_Button || control->ControlType == UIC_KeyBind)
	{
		control->SetSize(m_w / 2 - 10, 12);
	}
	else if (control->ControlType == UIC_Colorpicker)
	{
		control->SetSize(m_w - 10, 12);
	}
	else if (control->ControlType == UIC_ListBox)
	{
		Vector2D Size;
		Size.x = control->GetSize().x;
		Size.y = control->GetSize().y;

		if (Size.x < 55.0f || Size.x > m_w - 10)
			Size.x = m_w - 10;
		if (Size.y < 50.f)
			Size.y = 55.f;

		control->SetSize(Size.x, Size.y);
	}

	if (control)
	{
		if (pos.y + control->GetPush().y > m_h)
			m_h = (control->GetPosY() - GetPosY()) + control->GetPush().y + 5;
	}

	ParentTab->RegisterControl(control, Label, this, tab);
	PreviousControl = control;
	lasttab = tab;
}

void CGroupBox::Draw(bool hover)
{
	auto pos = GetPos();

	// draw outline
	Color outline(20, 20, 20, min(window_alpha,100));
	Drawing::DrawRect(pos.x, pos.y - 24, m_w, 20, Color(43, 44, 46, window_alpha));
	Drawing::DrawOutlinedRect(pos.x, pos.y - 25, m_w, m_h + 22, Color(43, 44, 46, window_alpha));
	Drawing::DrawStringFont(Drawing::SegoeUI, pos.x + 5 /*+ (m_w / 2)*/, pos.y - 23, Color::White(window_alpha), 0, label.c_str());
}

void CGroupBox::OnUpdate()
{
}

void CGroupBox::OnClick()
{
	auto pos = GetPos();
	if (subtabs.size() > 0)
	{
		int TabSize = (m_w - 10) / subtabs.size();
		for (int i = 0; i < subtabs.size(); ++i)
		{
			if (cheat::features::menu.Mousein(Vector(pos.x + 5 + (i*TabSize + 1), pos.y,0), Vector(TabSize, 30,0)))
				SelectedSubTab = i;
		}
	}
}

CComboBox::CComboBox()
{
	m_Flags = UI_Drawable | UI_Focusable;
	ControlType = UIC_ComboBox;
	m_w = 25;
	m_h = 60;

	m_SaveName = std::string("ComboBox" + std::to_string(AllControls.size()));
	AllControls.push_back({ m_SaveName, this });

}

void CComboBox::AddItems(std::vector<std::string> items)
{
	// Clear from previous frame
	Items.clear();
	// Set current frame
	for (auto text : items)
		Items.push_back(text);
}

void CComboBox::AddKnifes(std::vector<knife_t> items)
{
	// Clear from previous frame
	Items.clear();
	// Set current frame
	for (auto text : items)
		Items.push_back(text.translated_name);
}

Vector2D CComboBox::GetClickArea()
{
	if (parentgroup)
		return Vector2D(parentgroup->GetSize().x - 10, m_h);
	else
		return Vector2D(m_w, m_h);
}

void CComboBox::Draw(bool hover)
{
	auto pos = GetPos();
	Color Text(190, 190, 190, window_alpha);
	Color Outline(43, 44, 46, window_alpha);

	if (hover)
		Outline = Color(53, 54, 56, window_alpha);

	if (Items.size() > 0 && m_bIsOpen)
		m_h = 17 + 15 * Items.size();

	SelectedIndex = Math::clamp(SelectedIndex, 0, (int)Items.size());

	// outline
	Drawing::DrawRect(pos.x + (GetClickArea().x / 2) - 1, pos.y - 1, m_w + 2, m_h + 2, Outline);
	// main inside
	Drawing::DrawRectGradientVertical(pos.x + (GetClickArea().x / 2), pos.y, m_w, m_h, Color(43, 44, 46, window_alpha), Color(37, 38, 38, window_alpha));
	// Label
	Drawing::DrawStringFont(Drawing::SegoeUI, pos.x, pos.y - 2, Text, 0, label.c_str());
	// Current items
	Drawing::DrawStringFont(Drawing::hFont, pos.x + (GetClickArea().x / 2) + 2, pos.y, Text, 0, GetName().c_str());

	// scroll
	auto scroll = GetScroll();
	if (cheat::features::menu.Mousein(Vector(pos.x, pos.y,0), Vector(GetClickArea().x, GetClickArea().y,0)) && scroll)
	{
		if (scroll > 0)
		{
			if (SelectedIndex != 0)
				SelectedIndex -= 1;
		}
		else
		{

			if (SelectedIndex != Items.size() - 1)
				SelectedIndex += 1;
		}
	}

	// Draw the items
	if (Items.size() > 0 && m_bIsOpen)
	{
		for (int i = 0; i < Items.size(); i++)
		{
			Color Text = Color(180, 180, 180, window_alpha);

			if (cheat::features::menu.Mousein(Vector(pos.x + (GetClickArea().x / 2), pos.y + 16 + i * 15,0), Vector(m_w, 15,0)))
			{
				Text = Color::White(window_alpha);
				Drawing::DrawRect(pos.x + (GetClickArea().x / 2) + 1, pos.y + 16 + i * 15, m_w - 2, 14, Color(255, 255, 255, min(window_alpha,20)));
			}

			Drawing::DrawStringFont(Drawing::hFont, pos.x + (GetClickArea().x / 2) + 3, pos.y + 16 + i * 15, Text, 0, Items[i].c_str());
		}
	}
}

void CComboBox::OnUpdate()
{
	SelectedIndex = Math::clamp(SelectedIndex, 0, (int)Items.size());
}

void CComboBox::OnClick()
{
	if (m_bIsOpen)
	{
		auto pos = GetPos();
		for (int i = 0; i < Items.size(); i++)
		{
			if (cheat::features::menu.Mousein(Vector(pos.x + (GetClickArea().x / 2), pos.y + 16 + i * 15,0), Vector(m_w, 15,0)))
				SelectedIndex = i;
		}
		FinishedFocus = true;
		m_h = 12;
	}

	m_bIsOpen = !m_bIsOpen;
}

CSlider::CSlider()
{
	m_Flags = UI_Drawable | UI_Focusable; // forgot to put focus flag before lol
	ControlType = UIC_Slider;
	m_w = 25;
	m_h = 30;
	Min = 0.0f;
	Max = 15.0f;
	Value = 1.0f;

	m_SaveName = std::string("Slider" + std::to_string(AllControls.size()));
	AllControls.push_back({ m_SaveName, this });

}

void CSlider::Draw(bool hover)
{
	auto pos = GetPos();
	Color Text(190, 190, 190, window_alpha);
	Color Outline(43, 44, 46, window_alpha);

	if (hover || DoDrag)
		Outline = Color(53, 54, 56, window_alpha);

	if (DoDrag)
	{
		if (cheat::game::pressed_keys[1])
		{
			Vector m = cheat::features::menu._cursor_position;
			float NewX;
			float Ratio;
			NewX = m.x - (pos.x + ((parentgroup->GetSize().x - 10) / 2)) - 1;
			if (NewX < 0) NewX = 0;
			if (NewX > m_w) NewX = m_w;
			Ratio = NewX / float(m_w);
			Value = Min + (Max - Min)*Ratio;
		}
		else
		{
			DoDrag = false;
			FinishedFocus = true;
		}
	}
	else if (IsTyping)
	{
		if (cheat::game::get_key_press(VK_RETURN) && !this->Text.empty())
		{
			FinishedFocus = true;
			IsTyping = false;
			Value = Math::clamp(std::stof(this->Text), Min, Max);
		}

		for (int i = 0; i < 255; i++)
		{
			if (cheat::game::get_key_press(i))
			{
				if (i == VK_BACK && this->Text.length() > 0)
				{
					this->Text.pop_back();
					fasterase = clock();
				}

				auto textsize = Drawing::GetTextSize(Drawing::hFont, this->Text.c_str());
				if (textsize.right > (m_w - 10))
					continue;

				if (KeyStrings[i] == nullptr)
					continue;

				if (isdigit(KeyStrings[i][0]) || KeyStrings[i] == "." || KeyStrings[i] == "-")
					this->Text += KeyStrings[i];

			}
		}
		if (cheat::game::get_key_press(VK_BACK))
		{
			auto secondsPassed = (clock() - fasterase) / CLOCKS_PER_SEC;
			auto erasenext = (clock() - delayerase) / CLOCKS_PER_SEC;
			if (this->Text.length() > 0 && secondsPassed > 0.5f && erasenext > 0.05f)
			{
				this->Text.pop_back();
				delayerase = clock();
			}
		}
	}

	const auto dist_from = (Min < 0 ? abs(Min) + Value : Value);
	Location = (dist_from / (abs(Min) + abs(Max))) * (m_w - 2);

	// outline
	Drawing::DrawRect(pos.x + ((parentgroup->GetSize().x - 10) / 2) - 1, pos.y - 1, m_w + 2, m_h + 2, Outline);
	// main inside
	Drawing::DrawRectGradientVertical(pos.x + ((parentgroup->GetSize().x - 10) / 2), pos.y, m_w, m_h, Color(43, 44, 46, window_alpha), Color(37, 38, 38, window_alpha));
	// Label
	//Drawing::DrawStringFont(pos.x, pos.y, Text, 0, "%s - %.2f", label.c_str(), Value);
	Drawing::DrawStringFont(Drawing::SegoeUI, pos.x, pos.y - 2, Text, 0, label.c_str());
	// Draw Slider shadow and pos
	//Drawing::DrawRect(pos.x + Location, pos.y + 13, 5, m_h - 2, Color(43, 43, 43));
	Drawing::DrawRectGradientVertical(pos.x + ((parentgroup->GetSize().x - 10) / 2) + 1, pos.y + 1, Location, m_h - 2, cheat::Cvars.MenuTheme.GetColor().alpha(window_alpha), Color(43, 44, 46, window_alpha));
	
	if (IsTyping)
		Drawing::DrawStringFont(Drawing::hFont, pos.x + ((parentgroup->GetSize().x - 10) / 2) + 2, pos.y, Text, false, this->Text.c_str());
	else {
		/*auto isRange = (label.find("limit") != std::string::npos);

		if (isRange && (Value <= (Min + 1.f) || Value >= (Max - 1.f)))
			Drawing::DrawStringFont(Drawing::hFont, pos.x + ((parentgroup->GetSize().x - 10) / 2) + m_w / 2, pos.y, Text, true, ((Value >= (Max - 1.f)) ? "auto (balance)" : "auto (static)"));
		else {*/
			Drawing::DrawStringFont(Drawing::hFont, pos.x + ((parentgroup->GetSize().x - 10) / 2) + 2, pos.y, Text, false, format, Value);

		/*	auto text = Value < 35.f ? "static" : "balance";

			if (isRange) {

				auto textSize = Drawing::GetTextSize(Drawing::hFont, text);

				Drawing::DrawStringFont(Drawing::hFont, pos.x + ((parentgroup->GetSize().x - 10) / 2) + m_w - textSize.right, pos.y, Text, false, text);
			}
		}*/
	}
}

void CSlider::OnUpdate()
{
}

void CSlider::OnClick()
{
	auto pos = GetPos();
	if (cheat::features::menu.Mousein(Vector(pos.x + ((parentgroup->GetSize().x - 10) / 2), pos.y,0), Vector(m_w, m_h,0)))
	{
		if (cheat::game::pressed_keys[17])
		{
			IsTyping = true;
			Text = std::to_string(Value);

		}
		else
		{
			DoDrag = true;
		}
	}
	else
	{
		DoDrag = false;
		IsTyping = false;
	}
}

Vector2D CSlider::GetClickArea()
{
	if (parentgroup)
		return Vector2D(parentgroup->GetSize().x - 10, m_h);
	else
		return Vector2D(m_w, m_h);
}

CTextBox::CTextBox()
{
	m_Flags = UI_Drawable | UI_Focusable;
	ControlType = UIC_TextBox;
	m_w = 25;
	m_h = 30;
	AllowSpaces = true;
	OnlyDigits = false;
}

void CTextBox::Draw(bool hover)
{
	auto pos = GetPos();
	Color Text(190, 190, 190, window_alpha);
	Color Outline(43, 44, 46, window_alpha);

	if (hover || IsTyping)
		Outline = Color(53, 54, 56, window_alpha);

	if (IsTyping)
	{
		if (cheat::game::get_key_press(VK_RETURN))
		{
			FinishedFocus = true;
			IsTyping = false;
		}

		for (int i = 0; i < 255; i++)
		{
			if (cheat::game::get_key_press(i))
			{
				if (i == VK_LBUTTON)
				{
					continue;
				}

				if (i == VK_BACK && this->Text.length() > 0)
				{
					this->Text.erase(this->Text.end() - 1);
					fasterase = clock();
				}

				auto textsize = Drawing::GetTextSize(Drawing::hFont, this->Text.c_str());
				if (textsize.right > (m_w - 10))
					continue;

				// love me some operators
				if (AllowSpaces && i == VK_SPACE)
					this->Text += " ";

				if (KeyStringsTextBox[i] == nullptr)
					continue;

				if (OnlyDigits && !isdigit(KeyStringsTextBox[i][0]))
					continue;



				this->Text += (cheat::game::get_key_press(VK_SHIFT)) ? KeyStringsTextBox[i] : LowerCase(KeyStringsTextBox[i]);
			}
		}
		if (cheat::game::get_key_press(VK_BACK))
		{
			auto secondsPassed = (clock() - fasterase) / CLOCKS_PER_SEC;
			auto erasenext = (clock() - delayerase) / CLOCKS_PER_SEC;
			if (this->Text.length() > 0 && secondsPassed > 0.5f && erasenext > 0.05f)
			{
				this->Text.erase(this->Text.end() - 1);
				delayerase = clock();
			}
		}
	}

	// outline
	Drawing::DrawRect(pos.x + (GetClickArea().x / 2) - 1, pos.y - 1, m_w + 2, m_h + 2, Outline);
	// main inside
	Drawing::DrawRectGradientVertical(pos.x + (GetClickArea().x / 2), pos.y, m_w, m_h, Color(43, 44, 46, window_alpha), Color(37, 38, 38, window_alpha));
	// Label
	//Drawing::DrawStringFont(pos.x, pos.y, Text, 0, "%s - %.2f", label.c_str(), Value);
	Drawing::DrawStringFont(Drawing::SegoeUI, pos.x, pos.y - 3, Text, 0, "%s", label.c_str());
	Drawing::DrawStringFont(Drawing::hFont,pos.x + (GetClickArea().x / 2) + 1, pos.y, Text, 0, IsTyping ? "%s_" : "%s", this->Text.c_str());
	//Drawing::DrawRect(pos.x + Location + 1, pos.y + 14, 3, m_h - 4, Color(83, 83, 83));
}

void CTextBox::OnUpdate()
{
}

void CTextBox::OnClick()
{
	if (IsTyping)
	{
		FinishedFocus = true;
		IsTyping = false;
	}
	else
		IsTyping = true;
}

Vector2D CTextBox::GetClickArea()
{
	if (parentgroup)
		return Vector2D(parentgroup->GetSize().x - 10, m_h);
	else
		return Vector2D(m_w, m_h);
}

//Editor::Editor()
//{
//	AddInstruction("DrawLine", " ( int x, int y, int xend, int yend, Color_r_g_b )", 5);
//	AddInstruction("DrawText", " ( int x, int y, char text_, Color_r_g_b )", 4);
//	AddInstruction("DrawBox", " ( int x, int y, int width, int height, Color_r_g_b)", 5);
//	AddInstruction("DrawOutLineBox", " ( int x, int y, int width, int height, Color_r_g_b )", 5);
//	AddInstruction("DrawGradientV", " ( int x, int y, int w, int h, Color_r_g_b, Color_r_g_b )", 6);
//	AddInstruction("Loop_Start", " ( int x )", 1);
//	AddInstruction("Loop_End", " (  )", 0);
//	AddInstruction("Execute", " ( char command, char execute ) ", 2);
//	m_Flags = UI_Drawable | UI_Focusable;
//	ControlType = UIC_TextBox;
//	m_w = 25;
//	m_h = 30;
//	m_x = 5;
//	m_y = 5;
//}
//
//void Editor::UpdateShit()
//{
//}
//
//
//void Editor::Draw(bool hover)
//{
//	m_x = 0;
//	m_y = 0;
//	m_w = this->parent->m_w - 10;
//	m_h = this->parent->m_h - 70;
//	auto pos = GetPos();
//	std::vector<std::string> helper;
//
//	Color Text(190, 190, 190, 255);
//	Color Outline(17, 17, 17, 255);
//	if (hover || IsTyping)
//		Outline = Color(255, 255, 255, 60);
//	if (!this->Text.size())
//	{
//		this->Text.push_back({ "" });
//		line = this->Text.size();
//	}
//
//
//	if (IsTyping)
//	{
//		//Drawing::DrawStringFont(40, 40, Color::Green(), 0, "line: %i lines:%i", line, this->Text.size());
//		if (GetAsyncKeyState(VK_RETURN) && sstring.size() && this->Text.size() < 32)
//		{
//			// we are at the end
//			if (line >= this->Text.size())
//			{
//				// add each word to the vector of words in this line && create newline
//				std::stringstream ss(sstring);
//				std::istream_iterator<std::string> begin(ss);
//				std::istream_iterator<std::string> end;
//				std::vector<std::string> vstrings(begin, end);
//				std::copy(vstrings.begin(), vstrings.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
//				this->Text.push_back(vstrings);
//				sstring.clear();
//				line++;
//			}
//			else// we are editing a line
//			{
//				std::stringstream ss(sstring);
//				std::istream_iterator<std::string> begin(ss);
//				std::istream_iterator<std::string> end;
//				std::vector<std::string> vstrings(begin, end);
//				std::copy(vstrings.begin(), vstrings.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
//				this->Text.erase(this->Text.begin() + line);
//				this->Text.insert(this->Text.begin() + line, vstrings);
//				sstring.clear();
//				if (line == this->Text.size() - 1)
//				{
//					line = this->Text.size();
//				}
//				else
//				{
//					// create new line instead of continue to the next line incase they want to add something?
//					line++;
//					this->Text.insert(this->Text.begin() + line, { "" });
//					//for (int j = 0; j < this->Text[line].size(); ++j)
//					//{
//					//	sstring += this->Text[line][j];
//					//	sstring += " ";
//					//}
//				}
//			}
//		}
//
//		for (int i = 0; i < 255; i++)
//		{
//			if (GetAsyncKeyState(i))
//			{
//				if (i == VK_LBUTTON)
//				{
//					sstring.clear();
//					bool found = false;
//					for (int j = 1; j < this->Text.size(); ++j)
//					{
//						// Distance between each item is 12
//						if (cheat::features::menu.Mousein(pos.x, pos.y + 12 + (j * 12), m_w, 12) && windows.at(windows.size() - 1) == parent)
//						{
//							line = j;
//							for (int p = 0; p < this->Text[line].size(); ++p)
//							{
//								sstring += this->Text[line][p];
//								sstring += " ";
//							}
//							found = true;
//							break;
//						}
//					}
//					if (!found)
//						line = this->Text.size();
//
//					continue;
//				}
//
//				if (i == VK_UP || i == VK_DOWN && (this->Text.size() > 1))
//				{
//					std::string oldstring = sstring;
//					sstring.clear();
//					int oldline = line;
//					if (i == VK_UP)
//					{
//						if (line > 1)
//							line--;
//					}
//					else
//					{
//						if (line < this->Text.size())
//							line++;
//					}
//					// fill new string with next data
//					if (line != this->Text.size())
//					{
//						// fill old string with new data if its not the end
//						if (oldline != this->Text.size())
//						{
//							std::stringstream ss(oldstring);
//							std::istream_iterator<std::string> begin(ss);
//							std::istream_iterator<std::string> end;
//							std::vector<std::string> vstrings(begin, end);
//							std::copy(vstrings.begin(), vstrings.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
//							this->Text.erase(this->Text.begin() + oldline);
//							this->Text.insert(this->Text.begin() + oldline, vstrings);
//						}
//
//						for (int j = 0; j < this->Text[line].size(); ++j)
//						{
//							sstring += this->Text[line][j];
//							sstring += " ";
//						}
//					}
//
//					//if (line == this->Text.size() - 1)
//					//{
//					//	line = this->Text.size();
//					//}
//					//else
//				//	{
//						// create new line instead of continue to the next line incase they want to add something?
//						//this->Text.insert(this->Text.begin() + line, { "" });
//						//for (int j = 0; j < this->Text[line].size(); ++j)
//						//{
//						//	sstring += this->Text[line][j];
//						//	sstring += " ";
//						//}
//					//}
//					continue;
//				}
//
//				if (i == VK_DELETE)
//				{
//					this->Text.clear();
//					sstring.clear();
//				}
//
//				if (i == VK_BACK)
//				{
//					if (sstring.length() > 0)
//					{
//						sstring.erase(sstring.end() - 1);
//						fasterase = clock();
//					}
//					else
//					{
//						if (this->Text.size() == line)
//						{
//							line--;
//							// add each word back to the string
//							for (int j = 0; j < this->Text[line].size(); ++j)
//							{
//								sstring += this->Text[line][j];
//								sstring += " ";
//							}
//
//							this->Text.erase(this->Text.end() - 1);
//						}
//						else
//						{
//							int oldline = line;
//							if (line > 1)
//							{
//								line--;
//								// add each word back to the string
//								for (int j = 0; j < this->Text[line].size(); ++j)
//								{
//									sstring += this->Text[line][j];
//									sstring += " ";
//								}
//								this->Text.erase(this->Text.begin() + oldline);
//							}
//						}
//					}
//					break;
//				}
//
//				if (i == VK_SPACE)
//				{
//					sstring += " ";
//					break;
//				}
//
//				if (GetAsyncKeyState(VK_SHIFT))
//				{
//					if (i == 0x39)
//					{
//						sstring += "(";
//						break;
//					}
//					if (i == 0x30)
//					{
//						sstring += ")";
//						break;
//					}
//
//					if (i == 0xBD)
//					{
//						sstring += "_";
//						break;
//					}
//
//					if (i == VK_OEM_PLUS)
//					{
//						sstring += "+";
//						break;
//					}
//				}
//				else
//				{
//					if (i == 0xBC)
//					{
//						sstring += ",";
//						break;
//					}
//
//					if (i == 0xBE)
//					{
//						sstring += ".";
//						break;
//					}
//
//					if (i == 0xBD)
//					{
//						sstring += "-";
//						break;
//					}
//
//					if (i == VK_OEM_PLUS)
//					{
//						sstring += "=";
//						break;
//					}
//
//					if (i == VK_OEM_2)
//					{
//						sstring += "/";
//						break;
//					}
//				}
//
//				if (KeyStringsTextBox[i] == nullptr)
//					continue;
//
//				auto textsize = Drawing::GetTextSize(Drawing::hFont, sstring.c_str());
//				if (textsize.right > (m_w - 10))
//					continue;
//
//				sstring += (GetAsyncKeyState(VK_SHIFT)) ? KeyStringsTextBox[i] : Drawing::LowerCase(KeyStringsTextBox[i]);
//			}
//		}
//		// helper
//		for (auto valids : valdinstructions)
//		{
//			if (strstr(Drawing::LowerCase(valids.sinstruction).c_str(), (Drawing::LowerCase(sstring).c_str())))
//				helper.push_back(valids.sinstruction + valids.desc);
//			else
//			{
//				if (Drawing::LowerCase(sstring).find(Drawing::LowerCase(valids.sinstruction)) != std::string::npos)
//					helper.push_back(valids.sinstruction + valids.desc);
//			}
//		}
//
//		if (GetAsyncKeyState(VK_BACK))
//		{
//			auto secondsPassed = (clock() - fasterase) / CLOCKS_PER_SEC;
//			auto erasenext = (clock() - delayerase) / CLOCKS_PER_SEC;
//			if (sstring.length() > 0 && secondsPassed > 0.5f && erasenext > 0.05f)
//			{
//				sstring.erase(sstring.end() - 1);
//				delayerase = clock();
//			}
//		}
//	}
//
//	// outline
//	Drawing::DrawRect(pos.x - 1, pos.y + 11, m_w + 2, m_h + 2, Outline);
//	// main inside
//	Drawing::GradientV(pos.x, pos.y + 12, m_w, m_h, Color(63, 63, 63), Color(33,33,33));
//
//	variables.clear();
//	variables.push_back({"players.getall", std::to_string(Interfaces.Engine->GetMaxClients()), "int"});
//
//	//Drawing::DrawStringFont(40, 40, Color::White(), 0, "%i", startloop);
//	//Drawing::DrawStringFont(40, 50, Color::White(), 0, "%i", endloop);
//	// each line
//	for (int i = 1; i < this->Text.size(); ++i)
//	{
//
//		// skip current line if editing
//		if (i == line)
//			continue;
//
//		Drawing::DrawRect(pos.x, pos.y + 12 + (i * 12), m_w, 12, Color(255, 255, 255, 20));
//
//		// analyze each word
//		bool addtoparams = false;
//		std::vector<std::string> param;
//		int start = 0;
//		int numbofparams = 0;
//		bool validline = false;
//		bool validvariable = false;
//		int badcol = 0;
//		for (int j = 0; j < this->Text[i].size(); ++j)
//		{
//			Color textcol = Text;
//			std::string soperator = "none";
//			int textsize = 0;
//			if (this->Text[i][j] == "(")
//			{
//				textcol = Color(30, 144, 255);
//				param.clear();
//				addtoparams = true;
//				validline = true;
//				start = j;
//			}
//			if (Drawing::LowerCase(this->Text[i][j]) == "loop_start")
//				inloop = true;
//
//			if (Drawing::LowerCase(this->Text[i][j]) == "loop_end")
//				inloop = false;
//
//
//			if (this->Text[i][j] == ")")
//			{
//				textcol = Color(30, 144, 255);
//				addtoparams = false;
//			}
//			soperator = this->Text[i][j];
//
//			// this is a valid variable
//			if (std::find(validvariables.begin(), validvariables.end(), this->Text[i][j]) != validvariables.end())
//			{
//				validline = true;
//				textcol = Color(30, 144, 255);
//				validvariable = true;
//			}
//			else
//			{
//				// this is a valid function
//				for (auto valids : valdinstructions)
//				{
//					if (Drawing::LowerCase(valids.sinstruction) == (Drawing::LowerCase(this->Text[i][j])))
//					{
//						if (inloop && !(Drawing::LowerCase(this->Text[i][j]) == "loop_end" || Drawing::LowerCase(this->Text[i][j]) == "loop_start"))
//						{
//							textcol = Color(255, 153, 255);
//						}
//						else
//							textcol = Color::Green();
//
//
//						numbofparams = valids.params;
//						validline = true;
//					}
//				}
//			}
//
//			if (Drawing::LowerCase(this->Text[i][j]) == "loop_end" || Drawing::LowerCase(this->Text[i][j]) == "loop_start")
//				textcol = Color(255, 51, 255);
//
//
//
//			if (Drawing::LowerCase(this->Text[i][j]).find("color") != std::string::npos)
//			{
//				std::vector<std::string> colors;
//				std::stringstream f(this->Text[i][j].c_str());
//				std::string s;
//				while (getline(f, s, '_')) {
//					colors.push_back(s);
//				}
//				if (colors.size() > 3)
//					textcol = Color(std::stoi(colors[1]), std::stoi(colors[2]), std::stoi(colors[3]));
//				else
//					badcol = i;
//			}
//
//			bool canadd = true; 
//
//			if (soperator == "=")
//			{
//				// check if variable already taken
//				std::string value = this->Text[i][j + 1];
//				for (auto variable : variables)
//				{
//					if (variable.name == this->Text[i][j - 1])
//					{
//						canadd = false;
//						break;
//					}
//
//					if (variable.name == value)
//						value = variable.value;
//				}
//				// name, value, type
//				if (canadd)
//					variables.push_back({ this->Text[i][j - 1], value, this->Text[i][j - 2] });
//			}
//
//			// first word is a variable
//			for (auto &variable : variables)
//			{
//				if (variable.name == this->Text[i][j])
//					textcol = Color(30, 144, 255);
//
//				if (variable.name == this->Text[i][j] && j == 0)
//				{
//					validline = true;
//					std::string value = this->Text[i][2];
//					// check if assigned a value
//					for (auto variablez : variables)
//						if (variablez.name == value) {
//							value = variable.value; break;
//						}
//
//					if (this->Text[i][1] == "+=")
//					{
//						variable.value = std::to_string(std::stoi(variable.value) + std::stoi(value));
//					}
//					else if (this->Text[i][1] == "/=")
//					{
//						variable.value = std::to_string(std::stoi(variable.value) / std::stoi(value));
//					}
//					else if (this->Text[i][1] == "-=")
//					{
//						variable.value = std::to_string(std::stoi(variable.value) - std::stoi(value));
//					}
//					else if (this->Text[i][1] == "*=")
//					{
//						variable.value = std::to_string(std::stoi(variable.value) * std::stoi(value));
//					}
//
//					break;
//				}
//			}
//
//			if (addtoparams)
//			{
//				std::string parameter = this->Text[i][j];
//				if (j > start)
//				{
//					std::stringstream f(parameter);
//					std::string s;
//					while (getline(f, s, ',')) {
//						param.push_back(s);
//					}
//				}
//			}
//
//			for (int p = 0; p < j; ++p)
//				textsize += Drawing::GetTextSize(Drawing::hFont, this->Text[i][p].c_str()).right + 5;
//
//			Drawing::DrawStringFont(pos.x + 3 + (textsize), pos.y + 12 + (i * 12), textcol, 0, "%s", this->Text[i][j].c_str());
//			if (param.size() != numbofparams && this->Text[i][j] == this->Text[i].back())
//				Drawing::DrawStringFont(pos.x + 25 + (textsize), pos.y + 12 + (i * 12), Color::Red(), 0, "  [error] takes %i parameter(s) not %i", numbofparams, param.size());
//			if (!validline && this->Text[i][j] == this->Text[i].back())
//				Drawing::DrawStringFont(pos.x + 25 + (textsize), pos.y + 12 + (i * 12), Color::Red(), 0, "  [error] what is this?");
//			if (!canadd)
//				Drawing::DrawStringFont(pos.x + 25 + (textsize), pos.y + 12 + (i * 12), Color::Red(), 0, "  [error] variable name already taken");
//			if (badcol && this->Text[i][j] == this->Text[i].back())
//				Drawing::DrawStringFont(pos.x + 25 + (textsize), pos.y + 12 + (i * 12), Color::Red(), 0, " [error] too few arguement for color");
//
//
//		}
//		//for (auto params : param)
//		//	printf("%s\n", params.c_str());
//	}
//
//
//	//for (int i = 0; i < variables.size(); ++i)
//	//	Drawing::DrawStringFont(30, 30 + (i * 13), Color::White(), 0, "%s -> %s -> %s", variables[i].name.c_str(), variables[i].value.c_str(), variables[i].type.c_str());
//
//	Drawing::DrawStringFont(pos.x + 3, pos.y + 12 + (12 * line), Text, 0, "%s_", sstring.c_str());
//
//	if (IsTyping && sstring.size())
//	{
//		int longest = 20;
//		if (helper.size())
//		{
//			RECT textsize;
//			auto writesize = Drawing::GetTextSize(Drawing::hFont, sstring.c_str());
//
//			for (int i = 0; i < helper.size(); ++i)
//			{
//				textsize = Drawing::GetTextSize(Drawing::hFont, helper[i].c_str());
//				if (textsize.right > longest)
//					longest = textsize.right;
//			}
//			Drawing::DrawRect(pos.x - 4 + writesize.right, pos.y + (this->Text.size() * 13) + 14 + writesize.bottom, longest + 7, (helper.size() * 14), Color(25, 25, 25));
//			Drawing::GradientV(pos.x - 3 + writesize.right, pos.y + (this->Text.size() * 13) + 15 + writesize.bottom, longest + 5, (helper.size() * 13), Color(63, 63, 63), Color(25, 25, 25));
//			for (int i = 0; i < helper.size(); ++i)
//			{
//				textsize = Drawing::GetTextSize(Drawing::hFont, helper[i].c_str());
//				if (textsize.right > longest)
//					longest = textsize.right;
//
//				Drawing::DrawStringFont(pos.x + writesize.right, pos.y + (this->Text.size() * 13) + 15 + writesize.bottom + (i * 13), Text, 0, "%s", helper[i].c_str());
//			}
//		}
//	}
//	DoInstructions();
//}
//
//void Editor::OnUpdate()
//{
//}
//
//void Editor::OnClick()
//{
//	if (IsTyping)
//	{
//		if (!cheat::features::menu.Mousein(m_x, m_y, m_w, m_h))
//		{
//			FinishedFocus = true;
//			IsTyping = false;
//		}
//	}
//	else
//		IsTyping = true;
//}
//
//void Editor::DoInstructions()
//{
//	static bool startloop = false;
//	for (int i = 0; i < instructions.size(); ++i)
//	{
//		auto instruction = instructions[i].sinstruction;
//		auto params = instructions[i].param;
//		for (auto variable : variables)
//		{
//			for (auto &param : params)
//			{
//				if (variable.name == param)
//				{
//					param = variable.value; 
//				}
//			}
//		}
//		//if (Drawing::LowerCase(instruction).find("loop_start") != std::string::npos)
//		//{
//		//	startloop = true;
//		//	continue;
//		//}
//		//if (Drawing::LowerCase(instruction).find("loop_end") != std::string::npos)
//		//{
//		//	startloop = false;
//		//	continue;
//		//}
//
//
//		//if (startloop)
//		//{
//		//	auto pLocal = Interfaces.EntList->GetClientEntity<CPlayer>(Interfaces.Engine->GetLocalPlayer());
//
//		//	for (int j = 0; j < Interfaces.EntList->GetHighestEntityIndex(); ++j)
//		//	{
//
//		//		auto player = Interfaces.EntList->GetClientEntity<CPlayer>(j);
//		//		if (!player->IsValid(pLocal))
//		//			continue;
//
//		//		player_info_s pinfo;
//		//		if (!Interfaces.Engine->GetPlayerInfo(player->GetIndex(), &pinfo))
//		//			continue;
//
//		//		int xx;
//		//		int yy;
//		//		int ww;
//		//		int hh;
//		//		DWORD m_rgflCoordinateFrame = (DWORD)0x354 - 0x4C;//int 
//		//		const matrix3x4& trans = *(matrix3x4*)((DWORD)player + (DWORD)m_rgflCoordinateFrame);
//
//		//		Vector3 min = *reinterpret_cast<Vector3*>((DWORD)player + (DWORD)0x19C + 0x20);//m_angEyeAngles[0] //0x239C)
//		//		Vector3 max = *reinterpret_cast<Vector3*>((DWORD)player + (DWORD)0x19C + 0x2C);
//
//		//		Vector3 pointList[] = {
//		//			Vector3(min.x, min.y, min.z),
//		//			Vector3(min.x, max.y, min.z),
//		//			Vector3(max.x, max.y, min.z),
//		//			Vector3(max.x, min.y, min.z),
//		//			Vector3(max.x, max.y, max.z),
//		//			Vector3(min.x, max.y, max.z),
//		//			Vector3(min.x, min.y, max.z),
//		//			Vector3(max.x, min.y, max.z)
//		//		};
//
//		//		Vector3 transformed[8];
//
//		//		for (int i = 0; i < 8; i++)
//		//			Hack.Misc->VectorTransform(pointList[i], trans, transformed[i]);
//
//		//		Vector3 flb, brt, blb, frt, frb, brb, blt, flt;
//
//		//		if (!Hack.Misc->WorldToScreen(transformed[3], flb) ||
//		//			!Hack.Misc->WorldToScreen(transformed[0], blb) ||
//		//			!Hack.Misc->WorldToScreen(transformed[2], frb) ||
//		//			!Hack.Misc->WorldToScreen(transformed[6], blt) ||
//		//			!Hack.Misc->WorldToScreen(transformed[5], brt) ||
//		//			!Hack.Misc->WorldToScreen(transformed[4], frt) ||
//		//			!Hack.Misc->WorldToScreen(transformed[1], brb) ||
//		//			!Hack.Misc->WorldToScreen(transformed[7], flt))
//		//			continue;
//
//		//		Vector3 arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };
//
//		//		float left = flb.x;
//		//		float top = flb.y;
//		//		float right = flb.x;
//		//		float bottom = flb.y;
//
//		//		for (int i = 0; i < 8; i++) {
//		//			if (left > arr[i].x)
//		//				left = arr[i].x;
//		//			if (top < arr[i].y)
//		//				top = arr[i].y;
//		//			if (right < arr[i].x)
//		//				right = arr[i].x;
//		//			if (bottom > arr[i].y)
//		//				bottom = arr[i].y;
//		//		}
//
//		//		xx = left;
//		//		yy = bottom;
//		//		ww = right - left;
//		//		hh = top - bottom;
//
//		//		// get whatever they want
//		//		for (int p = 0; p < params.size(); ++p)
//		//		{
//		//			printf("%s\n", params[p].c_str());
//
//		//			if (params[p].find("player)") != std::string::npos)
//		//			{
//		//				printf("%s\n", params[p].c_str());
//		//				params[p] = std::string(pinfo.name);
//		//			}
//		//			//else if (strstr(Drawing::LowerCase(params[p]).c_str(), "player.weapon"))
//		//			//	params[p] = player->GetWeap()->GetWeaponName();
//		//			//else if (strstr(Drawing::LowerCase(params[p]).c_str(), "player.health"))
//		//			//	params[p] = std::to_string(player->GetHealth());
//		//			//else if (strstr(Drawing::LowerCase(params[p]).c_str(), "player.x"))
//		//			//	params[p] = std::to_string(xx);
//		//			//else if (strstr(Drawing::LowerCase(params[p]).c_str(), "player.y"))
//		//			//	params[p] = std::to_string(yy);
//		//			//else if (strstr(Drawing::LowerCase(params[p]).c_str(), "player.w"))
//		//			//	params[p] = std::to_string(ww);
//		//			//else if (strstr(Drawing::LowerCase(params[p]).c_str(), "player.h"))
//		//			//	params[p] = std::to_string(hh);
//		//		}
//
//
//		//		if (Drawing::LowerCase(instruction).find("drawline") != std::string::npos)
//		//		{
//		//			Drawing::DrawLine(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]), Color().FromHSB(std::stof(params[4]) / 100));
//		//		}
//		//		else if (Drawing::LowerCase(instruction).find("drawbox") != std::string::npos)
//		//		{
//		//			Drawing::DrawRect(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]), Color::White());
//		//		}
//		//		else if (Drawing::LowerCase(instruction).find("drawoutlinebox") != std::string::npos)
//		//		{
//		//			Drawing::DrawOutlineRect(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]), Color().FromHSB(std::stof(params[4]) / 100));
//		//		}
//		//		else if (Drawing::LowerCase(instruction).find("drawtext") != std::string::npos)
//		//		{
//		//			Drawing::DrawStringFont(std::stoi(params[0]), std::stoi(params[1]), Color().FromHSB(std::stof(params[3]) / 100), 0, params[2].c_str());
//		//		}
//		//		else if (Drawing::LowerCase(instruction).find("gradientv") != std::string::npos)
//		//		{
//		//			Drawing::GradientV(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]), Color().FromHSB(std::stof(params[4]) / 100)
//		//				, Color().FromHSB(std::stof(params[5]) / 100));
//		//		}
//		//	}
//		//}
//		//else
//		{
//			if (Drawing::LowerCase(instruction).find("drawline") != std::string::npos)
//			{
//				Drawing::DrawLine(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]), Color(std::stoi(params[4]), std::stoi(params[5]), std::stoi(params[6])));
//			}
//			else if (Drawing::LowerCase(instruction).find("drawbox") != std::string::npos)
//			{
//				Drawing::DrawRect(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]), Color(std::stoi(params[4]), std::stoi(params[5]), std::stoi(params[6])));
//			}
//			else if (Drawing::LowerCase(instruction).find("drawoutlinebox") != std::string::npos)
//			{
//				Drawing::DrawOutlineRect(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]), Color(std::stoi(params[4]), std::stoi(params[5]), std::stoi(params[6])));
//			}
//			else if (Drawing::LowerCase(instruction).find("drawtext") != std::string::npos)
//			{
//				ReplaceStringInPlace(params[2], "_", " ");
//				Drawing::DrawStringFont(std::stoi(params[0]), std::stoi(params[1]), Color(std::stoi(params[3]), std::stoi(params[4]), std::stoi(params[5])), 0, params[2].c_str());
//			}
//			else if (Drawing::LowerCase(instruction).find("execute") != std::string::npos)
//			{
//				ReplaceStringInPlace(params[1], "_", " ");
//				char buffer[60];
//				sprintf_s(buffer, "%s \"%s\"", params[0].c_str(), params[1].c_str());
//				Interfaces.Engine->ClientCmd_Unrestricted(buffer);
//			}
//			else if (Drawing::LowerCase(instruction).find("drawgradientv") != std::string::npos)
//			{
//				Drawing::GradientV(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]), Color(std::stoi(params[4]), std::stoi(params[5]), std::stoi(params[6])),
//					Color(std::stoi(params[7]), std::stoi(params[8]), std::stoi(params[9])));
//			}
//		}
//	}
//}


CListBox::CListBox()
{
	m_Flags = UI_Drawable | UI_Focusable;
	ControlType = UIC_ListBox;
	m_w = 0;
	m_h = 30;
}

void CListBox::Draw(bool hover)
{
	auto pos = GetPos();
	int Itemheight = 12;
	int ItemsToDraw = m_h / Itemheight;
	int drawnItems = 0;
	Color outline(43, 44, 46);

	if (hover)
		outline = Color(53, 54, 56);

	// Outline
	Drawing::DrawRect(pos.x - 1, pos.y - 1, m_w + 2, m_h + 2, outline);
	// base
	Drawing::DrawRectGradientVertical(pos.x, pos.y, m_w, m_h, Color(43, 44, 46, window_alpha), Color(37, 38, 38, window_alpha));
	// Scroll area
	Drawing::DrawRect(pos.x + m_w - 8, pos.y, 8, m_h, Color(80, 80, 80, window_alpha));
	// Scroll Bar

	auto unclamped_size = (m_h - (Itemheight * int(Items.size() - ItemsToDraw)));

	auto scroll_size = Math::clamp(unclamped_size, 18, m_h);

	Drawing::DrawRect(pos.x + m_w - 8, pos.y + SliderPos, 8, scroll_size, Color(110, 110, 110, window_alpha));
	//Drawing::DrawIcon(Vector2(pos.x + m_w - 11, pos.y + SliderPos + 3), upNdownarrow, Drawing::Checkmark);

	selectedindex = Math::clamp(selectedindex, 0, (int)(Items.size() - 1));
	// no clipping for u reis sorry babe :(
	if (Items.size() > 0)
	{
		for (int i = ScrollTop; i < Items.size() && drawnItems < ItemsToDraw; i++)
		{
			// Color of current Item
			Color itemcolor(Color(190, 190, 190, window_alpha));

			if (Items[i].clr != Color::White())
				itemcolor = Items[i].clr;

			// Distance between each item is 12
			if (cheat::features::menu.Mousein(Vector(pos.x, pos.y + drawnItems * Itemheight,0), Vector(m_w - 13, 12,0)) && windows.at(windows.size() - 1) == parent)
			{
				// Hovering over the item
				itemcolor = Color::White();

				// Selected Item
				if (cheat::features::menu.mouse1_pressed)
					selectedindex = i;
			}

			// Color the selected item;
			if (selectedindex == i)
			{
				itemcolor = cheat::Cvars.MenuTheme.GetColor();
				// hover selected
				Drawing::DrawRect(pos.x + 1, pos.y + drawnItems * Itemheight, m_w - 10, 12, Color(255, 255, 255, min(window_alpha,20)));
			}

			// Draw The Item
			Drawing::DrawStringFont(Drawing::hFont, pos.x + 3, pos.y + drawnItems * Itemheight, itemcolor, 0, Items[i].name.c_str());
			drawnItems++;
		}

		// scroll
		if (cheat::features::menu.Mousein(Vector(pos.x, pos.y,0), Vector(m_w, m_h,0)) && windows.at(windows.size() - 1) == parent)
		{
			int scroll = GetScroll();
			if (scroll != 0)
			{
				if (scroll > 0)
				{
					if (selectedindex != 0)
					{
						if (selectedindex == ScrollTop)
						{
							ScrollTop -= 1;
						}
						selectedindex -= 1;
					}
				}
				else
				{
					if (selectedindex != Items.size() - 1)
					{
						if (selectedindex == (ScrollTop + drawnItems) - 1)
						{
							ScrollTop += 1;
						}
						selectedindex += 1;
					}
				}
			}
		}
	}

	if (DoDrag)
	{
		if (cheat::game::pressed_keys[1])
		{
			SliderPos = cheat::features::menu._cursor_position.y - pos.y;
			if (pos.y + SliderPos < pos.y)
				SliderPos = 0;
			if (pos.y + SliderPos > pos.y + m_h - scroll_size)
				SliderPos = m_h - scroll_size;

			const float mPosRatio = float(SliderPos) / float(m_h - scroll_size);
			ScrollTop = mPosRatio * Items.size() - ItemsToDraw;
			if (ScrollTop < 0)
				ScrollTop = 0;
		}
		else
		{
			DoDrag = false;
			FinishedFocus = true;
		}
	}
	else
	{
		FinishedFocus = true;
	}
}

void CListBox::OnUpdate()
{
}

void CListBox::OnClick()
{
	auto pos = GetPos();

	auto scroll_size = Math::clamp((m_h - (12 * (Items.size() - (m_h / 12)))), 12, m_h);

	if (cheat::features::menu.Mousein(Vector(pos.x + m_w - 12, pos.y + SliderPos,0), Vector(12, scroll_size,0)))
	{
		DoDrag = true;
	}
	else
	{
		FinishedFocus = true;
	}
}


CColorPicker::CColorPicker()
{
	m_Flags = UI_Drawable | UI_Focusable;
	ControlType = UIC_Colorpicker;
	m_w = 25;
	m_h = 12;
	m_SaveName = std::string("Color" + std::to_string(AllControls.size()));
	AllControls.push_back({ m_SaveName, this });
}

void rgb_to_hsv(float r, float g, float b, float& out_h, float& out_s, float& out_v)
{
	float K = 0.f;
	if (g < b)
	{
		const auto temp = b;
		b = g;
		g = temp;
		K = -1.f;
	}
	if (r < g)
	{
		const auto temp = r;
		r = g;
		g = temp;
		K = -2.f / 6.f - K;
	}

	const float chroma = r - (g < b ? g : b);
	out_h = fabsf(K + (g - b) / (6.f * chroma + 1e-20f));
	out_s = chroma / (r + 1e-20f);
	out_v = r;
}

// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
// also http://en.wikipedia.org/wiki/HSL_and_HSV
void hsv_to_rgb(float h, float s, float v, float& out_r, float& out_g, float& out_b)
{
	if (s == 0.0f)
	{
		// gray
		out_r = out_g = out_b = v;
		return;
	}

	h = fmodf(h, 1.0f) / (60.0f / 360.0f);
	int   i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));

	switch (i)
	{
	case 0: out_r = v; out_g = t; out_b = p; break;
	case 1: out_r = q; out_g = v; out_b = p; break;
	case 2: out_r = p; out_g = v; out_b = t; break;
	case 3: out_r = p; out_g = q; out_b = v; break;
	case 4: out_r = t; out_g = p; out_b = v; break;
	case 5: default: out_r = v; out_g = p; out_b = q; break;
	}
}

void CColorPicker::SetColor(Color col)
{
	color = col;
	rgb_to_hsv(float(color.r()) / 255.f, float(color.g()) / 255.f, float(color.b()) / 255.f, m_hue, m_saturation, m_value);
}

void CColorPicker::Draw(bool hover)
{
	auto pos = GetPos();
	Color Text(190, 190, 190, window_alpha);
	Color Outline(Color(43, 44, 46, window_alpha));

	if (hover || m_bIsOpen)
		Outline = Color(53, 54, 56, window_alpha);

	// outline
	Drawing::DrawRect(pos.x - 1, pos.y + 11, m_w + 2, m_h + 2, Outline);
	// main inside
	Drawing::DrawRectGradientVertical(pos.x, pos.y + 12, m_w, m_h, Color(43, 44, 46, window_alpha), Color(37, 38, 38, window_alpha));
	// Label
	Drawing::DrawStringFont(Drawing::SegoeUI, pos.x, pos.y - 5, color, 0, label.c_str());
	// Current Color
	Drawing::DrawRectGradientVertical(pos.x + 2, pos.y + 14, m_w - 4, m_h - 4, color, Color(43, 44, 46, window_alpha));

	// Draw color picker
	if (m_bIsOpen)
	{
		color = Color().FromHSB(m_hue, m_saturation, m_value).alpha(window_alpha);

		const int t_height = m_w - 4;
		// outline
		Drawing::DrawRect(pos.x - 1, pos.y + 12 + m_h, m_w + 2, t_height + 2, Outline);
		// main inside
		Drawing::DrawRect(pos.x, pos.y + 13 + m_h, m_w, t_height, Color(63, 63, 63, window_alpha));

		const float colpalwidth = m_w - 4;
		const float colpalheight = m_h;

		// hue pallet outline
		Drawing::DrawRect(pos.x + 1, pos.y + 14 + m_h, m_w - 2, colpalheight + 2, Outline);
		// hue pallet
		for (int i = 0; i < colpalwidth; ++i) {
			Drawing::DrawRect(pos.x + i + 2, pos.y + 15 + m_h, 1, colpalheight, Color().FromHSB(i / colpalwidth, 1.f, 1.f).alpha(window_alpha));
		}

		// shade pallet outline
		Drawing::DrawRect(pos.x + 1, pos.y + 14 + m_h + colpalheight + 3, m_w - 2, t_height - colpalheight - 5, Outline);
		// shade pallet
		const auto shade_height = (t_height - colpalheight - 7);
		const int box_size = 8;
		for (int c = 0; c < colpalwidth; c += box_size) {
			for (int r = 0; r < shade_height; r += box_size) {
				const auto cpos = Vector2D(c, r);
				const auto saturation_percent = Math::clamp(cpos.x / colpalwidth, 0.f, 1.f);
				const auto brightness_percent = Math::clamp(1.f - (cpos.y / shade_height), 0.f, 1.f);
				auto size_x = box_size;
				auto size_y = box_size;
				// this part is used to clamp the box height to the size of the frame. 
				// this is needed because sometimes the boxes don't divide the side of the frame evenly and you'll
				// have overhang without this code.
				{
					if (pos.x + c + 2 + size_x > pos.x + m_w - 3)
						size_x = (pos.x + m_w - 2) - (pos.x + c + 2);
					if (pos.y + 14 + m_h + colpalheight + 4 + r + size_y > pos.y + 14 + m_h + colpalheight + 4 + shade_height)
						size_y = (pos.y + 14 + m_h + colpalheight + 4 + shade_height) - (pos.y + 14 + m_h + colpalheight + 4 + r);
				}
				float hue, saturation, value;
				rgb_to_hsv(float(color.r()) / 255.f, float(color.g()) / 255.f, float(color.b()) / 255.f, hue, saturation, value);
				const auto pixel_color = Color().FromHSB(hue, saturation_percent, brightness_percent);
				Drawing::DrawRect(pos.x + c + 2, pos.y + 14 + m_h + colpalheight + 4 + r, size_x, size_y, pixel_color);
			}
		}

		if (DoDragHue)
		{
			if (cheat::game::pressed_keys[1]) {
				if (cheat::features::menu.Mousein(Vector(pos.x + 2, pos.y + 14 + m_h,0), Vector(colpalwidth, shade_height,0)))
				{
					m_hue_pos.x = cheat::features::menu._cursor_position.x - (pos.x + 2);
					m_hue_pos.y = cheat::features::menu._cursor_position.y - (pos.y + 14 + m_h);
					m_hue = (cheat::features::menu._cursor_position.x - pos.x + 2) / (colpalwidth);
				}
			}
			else
			{
				DoDragHue = false;
			}
		}
		if (DoDragCol)
		{
			if (cheat::game::pressed_keys[1]) {
				if (cheat::features::menu.Mousein(Vector(pos.x + 2, pos.y + 14 + m_h + colpalheight + 4,0), Vector(colpalwidth, shade_height,0))) {
					m_saturation = Math::clamp((cheat::features::menu._cursor_position.x - (pos.x + 2)) / colpalwidth, 0.f, 1.f);
					m_value = Math::clamp(1.f - (cheat::features::menu._cursor_position.y - (pos.y + 14 + m_h + colpalheight + 4)) / shade_height, 0.f, 1.f);
					ColorPos.y = cheat::features::menu._cursor_position.y - pos.y;
					ColorPos.x = cheat::features::menu._cursor_position.x - pos.x;
				}
			}
			else
			{
				DoDragCol = false;
			}
		}

		color = Color().FromHSB(m_hue, m_saturation, m_value);

		// Draw rect where colorpicked
		if (ColorPos.x && ColorPos.y)
		{
			ColorPos.x = Math::clamp(ColorPos.x, 0.f, colpalwidth - 5);
			ColorPos.y = Math::clamp(ColorPos.y, 0.f, shade_height + 35);
			// Draw Outline
			//Drawing::DrawRect(ColorPos.x - 1 + pos.x, ColorPos.y - 1 + pos.y, 7, 7, Color::White());
			// Draw color
			Drawing::DrawOutlinedRect(ColorPos.x + pos.x, ColorPos.y + pos.y, 5, 5, Color(43, 44, 46, window_alpha));
		}
		// Draw rect where hue picked
		if (m_hue_pos.x && m_hue_pos.y)
		{
			m_hue_pos.x = Math::clamp(m_hue_pos.x, 2.f, colpalwidth - 2);
			// Draw Outline
			Drawing::DrawOutlinedRect(m_hue_pos.x - 2 + (pos.x + 2), pos.y + 15 + m_h, 4, colpalheight, Color(43, 44, 46, window_alpha));
			// Draw color
			//Drawing::DrawOutlineRect(m_hue_pos.x - 2 + (pos.x + 2) + 1, pos.y + 15 + m_h + 1, 2, colpalheight - 2, Color::Black());
		}
	}
}

void CColorPicker::OnUpdate()
{
}

void CColorPicker::OnClick()
{
	auto pos = GetPos();
	const float colpalwidth = m_w - 4;
	const float colpalheight = m_h;
	const int t_height = m_w - 4;
	const auto shade_height = (t_height - colpalheight - 7);

	if (m_bIsOpen)
	{
		if (!DoDragCol || !DoDragHue)
		{
			if (cheat::features::menu.Mousein(Vector(pos.x + 2, pos.y + 14 + m_h,0), Vector(colpalwidth, colpalheight,0)))
			{
				DoDragCol = false;
				DoDragHue = true;
			}

			if (cheat::features::menu.Mousein(Vector(pos.x + 2, pos.y + 14 + m_h + colpalheight + 4, 0), Vector(colpalwidth, shade_height, 0)))
			{
				DoDragCol = true;
				DoDragHue = false;
			}
		}
		if (!cheat::features::menu.Mousein(Vector(pos.x, pos.y,0), Vector(m_w, m_h + m_h + shade_height + 4,0)))
		{
			FinishedFocus = true;
			m_bIsOpen = false;
			DoDragHue = false;
			DoDragCol = false;
		}
	}
	else
	{
		m_bIsOpen = true;
		//m_h = 80;
	}
}

Vector2D CColorPicker::GetClickArea()
{
	return Vector2D(m_w, m_h + 14);
}

Color CColorPicker::GetColorFromPenPosition(int x, int y, int width, int height)
{
	auto pos = GetPos();
	static Color returncolor;
	if (cheat::features::menu.Mousein(Vector(x, y,0), Vector(width, height,0)))
	{
		ColorPos.y = cheat::features::menu._cursor_position.y - pos.y;
		ColorPos.x = cheat::features::menu._cursor_position.x - pos.x;
		float h = (cheat::features::menu._cursor_position.x - x) * (1.0f / width);
		float s = 1.0f - (cheat::features::menu._cursor_position.y - y) * (1.0f / height);
		returncolor = Color().FromHSB(h, s, 1.0f);
	}

	return returncolor;
}

CButton::CButton()
{
	m_Flags = UI_Drawable | UI_Clickable;
	ControlType = UIC_Button;
	m_w = 25;
	m_h = 12;
}

void CButton::Draw(bool hover)
{
	auto pos = GetPos();
	Color Text(190, 190, 190, window_alpha);
	Color Outline(43, 44, 46, window_alpha);

	Drawing::DrawRectGradientVertical(pos.x, pos.y, m_w, m_h, Color(43, 44, 46, window_alpha), Color(37, 38, 38, window_alpha));

	if (hover)
	{
		if (cheat::features::menu.mouse1_pressed)
		{
			if (clicked)
				Drawing::DrawRectGradientVertical(pos.x, pos.y, m_w, m_h, Color(37, 38, 38, window_alpha), Color(43, 44, 46, window_alpha));
		}
		else
		{
			clicked = false;
		}
		Outline = Color(53, 54, 56, window_alpha);
	}
	// outline
	Drawing::DrawOutlinedRect(pos.x - 1, pos.y - 1, m_w + 2, m_h + 2, Outline);
	// main inside

	// Label
	Drawing::DrawStringFont(Drawing::hFont, pos.x + (m_w / 2), pos.y + m_h - Drawing::GetTextSize(Drawing::hFont, label.c_str()).bottom, Text, 1, label.c_str());
}

void CButton::OnUpdate()
{
}

void CButton::OnClick()
{
	if (func)
		reinterpret_cast<void(*)()>(func)();

	clicked = true;
}

Vector2D CButton::GetClickArea()
{
	if (parentgroup)
		return Vector2D(parentgroup->GetSize().x - 10, m_h);
	else
		return Vector2D(m_w, m_h);
}

CKeyBind::CKeyBind()
{
	m_Flags = UI_Drawable | UI_Focusable;
	ControlType = UIC_KeyBind;
	m_w = 25;
	m_h = 60;
	m_SaveName = std::string("KeyBind" + std::to_string(AllControls.size()));
	AllControls.push_back({ m_SaveName, this });
}

bool CKeyBind::Holding()
{
	if (key > 0)
	{
		return cheat::game::pressed_keys[key];
	}
	else
	{
		return false;
	}
}

bool CKeyBind::Toggle()
{
	if (key > 0)
	{
		if (cheat::game::pressed_keys[key])
			m_bToggle = !m_bToggle;

		return m_bToggle;
	}
	else
	{
		return false;
	}
}

void CKeyBind::Draw(bool hover)
{
	auto pos = GetPos();
	Color Text(190, 190, 190, window_alpha);
	Color Outline(43, 44, 46, window_alpha);

	if (hover || m_bIsOpen)
		Outline = Color(53, 54, 56, window_alpha);

	char* keysay;
	if (key > 0)
		keysay = KeyStrings[key];
	else keysay = "";

	if (m_bIsOpen)
	{
		keysay = "Press a Key...";
		for (int i = 0; i < 255; i++)
		{
			if (cheat::game::get_key_press(i))
			{
				if (i == VK_BACK || i == VK_ESCAPE)
				{
					key = 0;
					m_bIsOpen = false;
					FinishedFocus = true;
					cheat::game::pressed_keys[VK_ESCAPE] = false;
					cheat::game::pressed_keys[VK_BACK] = false;
					break;
				}

				if (i == VK_LBUTTON)
					continue;

				key = i;
				m_bIsOpen = false;
				FinishedFocus = true;
			}
		}
	}

	// outline
	Drawing::DrawRect(pos.x + (GetClickArea().x / 2) - 1, pos.y - 1, m_w + 2, m_h + 2, Outline);
	// main inside
	Drawing::DrawRectGradientVertical(pos.x + (GetClickArea().x / 2), pos.y, m_w, m_h, Color(43, 44, 46, window_alpha), Color(37, 38, 38, window_alpha));
	//Drawing::DrawRoundedGradientV(pos.x + (GetClickArea().x / 2), pos.y, m_w, m_h, 1.5f, Color(43, 44, 46), Color(37, 38, 38));
	// Label
	Drawing::DrawStringFont(Drawing::SegoeUI, pos.x, pos.y - 3, Text, 0, label.c_str());
	// Current key
	Drawing::DrawStringFont(Drawing::hFont, pos.x + (GetClickArea().x / 2) + (m_w / 2), pos.y + 1, Text, 1, "%s", keysay);
}

void CKeyBind::OnUpdate()
{
}

void CKeyBind::OnClick()
{
	auto pos = GetPos();

	if (m_bIsOpen)
	{
		// make sure the user wants to change the key to mouse1 and is not just pressing away...
		if (cheat::features::menu.Mousein(Vector(pos.x + (GetClickArea().x / 2), pos.y,0), Vector(m_w, m_h,0)))
			key = VK_LBUTTON;

		FinishedFocus = true;
		m_bIsOpen = false;
	}
	else
	{
		m_bIsOpen = true;
	}
}

Vector2D CKeyBind::GetClickArea()
{
	if (parentgroup)
		return Vector2D(parentgroup->GetSize().x - 10, m_h);
	else
		return Vector2D(m_w, m_h);
}


//void CSubTabs::DrawTabs()
//{
//	auto pos = GetPos();
//
//	// Tabs
//	Drawing::DrawRect(pos.x + 4, pos.y + 4, m_w - 8, 27, Color::Black());
//	Drawing::DrawRect(pos.x + 5, pos.y + 5, m_w - 10, 25, Color(43, 43, 43));
//	int TabSize = (m_w - 10) / subtabs.size();
//
//	for (int i = 0; i < subtabs.size(); ++i)
//	{
//		Color text(255, 255, 255, 60);
//		if (SelectedSubTab == i)
//		{
//			text = Color::White();
//			Drawing::DrawRect(pos.x + 5 + (i*TabSize), pos.y + 5, TabSize, 1, Color(60, 120, 216));
//		}
//
//		Drawing::DrawStringFont(pos.x + 5 + (i*TabSize + 1) + TabSize / 2.f, pos.y + 10, text, 1, subtabs[i].c_str());
//		if (i)
//			Drawing::DrawRect(pos.x + 5 + (i*TabSize), pos.y + 5, 1, 25, Color::Black());
//	}
//}


CRadar::CRadar()
{
	m_Flags = UI_Drawable;
	m_w = 100;
	m_h = 100;
}

void CRadar::Draw(bool hover)
{
	m_x = this->parent->m_x + 10;
	m_y = this->parent->m_y + 23;
	m_w = this->parent->m_w - 30;
	m_h = this->parent->m_h - 30;


	Vector angles; Source::m_pEngine->GetViewAngles(angles);
	// |
	Drawing::DrawRect(m_x + (m_w * 0.5f), m_y, 1, m_h - 8, Color(139, 139, 139, min(95, window_alpha)));
	// _
	Drawing::DrawRect(m_x, m_y + (m_h * 0.5f), m_w + 8, 1, Color(139, 139, 139, min(window_alpha, 95)));
	// draw all the entities
	/*if (EntList.size())
	{
		for (int i = 0; i < EntList.size(); ++i)
		{
			if (!EntList[i].Ent->IsValid())
				continue;

			if (EntList[i].Ent == cheat::main::local())
				continue;

			auto ent = EntList[i].Ent;
			auto col = EntList[i].Col;
			auto size = EntList[i].size;
			Vector2D pos = WorldToRadar(ent->GetOrigin(), plocal->GetOrigin(), angles, zoom);
			pos -= size / 2;
			Drawing::DrawRect(m_x + pos.x - 1, m_y + pos.y - 1, size + 2, size + 2, Color::Black());
			Drawing::DrawRect(m_x + pos.x, m_y + pos.y, size, size, col);
		}
	}*/
	// clear all the entities
	EntList.clear();
}
#define M_PI       3.14159265358979323846   // pi
// stole this sue me lol
Vector2D CRadar::WorldToRadar(const Vector location, const Vector origin, const Vector angles, float scale = 16.f)
{
	float x_diff = location.x - origin.x;
	float y_diff = location.y - origin.y;

	int iRadarRadius = m_w;
	int iRadarRadiush = m_h;

	float flOffset = atanf(y_diff / x_diff);
	flOffset *= 180;
	flOffset /= M_PI;

	if ((x_diff < 0) && (y_diff >= 0))
		flOffset = 180 + flOffset;
	else if ((x_diff < 0) && (y_diff < 0))
		flOffset = 180 + flOffset;
	else if ((x_diff >= 0) && (y_diff < 0))
		flOffset = 360 + flOffset;

	y_diff = -1 * (sqrtf((x_diff * x_diff) + (y_diff * y_diff)));
	x_diff = 0;

	flOffset = angles.y - flOffset;

	flOffset *= M_PI;
	flOffset /= 180;

	float xnew_diff = x_diff * cosf(flOffset) - y_diff * sinf(flOffset);
	float ynew_diff = x_diff * sinf(flOffset) + y_diff * cosf(flOffset);

	xnew_diff /= scale;
	ynew_diff /= scale;

	xnew_diff = (iRadarRadiush / 2) + (int)xnew_diff;
	ynew_diff = ((iRadarRadius) / 2) + (int)ynew_diff;

	if (xnew_diff > iRadarRadiush)
		xnew_diff = iRadarRadiush - 4;
	else if (xnew_diff < 4)
		xnew_diff = 4;

	if (ynew_diff > iRadarRadius)
		ynew_diff = iRadarRadius;
	else if (ynew_diff < 4)
		ynew_diff = 0;

	return Vector2D(xnew_diff, ynew_diff);
}

void CRadar::OnUpdate()
{
}

void CRadar::OnClick()
{
}

CBaseList::CBaseList()
{
	m_Flags = UI_Drawable;
}


void CBaseList::Draw(bool hover)
{
	int space = 0;
	int sizen = 150;

	m_x = this->parent->m_x;
	m_y = this->parent->m_y + this->parent->grabheight + 5;
	m_w = this->parent->m_w - 5;
	for (auto text : list)
	{
		auto sizep = Drawing::GetTextSize(Drawing::hFont, text.string.c_str());
		if (sizep.right > sizen)
			sizen = sizep.right;

		Drawing::DrawStringFont(Drawing::hFont, m_x + 5, m_y + (space * 14), text.color.alpha(window_alpha), 0, text.string.c_str());
		Drawing::DrawLine(m_x + 5, m_y + (space * 14) + 12, m_x + m_w, m_y + (space * 14) + 12, Color(139, 139, 139, min(window_alpha,95)));
		space++;
	}
	if (space)
	{
		m_h = (list.size() * 14) + 10;
		this->parent->SetSize(sizen + 10, 50 + (list.size() * 14));
	}
	list.clear();
}

void CBaseList::OnUpdate()
{
}

void CBaseList::OnClick()
{
}

CMultiComboBox::CMultiComboBox()
{
	m_Flags = UI_Drawable | UI_Focusable;
	ControlType = UIC_MultiComboBox;
	m_w = 25;
	m_h = 60;

	m_SaveName = std::string("MultiComboBox" + std::to_string(AllControls.size()));
	AllControls.push_back({ m_SaveName, this });

}

void CMultiComboBox::AddItems(std::vector<std::string> items)
{
	// Clear from previous frame
	Items.clear();
	Selected.clear();
	selected = 0;
	// Set current frame
	for (auto text : items) {
		Items.push_back(text);
		Selected.push_back(false);
	}
}

void CMultiComboBox::Draw(bool hover)
{
	selected = 0;
	auto pos = GetPos();
	Color Text(190, 190, 190, window_alpha);
	Color Outline(43, 44, 46, window_alpha);

	if (hover)
		Outline = Color(53, 54, 56, window_alpha);

	if (Items.size() > 0 && m_bIsOpen)
		m_h = 17 + 15 * Items.size();

	// outline
	Drawing::DrawRect(pos.x + (GetClickArea().x / 2) - 1, pos.y - 1, m_w + 2, m_h + 2, Outline);
	// main inside
	Drawing::DrawRectGradientVertical(pos.x + (GetClickArea().x / 2), pos.y, m_w, m_h, Color(43, 44, 46, window_alpha), Color(37, 38, 38, window_alpha));
	// Label
	Drawing::DrawStringFont(Drawing::SegoeUI, pos.x, pos.y - 2, Text, 0, label.c_str());
	// Current items
	std::string drawnitems = "";
	int i = 0;
	for (auto lol = 0; lol < Selected.size(); lol++)
	{
		if (!Selected[lol] || Selected.size() <= 0)
			continue;

		if (i)
			drawnitems += ", ";

		drawnitems += GetName(lol);

		selected++;

		i++;
	}
	bool dots = false;
	while (static_cast<int>(Drawing::GetTextSize(Drawing::hFont, drawnitems.c_str()).right) > m_w)
	{
		drawnitems.pop_back();
		dots = true;
	}

	if (drawnitems.size() > 3 && dots)
	{
		// fuck erase
		for (int i = 0; i < 3; ++i)
			drawnitems.pop_back();
		drawnitems += "...";
	}
	Drawing::DrawStringFont(Drawing::hFont, pos.x + (GetClickArea().x / 2) + 2, pos.y, Text, 0, drawnitems.c_str());

	// Draw the items
	if (Items.size() > 0 && m_bIsOpen)
	{
		for (int i = 0; i < Items.size(); i++)
		{
			Color Text = Color(180, 180, 180);
			if (Has(i))
				Text = cheat::Cvars.MenuTheme.GetColor();

			if (cheat::features::menu.Mousein(Vector(pos.x + (GetClickArea().x / 2), pos.y + 16 + i * 15,0), Vector(m_w, 15,0)))
				Drawing::DrawRect(pos.x + (GetClickArea().x / 2) + 1, pos.y + 16 + i * 15, m_w - 2, 14, Color(255, 255, 255, min(window_alpha,20)));

			Drawing::DrawStringFont(Drawing::hFont, pos.x + (GetClickArea().x / 2) + 3, pos.y + 16 + i * 15, Text, 0, Items[i].c_str());
		}
	}

	//if (!desc.empty())
	//{
	//	Drawing::DrawStringFont(cheat::features::menu._cursor_position.x, cheat::features::menu._cursor_position.y, Color(255, 255, 255, 20), 0, Drawing::SegoeUI, desc.c_str());
	//}
}

void CMultiComboBox::OnUpdate()
{
}

void CMultiComboBox::OnClick()
{
	if (m_bIsOpen)
	{
		auto pos = GetPos();
		bool shouldclose = true;
		for (int i = 0; i < Items.size(); i++)
		{
			if (cheat::features::menu.Mousein(Vector(pos.x + (GetClickArea().x / 2), pos.y + 16 + i * 15,0), Vector(m_w, 15,0)))
			{
				shouldclose = false;
				/*auto it = std::find(Selected.begin(), Selected.end(), i);

				if (Has(i))
					Selected.erase(it);
				else
					Selected.push_back(i);*/
				Selected[i] = !Selected[i];
			}
		}
		if (shouldclose)
		{
			m_bIsOpen = false;
			FinishedFocus = true;
			m_h = 12;
		}
	}
	else
	{
		m_bIsOpen = true;
	}
}

Vector2D CMultiComboBox::GetClickArea()
{
	if (parentgroup)
		return Vector2D(parentgroup->GetSize().x - 10, m_h);
	else
		return Vector2D(m_w, m_h);
}

CVars::CVars()
{
}

CVars::~CVars()
{

}

void CVars::Save()
{
	if (cheat::Cvars.Configs.GetName().empty())
		return;

	CSimpleIniA ini;
	ini.SetUnicode(true);
	auto cfgname = cheat::Cvars.Configs.GetName() + ".zeus";

	int i = 0;
	for (auto control : AllControls)
	{
		// get control type
		std::string controltype = "Unknown";
		switch (control.control->ControlType)
		{
		case UIC_CheckBox:
			controltype = "CheckBox";
			break;
		case UIC_ComboBox:
			controltype = "ComboBox";
			break;
		case UIC_Slider:
			controltype = "Slider";
			break;
		case UIC_Colorpicker:
			controltype = "Color";
			break;
		case UIC_KeyBind:
			controltype = "KeyBind";
			break;
		case UIC_MultiComboBox:
			controltype = "Multibox";
			break;
		default:
			break;
		}
		// if current control is valid save it's value
		auto controlname = std::string(controltype + std::to_string(i));
		if (control.control->ControlType == UIC_Colorpicker)
		{
			if (control.name == controlname)
			{
				auto color = static_cast<CColorPicker*>(control.control)->GetColor();
				ini.SetDoubleValue("Eagle", std::string(controlname + "r").c_str(), color.r());
				ini.SetDoubleValue("Eagle", std::string(controlname + "g").c_str(), color.g());
				ini.SetDoubleValue("Eagle", std::string(controlname + "b").c_str(), color.b());
				ini.SetDoubleValue("Eagle", std::string(controlname + "a").c_str(), color.a());
			}
		}
		else if (control.control->ControlType == UIC_MultiComboBox)
		{
			auto Items = ((CMultiComboBox*)control.control)->Selected;
			ini.SetDoubleValue("Eagle", std::string(controlname + "size").c_str(), Items.size());
			for (int i = 0; i < Items.size(); ++i)
			{
				auto item = Items[i];

				ini.SetDoubleValue("Eagle", std::string(controlname + std::to_string(i)).c_str(), item);
			}
		}
		else
		{
			if (control.name == controlname)
				ini.SetDoubleValue("Eagle", controlname.c_str(), control.control->GetValue());
		}
		i++;
	}

	ini.SetDoubleValue("Eagle", "Spectatorlist.x", (double)cheat::settings.radar_pos.x);
	ini.SetDoubleValue("Eagle", "Spectatorlist.y", (double)cheat::settings.radar_pos.y);

	for (auto i = 0; i < parser::weapons.list.size(); i++)
	{
		auto itemdefidx = parser::weapons.list[i].id;

		auto controlname = std::string(parser::weapons.list[i].item_name + "_skin");

		ini.SetDoubleValue("Skins", controlname.c_str(), cheat::settings.paint[itemdefidx]);
	}

	ini.SaveFile((GetGameDirectory() + cfgname).c_str());
}

void CVars::Load(std::string name)
{
	if (name.empty())
		return;

	CSimpleIniA ini;
	ini.SetUnicode(true);
	ini.LoadFile((GetGameDirectory() + name).c_str());

	int i = 0;
	int penis = 0;

	for (auto control : AllControls)
	{
		// get control type
		std::string controltype = "Unknown";
		switch (control.control->ControlType)
		{
		case UIC_CheckBox:
			controltype = "CheckBox";
			break;
		case UIC_ComboBox:
			controltype = "ComboBox";
			break;
		case UIC_Slider:
			controltype = "Slider";
			break;
		case UIC_Colorpicker:
			controltype = "Color";
			break;
		case UIC_KeyBind:
			controltype = "KeyBind";
			break;
		case UIC_MultiComboBox:
			controltype = "Multibox";
			break;
		default:
			break;
		}

		// if current control is valid save it's value
		auto controlname = std::string(controltype  + std::to_string(i));
		if (control.control->ControlType == UIC_Colorpicker)
		{
			if (control.name == controlname)
			{
				Color col = Color::White();
				col = Color(ini.GetDoubleValue("Eagle", std::string(controlname + "r").c_str(), 0),
					ini.GetDoubleValue("Eagle", std::string(controlname + "g").c_str(), 0),
					ini.GetDoubleValue("Eagle", std::string(controlname + "b").c_str(), 0),
					ini.GetDoubleValue("Eagle", std::string(controlname + "a").c_str(), 0));

				if (col.aBase() == 0)
					penis++;

				static_cast<CColorPicker*>(control.control)->SetColor(col);
			}
		}
		else if (control.control->ControlType == UIC_MultiComboBox)
		{
			auto Items = static_cast<CMultiComboBox*>(control.control)->Selected;
			((CMultiComboBox*)control.control)->selected = 0;
			const auto size = Items.size();
			//auto size = static_cast<int>(ini.GetDoubleValue("Eagle", std::string(controlname + "size").c_str(), 0));
			static_cast<CMultiComboBox*>(control.control)->Selected.clear();
			for (int i = 0; i < size; ++i)
			{
				((CMultiComboBox*)control.control)->Selected.push_back((int)(ini.GetDoubleValue("Eagle", std::string(controlname + std::to_string(i)).c_str(), 0)));

				if (((CMultiComboBox*)control.control)->Selected[i])
					((CMultiComboBox*)control.control)->selected++;
			}
		}
		else
		{
			if (control.name == controlname)
				control.control->SetValue(ini.GetDoubleValue("Eagle", controlname.c_str(), 0));
		}
		i++;
	}

	cheat::settings.radar_pos.x = ini.GetDoubleValue("Eagle", "Spectatorlist.x");
	cheat::settings.radar_pos.y = ini.GetDoubleValue("Eagle", "Spectatorlist.y");

	if (penis > 4)
	{
		cheat::Cvars.GradientFrom.SetColor(Color(66, 140, 244));
		cheat::Cvars.GradientTo.SetColor(Color(75, 180, 242));
		cheat::Cvars.MenuTheme.SetColor(Color(66, 140, 244));

		cheat::Cvars.EnemyBoxCol.SetColor(Color(0, 129, 255));
		cheat::Cvars.Visuals_glow_color.SetColor(Color(0, 129, 255));
		cheat::Cvars.Visuals_lglow_color.SetColor(Color(0, 129, 255));
		cheat::Cvars.Visuals_SnaplinesColor.SetColor(Color(128, 0, 128));
		cheat::Cvars.Visuals_skeletonColor.SetColor(Color(105, 105, 105));
		cheat::Cvars.Visuals_chams_color.SetColor(Color(0, 129, 255));
		cheat::Cvars.Visuals_chams_hidden_color.SetColor(Color(0, 255, 129));
		cheat::Cvars.Visuals_chams_history_color.SetColor(Color(0, 255, 129));
		cheat::Cvars.Visuals_wrld_entities_color.SetColor(Color(0, 255, 129));
		cheat::Cvars.Misc_AntiUT.SetValue(1.f);
	}

	for (auto i = 0; i < parser::weapons.list.size(); i++)
	{
		auto itemdefidx = parser::weapons.list[i].id;

		auto controlname = std::string(parser::weapons.list[i].item_name + "_skin");

		cheat::settings.paint[itemdefidx] = ini.GetDoubleValue("Skins", controlname.c_str());
	}

	fully_update = true;

	//ini.SaveFile((GetGameDirectory() + name).c_str());
}
