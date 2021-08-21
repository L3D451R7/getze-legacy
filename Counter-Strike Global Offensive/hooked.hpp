#pragma once

#include "source.hpp"

namespace F
{
	extern vgui::HFont Menu;
	extern vgui::HFont ESPInfo;
	extern vgui::HFont NewESP;
	extern vgui::HFont OldESP;
	extern vgui::HFont LBY;
	extern vgui::HFont Against;
	extern vgui::HFont Weapons;
	extern vgui::HFont Eagle;
	extern vgui::HFont EagleLogo;
	extern vgui::HFont EagleMisc;
	extern vgui::HFont EagleTab;
}

namespace Drawing
{
	extern void CreateFonts();
	extern void DrawString(vgui::HFont font, int x, int y, Color color, DWORD alignment, const char* msg, ...);
	extern void DrawStringFont(vgui::HFont font, int x, int y, Color clrColor, bool bCenter, const char * szText, ...);
	extern void DrawStringUnicode(vgui::HFont font, int x, int y, Color color, bool bCenter, const wchar_t* msg, ...);
	extern void DrawRect(int x, int y, int w, int h, Color col);
	extern void Rectangle(float x, float y, float w, float h, float px, Color col);
	extern void Border(int x, int y, int w, int h, int line, Color col);
	extern void DrawRectRainbow(int x, int y, int w, int h, float flSpeed, float &flRainbow);
	extern void DrawRectGradientVertical(int x, int y, int w, int h, Color color1, Color color2);
	extern void DrawRectGradientHorizontal(int x, int y, int w, int h, Color color1, Color color2);
	extern void DrawPixel(int x, int y, Color col);
	extern void DrawOutlinedRect(int x, int y, int w, int h, Color col);
	extern void DrawOutlinedCircle(int x, int y, int r, Color col);
	extern void DrawLine(int x0, int y0, int x1, int y1, Color col);
	extern void DrawCorner(int iX, int iY, int iWidth, int iHeight, bool bRight, bool bDown, Color colDraw);
	extern void DrawRoundedBox(int x, int y, int w, int h, int r, int v, Color col);
	extern void Triangle(Vector ldcorner, Vector rucorner, Color col);
	extern void DrawPolygon(int count, Vertex_t* Vertexs, Color color);
	extern void DrawBox(int x, int y, int w, int h, Color color);
	extern bool ScreenTransform(const Vector &point, Vector &screen);
	extern bool WorldToScreen(const Vector &origin, Vector &screen);
	extern RECT GetViewport();
	extern int	GetStringWidth(vgui::HFont font, const char* msg, ...);
	extern RECT GetTextSize(vgui::HFont font, const char* text);
	extern void Draw3DBox(Vector* boxVectors, Color color);
	extern void rotate_point(Vector2D & point, Vector2D origin, bool clockwise, float angle);
	extern void DrawFilledCircle(int x, int y, int radius, int segments, Color color);
	extern void DrawFilledCircle(int x, int y, int radius, int segments, float factor, Color color);
	extern void LimitDrawingArea(int x, int y, int w, int h);
	extern void GetDrawingArea(int & x, int & y, int & w, int & h);
	extern void TexturedPolygon(int n, std::vector<Vertex_t> vertice, Color color);
	extern void filled_tilted_triangle(Vector2D position, Vector2D size, Vector2D origin, bool clockwise, float angle, Color color, bool rotate = true);
	extern void DrawCircle(float x, float y, float r, float s, Color color);

	extern vgui::HFont	hFont;
	extern vgui::HFont	ESPFont;
	extern vgui::HFont	Eagle;
	extern vgui::HFont	mideagle;
	extern vgui::HFont	Tabfont;
	extern vgui::HFont	Checkmark;
	extern vgui::HFont	SegoeUI;
	extern vgui::HFont	MenuText;
	extern vgui::HFont	BackArrow;
}

namespace Hooked
{

	void __fastcall PacketStart(void* ecx, void* edx, int incoming_sequence, int outgoing_acknowledged);
	void __fastcall PacketEnd(pPastaState* cl_state, void* EDX);
	void __fastcall OverrideView(void* ecx, void* edx, CViewSetup* vsView);
	bool __fastcall DoPostScreenEffects(void* clientmode, void*, int a1);
	bool __stdcall CreateMove(float flInputSampleTime, CUserCmd* cmd);//int sequence_number, float input_sample_frametime, bool active );
	void __fastcall FrameStageNotify( void* ecx, void* edx, ClientFrameStage_t stage );
	void __fastcall RunCommand( void* ecx, void* edx, C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper );
	bool __stdcall InPrediction();
	void __fastcall PaintTraverse( void* ecx, void* edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce );
	void __fastcall SceneEnd(void* ecx, void* edx);
	int __stdcall DrawModelExecute(void* context, void* state, ModelRenderInfo_t& info, matrix3x4_t* pCustomBoneToWorld);
	float __fastcall GetScreenAspectRatio(void *pEcx, void *pEdx, int32_t iWidth, int32_t iHeight);
	float __stdcall GetViewModelFOV();
	void _stdcall EngineVGUI_Paint(int mode);
	void __stdcall LockCursor();
	bool __fastcall SetupBones(void* ECX, void* EDX, matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);
	bool __fastcall IsHLTV(IVEngineClient *_this, void* EDX);
	bool __fastcall SendNetMsg(INetChannel* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice);
	void __fastcall UpdateClientSideAnimation(C_BasePlayer* ecx, void* edx);
	void __fastcall CL_Move(void* ecx, void* edx, float accumulated_extra_samples, bool bFinalTick);
	//void __fastcall UpdateAnimationState(PVOID state, PVOID edx, PVOID suck_my_dick, float angle_y, float angle_x, float a4, PVOID);

	extern WNDPROC oldWindowProc;
	extern LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

}