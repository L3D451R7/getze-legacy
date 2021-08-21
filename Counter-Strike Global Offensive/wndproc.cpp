#include "sdk.hpp"
#include "hooked.hpp"
#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include "rmenu.hpp"
#include "menu.h"

WNDPROC Hooked::oldWindowProc;
void OpenMenu();

LRESULT __stdcall Hooked::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	bool lol = false;

	switch (uMsg) {
	case WM_LBUTTONDOWN:
		cheat::game::pressed_keys[VK_LBUTTON] = true;
		cheat::features::menu.mouse1_pressed = true;
		lol = true;
		break;
	case WM_LBUTTONUP:
		cheat::game::pressed_keys[VK_LBUTTON] = false;
		cheat::features::menu.mouse1_pressed = false;
		lol = true;
		break;
	case WM_RBUTTONDOWN:
		cheat::game::pressed_keys[VK_RBUTTON] = true;
		lol = true;
		break;
	case WM_RBUTTONUP:
		cheat::game::pressed_keys[VK_RBUTTON] = false;
		lol = true;
		break;
	case WM_MBUTTONDOWN:
		cheat::game::pressed_keys[VK_MBUTTON] = true;
		lol = true;
		break;
	case WM_MBUTTONUP:
		cheat::game::pressed_keys[VK_MBUTTON] = false;
		lol = true;
		break;
	case WM_MOUSEMOVE:
		cheat::features::menu._cursor_position.x = (signed short)(lParam);
		cheat::features::menu._cursor_position.y = (signed short)(lParam >> 16);
		lol = true;
		break;
	case WM_XBUTTONDOWN:
	{
		UINT button = GET_XBUTTON_WPARAM(wParam);
		if (button == XBUTTON1)
		{
			cheat::game::pressed_keys[VK_XBUTTON1] = true;
		}
		else if (button == XBUTTON2)
		{
			cheat::game::pressed_keys[VK_XBUTTON2] = true;
		}
		lol = true;
		break;
	}
	case WM_XBUTTONUP:
	{
		UINT button = GET_XBUTTON_WPARAM(wParam);
		if (button == XBUTTON1)
		{
			cheat::game::pressed_keys[VK_XBUTTON1] = false;
		}
		else if (button == XBUTTON2)
		{
			cheat::game::pressed_keys[VK_XBUTTON2] = false;
		}
		lol = true;
		break;
	}
	case WM_MOUSEWHEEL:
	{
		//DWORD x = GET_WHEEL_DELTA_WPARAM(wParam);

		cheat::features::menu.mwheel_value = ((short)HIWORD(wParam) < 0) ? -1 : 1;

		lol = true;
		break;
	}
	case WM_SYSKEYDOWN:
		cheat::game::pressed_keys[18] = true;
		break;
	case WM_SYSKEYUP:
		cheat::game::pressed_keys[18] = false;
		break;
	case WM_KEYDOWN:
		cheat::game::pressed_keys[wParam] = true;
		lol = true;
		break;
	case WM_KEYUP:
		cheat::game::pressed_keys[wParam] = false;
		lol = true;
		break;
	default: break;
	}

	OpenMenu();

	cheat::menu.HandleInput();

	if (cheat::features::menu.menu_opened && lol)
		return true;

	return CallWindowProc(oldWindowProc, hWnd, uMsg, wParam, lParam);
}

void OpenMenu()
{
	static bool is_down = false;
	static bool is_clicked = false;

	if (cheat::game::pressed_keys[VK_INSERT])
	{
		is_clicked = false;
		is_down = true;
	}
	else if (!cheat::game::pressed_keys[VK_INSERT] && is_down)
	{
		is_clicked = true;
		is_down = false;
	}
	else
	{
		is_clicked = false;
		is_down = false;
	}

	if (is_clicked)
	{
		Source::m_pInputSystem->EnableInput(cheat::features::menu.menu_opened);
		cheat::features::menu.menu_opened = !cheat::features::menu.menu_opened;
	}
}