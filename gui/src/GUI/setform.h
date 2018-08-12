#ifndef SETFORM_H
#define SETFORM_H

#include "component/component.h"
#include "viewset.h"
#include "../device/device.h"

class CSetForm{
public:
	CSetForm();
	~CSetForm();


	void refresh(int type);
	void set_update(){ update = true; }
	bool Locked(){ return set_view->Locked(); };

	void return_from_set(){ set_view->return_from_set(KEY_ESC); }
	void show_cursor(){ set_view->show_cursor(); }
	void ini_para(){ set_view->ini_para(); }
	int view_set_key(int ktype){ return set_view->view_set_key(ktype); }
	int harm_refresh(){ return set_view->harm_refresh(); }

protected:
private:
	void draw();

	CSetView *set_view;
	int left;
	int top;
	int width;
	int height;
	int color;
	SEdges border;
	bool update;
};

extern CSetForm * set_form;

#endif /* SETFORM_H */
