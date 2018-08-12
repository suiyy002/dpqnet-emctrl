#ifndef COMPONENT_H
#define COMPONENT_H
#include "../display/display.h"
#include "conversion.h"

typedef struct {
	int asc_size; //ASCII字符大小
	int cn_size; //中文字符大小
	int space; //字符间距
	int color; //字符颜色
}SFont;

//边框
typedef struct { 
	int left_width; //左边线的宽度,0:无
	int top_width; 
	int right_width; 
	int bottom_width; 
	int color;
}SEdges;


class CLabel{
public:
	CLabel(char *str="");
	~CLabel();
	CLabel & operator =(const CLabel &other);

	int left;
	int top;
	int width;
	int now_width; //自适应大小时，当前标注宽度
	int height;
	SFont font;
	int left_space; //左边距
	int top_space; //上边距
	bool transparent; //背景是否透明
	int bgcolor; //背景色

	bool auto_size; //是否自适应大小
	
	void set_txt(const char *str);
	void draw();
	void refresh();
	void set_visible(bool yn);
///[LB
	bool get_visible();
	void set_update(bool yn);
	bool get_update();
///]	
protected:
private:
	bool visible;
	bool update;

	char *caption;
	char *text;
	void calc_vwsz();
};

typedef struct SLabel{ 
	int left;
	int top;
	int width;
	int rfrs_width; //当前刷新区域的宽度
	int height;
	char *text;
	bool auto_size; //自适应大小
	bool self_font; //是否使用自己的字体
	SFont font;
	bool update;
}SLabel;

class CLabelSet{ //标注集
public:
	CLabelSet(int cnt);
	~CLabelSet();
	CLabelSet & operator =(const CLabelSet &other);

	bool allrefresh;
	int left;
	int top;
	int width;
	int height;
	SFont font;
	bool transparent; //背景是否透明
	int bgcolor; //背景色
	SEdges border; //边框
	
	SLabel *labels_;

	void set_txt(int indx, const char *str);
	void draw();
	void refresh();
	int get_count();
	void clear();
	void set_visible(bool yn);
	void set_labels_font(int idx, int color) {labels_[idx].font.color = color; };
protected:
private:
	bool visible;
	int count;

	bool update;
	void drawone(int indx);
	void calc_vwsz(int indx);
};

class CTabControl{
public:
	CTabControl(int num);
	~CTabControl();

	bool set_tab_index(int num);
	bool set_tabs_title(int num, char *str);
	bool set_tab_start(int pos){tab_idx_dis_ = tab_start_ = pos; };
	void set_tab_num(int num){ if (num<=tab_max_) tab_num_ = num; };
	void draw();
	void refresh();
	void enable_tab(bool type);

	int left;
	int top;
	int width;
	int height;
	SFont font;
	int color;  //
	SEdges border;

	int tab_position;   //标签位于窗体的位置
	int tab_height;     //标签高度，=0:auto
	int tab_index;      //当前标签
	int *tabs_fcolor;   //标签的标题颜色
	int tab_vofst;      //标签标题垂直偏移量
	bool tab_enable;    //标签使能
protected:
private:
	bool update;
	int tabs_tol_width_; //所有标签的总宽度
	int tab_idx_dis_;    //已标签在顶部为例：当前标签左边线与最左侧标签的左边线的距离
	int tab_start_;      //标签的偏移量
	int *tabs_width_;    //标签宽度
	char **tabs_title_;  //标签的标题
	int *tabs_indent_;   //标签标题的缩进量
	int tab_num_;        //标签的数量
	int tab_max_;        //标签最大数量，即创建的数量
};

class CCartoon{
public:
	CCartoon(int w, int h);
	virtual ~CCartoon();

	int color;
	
	void draw(); //先备份当前显示区域，再调用paint()绘制指定图形
	void move(int x, int y);
	void refresh();
	virtual void paint();
	void set_visible(bool yn);
protected:
	bool visible;
	int left;
	int top;
	int width;
	int height;
private:

	int *dis_data_bak; //显示区域备份
	bool update;  //是否刷新显示区域
};

const int tpTOP = 0;
const int tpBOTTOM = 1;
const int tpLEFT = 2;
const int tpRIGHT = 3;

#endif /* COMPONENT_H */
