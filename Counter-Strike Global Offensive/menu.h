#pragma once

#include "sdk.hpp"

//struct groupbox {
//
//	groupbox()
//	{
//		in_group = false;
//		line = 0;
//	}
//
//	groupbox(int _line)
//	{
//		in_group = true;
//		line = _line;
//	}
//
//	bool in_group;
//	int line;
//};

struct MultiSelectable
{
	std::string		name;
	bool*			value;

	MultiSelectable(const std::string& name_, bool *value_)
	{
		name = name_;
		value = value_;
	}
};

class c_drawhack
{
public:
	void render();

	bool mouse_in_pos(Vector start, Vector end);

	bool Mousein(Vector start, Vector end);

	//menu items below
	bool menuitem(int & n, std::string text, bool& var, bool parent = false);
	void scrollbar(Vector pos, float sizeY, int & n);
	bool combobox(int & n, std::string name, std::vector<std::string> items, int & selected, bool parent = false);
	bool multicombo(int & n, std::string name, std::vector<MultiSelectable> items, bool parent);
	void colorpicker(int & n, std::string name, float *value, bool parent = false);
	void menugroupbox(int & n, int size, std::string text = "");
	void separator(int & n, std::string text = "");
	bool menuitem(int & n, std::string name, std::vector<std::string> items, int & var, bool parent = false);
	/*virtual void menu_slider(int & n, const char * name, float min, float max, float & value, const int & display_decimal, const char * mark);
	virtual void menu_slider(int & n, const char * name, int min, int max, int & value, const char * mark);
	virtual void change_line(int _line, int & i, int & old);
	virtual bool menubutton(int & n, std::string text);*/
	bool menutab(int &n, std::string text, bool selected, bool parent = false);
	/*virtual bool menuitem(int & n, std::string text_t, int & var, int min, int max);
	virtual void get_keys(bool & IsGettingKey, std::string & text, bool secondtry);
	virtual void get_key(bool & IsGettingKey, int & get);
	virtual bool menuinput(int & n, const char * name, char * text, int buf_size);
	virtual bool menuinput(int & n, const char * name, std::string & text);
	virtual bool menukeybind(int & n, int * var);*/

	bool menubuttons(int & tab, std::vector<std::string> items, int & var, bool parent = false, int adjustY = 0);

	void menu_slider(int & n, const char * name, float min, float max, float & value, bool parent = false, const int & display_decimal = 1, const char * mark = "");

	void menu_slider(int & n, const char * name, int min, int max, int & value, bool parent = false, const char * mark = "", bool ll = false, const char * nn = "");

	//boxes to render
	void draw_gui();
	//virtual void draw_speclist_box();

	//misc shit
	Vector _cursor_position;
	bool menu_opened;
	bool save_pos = false;
	int saved_x = 0;
	int saved_y = 0;
	int line = 0/*left*/;
	int contents_posX = 100;
	int contents_posY = 100;
	int menu_posX = 100;
	int menu_posY = 100;
	bool combobox_opened = false;

	int saved_box_x = 0;
	int saved_box_y = 0;
	bool save_box_pos = false;

	int saved_spec_x = 0;
	int saved_spec_y = 0;
	bool save_spec_pos = false;

	int alpha = 0;

	bool mouse1_pressed;
	int mwheel_value = 0;
};