#include "hooked.hpp"

vgui::HFont F::Menu;
vgui::HFont F::ESPInfo;
vgui::HFont F::NewESP;
vgui::HFont F::OldESP;
vgui::HFont F::LBY;
vgui::HFont F::Against;
vgui::HFont F::Weapons;
vgui::HFont F::Eagle;
vgui::HFont F::EagleLogo;
vgui::HFont F::EagleMisc;
vgui::HFont F::EagleTab;

vgui::HFont	Drawing::hFont;
vgui::HFont	Drawing::ESPFont;
vgui::HFont	Drawing::Eagle;
vgui::HFont	Drawing::mideagle;
vgui::HFont	Drawing::Tabfont;
vgui::HFont	Drawing::Checkmark;
vgui::HFont	Drawing::SegoeUI;
vgui::HFont	Drawing::MenuText;
vgui::HFont	Drawing::BackArrow;

void Drawing::CreateFonts()
{
	//SendMessage(FindWindowA("Valve001",0), WM_FONTCHANGE, 0, 0);

	//Source::m_pSurface->ResetFontCaches();

	// "Courier New", 14, 450, 0, 0, FONTFLAG_OUTLINE, 0, 0); <- STYLES FONT
	//Source::m_pSurface->SetFontGlyphSet(F::Menu = Source::m_pSurface->CreateFont_(), "Verdana", 12, 700, NULL, NULL, 16); // aimware menu
	Source::m_pSurface->SetFontGlyphSet(F::Menu = Source::m_pSurface->CreateFont_(), "Choktoff", 12, 300, NULL, NULL, FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS);
	Source::m_pSurface->SetFontGlyphSet(F::OldESP = Source::m_pSurface->CreateFont_(), "Verdana", 12, 700, NULL, NULL, 128); // aimware esp
	Source::m_pSurface->SetFontGlyphSet(F::ESPInfo = Source::m_pSurface->CreateFont_(), "hooge 05_55 Cyr2", 9, 100, NULL, NULL, FONTFLAG_OUTLINE);
	Source::m_pSurface->SetFontGlyphSet(F::NewESP = Source::m_pSurface->CreateFont_(), "Verdana", 12, 550, NULL, NULL, FONTFLAG_OUTLINE);//"Tahoma", 11, 500, 0, 0, 512);//"Visitor_Rus", 12, 400, NULL, NULL, FONTFLAG_OUTLINE/*FONTFLAG_DROPSHADOW*/);
	Source::m_pSurface->SetFontGlyphSet(F::LBY = Source::m_pSurface->CreateFont_(), "Verdana", 26, 700, NULL, NULL, 144);
	Source::m_pSurface->SetFontGlyphSet(F::Weapons = Source::m_pSurface->CreateFont_(), "undefeated", 12, 500, NULL, NULL, FONTFLAG_ANTIALIAS); // kms using undefeated fonts
	Source::m_pSurface->SetFontGlyphSet(F::Against = Source::m_pSurface->CreateFont_(), "Againts", 32, 550, NULL, NULL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW); // kms using bameware font
	Source::m_pSurface->SetFontGlyphSet(F::Eagle = Source::m_pSurface->CreateFont_(), "eagle", 36, 550, NULL, NULL, FONTFLAG_ANTIALIAS); 
	Source::m_pSurface->SetFontGlyphSet(F::EagleLogo = Source::m_pSurface->CreateFont_(), "eagle", 52, 550, NULL, NULL, FONTFLAG_ANTIALIAS);
	Source::m_pSurface->SetFontGlyphSet(F::EagleMisc = Source::m_pSurface->CreateFont_(), "eagle", 9, 100, NULL, NULL, FONTFLAG_ANTIALIAS);
	Source::m_pSurface->SetFontGlyphSet(F::EagleTab = Source::m_pSurface->CreateFont_(), "Tahoma", 16, 600, NULL, NULL, FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS);

	//Source::m_pSurface->SetFontGlyphSet(Drawing::hFont = Source::m_pSurface->CreateFont_(), "Choktoff", 12, FW_MEDIUM, NULL, NULL, FONTFLAG_ANTIALIAS);
	//Source::m_pSurface->SetFontGlyphSet(Drawing::ESPFont = Source::m_pSurface->CreateFont_(), "Tahoma", 12, FW_MEDIUM, NULL, NULL, FONTFLAG_DROPSHADOW);
	////Interfaces.Surface->SetFontGlyphSet(ESPFont, "Nasalization Rg", 10, FW_NORMAL, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	//Source::m_pSurface->SetFontGlyphSet(Drawing::Eagle = Source::m_pSurface->CreateFont_(), "eagle", 14, FW_MEDIUM, NULL, NULL, FONTFLAG_ANTIALIAS);
	//Source::m_pSurface->SetFontGlyphSet(Drawing::mideagle = Source::m_pSurface->CreateFont_(), "eagle", 30, FW_MEDIUM, NULL, NULL, FONTFLAG_ANTIALIAS);
	//Source::m_pSurface->SetFontGlyphSet(Drawing::Tabfont = Source::m_pSurface->CreateFont_(), "eagle", 42, FW_MEDIUM, NULL, NULL, FONTFLAG_ANTIALIAS);
	//Source::m_pSurface->SetFontGlyphSet(Drawing::Checkmark = Source::m_pSurface->CreateFont_(), "eagle", 10, FW_MEDIUM, NULL, NULL, FONTFLAG_ANTIALIAS);
	//Source::m_pSurface->SetFontGlyphSet(Drawing::BackArrow = Source::m_pSurface->CreateFont_(), "eagle", 18, FW_MEDIUM, NULL, NULL, FONTFLAG_ANTIALIAS);
	//Source::m_pSurface->SetFontGlyphSet(Drawing::SegoeUI = Source::m_pSurface->CreateFont_(), "Segoe UI", 16, FW_LIGHT, NULL, NULL, FONTFLAG_ANTIALIAS);
	////Interfaces.Surface->SetFontGlyphSet(LastTabsText, "Raleway", 8, FW_DONTCARE, 0, 0, FONTFLAG_ANTIALIAS);
	//Source::m_pSurface->SetFontGlyphSet(Drawing::MenuText = Source::m_pSurface->CreateFont_(), "Verdana", 12, FW_DONTCARE, NULL, NULL, FONTFLAG_ANTIALIAS);
	////Interfaces.Surface->SetFontGlyphSet(LegitTabs, "csgo_icons_outline", 12, FW_DONTCARE, 0, 0, FONTFLAG_ANTIALIAS);
	////Interfaces.Surface->SetFontGlyphSet(hFont, "Tahoma", 12, FW_MEDIUM, 0, 0, FONTFLAG_ANTIALIAS);

	hFont = Source::m_pSurface->CreateFont_();
	ESPFont = Source::m_pSurface->CreateFont_();
	Eagle = Source::m_pSurface->CreateFont_();
	mideagle = Source::m_pSurface->CreateFont_();
	Tabfont = Source::m_pSurface->CreateFont_();
	Checkmark = Source::m_pSurface->CreateFont_();
	SegoeUI = Source::m_pSurface->CreateFont_();
	MenuText = Source::m_pSurface->CreateFont_();
	BackArrow = Source::m_pSurface->CreateFont_();
	//LegitTabs = Interfaces.Surface->CreateFnt();
	//LastTabsText = Interfaces.Surface->CreateFnt();



	Source::m_pSurface->SetFontGlyphSet(hFont, "Choktoff", 12, FW_MEDIUM, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	Source::m_pSurface->SetFontGlyphSet(ESPFont, "Tahoma", 12, FW_MEDIUM, 0, 0, FONTFLAG_DROPSHADOW);
	//Interfaces.Surface->SetFontGlyphSet(ESPFont, "Nasalization Rg", 10, FW_NORMAL, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	Source::m_pSurface->SetFontGlyphSet(Eagle, "eagle", 14, FW_MEDIUM, 0, 0, FONTFLAG_ANTIALIAS);
	Source::m_pSurface->SetFontGlyphSet(mideagle, "eagle", 30, FW_MEDIUM, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_ADDITIVE);
	Source::m_pSurface->SetFontGlyphSet(Tabfont, "eagle", 42, FW_MEDIUM, 0, 0, FONTFLAG_ANTIALIAS);
	Source::m_pSurface->SetFontGlyphSet(Checkmark, "eagle", 10, FW_MEDIUM, 0, 0, FONTFLAG_ANTIALIAS);
	Source::m_pSurface->SetFontGlyphSet(BackArrow, "eagle", 18, FW_MEDIUM, 0, 0, FONTFLAG_ANTIALIAS);
	Source::m_pSurface->SetFontGlyphSet(SegoeUI, "Segoe UI", 16, FW_LIGHT, 0, 0, FONTFLAG_ANTIALIAS);
	//Interfaces.Surface->SetFontGlyphSet(LastTabsText, "Raleway", 8, FW_DONTCARE, 0, 0, FONTFLAG_ANTIALIAS);
	Source::m_pSurface->SetFontGlyphSet(MenuText, "Verdana", 12, FW_DONTCARE, 0, 0, FONTFLAG_ANTIALIAS);
	//Interfaces.Surface->SetFontGlyphSet(LegitTabs, "csgo_icons_outline", 12, FW_DONTCARE, 0, 0, FONTFLAG_ANTIALIAS);
	//Interfaces.Surface->SetFontGlyphSet(hFont, "Tahoma", 12, FW_MEDIUM, 0, 0, FONTFLAG_ANTIALIAS);
}

void Drawing::DrawString(vgui::HFont font, int x, int y, Color color, DWORD alignment, const char* msg, ...)
{
	va_list va_alist;
	char buf[1024];
	va_start(va_alist, msg);
	_vsnprintf_s(buf, sizeof(buf), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_UTF8, 0, buf, 256, wbuf, 256);

	int r = 255, g = 255, b = 255, a = 255;
	color.GetColor(r, g, b, a);

	int width, height;
	Source::m_pSurface->GetTextSize(font, wbuf, width, height);

	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	Source::m_pSurface->DrawSetTextFont(font);

	if (width > 65536)
		return;

	if (alignment & FONT_OUTLINE)
	{
		Source::m_pSurface->DrawSetTextColor(Color(0, 0, 0, color.a()));
		Source::m_pSurface->DrawSetTextPos(x + 1, y /*+ 1*/);
		Source::m_pSurface->DrawPrintText(wbuf, wcslen(wbuf));

		Source::m_pSurface->DrawSetTextColor(Color(0, 0, 0, color.a()));
		Source::m_pSurface->DrawSetTextPos(x - 1, y /*- 1*/);
		Source::m_pSurface->DrawPrintText(wbuf, wcslen(wbuf));
	}

	Source::m_pSurface->DrawSetTextColor(r, g, b, a);
	Source::m_pSurface->DrawSetTextPos(x, y /*- height / 2*/);
	Source::m_pSurface->DrawPrintText(wbuf, wcslen(wbuf));
}

void Drawing::DrawStringFont(vgui::HFont font, int x, int y, Color clrColor, bool bCenter, const char * szText, ...)
{
	if (!szText)
		return;

	va_list va_alist;
	char buf[1024];
	va_start(va_alist, szText);
	_vsnprintf_s(buf, sizeof(buf), szText, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_UTF8, 0, buf, 256, wbuf, 256);

	if (bCenter)
	{
		int Wide = 0, Tall = 0;

		Source::m_pSurface->GetTextSize(font, wbuf, Wide, Tall);

		x -= Wide / 2;
	}

	Source::m_pSurface->DrawSetTextFont(font);
	Source::m_pSurface->DrawSetTextPos(x, y);
	Source::m_pSurface->DrawSetTextColor(clrColor);
	Source::m_pSurface->DrawPrintText(wbuf, wcslen(wbuf));
}

void Drawing::DrawStringUnicode(vgui::HFont font, int x, int y, Color color, bool bCenter, const wchar_t* msg, ...)
{
	int r = 255, g = 255, b = 255, a = 255;
	color.GetColor(r, g, b, a);

	int iWidth, iHeight;

	Source::m_pSurface->GetTextSize(font, msg, iWidth, iHeight);
	Source::m_pSurface->DrawSetTextFont(font);
	Source::m_pSurface->DrawSetTextColor(r, g, b, a);
	Source::m_pSurface->DrawSetTextPos(!bCenter ? x : x - iWidth / 2, y - iHeight / 2);
	Source::m_pSurface->DrawPrintText(msg, wcslen(msg));
}

void Drawing::DrawRect(int x, int y, int w, int h, Color col)
{
	Source::m_pSurface->DrawSetColor(col);
	Source::m_pSurface->DrawFilledRect(x, y, x + w, y + h);
}

void Drawing::Rectangle(float x, float y, float w, float h, float px, Color col)
{
	DrawRect(x, (y + h - px), w, px, col);
	DrawRect(x, y, px, h, col);
	DrawRect(x, y, w, px, col);
	DrawRect((x + w - px), y, px, h, col);
}

void Drawing::Border(int x, int y, int w, int h, int line, Color col)
{
	DrawRect(x, y, w, line, col);
	DrawRect(x, y, line, h, col);
	DrawRect((x + w), y, line, h, col);
	DrawRect(x, (y + h), (w + line), line, col);
}

void Drawing::DrawRectRainbow(int x, int y, int width, int height, float flSpeed, float &flRainbow)
{
	Color colColor(0, 0, 0);

	flRainbow += flSpeed;
	if (flRainbow > 1.f) flRainbow = 0.f;

	for (int i = 0; i < width; i++)
	{
		float hue = (1.f / (float)width) * i;
		hue -= flRainbow;
		if (hue < 0.f) hue += 1.f;

		Color colRainbow = colColor.FromHSB(hue, 1.f, 1.f);
		Drawing::DrawRect(x + i, y, 1, height, colRainbow);
	}
}

void Drawing::DrawRectGradientVertical(int x, int y, int width, int height, Color color1, Color color2)
{
	float flDifferenceR = (float)(color2.r() - color1.r()) / (float)height;
	float flDifferenceG = (float)(color2.g() - color1.g()) / (float)height;
	float flDifferenceB = (float)(color2.b() - color1.b()) / (float)height;

	for (float i = 0.f; i < height; i++)
	{
		Color colGradient = Color(color1.r() + (flDifferenceR * i), color1.g() + (flDifferenceG * i), color1.b() + (flDifferenceB * i), color1.a());
		Drawing::DrawRect(x, y + i, width, 1, colGradient);
	}
}

void Drawing::DrawRectGradientHorizontal(int x, int y, int width, int height, Color color1, Color color2)
{
	float flDifferenceR = (float)(color2.r() - color1.r()) / (float)width;
	float flDifferenceG = (float)(color2.g() - color1.g()) / (float)width;
	float flDifferenceB = (float)(color2.b() - color1.b()) / (float)width;

	for (float i = 0.f; i < width; i++)
	{
		Color colGradient = Color(color1.r() + (flDifferenceR * i), color1.g() + (flDifferenceG * i), color1.b() + (flDifferenceB * i), color1.a());
		Drawing::DrawRect(x + i, y, 1, height, colGradient);
	}
}

void Drawing::DrawPixel(int x, int y, Color col)
{
	Source::m_pSurface->DrawSetColor(col);
	Source::m_pSurface->DrawFilledRect(x, y, x + 1, y + 1);
}

void Drawing::DrawOutlinedRect(int x, int y, int w, int h, Color col)
{
	Source::m_pSurface->DrawSetColor(col);
	Source::m_pSurface->DrawOutlinedRect(x, y, x + w, y + h);
}

void Drawing::DrawOutlinedCircle(int x, int y, int r, Color col)
{
	Source::m_pSurface->DrawSetColor(col);
	Source::m_pSurface->DrawOutlinedCircle(x, y, r, 1);
}

void Drawing::DrawLine(int x0, int y0, int x1, int y1, Color col)
{
	Source::m_pSurface->DrawSetColor(col);
	Source::m_pSurface->DrawLine(x0, y0, x1, y1);
}

void Drawing::DrawCorner(int iX, int iY, int iWidth, int iHeight, bool bRight, bool bDown, Color colDraw)
{
	int iRealX = bRight ? iX - iWidth : iX;
	int iRealY = bDown ? iY - iHeight : iY;

	if (bDown && bRight)
		iWidth = iWidth + 1;

	Drawing::DrawRect(iRealX, iY, iWidth, 1, colDraw);
	Drawing::DrawRect(iX, iRealY, 1, iHeight, colDraw);

	Drawing::DrawRect(iRealX, bDown ? iY + 1 : iY - 1, !bDown && bRight ? iWidth + 1 : iWidth, 1, Color(0, 0, 0, 255));
	Drawing::DrawRect(bRight ? iX + 1 : iX - 1, bDown ? iRealY : iRealY - 1, 1, bDown ? iHeight + 2 : iHeight + 1, Color(0, 0, 0, 255));
}
//
//#define COL2DWORD(x) (D3DCOLOR_ARGB(x.alpha, x.red, x.green, x.blue))
//void Drawing::Triangle(Vector2D pos1, Vector2D pos2, Vector2D pos3, Color color) const
//{
//	const auto dwColor = COL2DWORD(color);
//	Vertex vert[4] =
//	{
//		{ pos1.x, pos1.y, 0.0f, 1.0f, dwColor },
//		{ pos2.x, pos2.y, 0.0f, 1.0f, dwColor },
//		{ pos3.x, pos3.y, 0.0f, 1.0f, dwColor },
//		{ pos1.x, pos1.y, 0.0f, 1.0f, dwColor }
//	};
//
//	this->pDevice->SetTexture(0, nullptr);
//	this->pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 3, &vert, sizeof(Vertex));
//}
//
//
//void Drawing::TriangleFilled(Vector2D pos1, Vector2D pos2, Vector2D pos3, Color color) const
//{
//	const auto dwColor = COL2DWORD(color);
//	Vertex vert[3] =
//	{
//		{ pos1.x, pos1.y, 0.0f, 1.0f, dwColor },
//		{ pos2.x, pos2.y, 0.0f, 1.0f, dwColor },
//		{ pos3.x, pos3.y, 0.0f, 1.0f, dwColor }
//	};
//
//	this->pDevice->SetTexture(0, nullptr);
//	this->pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, &vert, sizeof(Vertex));
//}

void Drawing::Triangle(Vector ldcorner, Vector rucorner, Color col)
{
	DrawLine(ldcorner.x, ldcorner.y, (rucorner.x / 2) - 1, rucorner.y, col); // left shit

	DrawLine(rucorner.x, ldcorner.y, (rucorner.x / 2) - 1, rucorner.y, col); // right shit

	DrawLine(ldcorner.x, ldcorner.y, rucorner.x, ldcorner.y/*(ldcorner.x - rucorner.x), (ldcorner.y - rucorner.y)*/, col); // down shit
}

void Drawing::DrawPolygon(int count, Vertex_t* Vertexs, Color color)
{
	static int Texture = Source::m_pSurface->CreateNewTextureID(true);
	unsigned char buffer[4] = { 255, 255, 255, 255 };

	Source::m_pSurface->DrawSetTextureRGBA(Texture, buffer, 1, 1);
	Source::m_pSurface->DrawSetColor(color);
	Source::m_pSurface->DrawSetTexture(Texture);

	Source::m_pSurface->DrawTexturedPolygon(count, Vertexs);
}

void Drawing::DrawBox(int x, int y, int w, int h, Color color)
{
	// top
	DrawRect(x, y, w, 1, color);
	// right
	DrawRect(x, y + 1, 1, h - 1, color);
	// left
	DrawRect(x + w - 1, y + 1, 1, h - 1, color);
	// bottom
	DrawRect(x, y + h - 1, w - 1, 1, color);
}

void Drawing::DrawRoundedBox(int x, int y, int w, int h, int r, int v, Color col)
{
	std::vector<Vertex_t> p;
	for (int _i = 0; _i < 3; _i++)
	{
		int _x = x + (_i < 2 && r || w - r);
		int _y = y + (_i % 3 > 0 && r || h - r);
		for (int i = 0; i < v; i++)
		{
			int a = RAD2DEG((i / v) * -90 - _i * 90);
			p.push_back(Vertex_t(Vector2D(_x + sin(a) * r, _y + cos(a) * r)));
		}
	}

	Drawing::DrawPolygon(4 * (v + 1), &p[0], col);
	/*
	function DrawRoundedBox(x, y, w, h, r, v, col)
	local p = {};
	for _i = 0, 3 do
	local _x = x + (_i < 2 && r || w - r)
	local _y = y + (_i%3 > 0 && r || h - r)
	for i = 0, v do
	local a = math.rad((i / v) * - 90 - _i * 90)
	table.insert(p, {x = _x + math.sin(a) * r, y = _y + math.cos(a) * r})
	end
	end

	surface.SetDrawColor(col.r, col.g, col.b, 255)
	draw.NoTexture()
	surface.DrawPoly(p)
	end
	*/

	// Notes: amount of vertexes is 4(v + 1) where v is the number of vertices on each corner bit.
	// I did it in lua cause I have no idea how the vertex_t struct works and i'm still aids at C++
}

bool Drawing::ScreenTransform(const Vector &point, Vector &screen) // tots not pasted
{
	float w;
	const VMatrix &worldToScreen = Source::m_pEngine->WorldToScreenMatrix();

	screen.x = worldToScreen[0][0] * point[0] + worldToScreen[0][1] * point[1] + worldToScreen[0][2] * point[2] + worldToScreen[0][3];
	screen.y = worldToScreen[1][0] * point[0] + worldToScreen[1][1] * point[1] + worldToScreen[1][2] * point[2] + worldToScreen[1][3];
	w = worldToScreen[3][0] * point[0] + worldToScreen[3][1] * point[1] + worldToScreen[3][2] * point[2] + worldToScreen[3][3];
	screen.z = 0.0f;

	bool behind = false;

	if (w < 0.001f)
	{
		behind = true;
		screen.x *= 100000;
		screen.y *= 100000;
	}
	else
	{
		behind = false;
		float invw = 1.0f / w;
		screen.x *= invw;
		screen.y *= invw;
	}

	return behind;
}

bool Drawing::WorldToScreen(const Vector &origin, Vector &screen)
{
	if (!ScreenTransform(origin, screen))
	{
		int ScreenWidth, ScreenHeight;
		Source::m_pEngine->GetScreenSize(ScreenWidth, ScreenHeight);
		float x = ScreenWidth / 2;
		float y = ScreenHeight / 2;
		x += 0.5 * screen.x * ScreenWidth + 0.5;
		y -= 0.5 * screen.y * ScreenHeight + 0.5;
		screen.x = x;
		screen.y = y;
		return true;
	}

	return false;
}

RECT Drawing::GetViewport()
{
	RECT Viewport = { 0, 0, 0, 0 };
	int w, h;
	Source::m_pEngine->GetScreenSize(w, h);
	Viewport.right = w; Viewport.bottom = h;
	return Viewport;
}

int Drawing::GetStringWidth(vgui::HFont font, const char* msg, ...)
{
	va_list va_alist;
	char buf[1024];
	va_start(va_alist, msg);
	_vsnprintf_s(buf, sizeof(buf), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_UTF8, 0, buf, 256, wbuf, 256);

	int iWidth, iHeight;

	Source::m_pSurface->GetTextSize(font, wbuf, iWidth, iHeight);

	return iWidth;
}

RECT Drawing::GetTextSize(vgui::HFont font, const char* text)
{
	/*size_t origsize = strlen(text) + 1;
	const size_t newsize = 500;
	size_t convertedChars = 0;
	wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, wcstring, origsize, text, _TRUNCATE);*/

	RECT rect;

	rect.left = rect.right = rect.bottom = rect.top = 0;

	wchar_t wbuf[1024];
	if (MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf, 256) > 0) {
		int x, y;
		Source::m_pSurface->GetTextSize(font, wbuf, x, y);
		rect.left = x; rect.bottom = y;
		rect.right = x; rect.top = y;
	}

	return rect;
}

void Drawing::Draw3DBox(Vector* boxVectors, Color color)
{
	Vector boxVectors0, boxVectors1, boxVectors2, boxVectors3,
		boxVectors4, boxVectors5, boxVectors6, boxVectors7;


	if (Drawing::WorldToScreen(boxVectors[0], boxVectors0) &&
		Drawing::WorldToScreen(boxVectors[1], boxVectors1) &&
		Drawing::WorldToScreen(boxVectors[2], boxVectors2) &&
		Drawing::WorldToScreen(boxVectors[3], boxVectors3) &&
		Drawing::WorldToScreen(boxVectors[4], boxVectors4) &&
		Drawing::WorldToScreen(boxVectors[5], boxVectors5) &&
		Drawing::WorldToScreen(boxVectors[6], boxVectors6) &&
		Drawing::WorldToScreen(boxVectors[7], boxVectors7))
	{

		/*
		.+--5---+
		.8 4    6'|
		+--7---+'  11
		9   |  10  |
		|  ,+--|3--+
		|.0    | 2'
		+--1---+'
		*/

		Vector2D lines[12][2];
		//bottom of box
		lines[0][0] = { boxVectors0.x, boxVectors0.y };
		lines[0][1] = { boxVectors1.x, boxVectors1.y };
		lines[1][0] = { boxVectors1.x, boxVectors1.y };
		lines[1][1] = { boxVectors2.x, boxVectors2.y };
		lines[2][0] = { boxVectors2.x, boxVectors2.y };
		lines[2][1] = { boxVectors3.x, boxVectors3.y };
		lines[3][0] = { boxVectors3.x, boxVectors3.y };
		lines[3][1] = { boxVectors0.x, boxVectors0.y };

		lines[4][0] = { boxVectors0.x, boxVectors0.y };
		lines[4][1] = { boxVectors6.x, boxVectors6.y };

		// top of box
		lines[5][0] = { boxVectors6.x, boxVectors6.y };
		lines[5][1] = { boxVectors5.x, boxVectors5.y };
		lines[6][0] = { boxVectors5.x, boxVectors5.y };
		lines[6][1] = { boxVectors4.x, boxVectors4.y };
		lines[7][0] = { boxVectors4.x, boxVectors4.y };
		lines[7][1] = { boxVectors7.x, boxVectors7.y };
		lines[8][0] = { boxVectors7.x, boxVectors7.y };
		lines[8][1] = { boxVectors6.x, boxVectors6.y };


		lines[9][0] = { boxVectors5.x, boxVectors5.y };
		lines[9][1] = { boxVectors1.x, boxVectors1.y };

		lines[10][0] = { boxVectors4.x, boxVectors4.y };
		lines[10][1] = { boxVectors2.x, boxVectors2.y };

		lines[11][0] = { boxVectors7.x, boxVectors7.y };
		lines[11][1] = { boxVectors3.x, boxVectors3.y };

		for (int i = 0; i < 12; i++)
		{
			Drawing::DrawLine(lines[i][0].x, lines[i][0].y, lines[i][1].x, lines[i][1].y, color);
		}
	}
}

void Drawing::rotate_point(Vector2D &point, Vector2D origin, bool clockwise, float angle) {
	Vector2D delta = point - origin;
	Vector2D rotated;

	if (clockwise) {
		rotated = Vector2D(delta.x * cos(angle) - delta.y * sin(angle), delta.x * sin(angle) + delta.y * cos(angle));
	}
	else {
		rotated = Vector2D(delta.x * sin(angle) - delta.y * cos(angle), delta.x * cos(angle) + delta.y * sin(angle));
	}

	point = rotated + origin;
}

void Drawing::DrawFilledCircle(int x, int y, int radius, int segments, Color color) {
	std::vector< Vertex_t > vertices;
	float step = M_PI * 2.0f / segments;
	for (float a = 0; a < (M_PI * 2.0f); a += step)
		vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + x, radius * sinf(a) + y)));

	TexturedPolygon(vertices.size(), vertices, color);
}

void Drawing::DrawFilledCircle(int x, int y, int radius, int segments, float factor, Color color) {
	std::vector< Vertex_t > vertices;
	float step = M_PI * 2.0f / segments;
	for (float a = 0; a < ((M_PI * 2.0f) * factor); a += step)
		vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + x, radius * sinf(a) + y)));

	TexturedPolygon(vertices.size(), vertices, color);
}

void Drawing::LimitDrawingArea(int x, int y, int w, int h) {
	typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
	Memory::VCall<OriginalFn>(Source::m_pSurface, 147)(Source::m_pSurface, x, y, x + w + 1, y + h + 1);
}

void Drawing::GetDrawingArea(int &x, int &y, int &w, int &h) {
	typedef void(__thiscall* OriginalFn)(void*, int&, int&, int&, int&);
	Memory::VCall<OriginalFn>(Source::m_pSurface, 146)(Source::m_pSurface, x, y, w, h);
}

void Drawing::TexturedPolygon(int n, std::vector< Vertex_t > vertice, Color color) {
	static int texture_id = Source::m_pSurface->CreateNewTextureID(true); // 
	static unsigned char buf[4] = { 255, 255, 255, 255 };
	Source::m_pSurface->DrawSetTextureRGBA(texture_id, buf, 1, 1); //
	Source::m_pSurface->DrawSetColor(color); //
	Source::m_pSurface->DrawSetTexture(texture_id); //
	Source::m_pSurface->DrawTexturedPolygon(n, vertice.data()); //
}

void Drawing::filled_tilted_triangle(Vector2D position, Vector2D size, Vector2D origin, bool clockwise, float angle, Color color, bool rotate) {
	std::vector< Vertex_t > vertices =
	{
		Vertex_t{ Vector2D(position.x - size.x, position.y + size.y), Vector2D() },
		Vertex_t{ Vector2D(position.x, position.y - size.y), Vector2D() },
		Vertex_t{ position + size, Vector2D() }
	};

	if (rotate) {
		for (unsigned int p = 0; p < vertices.size(); p++) {
			rotate_point(vertices[p].m_Position, origin, clockwise, angle);
		}
	}

	TexturedPolygon(vertices.size(), vertices, color);

	vertices.clear();
}

void Drawing::DrawCircle(float x, float y, float r, float s, Color color)
{
	float Step = M_PI * 2.0 / s;
	for (float a = 0; a < (M_PI*2.0); a += Step)
	{
		float x1 = r * cos(a) + x;
		float y1 = r * sin(a) + y;
		float x2 = r * cos(a + Step) + x;
		float y2 = r * sin(a + Step) + y;

		DrawLine(x1, y1, x2, y2, color);
	}
}