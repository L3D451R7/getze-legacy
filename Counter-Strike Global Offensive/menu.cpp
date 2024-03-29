#include "hooked.hpp"
#include "menu.h"
#include <algorithm>
#include <unordered_map>
#include <sstream>

#define width_menu 600
#define height_menu 445

static auto main_color = Color(57, 121, 217);//Color(175, 255, 0);
static auto main_color_fade = Color(57, 121, 217);//Color(120, 175, 0);

bool c_drawhack::mouse_in_pos(Vector start, Vector end)
{
	return ((_cursor_position.x >= start.x) && (_cursor_position.y >= start.y) && (_cursor_position.x <= end.x) && (_cursor_position.y <= end.y));
}

bool c_drawhack::Mousein(Vector start, Vector end)
{
	if (_cursor_position.x > start.x && _cursor_position.x < start.x + end.x && _cursor_position.y > start.y && _cursor_position.y < start.y + end.y)
		return true;

	return false;
}

int ignore_items = -1;

bool c_drawhack::menuitem(int &n, std::string text, bool &var, bool parent)
{
	bool value_changed = false;
	Vector menu_pos = Vector(contents_posX + 5, contents_posY + (parent ? 9 : 0) + (14 * n), 0);
	if (line != 0) menu_pos.x += 200 * line;
	Vector max_menu_pos = Vector(menu_pos.x + 90, menu_pos.y + 14, 0);

	if (ignore_items < (n + line * 50)) {

		auto hovered = mouse_in_pos(menu_pos, max_menu_pos);

		Drawing::DrawString(F::Menu, menu_pos.x + 12, menu_pos.y, Color::White(alpha* 0.7), FONT_LEFT, text.c_str());

		Drawing::DrawRect(menu_pos.x, menu_pos.y, 10, 11, Color(60, 60, 60, 0.7 * alpha));
		Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y, 10, 11, hovered ? Color(120, 120, 120, alpha) : Color(9, 9, 9, alpha));

		//if (!hovered)
		//Drawing::DrawRectGradientVertical(menu_pos.x, menu_pos.y + 2, 7, 7, Color(var ? main_color.r() : 0, var ? main_color.g() : 0, var ? main_color.b() : 0, alpha), Color(var ? main_color.r() : 0, var ? main_color.g() : 0, var ? main_color.b() : 0, alpha));

		if (var)
			Drawing::DrawString(F::EagleMisc, menu_pos.x, menu_pos.y + 1, Color::White(alpha), FONT_LEFT, "g");

		if (hovered && mouse1_pressed) {
			var = !var;
			value_changed = true;
		}
	}

	n += 1;

	return value_changed;
}

bool combo_opened[1000];

void c_drawhack::scrollbar(Vector pos, float sizeY, int&n)
{
	int col = 80;
	/*Drawing::DrawRect(pos.x, pos.y + 3, 5, 1, Color(col, col, col, 255));
	Drawing::DrawRect(pos.x, pos.y + 1 + 3, 5, 1, Color(col - 2, col - 2, col - 2, 255));
	Drawing::DrawRect(pos.x, pos.y + 2 + 3, 5, 1, Color(col - 4, col - 4, col - 4, 255));
	Drawing::DrawRect(pos.x, pos.y + 3 + 3, 5, 1, Color(col - 8, col - 8, col - 8, 255));
	Drawing::DrawRect(pos.x, pos.y + 4 + 3, 5, 1, Color(col - 12, col - 12, col - 12, 255));
	Drawing::DrawRect(pos.x, pos.y + 5 + 3, 5, 1, Color(col - 14, col - 14, col - 14, 255));
	Drawing::DrawRect(pos.x, pos.y + 6 + 3, 5, sizeY, Color(col - 16, col - 16, col - 16, 255));*/

	static bool startedscroll = false;

	if (mouse_in_pos(Vector(pos.x, pos.y + 3,0), Vector(pos.x, pos.y + 3, 0) + Vector(5, sizeY, 0))) {
		Drawing::DrawRectGradientHorizontal(pos.x, pos.y + 3, 5, sizeY, main_color, main_color_fade);

		if (cheat::game::pressed_keys[1]) 
			startedscroll = true;
	}

	if (!cheat::game::pressed_keys[1])
		startedscroll = false;

	static int yX = 0;

	startedscroll ? Drawing::DrawRectGradientHorizontal(pos.x, pos.y + 3, 5, sizeY, main_color, main_color_fade) : Drawing::DrawRectGradientHorizontal(pos.x, pos.y + 3, 5, sizeY, Color(col, col, col, 255), Color(col - 16, col - 16, col - 16, 255));

	if (startedscroll) {
		int speed = _cursor_position.x - yX;

		if (n == 0 && ((speed) <= 0 || (speed - 1) < 1))
			n = 0;
		else
			n += _cursor_position.y - yX;
	
		yX = _cursor_position.y;
	}
	else yX = _cursor_position.y;

}

bool c_drawhack::combobox(int&n, std::string name, std::vector<std::string> items, int&value, bool parent)//c_drawhack::combobox(std::string name, std::vector<std::string> items, int & value, bool )
{
	n++;
	auto value_changed = false;
	auto menu_pos = Vector(contents_posX + 5, contents_posY + (parent ? 7 : 0) + (14 * n), 0);
	if (line != 0) menu_pos.x += 200 * line;
	auto max_menu_pos = Vector(menu_pos.x + 180, menu_pos.y + 14, 0);

	static auto start_idx = 0;
	auto nSize = items.size();
	auto bHover = mouse_in_pos(menu_pos, max_menu_pos);

	//Draw->DrawOutlinedRect(MX, MY, MWidth, MHeight, Color(30, 30, 30, 200)); // Tabs main //OUTLINE

	RECT textsizeсombo = Drawing::GetTextSize(F::Menu, items[value].c_str());

	if (nSize > 8)
		nSize = 8;

	auto combo_idx = (line != 0 ? (n + 50 * line) : n);

	if (combo_opened[combo_idx])
		ignore_items = n + nSize + line * 50;

	if (ignore_items < (n - 1 + line * 50) || combo_opened[combo_idx]) {

		Drawing::DrawString(F::Menu, menu_pos.x, menu_pos.y - 14, Color(255, 255, 255, alpha* 0.7), false, name.c_str());

		//Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y, 180, 13, Color(65, 65, 65, alpha));
		Drawing::DrawRect(menu_pos.x, menu_pos.y, 180, 13, Color(45, 45, 45, alpha));

		auto scroll_pos = Vector(max_menu_pos.x - 7, menu_pos.y + 13 + start_idx, 0); auto in_scroll_pos = mouse_in_pos(scroll_pos, scroll_pos + Vector(5, ((items.size() * 40) / max(items.size() - min(8, nSize), 1))/*14*/, 0));

		if (bHover)
		{
			if (mouse1_pressed && !in_scroll_pos)
				combo_opened[combo_idx] = !combo_opened[combo_idx];
		}
		else
		{
			if (mouse1_pressed) {

				if (combo_opened[combo_idx]) {
					for (int i = 0; i < nSize; i++)
					{
						if (mouse_in_pos(Vector(menu_pos.x, menu_pos.y + 14 + i * 14, 0), Vector(menu_pos.x + 180, menu_pos.y + 14 + i * 14 + 13, 0)) && !in_scroll_pos) {
							value = start_idx + i;
							value_changed = true;
						}
					}
				}

				if ((value_changed || !mouse_in_pos(menu_pos + Vector(0, nSize, 0), max_menu_pos + Vector(0, nSize * 14, 0))) && !in_scroll_pos)
					combo_opened[combo_idx] = false;
			}
		}

		//if (bHover)	// hover on top item that in our bar
		//	Drawing::DrawRect(menu_pos.x, menu_pos.y, 180, 12, Color(192, 192, 192, 0.15 * alpha));

		Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y, 180, 14, bHover ? main_color.alpha(0.78431372549 * alpha) : Color(30, 30, 30, 0.78431372549 * alpha));

		if (nSize > 0)// checking our items
		{
			if (combo_opened[combo_idx])
			{
				if (cheat::features::menu.mwheel_value != 0 && items.size() > nSize) {
					start_idx += cheat::features::menu.mwheel_value * -1;

					cheat::features::menu.mwheel_value = 0;
				}

				if (start_idx < 0)
					start_idx = 0;
				else if (start_idx >= (items.size() - nSize))
					start_idx = items.size() - nSize;

				Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y + 14, 180, nSize * 14, Color(30, 30, 30, 0.78431372549 * alpha));
				Drawing::DrawRect(menu_pos.x, menu_pos.y + 14, 179, nSize * 14, Color(45, 45, 45, alpha));
				//Drawing::DrawRect(menu_pos.x, menu_pos.y + 13, 180, nSize * 13, Color(161, 161, 161, 255));

				for (int i = 0; i < nSize; i++)// drawing all items
				{
					auto hovered_item = mouse_in_pos(Vector(menu_pos.x, menu_pos.y + 14 + i * 14, 0), Vector(menu_pos.x + 180, menu_pos.y + 14 + i * 14 + 13, 0));

					auto string_unformatted = items[(i + start_idx)]; if (auto pos = string_unformatted.find(".wav"); pos != std::string::npos) string_unformatted.erase(pos); if (string_unformatted.size() > 31) string_unformatted.resize(31);

					Drawing::DrawString(F::Menu, menu_pos.x + 2, menu_pos.y + 14 + i * 14, (hovered_item || ((i + start_idx) == value)) ? Color(255, 255, 255, alpha* 0.7) : Color(255, 255, 255, 0.3 * alpha), FONT_LEFT, string_unformatted.c_str());
				}

				if (items.size() > nSize)
					scrollbar(Vector(max_menu_pos.x - 7, menu_pos.y + 13 + start_idx, 0), ((items.size() * 40) / max(items.size() - min(8, nSize), 1)), start_idx);
			}
		}

		if (value >= items.size())
			value = items.size() - 1;

		auto main_string_unformatted = items[value]; if (auto pos = main_string_unformatted.find(".wav"); pos != std::string::npos) main_string_unformatted.erase(pos); if (main_string_unformatted.size() > 31) main_string_unformatted.resize(31);

		Drawing::DrawString(F::Menu, menu_pos.x + 3/* + (180 / 2) - (textsizeсombo.right / 2)*/, menu_pos.y + 1, Color(255, 255, 255, alpha* 0.7), FONT_LEFT, main_string_unformatted.c_str());
	}

	n++;

	if (value_changed)
		mouse1_pressed = cheat::game::pressed_keys[1] = false;

	return value_changed;
}

bool c_drawhack::multicombo(int&n, std::string name, std::vector<MultiSelectable> items, bool parent)//c_drawhack::combobox(std::string name, std::vector<std::string> items, int & value, bool )
{
	n++;
	auto value_changed = false;
	auto menu_pos = Vector(contents_posX + 5, contents_posY + (parent ? 7 : 0) + (14 * n), 0);
	if (line != 0) menu_pos.x += 200 * line;
	auto max_menu_pos = Vector(menu_pos.x + 180, menu_pos.y + 14, 0);

	auto nSize = items.size();
	auto bHover = mouse_in_pos(menu_pos, max_menu_pos);
	//auto bkeypress = (name.find("hitscan") != std::string::npos ? mouse1_pressed : cheat::game::get_key_press(1,2));
	//auto bState = &combo_opened[combo_idx];

	std::string items_selected = "";

	//Draw->DrawOutlinedRect(MX, MY, MWidth, MHeight, Color(30, 30, 30, 200)); // Tabs main //OUTLINE

	//RECT textsizeсombo = Drawing::GetTextSize(F::Menu, items_selected.c_str());

	//if (mouse1_pressed)
	//{
	//	if (bHover)
	//		bState = !bState;

	//	combobox_opened = bState;

	//	if (bState)
	//	{
	//		if (!bHover)
	//		{
	//			for (int i = 0; i < nSize; i++)
	//			{
	//				if (mouse_in_pos(Vector(menu_pos.x, menu_pos.y + 14 + i * 14, 0), Vector(menu_pos.x + 180, menu_pos.y + 14 + i * 14 + 13, 0))) {
	//					auto old_val = *items[i].value;
	//					*items[i].value = !old_val;
	//					value_changed = true;
	//				}
	//			}
	//		}
	//		
	//		if (!mouse_in_pos(menu_pos + Vector(0,nSize,0), max_menu_pos + Vector(0, nSize * 14,0)))
	//			bState = combobox_opened = false;
	//	}

	//	items_selected = 0;

	//	for (int i = 0; i < nSize; i++)// drawing all we got
	//	{
	//		if (*items[i].value == true)
	//			items_selected++;
	//	}
	//}

	auto combo_idx = (line != 0 ? (n + 50 * line) : n);

	if (combo_opened[combo_idx])
		ignore_items = n + nSize + line * 50;

	if (ignore_items < (n - 1 + line * 50) || combo_opened[combo_idx]) {


		Drawing::DrawString(F::Menu, menu_pos.x, menu_pos.y - 14, Color(255, 255, 255, alpha * 0.7), false, name.c_str());

		//Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y, 180, 13, Color(65, 65, 65, alpha));
		Drawing::DrawRect(menu_pos.x, menu_pos.y, 180, 13, Color(45, 45, 45, alpha));

		for (int i = 0; i < nSize; i++)// drawing all we got
		{
			if (*items[i].value == true)
				items_selected += items[i].name;

			if ((i + 1) < nSize && items_selected.size() > 1 && *items[i + 1].value == true)
				items_selected += ", ";

			if (Drawing::GetTextSize(F::Menu, items_selected.c_str()).right > 170) {
				items_selected.resize(32);
				items_selected += "...";
				break;
			}
		}

		if (bHover)
		{
			if (mouse1_pressed)
				combo_opened[combo_idx] = !combo_opened[combo_idx];
		}
		else
		{
			if (mouse1_pressed) {

				if (combo_opened[combo_idx]) {
					for (int i = 0; i < nSize; i++)
					{
						if (mouse_in_pos(Vector(menu_pos.x, menu_pos.y + 14 + i * 14, 0), Vector(menu_pos.x + 180, menu_pos.y + 14 + i * 14 + 13, 0))) {
							auto old_val = *items[i].value;
							*items[i].value = !old_val;
							value_changed = true;
						}
					}
				}

				if (!mouse_in_pos(menu_pos + Vector(0, nSize, 0), max_menu_pos + Vector(0, nSize * 14, 0)))
					combo_opened[combo_idx] = false;
			}
		}

		Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y, 180, 14, bHover ? main_color.alpha(0.78431372549 * alpha) : Color(30, 30, 30, 0.78431372549 * alpha));

		if (nSize > 0)
		{
			if (combo_opened[combo_idx])
			{
				Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y + 14, 180, nSize * 14, Color(30, 30, 30, 0.78431372549 * alpha));
				Drawing::DrawRect(menu_pos.x, menu_pos.y + 14, 179, nSize * 14, Color(45, 45, 45, alpha));

				for (int i = 0; i < nSize; i++)
				{
					auto hovered_item = mouse_in_pos(Vector(menu_pos.x, menu_pos.y + 14 + i * 14, 0), Vector(menu_pos.x + 180, menu_pos.y + 14 + i * 14 + 13, 0));

					Drawing::DrawString(F::Menu, menu_pos.x + 2, menu_pos.y + 14 + i * 14, (hovered_item || *items[i].value) ? Color(255, 255, 255, alpha* 0.7) : Color(255, 255, 255, 0.3 * alpha), FONT_LEFT, items[i].name.c_str());
				}
			}
		}
	}

	Drawing::DrawString(F::Menu, menu_pos.x + 3, menu_pos.y + 1, Color(150, 150, 150, alpha * 0.9), FONT_LEFT, /*"%d items selected", */items_selected.c_str());

	n++;

	return value_changed;
}

//bool c_drawhack::combobox(int&n, std::string name, std::vector<std::string> items, int&selected, bool parent)
//{
//	n++;
//	const auto COMBO_SCROLL_WIDTH = 12;
//	const auto COMBO_MAX_ITEMS = 10;
//	static int m_iComboOffset = 0;
//	static bool m_bActiveScrollbar = false;
//	const auto COMBO_SCROLL_BUTTON_SIZE = COMBO_SCROLL_WIDTH / 2;
//
//	auto value_changed = false;
//	auto menu_pos = Vector(contents_posX + 5, contents_posY + (parent ? 7 : 0) + (12 * n), 0);
//	if (line != 0) menu_pos.x += 200 * line;
//	auto max_menu_pos = Vector(menu_pos.x + 180, menu_pos.y + 12, 0);
//
//	auto hover = mouse_in_pos(menu_pos, max_menu_pos);
//
//	static bool opened = false;
//
//	if (hover && mouse1_pressed) opened = !opened;
//
//	Drawing::DrawString(F::Menu, menu_pos.x, menu_pos.y - 13, Color::White(), FONT_LEFT, name.c_str());
//
//	if (opened)
//	{
//		int max_item_count = items.size();
//		int item_count = min(max_item_count, COMBO_MAX_ITEMS);
//		int iStartIndex = 0;
//		bool bScrollable = max_item_count > COMBO_MAX_ITEMS;
//		bool bInScrollbarZone = false;
//
//		if (bScrollable)
//		{
//			/*for (int j = 0; j < item_count; j++)
//			{
//				auto lul = Vector(menu_pos.x, menu_pos.y + 12 * (j + 1), 0);
//
//				if (mouse_in_pos(menu_pos, max_menu_pos))
//				{
//					m_iComboOffset = j;
//					selected = j;
//				}
//			}*/
//
//			// Handling combo offset
//			int iShit = max_item_count - item_count;
//			if (m_iComboOffset > iShit)
//				m_iComboOffset = iShit;
//			if (m_iComboOffset < 0)
//				m_iComboOffset = 0;
//			iStartIndex = m_iComboOffset;
//
//			// Scrollbar
//			int iX = menu_pos.x + 180 - COMBO_SCROLL_WIDTH;
//			int iY = menu_pos.y;
//			int iWidth = COMBO_SCROLL_WIDTH;
//			int iHeight = 12 * item_count;
//			bInScrollbarZone = mouse_in_pos(Vector(iX, iY, 0), Vector(iWidth, iHeight, 0));
//
//			float iScrollComboHeight = iHeight / max_item_count; // Dont try to understand this variable name please
//																 //float flShit = (float)iMaxItemsCount / (float)iItemsCount;
//																 //float iScrollbarHeight = (float)iHeight / flShit;
//			int iScrollbarHeight = iHeight - ((max_item_count - item_count) * iScrollComboHeight);
//			//if (iScrollbarHeight == iHeight) iScrollbarHeight = 6; // nice fix meme
//			int iScrollbarY = iScrollComboHeight * iStartIndex;
//			bool bInScrollbarComboZone = mouse_in_pos(Vector(iX, iY + iScrollbarY,0), Vector(iWidth, iScrollbarHeight,0)); if (bInScrollbarComboZone && mouse1_pressed) m_bActiveScrollbar = true;
//			Drawing::DrawRect(iX, iY, iWidth, iHeight, Color(176, 176, 176));
//			Drawing::DrawRect(iX, iY + iScrollbarY, iWidth, iScrollbarHeight, m_bActiveScrollbar ? Color(180, 180, 180) : Color(140, 140, 140));
//
//
//			/*if (m_bActiveScrollbar)
//			{
//				if (m_iMouseDeltaY < -iScrollComboHeight)
//				{
//					m_iComboOffset--;
//					m_iMouseDeltaY += iScrollComboHeight;
//				}
//				if (m_iMouseDeltaY > iScrollComboHeight)
//				{
//					m_iComboOffset++;
//					m_iMouseDeltaY -= iScrollComboHeight;
//				}
//			}*/
//		}
//
//		int j = 0;
//		for (int i = iStartIndex; i < item_count + iStartIndex; i++)
//		{
//			Drawing::DrawString(F::Menu, menu_pos.x, menu_pos.y + (j * 12), selected == i ? Color(103, 103, 103) : Color(0, 0, 0), FONT_LEFT, name.c_str());
//			auto lul = Vector(menu_pos.x, menu_pos.y + 12 * (j * 12), 0);
//
//			if (mouse_in_pos(lul, lul + Vector(180 - (bScrollable ? COMBO_SCROLL_WIDTH : 0), 12, 0)) && !m_bActiveScrollbar)//BaseControl(GetStringForItem(pszItems, i), m_iDrawX, m_iDrawY + (j * COMBO_HEIGHT), COMBO_WIDTH - (bScrollable ? COMBO_SCROLL_WIDTH : 0), COMBO_HEIGHT, iSelectedItem == i ? Color(103, 103, 103) : Color(0, 0, 0), true) && !m_bActiveScrollbar)
//			{
//				selected = i;
//				value_changed = true;
//			}
//			j++;
//		}
//		if (!hover &&  mouse1_pressed && !bInScrollbarZone && !m_bActiveScrollbar)
//			opened = false;
//	}
//
//	n++;
//	return value_changed;
//}

void c_drawhack::colorpicker(int&n, std::string name, float* value, bool parent)
{
	n++;

	auto menu_pos = Vector(contents_posX + 5, contents_posY + (parent ? 7 : 0) + (14 * n), 0);
	if (line != 0) menu_pos.x += 200 * line;
	auto max_menu_pos = Vector(menu_pos.x + 180, menu_pos.y + 14, 0);
	if (ignore_items < (n + line * 50)) {
		Color color = Color(40, 210, 116);

		Drawing::DrawString(F::Menu, menu_pos.x + 2, menu_pos.y - 14, Color(255, 255, 255), false, name.c_str());
		Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y, 180, 14, Color(40, 40, 40));

		if (value[0] >= 0.97f)
			value[2] = 0.f;
		else
			value[2] = 1.f;

		if (!mouse_in_pos(menu_pos, max_menu_pos) || !cheat::game::pressed_keys[1])
			Drawing::DrawRect(menu_pos.x + 2, menu_pos.y + 2, value[0] * ((180.f - 4.f) / 1.f), 10, Color().FromHSB(value[0], value[2], 1.f).alpha(value[1] * 255));

		if (mouse_in_pos(menu_pos, max_menu_pos))
		{
			int start = value[0] * ((180.f - 4.f) / 1.f);
			float flColor = std::clamp((_cursor_position.x - menu_pos.x) * 1.f / (180.f - 4.f), 0.f, 1.f);
			Color newColor = Color().FromHSB(flColor, value[2], 1.f);

			if (cheat::game::pressed_keys[1]) {
				value[0] = std::clamp((_cursor_position.x - menu_pos.x) * 1.f / (180.f - 4.f), 0.f, 1.f);
				//if (_cursor_position.x > (menu_pos.x + 1 + start))
				//	Drawing::DrawRect(menu_pos.x + 2 + start, menu_pos.y + 2, _cursor_position.x - menu_pos.x - start - 3.f, 10, newColor.alpha(alpha * 0.7));
				//else
				Drawing::DrawRect(menu_pos.x + 2, menu_pos.y + 2, _cursor_position.x - menu_pos.x - 3.f, 10, newColor.alpha(value[1] * 255));
			}
			else if (cheat::game::pressed_keys[2]) {
				value[1] = std::clamp((_cursor_position.x - menu_pos.x) * 1.f / (180.f - 4.f), 0.f, 1.f);
				Drawing::DrawRect(menu_pos.x + 2, menu_pos.y + 2, _cursor_position.x - menu_pos.x - 3.f, 10, Color().FromHSB(value[0], 1.f, value[1]));
			}

			Drawing::DrawRect(_cursor_position.x + 12, _cursor_position.y + 2, 312, 16, Color::Black(200));
			Drawing::DrawString(F::Menu, _cursor_position.x + 14, _cursor_position.y + 4, Color(255, 255, 255), false, "hold mouse1 to change color / hold mouse2 to change alpha.");
		}
	}
	n++;
}

void c_drawhack::menugroupbox(int &n, int size, std::string text)
{
	Vector menu_pos = Vector(contents_posX, contents_posY + (12 * n), 0);
	if (line != 0) menu_pos.x += 200 * line;
	Vector max_menu_pos = Vector(menu_pos.x + 190, menu_pos.y + 16, 0);
	if (ignore_items < (n + line * 50)) {
		Drawing::DrawRect(menu_pos.x, menu_pos.y, max_menu_pos.x - menu_pos.x, (size + 1) * 12, Color(28, 28, 28, alpha - 60));

		if (!text.empty()) {
			auto tsize = Drawing::GetTextSize(F::Menu, text.c_str());
			Drawing::DrawString(F::Menu, /*menu_pos.x + 150 / 2 - tsize.right / 2*/menu_pos.x + 6, menu_pos.y - 7, Color(200, 200, 200, alpha* 0.7), FONT_LEFT, text.c_str());

			//Drawing::DrawRectGradientVertical(menu_pos.x, menu_pos.y, /*menu_pos.x*/1, /*menu_pos.y +*/ (size + 1) * 12, Color::White(alpha * 0.6), Color(15, 15, 15, alpha - 20)); // left

			Drawing::DrawLine(menu_pos.x, menu_pos.y + (size + 1) * 12, max_menu_pos.x, menu_pos.y + (size + 1) * 12, Color(53, 53, 53, alpha)); // down

			Drawing::DrawLine(menu_pos.x, menu_pos.y, menu_pos.x, menu_pos.y + (size + 1) * 12, Color(53, 53, 53, alpha)); // left

			//Drawing::DrawRectGradientVertical(max_menu_pos.x, menu_pos.y, 1, /*menu_pos.y + */(size + 1) * 12, Color::White(alpha * 0.6), Color(15, 15, 15, alpha - 20)); // right

			Drawing::DrawLine(max_menu_pos.x, menu_pos.y, max_menu_pos.x ,menu_pos.y + (size + 1) * 12, Color(53, 53, 53, alpha)); // right

			Drawing::DrawLine(menu_pos.x, menu_pos.y, menu_pos.x + 2, menu_pos.y, Color(53, 53, 53, alpha)); // top

			Drawing::DrawLine(menu_pos.x + tsize.right + 8, menu_pos.y, max_menu_pos.x, menu_pos.y, Color(53, 53, 53, alpha)); // top
		}
		else
			Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y, max_menu_pos.x - menu_pos.x, (size + 1) * 12, Color::White(alpha * 0.6));
	}
	//return groupbox(line);
}

void c_drawhack::separator(int &n, std::string text)
{
	Vector menu_pos = Vector(contents_posX, contents_posY + (12 * n+12), 0);
	if (line != 0) menu_pos.x += 200 * line;
	Vector max_menu_pos = Vector(menu_pos.x + 190, menu_pos.y + 16, 0);
	if (ignore_items < (n + line * 50)) {
		if (!text.empty()) {
			auto tsize = Drawing::GetTextSize(F::Menu, text.c_str());
			Drawing::DrawString(F::Menu, /*menu_pos.x + 150 / 2 - tsize.right / 2*/menu_pos.x + 6, menu_pos.y - 7, Color(200, 200, 200, alpha* 0.7), FONT_LEFT, text.c_str());

			Drawing::DrawLine(menu_pos.x, menu_pos.y, menu_pos.x + 2, menu_pos.y, Color(53, 53, 53, alpha)); // top

			Drawing::DrawLine(menu_pos.x + tsize.right + 8, menu_pos.y, max_menu_pos.x, menu_pos.y, Color(53, 53, 53, alpha)); // top
		}
		else
			Drawing::DrawLine(menu_pos.x, menu_pos.y, max_menu_pos.x - menu_pos.x, 12, Color(53, 53, 53, alpha));
	}
	n++;
	//return groupbox(line);
}
//bool c_drawhack::menubutton(int &n, std::string text)
//{
//	bool value_changed = false;
//	Vector menu_pos = Vector(contents_posX, contents_posY + (16 * n), 0);
//	if (line != 0) menu_pos.x += 100;
//	Vector max_menu_pos = Vector(menu_pos.x + 70, menu_pos.y + 16, 0);
//
//	float text_size = get_text_w(text.c_str(), cheat::features::directx::dx_font_verdana);
//
//	fill_rgba(menu_pos.x, menu_pos.y, text_size + 5, 16, 36, 36, 36, alpha);
//
//	draw_quad(menu_pos.x, menu_pos.y, menu_pos.x + text_size + 5, menu_pos.y + 16, 255, 255, 255, 50);
//
//	Drawing::DrawString(F::Menu, menu_pos.x + 3, menu_pos.y + 1, Color::White(alpha), FONT_LEFT, text.c_str());
//
//	if (mouse_in_pos(menu_pos, max_menu_pos) && mouse1_pressed) {
//		value_changed = true;
//	}
//
//	n += 1;
//
//	return value_changed;
//}
//
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

bool c_drawhack::menutab(int &tab, std::string text, bool selected = false, bool parent)
{
	std::vector<std::string> lmao_wtf = {};

	bool value_changed = false;

	if (text.find(";") != std::string::npos)
		lmao_wtf = split(text, ';');
	else
		lmao_wtf.push_back(text);

	auto text_size = Drawing::GetTextSize(F::Eagle, lmao_wtf.front().c_str());

	//Vector menu_pos = Vector(contents_posX, contents_posY + (parent ? 12 : 5) + tab * 36, 0);
	//Vector menu_pos = Vector(contents_posX + width_menu - tab - 5, menu_posY, 0);
	Vector menu_pos = Vector(contents_posX + tab + 5, menu_posY + 60, 0);

	Vector max_menu_pos = Vector(menu_pos.x + text_size.right + 2, menu_pos.y + text_size.bottom + 14, 0);

	auto hovered = mouse_in_pos(menu_pos, max_menu_pos);

	//else
	//	fill_rgba(menu_pos.x, menu_pos.y, text_size + 5, 11, 36, 36, 36, 255);

	//draw_quad(menu_pos.x, menu_pos.y, menu_pos.x + text_size + 5, menu_pos.y + 11, 255, 255, 255, 50);

	//Drawing::DrawString(F::LBY, menu_pos.x + 4, max_menu_pos.y - 35 / 2 - text_size.bottom / 2, Color::White(alpha), FONT_LEFT, text.c_str());

	//if (selected)
	//	Drawing::DrawRect(menu_pos.x, menu_pos.y - 2, text_size.right + 4, 16, Color(255, 255, 255, 0.15 * alpha));
	//else if (hovered)
	//	Drawing::DrawRect(menu_pos.x, menu_pos.y - 2, text_size.right + 4, 16, Color(192, 192, 192, 0.15 * alpha));

	Drawing::DrawString(F::Eagle, menu_pos.x + 2, menu_pos.y, Color::White(alpha* (selected ? 1 : (hovered ? 0.8 : 0.5))), FONT_LEFT, lmao_wtf.front().c_str());

	if (lmao_wtf.size() > 1)
		Drawing::DrawString(F::EagleTab, menu_pos.x + 2 + text_size.right / 2, menu_pos.y + text_size.bottom + 2, Color::White(alpha* (selected ? 1 : (hovered ? 0.8 : 0.5))), FONT_CENTER, lmao_wtf.at(1).c_str());

	if (hovered && mouse1_pressed) {
		value_changed = true;
	}

	tab += text_size.right + (width_menu / 3);

	lmao_wtf.clear();

	return value_changed;
}
//
//bool c_drawhack::menuitem(int &n, std::string text_t, int &var, int min, int max)
//{
//	bool value_changed = false;
//
//	Vector menu_pos = Vector(contents_posX, contents_posY + (16 * n), 0);
//	if (line != 0) menu_pos.x += 100;
//	Vector max_menu_pos = Vector(menu_pos.x + 70, menu_pos.y + 16, 0);
//
//	if (var > max)
//		var = max;
//	else if (var < min)
//		var = min;
//
//	auto text = text_t + " [" + std::to_string(var) + "]";
//
//	float text_size = get_text_w(text.c_str(), cheat::features::directx::dx_font_pixel);
//
//	draw_string(text.c_str(), menu_pos.x + 7, menu_pos.y, Color::White(alpha), FONT_LEFT,);
//
//	//draw_box(menu_pos.x, menu_pos.y, 73, 11, 1, 65, 65, 65, 255);
//
//	fill_rgba(menu_pos.x, menu_pos.y + 3, 5, 5, 35, 35, 25, 255);
//
//	Drawing::DrawString(F::Menu, menu_pos.x + 1, menu_pos.y, Color::White(alpha), FONT_LEFT, xor_string("-"));
//
//	fill_rgba(menu_pos.x + text_size + 8, menu_pos.y + 3, 5, 5, 35, 35, 25, alpha);
//
//	draw_string(xor_string("+"), menu_pos.x + text_size + 9, menu_pos.y, Color::White(alpha), FONT_LEFT,);
//
//	if (mouse_in_pos(Vector(menu_pos.x, menu_pos.y + 3, 0), Vector(menu_pos.x + 4, menu_pos.y + 7, 0)) && mouse1_pressed) {
//		if (var > min)
//			var -= 1;
//
//		value_changed = true;
//	}
//	else if (mouse_in_pos(Vector(menu_pos.x + text_size + 7, menu_pos.y + 3, 0), Vector(menu_pos.x + text_size + 12, menu_pos.y + 7, 0)) && mouse1_pressed) {
//		if (var < max)
//			var += 1;
//
//		value_changed = true;
//	}
//
//	n += 1;
//
//	return value_changed;
//}
//
bool c_drawhack::menuitem(int &n, std::string name, std::vector < std::string > items, int &var, bool parent)
{
	bool value_changed = false;

	int max = items.size() - 1;

	int max_size = strstr(name.c_str(), "configs") ? 139 : 73;

	Vector menu_pos = Vector(contents_posX + 5, contents_posY + (16 * n), 0);
	if (line != 0) menu_pos.x += 200 * line;
	Vector max_menu_pos = Vector(menu_pos.x + max_size, menu_pos.y + 16, 0);
	if (ignore_items < (n + line * 50)) {
		if (var > max)
			var = max;
		else if (var < 0)
			var = 0;

		if (mouse_in_pos(Vector(menu_pos.x, menu_pos.y + 3, 0), Vector(menu_pos.x + 5, menu_pos.y + 7, 0)) && mouse1_pressed) {
			if (var == 0)
				var = max;
			else
				var--;

			value_changed = true;
		}
		else if (mouse_in_pos(Vector(menu_pos.x + max_size - 6, menu_pos.y + 3, 0), Vector(menu_pos.x + max_size + 1, menu_pos.y + 7, 0)) && mouse1_pressed) {
			if (var == max)
				var = 0;
			else
				var++;

			value_changed = true;
		}

		//Drawing::DrawRect(menu_pos.x, menu_pos.y, max_size, 11, Color(65, 65, 65, alpha));

		Drawing::DrawRect(menu_pos.x, menu_pos.y, max_size, 11, Color(35, 35, 25, alpha));

		Drawing::DrawString(F::Menu, menu_pos.x + (max_size + 1) / 2 - Drawing::GetTextSize(F::Menu, items.at(var).c_str()).right / 2/*7*/, menu_pos.y, Color::White(alpha* 0.7), FONT_LEFT, items.at(var).c_str());

		Drawing::DrawString(F::Menu, menu_pos.x + max_size + 5, menu_pos.y, Color::White(alpha* 0.7), FONT_LEFT, items.at(var).c_str());

		Drawing::DrawRect(menu_pos.x, menu_pos.y + 3, 5, 5, Color(85, 85, 85, alpha));

		Drawing::DrawString(F::Menu, menu_pos.x + 1, menu_pos.y, Color::White(alpha* 0.7), FONT_LEFT, "-");

		Drawing::DrawRect(menu_pos.x + max_size - 5, menu_pos.y + 3, 5, 5, Color(85, 85, 85, alpha));

		Drawing::DrawString(F::Menu, menu_pos.x + max_size - 4, menu_pos.y, Color::White(alpha* 0.7), FONT_LEFT, "+");
	}

	n += 1;

	return value_changed;
}
//
//char* key_digits_uppercase[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
//"Y", "Z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
//"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, ",", nullptr, ".", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 199
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
//
//char* key_digits_lowercase[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
//"y", "z", nullptr, nullptr, nullptr, nullptr, nullptr, "0", "1", "2", "3", "4", "5", "6",
//"7", "8", "9", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, ",", nullptr, ".", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 199
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
//nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
////188
//const char* KeyMapArray[124]
//{
//	"None",
//	"Mouse 1",
//	"Mouse 2",
//	"None",
//	"Mouse 3",
//	"Mouse 4",
//	"Mouse 5",
//	"None",
//	"Backspace",
//	"Tab",
//	"None",
//	"None",
//	"None",
//	"Enter",
//	"None",
//	"None",
//	"Shift",
//	"Ctrl",
//	"Alt",
//	"Pause",
//	"Caps Lock",
//	"None",
//	"None",
//	"None",
//	"None",
//	"None",
//	"None",
//	"Escape",
//	"None",
//	"None",
//	"None",
//	"None",
//	"Space",
//	"Page Up",
//	"Page Down",
//	"End",
//	"Home",
//	"Left",
//	"Up",
//	"Right",
//	"Down",
//	"None",
//	"None",
//	"None",
//	"Print",
//	"Insert",
//	"Delete",
//	"None",
//	"0",
//	"1",
//	"2",
//	"3",
//	"4",
//	"5",
//	"6",
//	"7",
//	"8",
//	"9",
//	"None",
//	"None",
//	"None",
//	"None",
//	"None",
//	"None",
//	"None",
//	"A",
//	"B",
//	"C",
//	"D",
//	"E",
//	"F",
//	"G",
//	"H",
//	"I",
//	"J",
//	"K",
//	"L",
//	"M",
//	"N",
//	"O",
//	"P",
//	"Q",
//	"R",
//	"S",
//	"T",
//	"U",
//	"V",
//	"W",
//	"X",
//	"Y",
//	"Z",
//	"None",
//	"None",
//	"None",
//	"None",
//	"None",
//	"Numpad 0",
//	"Numpad 1",
//	"Numpad 2",
//	"Numpad 3",
//	"Numpad 4",
//	"Numpad 5",
//	"Numpad 6",
//	"Numpad 7",
//	"Numpad 8",
//	"Numpad 9",
//	"Multiply",
//	"Add",
//	"None",
//	"Subtract",
//	"Decimal",
//	"Divide",
//	"F1",
//	"F2",
//	"F3",
//	"F4",
//	"F5",
//	"F6",
//	"F7",
//	"F8",
//	"F9",
//	"F10",
//	"F11",
//	"F12"
//};
//
//void c_drawhack::get_keys(bool &IsGettingKey, std::string & text, bool secondtry = true)
//{
//	if (!IsGettingKey)
//		return;
//
//	const char *strg = text.c_str();
//
//	for (int i = 0; i < ARRAYSIZE(key_digits_lowercase); i++)
//	{
//		if (cheat::game::pressed_keys[16])//191
//		{
//			if (cheat::game::get_key_press(191))
//			{
//				text = text + xor_string("?");
//			}
//			else if (cheat::game::get_key_press(186))
//			{
//				text = text + xor_string(":");
//			}
//			else if (cheat::game::get_key_press(56))
//			{
//				text = text + xor_string("*");
//			}
//			else if (cheat::game::get_key_press(52))
//			{
//				text = text + xor_string("$");
//			}
//			else if (cheat::game::get_key_press(50))
//			{
//				text = text + xor_string("@");
//			}
//			else if (cheat::game::get_key_press(49))
//			{
//				text = text + xor_string("!");
//			}
//			else if (cheat::game::get_key_press(187))
//			{
//				text = text + xor_string("+");
//			}
//			else if (cheat::game::get_key_press(48))
//			{
//				text = text + xor_string(")");
//			}
//			else if (cheat::game::get_key_press(57))
//			{
//				text = text + xor_string("(");
//			}
//			else if (cheat::game::get_key_press(i))
//			{
//				if (i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT)
//				{
//					IsGettingKey = false;
//					return;
//				}
//
//				if (IsGettingKey)
//				{
//					if (i == VK_BACK && pHideFunc.str_len(strg) != 0)
//					{
//						text = text.substr(0, pHideFunc.str_len(strg) - 1);
//					}
//
//					if (pHideFunc.str_len(strg) < 20 && i != NULL && key_digits_uppercase[i] != nullptr && secondtry)
//					{
//						text = text + key_digits_uppercase[i];
//						return;
//					}
//
//					if (pHideFunc.str_len(strg) < 20 && i == 32)
//					{
//						text = text + " ";
//						return;
//					}
//				}
//			}
//		}
//		else if (cheat::game::get_key_press(i))
//		{
//			if (i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT)
//			{
//				IsGettingKey = false;
//				return;
//			}
//
//			if (IsGettingKey)
//			{
//				if (i == VK_BACK && pHideFunc.str_len(strg) != 0)
//				{
//					text = text.substr(0, pHideFunc.str_len(strg) - 1);
//				}
//
//				if (pHideFunc.str_len(strg) < 20 && i != NULL && key_digits_lowercase[i] != nullptr && secondtry)
//				{
//					text = text + key_digits_lowercase[i];
//					return;
//				}
//
//				if (pHideFunc.str_len(strg) < 20 && i == 32)
//				{
//					text = text + " ";
//					return;
//				}
//			}
//		}
//	}
//}
//
//void c_drawhack::get_key(bool &IsGettingKey, int& get)
//{
//	if (!IsGettingKey)
//		return;
//
//	for (int i = 0; i < 255; i++)
//	{
//		if (cheat::game::get_key_press(i))
//		{
//			if (i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT)
//			{
//				get = 0;
//				IsGettingKey = false;
//				break;
//			}
//
//			if (KeyMapArray[i] != nullptr && i != NULL) {
//				get = i;
//				IsGettingKey = false;
//				break;
//			}
//			else if (i == 18) {
//				get = 18;
//				IsGettingKey = false;
//				break;
//			}
//			else {
//				get = 0;
//				IsGettingKey = false;
//				break;
//			}
//
//			/*if (i == VK_BACK)
//			{
//			text = text.substr(0, strlen(strg) - 1);
//			IsGettingKey = false;
//			}
//
//			if (strlen(strg) < 20 && i != NULL && key_digits[i] != nullptr && secondtry)
//			{
//			text = text + key_digits[i];
//			IsGettingKey = false;
//			return;
//			}
//
//			if (strlen(strg) < 20 && i == 32)
//			{
//			text = text + " ";
//			IsGettingKey = false;
//			return;
//			}*/
//		}
//	}
//}
//
//std::vector<std::string> configg;
//
//void GetConfigMassive()
//{
//	//get all files on folder
//
//	configg.clear();
//
//	static char path[MAX_PATH];
//	std::string szPath1;
//
//	if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path)))
//		return;
//
//	szPath1 = std::string(path) + "\\" + cheat::features::security->get_login() + "\\*";
//
//
//	WIN32_FIND_DATA FindFileData;
//	HANDLE hf;
//	configg.push_back("main.mex");
//
//	hf = FindFirstFile(szPath1.c_str(), &FindFileData);
//	if (hf != INVALID_HANDLE_VALUE) {
//		do {
//			std::string filename = FindFileData.cFileName;
//
//			if (filename == ".")
//				continue;
//
//			if (filename == "..")
//				continue;
//
//			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
//			{
//				if (filename.find(".mex") != std::string::npos)
//				{
//					configg.push_back(std::string(filename));
//				}
//			}
//		} while (FindNextFile(hf, &FindFileData) != 0);
//		FindClose(hf);
//	}
//}
//
//bool c_drawhack::menuinput(int & n, const char* name, char * text, int buf_size)
//{
//	bool value_changed = false;
//
//	static bool listening = false;
//
//	std::string buf = text;
//
//	const char *cstr = buf.c_str();
//
//	Vector menu_pos = Vector(contents_posX, contents_posY + (16 * n), 0);
//	if (line != 0) menu_pos.x += 90;
//	Vector max_menu_pos = Vector(menu_pos.x + 140, menu_pos.y + 16, 0);
//
//	if (mouse_in_pos(menu_pos, max_menu_pos) && mouse1_pressed) {
//		if (!listening)
//			listening = true;
//		else
//			listening = false;
//	}
//
//	if (cheat::game::get_key_press(13) || (mouse1_pressed && !mouse_in_pos(menu_pos, max_menu_pos))) {
//		listening = false;
//	}
//
//	float text_size = get_text_w("1234567890123456789", cheat::features::directx::dx_font_verdana);
//
//	if (listening)
//		fill_rgba(menu_pos.x, menu_pos.y, text_size + 6, 16, 96, 96, 96, alpha);
//	else
//		fill_rgba(menu_pos.x, menu_pos.y, text_size + 6, 16, 36, 36, 36, alpha);
//
//	draw_box(menu_pos.x, menu_pos.y, text_size + 6, 16, 1, 65, 65, 65, alpha);
//	//draw_quad(menu_pos.x, menu_pos.y, menu_pos.x + text_size + 6, menu_pos.y + 11, 255, 255, 255, 50);
//
//	if (listening && cheat::main::menu_opened)
//		get_keys(listening, buf);
//
//	if (strcmp(text, buf.c_str()) != 0/*.compare(text) != std::string::npos*/)
//	{
//		value_changed = true;
//	}
//
//	if ((int)buf.size() < buf_size)
//		strcpy(text, buf.c_str());
//
//	draw_string(text, menu_pos.x + 3, menu_pos.y + 1, 255, 255, 255, cheat::features::directx::dx_font_verdana, false, alpha);
//
//	draw_string(name, menu_pos.x + text_size + 10, menu_pos.y, 255, 255, 255, cheat::features::directx::dx_font_verdana, false, alpha);
//
//	n += 1;
//
//	return value_changed;
//}
//
//bool c_drawhack::menuinput(int & n, const char* name, std::string& text)
//{
//	bool value_changed = false;
//
//	static bool listening = false;
//
//	std::string buf = text;
//
//	const char *cstr = buf.c_str();
//
//	Vector menu_pos = Vector(contents_posX, contents_posY + (16 * n), 0);
//	if (line != 0) menu_pos.x += 90;
//	Vector max_menu_pos = Vector(menu_pos.x + 100, menu_pos.y + 16, 0);
//
//	if (mouse_in_pos(menu_pos, max_menu_pos) && mouse1_pressed) {
//		if (!listening)
//			listening = true;
//		else
//			listening = false;
//	}
//
//	if (cheat::game::get_key_press(13) || (mouse1_pressed && !mouse_in_pos(menu_pos, max_menu_pos))) {
//		listening = false;
//	}
//
//	float text_size = get_text_w(xor_string("1234567890123456789"), cheat::features::directx::dx_font_verdana);
//
//	if (listening)
//		fill_rgba(menu_pos.x, menu_pos.y, text_size + 6, 11, 96, 96, 96, 255);
//	else
//		fill_rgba(menu_pos.x, menu_pos.y, text_size + 6, 11, 36, 36, 36, 255);
//
//	draw_box(menu_pos.x, menu_pos.y, menu_pos.x + text_size + 6, menu_pos.y + 11, 1, 65, 65, 65, 255);
//	//draw_quad(menu_pos.x, menu_pos.y, menu_pos.x + text_size + 6, menu_pos.y + 11, 255, 255, 255, 50);
//
//	if (listening && cheat::main::menu_opened)
//		get_keys(listening, buf);
//
//	if (buf.compare(text) == std::string::npos)
//	{
//		value_changed = true;
//	}
//
//	if ((int)buf.size() < (int)text.size())
//		text = buf;
//
//	draw_string(text.c_str(), menu_pos.x + 3, menu_pos.y, 255, 255, 255, cheat::features::directx::dx_font_verdana);
//
//	draw_string(name, menu_pos.x + text_size + 10, menu_pos.y, 255, 255, 255, cheat::features::directx::dx_font_verdana);
//
//	n += 1;
//
//	return value_changed;
//}
//
//bool c_drawhack::menukeybind(int &n, int *var)
//{
//	static std::vector<std::string> types = { xor_string("always"),  xor_string("on key"),  xor_string("avoid key"),  xor_string("toggle") };
//
//	static auto old_key = 0;
//
//	bool pressed_smth_this_tick = false;
//
//	bool value_changed = false;
//	static bool listening = false;
//	static bool change_mode = false;
//
//	Vector menu_pos = Vector(contents_posX, contents_posY + (16 * n) + 2, 0);
//	if (line != 0) menu_pos.x += 90;
//	Vector max_menu_pos = Vector(menu_pos.x + 50, menu_pos.y + 16, 0);
//
//	if (mouse_in_pos(menu_pos, max_menu_pos)) {
//		if (mouse1_pressed) {
//			/*if (!listening)
//			listening = true;
//			else*/
//			listening = !listening;
//
//			if (change_mode)
//				change_mode = false;
//		}
//		else if (cheat::game::get_key_press(2) && !listening)
//		{
//			change_mode = !change_mode;
//		}
//	}
//
//	if (listening && (cheat::game::get_key_press(13) || (!mouse_in_pos(menu_pos, max_menu_pos) && mouse1_pressed))) {
//		listening = false;
//	}
//
//	static int key = 0;
//
//	static std::string text = "";
//
//	if (listening)
//		get_key(listening, key);
//
//	if (change_mode && !listening)
//	{
//		int lol = 0;
//		draw_box(max_menu_pos.x, menu_pos.y, 60, 11, 1, 65, 65, 65, 255);
//		fill_rgba(max_menu_pos.x, menu_pos.y, 60, 11, 36, 36, 36, 255);
//		draw_quad(max_menu_pos.x, menu_pos.y, max_menu_pos.x + 60, menu_pos.y + 12 * (types.size() + 1), 255, 255, 255, 50);
//
//		draw_string(xor_string("type:"), max_menu_pos.x + 5 + get_text_w(xor_string("type:"), cheat::features::directx::dx_font_verdana) / 2, menu_pos.y, 255, 255, 255, cheat::features::directx::dx_font_verdana);
//
//		for (auto string : types)
//		{
//			//auto text_size = get_text_w(text.c_str(), cheat::features::directx::dx_font_verdana);
//
//			auto min_pos = Vector(max_menu_pos.x, max_menu_pos.y + 11 * lol, 0), max_pos = max_menu_pos + Vector(58, 11 * (lol + 1) - 1, 0);
//
//			//draw_quad(min_pos.x, min_pos.y, max_pos.x, max_pos.y, 255, 0, 0, 255);
//
//			if (mouse_in_pos(min_pos, max_pos) && cheat::game::get_key_press(1, 2)) {
//				//if (cheat::game::get_key_press(1, 2)/*max_menu_pos + Vector(-1, 11 * lol , 0), max_menu_pos + Vector(45, 11 * (lol+1), 0))*/) {
//				var[1] = lol;
//				pressed_smth_this_tick = true;
//				//change_mode = false;
//				//}
//			}
//
//			draw_string(string.c_str(), max_menu_pos.x + 3, menu_pos.y + 11 * ++lol, 255, (var[1] == lol - 1) ? 130 : 255, (var[1] == lol - 1) ? 0 : 255, cheat::features::directx::dx_font_verdana);
//		}
//	}
//
//	if (!pressed_smth_this_tick && (cheat::game::get_key_press(2) || cheat::game::get_key_press(1, 2)) && !mouse_in_pos(Vector(max_menu_pos.x, contents_posY + (16 * n), 0), max_menu_pos + Vector(48, 11 * types.size(), 0)) && change_mode)
//		change_mode = false;
//
//
//	if (key != old_key)
//	{
//		value_changed = true;
//
//		old_key = key;
//	}
//
//	if (listening) {
//		text = xor_string("press key");
//		var[0] = 0;
//	}
//	else if (key == 0)
//	{
//		text = xor_string("no key");
//		var[0] = 0;
//	}
//	else if (key == 18)
//	{
//		text = xor_string("alt");
//		var[0] = key;
//	}
//	else {
//		text = KeyMapArray[key];
//		var[0] = key;
//	}
//
//	float text_size = get_text_w(text.c_str(), cheat::features::directx::dx_font_verdana);
//
//	static std::string end_text = "";
//
//	if (line != 1) {
//		draw_box(menu_pos.x, menu_pos.y, text_size + 5, 11, 1, 65, 65, 65, alpha);
//		fill_rgba(menu_pos.x, menu_pos.y, text_size + 5, 11, 36, 36, 36, alpha);
//		draw_quad(menu_pos.x, menu_pos.y, menu_pos.x + text_size + 5, menu_pos.y + 11, 255, 255, 255, 50);
//		end_text = text;
//	}
//	else
//	{
//		end_text = xor_string("[") + text + xor_string("]");
//	}
//
//	draw_string(end_text.c_str(), menu_pos.x + 3, menu_pos.y, 180, 180, 180, cheat::features::directx::dx_font_pixel);
//
//	n += 1;
//
//	return value_changed;
//}
//
bool c_drawhack::menubuttons(int &n, std::vector < std::string > items, int &var , bool parent, int adjustY)
{
	bool value_changed = false;
	//Vector menu_pos = Vector(contents_posX, contents_posY + (parent ? 12 : 5) + tab * 36, 0);
	if (ignore_items < (n + line * 50)) {
		for (auto i = 0; i < items.size(); i++) {
			auto text_size = Drawing::GetTextSize(F::Menu, items.at(i).c_str());

			auto maxsize = (180 / items.size());

			Vector menu_pos = Vector(contents_posX + 5 + maxsize * i, contents_posY + (parent ? 7 : 0) + (14 * n) + adjustY, 0);
			if (line != 0) menu_pos.x += 200 * line;
			Vector max_menu_pos = Vector(menu_pos.x + maxsize, menu_pos.y + 14, 0);

			auto hovered = mouse_in_pos(menu_pos, max_menu_pos);

			//if (var == i)
			//	Drawing::DrawRect(menu_pos.x, menu_pos.y, maxsize, 14, main_color_fade.alpha(0.15 * alpha));
			//else if (hovered)
			//	Drawing::DrawRect(menu_pos.x, menu_pos.y, maxsize, 14, Color(192, 192, 192, 0.15 * alpha));

			Drawing::DrawString(F::Menu, menu_pos.x + maxsize / 2 - text_size.right / 2, menu_pos.y, (hovered || var == i) ? Color::White(alpha* 0.7) : Color::White(0.3 * alpha), FONT_LEFT, items.at(i).c_str());

			if (hovered && mouse1_pressed) {
				var = i;
				value_changed = true;
			}
		}
	}

	n++;

	return value_changed;
}

void c_drawhack::menu_slider(int &n, const char* name, float min, float max, float &value, bool parent, const int &display_decimal, const char* mark)
{
	n++;
	Vector menu_pos = Vector(contents_posX + 5, contents_posY + (parent ? 7 : 0) + (14 * n), 0);
	if (line != 0) menu_pos.x += 200 * line;
	Vector max_menu_pos = Vector(menu_pos.x + 180, menu_pos.y + 14, 0);
	if (ignore_items < (n - 1 + line * 50)) {
		//g_Drawing.MenuStringNormal(false, true, columns[column] + (tabs[3].w / 2) + tabs[2].w - 43, itemheight[column] + 7, Color(255, 255, 255, 200), "%s: %.1f", name, value);

		std::string noob = "%.1f";

		if (display_decimal <= 9)
			noob[2] = std::to_string(display_decimal).c_str()[0];

		if ((max - min) == 0)
			max += 0.001f;

		char text[255]; sprintf(text, noob.c_str(), value);

		if (strlen(mark) > 0)
			sprintf(text, "%s %s", text, mark);

		if (value > max) value = max;
		if (value < min) value = min;

		if (mouse_in_pos(menu_pos, max_menu_pos) && cheat::game::pressed_keys[1] && !save_pos)
		{
			value = min + (((min(max(_cursor_position.x, menu_pos.x + 2), menu_pos.x + 180) - menu_pos.x + 2) / 180) * (max - min));//(_cursor_position.x - menu_pos.x) * (max - min) / (180 - 7);

			if (value > max) value = max;
			if (value < min) value = min;
		}

		int val = ((value - min) * 180 / (max - min));

		float text_size = Drawing::GetTextSize(F::Menu, text).right;

		Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y, 180, 13, Color(65, 65, 65, alpha));
		Drawing::DrawRect(menu_pos.x, menu_pos.y, 180, 13, Color(45, 45, 45, alpha));

		if (val > 0)
			Drawing::DrawRectGradientVertical(menu_pos.x + 2, menu_pos.y + 2, val - 4, 10, main_color_fade.alpha(alpha), main_color.alpha(alpha));

		/*int col = 80;
		g_Drawing.FilledRect(x, itemheight[column], width, 1, Color(col, col, col, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 1, width, 1, Color(col - 2, col - 2, col - 2, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 2, width, 1, Color(col - 4, col - 4, col - 4, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 3, width, 1, Color(col - 8, col - 8, col - 8, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 4, width, 1, Color(col - 12, col - 12, col - 12, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 5, width, 1, Color(col - 14, col - 14, col - 14, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 6, width, 9, Color(col - 16, col - 16, col - 16, 255));

		g_Drawing.GradientHorizontal(x, itemheight[column], val, barheight, Colors.maincolor, Colors.maincolorfade, 128);

		g_Drawing.OutlinedRect(x, itemheight[column], width, barheight, Color(51, 51, 51, 255));
		g_Drawing.FilledRect(x, itemheight[column], width - 1, 1, Color(91, 91, 91, 255));
		g_Drawing.FilledRect(x, itemheight[column], 1, 14, Color(91, 91, 91, 255));*/

		Drawing::DrawString(F::Menu, menu_pos.x + (180 + 1) / 2 - text_size / 2, menu_pos.y+1, Color::White(alpha* 0.7), FONT_LEFT, text);

		Drawing::DrawString(F::Menu, menu_pos.x /*+ 110 + 2*/, menu_pos.y - 14, Color::White(alpha* 0.7), FONT_LEFT, name);
	}

	n++;
}

void c_drawhack::menu_slider(int &n, const char* name, int min, int max, int &value, bool parent, const char* mark, bool if_zero_display_text, const char* ntext)
{
	n++;

	if (ignore_items < (n - 1 + line * 50)) {
		Vector menu_pos = Vector(contents_posX + 5, contents_posY + (parent ? 7 : 0) + (14 * n), 0);
		if (line != 0) menu_pos.x += 200 * line;
		Vector max_menu_pos = Vector(menu_pos.x + 180, menu_pos.y + 14, 0);

		auto hover = mouse_in_pos(menu_pos, max_menu_pos);
		//g_Drawing.MenuStringNormal(false, true, columns[column] + (tabs[3].w / 2) + tabs[2].w - 43, itemheight[column] + 7, Color(255, 255, 255, 200), "%s: %.1f", name, value);

		char text[255]; sprintf(text, "%i %s", value, mark);

		if (value > max) value = max;
		if (value < min) value = min;

		if (hover && cheat::game::pressed_keys[1] && !save_pos)
		{
			value = min + (((min(max(_cursor_position.x, menu_pos.x), menu_pos.x + 176) - menu_pos.x) / 176) * (max - min));//(_cursor_position.x - menu_pos.x) * (max - min) / (180 - 7);

			if (value > max) value = max;
			if (value < min) value = min;
		}

		//int valueX = menu_pos.x + ((value - min) * 180 / (max - min));
		//int val = valueX - menu_pos.x;
		int val = ((value - min) * 176 / (max - min));

		float text_size = Drawing::GetTextSize(F::Menu, text).right;

		Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y, 180, 13, Color(65, 65, 65, alpha));
		Drawing::DrawRect(menu_pos.x, menu_pos.y, 180, 13, Color(45, 45, 45, alpha));

		if (val > 0)
			Drawing::DrawRectGradientVertical(menu_pos.x + 2, menu_pos.y + 2, val, 10, main_color_fade.alpha(alpha), main_color.alpha(alpha));

		Drawing::DrawOutlinedRect(menu_pos.x, menu_pos.y, 180, 14, hover ? main_color.alpha(0.78431372549 * alpha) : Color(30, 30, 30, 0.78431372549 * alpha));

		/*int col = 80;
		g_Drawing.FilledRect(x, itemheight[column], width, 1, Color(col, col, col, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 1, width, 1, Color(col - 2, col - 2, col - 2, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 2, width, 1, Color(col - 4, col - 4, col - 4, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 3, width, 1, Color(col - 8, col - 8, col - 8, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 4, width, 1, Color(col - 12, col - 12, col - 12, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 5, width, 1, Color(col - 14, col - 14, col - 14, 255));
		g_Drawing.FilledRect(x, itemheight[column] + 6, width, 9, Color(col - 16, col - 16, col - 16, 255));

		g_Drawing.GradientHorizontal(x, itemheight[column], val, barheight, Colors.maincolor, Colors.maincolorfade, 128);

		g_Drawing.OutlinedRect(x, itemheight[column], width, barheight, Color(51, 51, 51, 255));
		g_Drawing.FilledRect(x, itemheight[column], width - 1, 1, Color(91, 91, 91, 255));
		g_Drawing.FilledRect(x, itemheight[column], 1, 14, Color(91, 91, 91, 255));*/

		if (if_zero_display_text && val == 0)
			strcpy(text, ntext);

		Drawing::DrawString(F::Menu, menu_pos.x + (180 + 1) / 2 - text_size / 2, menu_pos.y + 1, Color::White(alpha* 0.7), FONT_OUTLINE, text);

		Drawing::DrawString(F::Menu, menu_pos.x /*+ 110 + 2*/, menu_pos.y - 14, Color::White(alpha* 0.7), FONT_LEFT, name);
	}

	n++;
}
//
//void c_drawhack::change_line(int _line, int& i, int& old)
//{
//	line = _line;
//	if (line == 0)
//	{
//		i = old;
//		old = i;
//	}
//	else {
//		old = i;
//		i = 0;
//	}
//}
bool was_moved;

void c_drawhack::draw_gui()
{
	auto draw_quad = [](int x1, int y1, int x2, int y2, Color rgba) -> void
	{
		Drawing::DrawLine(x1, y1, x2, y1, rgba);
		Drawing::DrawLine(x2, y1, x2, y2, rgba);
		Drawing::DrawLine(x2, y2, x1, y2, rgba);
		Drawing::DrawLine(x1, y2, x1, y1, rgba);
	};

	ignore_items = -1;

	if (alpha < 10) return;

	int i = 0, io[3] = { 0,0,0 }; static int curtab = 0; // i don't remember where i saw this system but i liked it :D

	contents_posX = menu_posX;
	contents_posY = menu_posY;

	Drawing::DrawRect(menu_posX - 16, menu_posY - 16, width_menu + 10 + 11, height_menu + 20 + 11, Color(28, 28, 28, alpha - 10));
	//Drawing::DrawRectGradientHorizontal(menu_posX - 9, menu_posY - 9, width_menu + 9, 2, Color(255, 0, 255, alpha), Color(0, 255, 255, alpha));

	//Drawing::DrawRect(menu_posX - 16, menu_posY - 51, width_menu + 10 + 11, 31, Color(15, 15, 15, alpha - 20));

	//Drawing::DrawString(F::Against, menu_posX, menu_posY - 51 / 2 - 32 / 2 - 12, Color::White(alpha* 0.7), FONT_LEFT, "gangster");

	//// draws the window border (code looks gay as fuck but looks nice in-game)
	//draw_quad(menu_posX - 16, menu_posY - 16 - 35, menu_posX + width_menu + 5, menu_posY - 35 + 15, Color(0, 0, 0, alpha));
	//draw_quad(menu_posX - 15, menu_posY - 15 - 35, menu_posX + width_menu + 4, menu_posY - 35 + 14, Color(60, 60, 60, alpha));
	//draw_quad(menu_posX - 14, menu_posY - 14 - 35, menu_posX + width_menu + 3, menu_posY - 35 + 13, Color(40, 40, 40, alpha));
	//draw_quad(menu_posX - 13, menu_posY - 13 - 35, menu_posX + width_menu + 2, menu_posY - 35 + 12, Color(40, 40, 40, alpha));
	//draw_quad(menu_posX - 12, menu_posY - 12 - 35, menu_posX + width_menu + 1, menu_posY - 35 + 11, Color(40, 40, 40, alpha));
	//draw_quad(menu_posX - 11, menu_posY - 11 - 35, menu_posX + width_menu, menu_posY - 35 + 10, Color(60, 60, 60, alpha));
	//// draws the window border (code looks gay as fuck but looks nice in-game)

	Drawing::DrawRectGradientVertical(menu_posX - 16, menu_posY - 16, width_menu + 10 + 11, 62.5, Color(36, 36, 36, alpha - 10), Color(43, 43, 43, alpha - 10));

	// draws the window border (code looks gay as fuck but looks nice in-game)
	draw_quad(menu_posX - 16, menu_posY - 16, menu_posX + width_menu + 5, menu_posY + height_menu + 15, Color(53, 53, 53, alpha - 5));
	/*draw_quad(menu_posX - 15, menu_posY - 15, menu_posX + width_menu + 4, menu_posY + height_menu + 14, Color(60, 60, 60, alpha));
	draw_quad(menu_posX - 14, menu_posY - 14, menu_posX + width_menu + 3, menu_posY + height_menu + 13, Color(40, 40, 40, alpha));
	draw_quad(menu_posX - 13, menu_posY - 13, menu_posX + width_menu + 2, menu_posY + height_menu + 12, Color(40, 40, 40, alpha));
	draw_quad(menu_posX - 12, menu_posY - 12, menu_posX + width_menu + 1, menu_posY + height_menu + 11, Color(40, 40, 40, alpha));
	draw_quad(menu_posX - 11, menu_posY - 11, menu_posX + width_menu, menu_posY + height_menu + 10, Color(60, 60, 60, alpha));*/
	// draws the window border (code looks gay as fuck but looks nice in-game)

	Drawing::DrawString(F::EagleLogo, menu_posX + width_menu / 2, menu_posY - 10, Color(218, 165, 32, alpha), FONT_CENTER, "a");

	static std::vector<std::string> tabs = { "b;Rage Bot",  "c;Visuals",  "i;Misc"/*, "ANTI-AIM"*/ };
	//static std::vector<std::string> aimbot_main_buttons = { "main",  "accuracy",  "hitscan" };
	//static std::vector<std::string> visuals_wm_main_buttons = { "effects",  "removals" };
	//static std::vector<std::string> visuals_pl_main_buttons = { "esp", "esp colors",  "models" };
	//static std::vector<std::string> antiaim_main_buttons = { "move",  "still/air",  "fake",  "misc" };
	//static std::vector<std::string> delay_shot_options = { "none",  "adaptive", "unlag" };
	//static std::vector<std::string> desync_walk_options = { "none",  "min walk", "slow walk" };
	//static std::vector<std::string> direction_options = { "none",  "auto", "target" };

	//static std::vector<std::string> anti_aim_pitch_variants = { "none", "down", "adaptive" };
	//static std::vector<std::string> anti_aim_yaw_variants = { "none", "backward", "static", "crooked", "spin" };
	//static std::vector<std::string> anti_aim_desync_variations = { "none", "tranquil", "balance", "strench", "deviance" };
	//static std::vector<std::string> anti_aim_desync_range = { "none", "adaptive", "static" };
	//static std::vector<std::string> anti_aim_resolver_modes = { "none", "brute", "lean", "velocity lean", "torso" };

	static std::vector<MultiSelectable> hitscan_items = {};
	static std::vector<MultiSelectable> pointscan_items = {};
	static std::vector<MultiSelectable> bodyaim_if_items = {};
	//static std::vector<MultiSelectable> headaim_if_items = {};
	//static std::vector<MultiSelectable> fakelag_triggers_items = {};

	/*if (hitscan_items.empty())
	{
		hitscan_items.emplace_back(MultiSelectable("head", &cheat::settings.ragebot_hitscan[0]));
		hitscan_items.emplace_back(MultiSelectable("arms", &cheat::settings.ragebot_hitscan[1]));
		hitscan_items.emplace_back(MultiSelectable("body", &cheat::settings.ragebot_hitscan[2]));
		hitscan_items.emplace_back(MultiSelectable("legs", &cheat::settings.ragebot_hitscan[3]));
		hitscan_items.emplace_back(MultiSelectable("foot", &cheat::settings.ragebot_hitscan[4]));
	}

	if (pointscan_items.empty())
	{
		pointscan_items.emplace_back(MultiSelectable("head", &cheat::settings.ragebot_pointscan[0]));
		pointscan_items.emplace_back(MultiSelectable("arms", &cheat::settings.ragebot_pointscan[1]));
		pointscan_items.emplace_back(MultiSelectable("body", &cheat::settings.ragebot_pointscan[2]));
		pointscan_items.emplace_back(MultiSelectable("legs", &cheat::settings.ragebot_pointscan[3]));
		pointscan_items.emplace_back(MultiSelectable("foot", &cheat::settings.ragebot_pointscan[4]));
	}*/

	//if (bodyaim_if_items.empty())
	//{
	//	bodyaim_if_items.emplace_back(MultiSelectable("jumping", &cheat::settings.ragebot_hs_baim_preferences[0]));
	//	//bodyaim_if_items.emplace_back(MultiSelectable("spread", &cheat::settings.ragebot_hs_baim_preferences[1]));
	//	bodyaim_if_items.emplace_back(MultiSelectable("fake", &cheat::settings.ragebot_hs_baim_preferences[2]));
	//	//bodyaim_if_items.emplace_back(MultiSelectable("distance", &cheat::settings.ragebot_hs_baim_preferences[3]));
	//	//bodyaim_if_items.emplace_back(MultiSelectable("unplugged", &cheat::settings.ragebot_hs_baim_preferences[4]));
	//}

	/*if (headaim_if_items.empty())
	{
		headaim_if_items.emplace_back(MultiSelectable("lby update", &cheat::settings.ragebot_hs_headaim_preferences[0]));
		headaim_if_items.emplace_back(MultiSelectable("shot update", &cheat::settings.ragebot_hs_headaim_preferences[1]));
		headaim_if_items.emplace_back(MultiSelectable("high speed", &cheat::settings.ragebot_hs_headaim_preferences[2]));
	}*/

	/*if (fakelag_triggers_items.empty())
	{
		fakelag_triggers_items.emplace_back(MultiSelectable("lby update", &cheat::settings.ragebot_hs_headaim_preferences[0]));
		fakelag_triggers_items.emplace_back(MultiSelectable("shot update", &cheat::settings.ragebot_hs_headaim_preferences[1]));
		fakelag_triggers_items.emplace_back(MultiSelectable("high speed", &cheat::settings.ragebot_hs_headaim_preferences[2]));
	}*/

	int tabsiz = (width_menu / tabs.size()) / 5;

	for (auto x = 0; x < (int)tabs.size(); x++)
	{
		if (menutab(tabsiz, tabs.at(x), x == curtab, 0))
			curtab = x;
	}

	line = 0;

	contents_posY += 125;

	//static bool paste[6] = { false,false,false,false,false };

	//mouse1_pressed = mouse1_pressed;
	switch (curtab)
	{
	case 0:
	{
		menugroupbox(i, 26, "aimbot");
		static int aimbot_main_subtab = 0;
		menubuttons(i, { "main",  "accuracy",  "hitscan" }, aimbot_main_subtab, true);
		if (aimbot_main_subtab == 0)
		{
			/*menuitem(i, "master switch", cheat::settings.ragebot_enabled, true);
			menuitem(i, "silent aim", cheat::settings.ragebot_lagshoot, true);
			menuitem(i, "remove inaccuracy", cheat::settings.ragebot_removals, true);
			menuitem(i, "auto revolver", cheat::settings.ragebot_autor8, true);
			menu_slider(i, "minimum penetration damage", 0, 100, cheat::settings.ragebot_min_damage, true, "hp");
			menuitem(i, "scale penetration dmg on hp", cheat::settings.ragebot_scale_damage_on_hp, true);
			menuitem(i, "scope automatically", cheat::settings.ragebot_autoscope, true);*/
		}
		else if (aimbot_main_subtab == 1)
		{
			//menuitem(i, "resolver", cheat::settings.ragebot_resolver, true);
			//menuitem(i, "jitter correction", cheat::settings.resolver_autoresolve, true);
			//menuitem(i, "freestand resolver", cheat::settings.resolver_freestand, true);
			//menuitem(i, "ML resolver", cheat::settings.resolver_lastmovelby, true);
			//menuitem(i, "adjust positions", cheat::settings.ragebot_lagcomp, true);
			//combobox(i, "resolver", delay_shot_options, cheat::settings.ragebot_resolver, true);

			//if (cheat::settings.ragebot_lagcomp)
			//	menuitem(i, "fix lag", cheat::settings.ragebot_lagfix, true);
			//menuitem(i, "interpolate", cheat::settings.ragebot_backshot, true);

			//menu_slider(i, "minimum hit chance", 0, 100, cheat::settings.ragebot_hitchance, true);
			//menu_slider(i, "fakeduck hit chance", 0, 100, cheat::settings.ragebot_hitchance_fakeduck, true);
			//combobox(i, "delay shot", { "none",  "adaptive", "unlag" }, cheat::settings.ragebot_delay_shot, true);
			//if (!combo_opened[i - 1])
			//	menu_slider(i, "minimum hit chance", 0, 100, cheat::settings.ragebot_hitchance, true);
			//else
			//	i++;
		}
		else
		{
			////menuitem(i, "point scan", cheat::settings.ragebot_hitscan, true);

			//multicombo(i, "hitboxes to scan", hitscan_items, true);

			//multicombo(i, "points to scan", pointscan_items, true);
			//combobox(i, "prefer baim", { "never", "lethal", "adaptive" }, cheat::settings.ragebot_hs_baim_preference, true);
			//multicombo(i, "baim when", bodyaim_if_items, true);
			////multicombo(i, "prefer headaim when", headaim_if_items, true);

			//menuitem(i, "auto awp", cheat::settings.ragebot_hs_baim_awp, true);

			//menu_slider(i, "bodyaim trigger", 0, 100, cheat::settings.ragebot_hs_baim_hp, true, "hp");

			//menu_slider(i, "head multipoint", 0, 100, cheat::settings.ragebot_pointscale_head, true, "");
			//menu_slider(i, "body multipoint", 0, 100, cheat::settings.ragebot_pointscale_body, true, "");
		}
		//menuitem(i, "Select by hp", cheat::settings.ragebot_select_by_hp, true);
		//menuitem(i, "Select unlag", cheat::settings.ragebot_select_unlag, true);

		io[line] = i + 2;
		i = 0;
		line = 1;

		//menugroupbox(i, 26, "anti-aim settings");
		//menuitem(i, "master switch", cheat::settings.ragebot_anti_aim_enabled, true);
		//combobox(i, "pitch", { "none", "down", "adaptive" }, cheat::settings.ragebot_anti_aim_pitch, true);
		//i++;
		//separator(i, "anti-aim yaw");
		//static int antiaim_main_subtab = 0;
		//menubuttons(i, { "move",  "still/air",  "fake",  "misc" }, antiaim_main_subtab, true, -8);

		//if (antiaim_main_subtab < 2)
		//{
		//	combobox(i, "yaw", { "none", "backward", "static", "crooked", "spin" }, cheat::settings.ragebot_anti_aim_settings[antiaim_main_subtab].anti_aim_yaw, true);
		//	//combobox(i, "jitter mode", anti_aim_jitter_variations, cheat::settings.ragebot_anti_aim_settings[antiaim_main_subtab].ragebot_anti_aim_jitter_mode, true);

		//	//menuitem(i, "randomize", cheat::settings.ragebot_anti_aim_settings[antiaim_main_subtab].ragebot_anti_aim_jitter_randomize, true);
		//	//menu_slider(i, "jitter range", 0, 180, cheat::settings.ragebot_anti_aim_settings[antiaim_main_subtab].ragebot_anti_aim_jitter_range, true, u8"°");
		//}
		//else if (antiaim_main_subtab == 2)
		//{
		//	combobox(i, "fake mode", { "none", "balance", "static" }, cheat::settings.ragebot_anti_aim_settings[antiaim_main_subtab].desync_mode, true);
		//	menu_slider(i, "desync range", 0, 65, cheat::settings.ragebot_anti_aim_settings[antiaim_main_subtab].desync_range, true, u8"°", true, "auto");

		//	if (cheat::settings.ragebot_anti_aim_settings[antiaim_main_subtab].desync_range > 1)
		//		menuitem(i, "randomize", cheat::settings.ragebot_anti_aim_settings[antiaim_main_subtab].desync_randomize, true);
		//}
		//else
		//{
		//	//menuitem(i, "break lby", cheat::settings.ragebot_anti_aim_break_lby, true);
		//	combobox(i, "direction", { "none",  "auto", "target" }, cheat::settings.ragebot_anti_aim_direction, true);
		//	combobox(i, "desyncwalk mode", { "none",  "min walk", "slow walk" }, cheat::settings.ragebot_anti_aim_desync_walk, true);
		//	menu_slider(i, "desyncwalk speed", 1, 100, cheat::settings.ragebot_anti_aim_desync_walk_speed, true, u8"°");
		//}

		//menu_slider(i, "desync angle", 1, 180, cheat::settings.ragebot_anti_aim_lby_delta, true, u8"°");

		io[line] = i + 2;
		i = 0;
		line = 2;

		menugroupbox(i, 26, "anti-aim options");
		/*menuitem(i, "fakelag enabled", cheat::settings.ragebot_lag, true);
		combobox(i, "base factor", { "maximum","adaptive", "jitter", "fluctuate" }, cheat::settings.misc_fakelag_base_factor, true);
		combobox(i, "when", { "always", "in air","on ground","while moving" }, cheat::settings.misc_fakelag_trigger, true);
		menu_slider(i, "limit", 1.f, 16.f, cheat::settings.misc_fakelag_value, true, 2, "ticks");
		menu_slider(i, "variance", 0.f, 100.f, cheat::settings.misc_fakelag_variance, true, 2);*/
		//menuitem(i, "Use LMBY", cheat::settings.resolver_lastmovelby, true);
		//menuitem(i, "Use FS", cheat::settings.resolver_freestand, true);
		break;
	}
	case 1:
	{
		menugroupbox(i, 26, "world modifications");
		static int visuals_wm_subtab = 0;
		menubuttons(i, { "effects",  "removals" }, visuals_wm_subtab, true);
		if (visuals_wm_subtab == 0)
		{
		/*	menuitem(i, "disable post process", cheat::settings.visuals_effects_no_post_process, true);
			menuitem(i, "night mode", cheat::settings.visuals_effects_nightmode, true);
			menu_slider(i, "props transparency", 0.f, 100.f, cheat::settings.visuals_effects_misc_props_modifier, true, 3);*/
		}
		else
		{
			/*menuitem(i, "remove smoke", cheat::settings.visuals_removals_smoke, true);
			menuitem(i, "remove flash", cheat::settings.visuals_removals_flash, true);
			menuitem(i, "remove scope", cheat::settings.visuals_removals_scope, true);
			menuitem(i, "remove punch", cheat::settings.visuals_removals_punch, true);*/
		}

		io[line] = i + 2;
		i = 0;
		line = 1;

		menugroupbox(i, 26, "players");
		static int visuals_pl_subtab = 0;
		menubuttons(i, { "esp", "esp colors",  "models" }, visuals_pl_subtab, true);
		if (visuals_pl_subtab == 0)
		{
			//menuitem(i, "enabled", cheat::settings.esp_enabled, true);
			//menuitem(i, "teammates", cheat::settings.esp_teammates, true);
			////menuitem(i, "bounding box", cheat::settings.esp_box, true);
			//combobox(i, "bounding box", { "none","static", "dynamic" }, cheat::settings.esp_box, true);
			//combobox(i, "history skeleton", { "none","basic", "history" }, cheat::settings.esp_skeleton, true);
			//menuitem(i, "names", cheat::settings.esp_name, true);
			//menuitem(i, "health bar", cheat::settings.esp_health, true);
			//menuitem(i, "weapons", cheat::settings.esp_weapon, true);
			//menuitem(i, "snap lines", cheat::settings.esp_snaplines, true);
			//menuitem(i, "glow", cheat::settings.glow_enabled, true);
			//menuitem(i, "out of pov arrows", cheat::settings.esp_pov_arrows, true);
			////menuitem(i, "split health", cheat::settings.esp_split_health, true);
		}
		else if (visuals_pl_subtab == 1)
		{
			//colorpicker(i, "bounding box color", cheat::settings.esp_color, true);
			//colorpicker(i, "skeletons color", cheat::settings.skeleton_color, true);
			//colorpicker(i, "snap lines color", cheat::settings.snap_lines_color, true);
			//colorpicker(i, "glow color", cheat::settings.glow_color, true);
			//colorpicker(i, "map color", cheat::settings.map_color, true);
			//colorpicker(i, "sky color", cheat::settings.sky_color, true);
			//colorpicker(i, "hidden chams color", cheat::settings.hidden_chams_color, true);
		}
		else
		{
			/*menuitem(i, "teammates", cheat::settings.chams_teammates, true);
			combobox(i, "player chams", { "none","basic", "basic + history" }, cheat::settings.chams_enabled, true);
			colorpicker(i, "player color", cheat::settings.chams_color, true);
			menuitem(i, "while hidden", cheat::settings.chams_hidden, true);
			if (cheat::settings.chams_hidden)
				colorpicker(i, "player (hidden) color", cheat::settings.hidden_chams_color, true);*/
		}

		io[line] = i + 2;
		i = 0;
		line = 2;

		menugroupbox(i, 26, "misc");
		static int visuals_msc_subtab = 0;
		menubuttons(i, { "part 1",  "part 2" }, visuals_msc_subtab, true);
		if (visuals_msc_subtab == 0)
		{
			/*combobox(i, "crosshair", { "none", "basic", "autowall" }, cheat::settings.visuals_misc_crosshair, true);
			combobox(i, "show weapon spread", { "none", "always", "scoped" }, cheat::settings.visuals_misc_spread_visualize, true);
			menuitem(i, "force killfeed", cheat::settings.visuals_misc_preserve_killfeed, true);
			menuitem(i, "force thirdperson", cheat::settings.visuals_misc_thirdperson, true);
			menu_slider(i, "thirdperson distance", 30.f, 1000.f, cheat::settings.visuals_misc_thirdperson_dist, true, 0, u8"°");
			menuitem(i, "force thirdperson [grenade]", cheat::settings.visuals_misc_thirdperson_grenade, true);
			menuitem(i, "grenade trajectory", cheat::settings.visuals_misc_grenade_trajectory, true);
			menuitem(i, "clan tag", cheat::settings.misc_ctag, true);*/
		}
		else
		{
			/*menuitem(i, "manual aa indicator", cheat::settings.visuals_misc2_manual_indicator, true);
			menuitem(i, "desync indicator", cheat::settings.visuals_misc2_desync_indicator, true);
			menuitem(i, "fakelag indicator", cheat::settings.visuals_misc2_fakelag_indicator, true);
			menuitem(i, "event log", cheat::settings.visuals_misc2_event_log, true);*/
			//menu_slider(i, "viewmodel fov", 0.f, 90.f, cheat::settings.visuals_misc_viewmodel_fov, true, 0, u8"°");
			//menu_slider(i, "viewmodel x", -10.f, 10.f, cheat::settings.visuals_misc_viewmodel[0], true, 1);
			//menu_slider(i, "viewmodel y", -10.f, 10.f, cheat::settings.visuals_misc_viewmodel[1], true, 1);
			//menu_slider(i, "viewmodel z", -10.f, 10.f, cheat::settings.visuals_misc_viewmodel[2], true, 1);
		}
		//menu_slider(i, "map light", 0.f, 1.f, cheat::settings.visuals_misc_map_modifier, true, 3);
		//menu_slider(i, "viewmodel fov", 0.f, 90.f, cheat::settings.visuals_misc_viewmodel_fov, true, 0, u8"°");
		break;
	}
	case 2:
	{
		menugroupbox(i, 26, "local player");
		static int misc_subtab = 0;
		menubuttons(i, { "movement", "lag",  "safety" }, misc_subtab, true);
		//if (misc_subtab == 0) {
		//	menuitem(i, "bhop", cheat::settings.misc_bhop, true);
		//	menuitem(i, "strafe", cheat::settings.misc_strafer, true);
		//	menuitem(i, "circle strafe", cheat::settings.misc_cstrafer, true);
		//}
		//else if (misc_subtab == 1)
		//{ 
		//	//menuitem(i, "fakelag enabled", cheat::settings.ragebot_lag, true);
		//	combobox(i, "base factor", { "maximum","adaptive", "jitter", "fluctuate" }, cheat::settings.misc_fakelag_base_factor, true);
		//	combobox(i, "when", { "always", "in air","on ground","while moving" }, cheat::settings.misc_fakelag_trigger, true);
		//	menu_slider(i, "limit", 1.f, 16.f, cheat::settings.misc_fakelag_value, true, 2, "ticks");
		//	menu_slider(i, "variance", 0.f, 100.f, cheat::settings.misc_fakelag_variance, true, 2);
		//}
		//else
		//{

		//}

		if (cheat::features::music.m_sound_files.size() > 0)
			combobox(i, "music list", cheat::features::music.m_sound_files, cheat::settings.music_curtrack, true);

		break;
	}
	case 3:
	{
		//menugroupbox(i, 26, "Switches");
		//menuitem(i, "Simulate Lags", cheat::settings.ragebot_lag, true);

		//i += 2;

		//menugroupbox(i, 26, "Sliders");
		//menu_slider(i, "FWalk Speed", 1, 15, cheat::settings.misc_fwalk_speed, true, "u");
		//menu_slider(i, "Lag Value", 1, 16, cheat::settings.misc_fakelag_value, true, "t");
		break;
	}
	default:
		break;
	}

	if (cheat::game::pressed_keys[1] && mouse_in_pos(Vector(menu_posX, menu_posY, 0), Vector(menu_posX + width_menu, menu_posY + 60, 0)) || was_moved)
	{
		if (save_pos == false)
		{
			saved_x = _cursor_position.x - menu_posX;
			saved_y = _cursor_position.y - menu_posY;
			save_pos = true;
		}
		menu_posX = _cursor_position.x;
		menu_posY = _cursor_position.y;
		menu_posX = menu_posX - saved_x;
		menu_posY = menu_posY - saved_y;
	}
	else
		save_pos = was_moved = false;

	if (!was_moved)
		was_moved = cheat::game::pressed_keys[1] && mouse_in_pos(Vector(menu_posX, menu_posY, 0), Vector(menu_posX + width_menu, menu_posY + 60, 0));
	else
		was_moved = cheat::game::pressed_keys[1];

	cheat::features::menu.mouse1_pressed = false;
}

void c_drawhack::render()
{
	//draw_string("gangster-paradise.net", 5, 0, 240, 240, 250, cheat::features::directx::dx_font_verdana);

	//if (cheat::settings._spectator_list)
	//	draw_speclist_box();

	int pX, pY;
	Source::m_pInputSystem->GetCursorPosition(&pX, &pY); 
	_cursor_position.x = (float)(pX);
	_cursor_position.y = (float)(pY);

	//if (menu_opened) {
	//	//GetConfigMassive();

	//	Drawing::DrawString(F::ESP, _cursor_position.x, _cursor_position.y, Color::White(), FONT_LEFT, "+");

	//	//draw_string("+", cheat::main::_cursor_position.x - 4, cheat::main::_cursor_position.y - 7, 255, 255, 255, cheat::features::directx::dx_font_verdana);

	//	if (alpha < 255)
	//		alpha += min(255 - alpha, 20);
	//}
	//else {
	//	if (alpha > 0)
	//		alpha -= min(alpha, 5);
	//}

	//if (alpha > 10)
	//	draw_gui();
}