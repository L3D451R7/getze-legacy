#pragma once
#include "sdk.hpp"
#include <deque>
#include "source.hpp"

struct _event
{
	_event(const float& time, const std::string& msg, const std::string& secretmsg = "")
	{
		_time = time;
		_displayticks = time - Source::m_pGlobalVars->curtime;
		_msg = msg;
		_displayed = false;
		_secretmsg = secretmsg;
	}

	float _time = 0;
	float _displayticks = 0;
	bool _displayed = false;
	std::string _msg = "";
	std::string _secretmsg = "";
};

//struct _event
//{
//	_event(const float& time, const std::string& msg, Color col = Color::White())
//	{
//		_time = Source::m_pGlobalVars->curtime + 4.f/*time*/;
//		_displayticks = _time - Source::m_pGlobalVars->curtime;//time - Source::m_pGlobalVars->curtime;
//		_msg = msg;
//		_displayed = false;
//		_x = 0.f;
//		_colr = col;
//	}
//
//	float _time = 0;
//	float _displayticks = 0;
//	bool _displayed = false;
//	float _x = 0.f;
//	Color _colr = 0;
//	std::string _msg = "";
//};

extern std::vector<_event> _events;

class c_bullet_tracer
{
public:
	c_bullet_tracer(Vector src, Vector dst, float time, Color colorLine)
	{
		this->src = src;
		this->dst = dst;
		this->time = time;
		this->color1 = colorLine;
	}

	Vector src, dst;
	float time;
	Color color1;
};

extern std::vector<c_bullet_tracer> bullet_tracers;

class c_visuals
{
public:
	virtual bool get_espbox(C_BasePlayer * entity, int & x, int & y, int & w, int & h);
	virtual void draw_beam(Vector Start, Vector End, Color color, float Width);
	virtual void render_tracers();
	virtual void logs();
	virtual void skeleton(C_BasePlayer * Entity, Color color, matrix3x4_t * pBoneToWorldOut);
	virtual void draw_pov_arrows(C_BasePlayer * entity, float alpha);
	//virtual void draw_capsule(Vector Min, Vector Max, float Radius, matrix3x4_t & Transform, Color color);
	virtual void render(bool reset);

	float dormant_alpha[128];
	bool save_pos = false;
	int saved_x = 0;
	int saved_y = 0;
	bool was_moved = false;
};