#ifndef CHART_H
#define CHART_H
#include "component.h"

typedef struct SAxis{
	bool visible;
	int color;
	
	int position; //轴位置，单位:1/1000，50％ 在正中
	int axis_border; //轴的线型, 0=solid; 1=short dash; 2=dash; 3=dot

	int ticks_spc; //刻度间距，单位:1/1000
	int ticks_len; //坐标刻度的长度，小于0在内侧
	bool ticks_origin; //是否显示原点刻度
	bool ticks_grid; //是否显示刻度栅格
	int ticks_grid_border; //刻度栅格的线型

	int minor_len; //辅助刻度的长度，小于0在内侧
	int minor_num; //辅助刻度的数量
	bool minor_grid; //是否显示辅助刻度栅格
	int minor_grid_border; //辅助刻度栅格的线型
	
	bool labels_visible; //是否显示刻度标注
	SFont labels_font; //标注字体
	int labels_align; //对齐方式，0右对齐，1左对齐
	int labels_x;
	int labels_y; //刻度标注的位置，与坐标刻度的相对位置
	float start_value; //刻度的起始值
	float scale_value; //相邻刻度的差值
	int val_prec; //刻度值的精度
	int val_decimal; //刻度值的小数位数
	char * val_unit; //刻度值的显示单位
	bool labels_1st_minor; //是否标注第一个辅助刻度
	
	char * title; //轴标题
	SFont title_font; //轴标题字体
	int title_x; //轴标题的位置,单位1/1000
	int title_y; 
	
	int top_margin; //轴刷新区域上边距
	int left_margin; //轴刷新区域左边距
	int bottom_margin; //轴刷新区域下边距
	int right_margin; //轴刷新区域右边距
	
//	int scale_min; //轴起点对应的最小值
//	int scale_max; //轴终点对应的最大值
//	bool scale_auto; //是否根据输入数据的范围自动决定最大、最小值
}SAxis;

enum SeriesType {
    kSeriesQLine=1, kSeriesLine, kSeriesBar, kSeriesPoint
};

class CChart{
public:
	CChart(int w=160, int h=80, int num=1);
	~CChart();
	CChart & operator =(const CChart &other);

	int left;
	int top;
	//int color;
	SEdges border;
	SAxis axis_x; //X轴
	SAxis axis_y; //Y轴
	
	int series_type(int grps=0){ return series_type_[grps]; };
	void set_series_type(SeriesType val, int grps=0){ series_type_[grps] = val; };
	void set_y_scale(float val){ y_scale_ = val;};
	float scale_val(int xy) { if (xy) return axis_y.scale_value; else return axis_x.scale_value; };
	void set_last_tick(bool val){last_tick_ = val;};
	int bar_width; //柱状图柱子的宽度
	int x_dotnum; //X轴点数，0=1对1
	int start_point; //图形显示起点
	int draw_range; //图形显示范围
	bool wline_; //是否显示为加粗线
	
	void add_dataset(int pos, float *data, int num, int color, int grp=0);
	void add_data(int pos, float data, int color, int grp=0);
	void draw();
	void draw_frame();
	void ClearData();
	void ClearData(int pos, int sz);
	void set_sz(int w, int h); //设置宽窄
	void set_start_val(int xy, float val);
	void set_scale_val(int xy, float val);
	int height(){return height_;};
	int width(){return width_;};
	void refresh();
	int get_dotnum();
	void set_visible(bool yn);
	void set_update(){update = true;};
protected:
private:
	bool visible;
	int width_;
	int height_;

	int dataset_num_; //数据集的数目
	int **data_value; //每个点的值
	int **data_color; //每个点的颜色
	char ** data_yn_; //是否存在此点
	
	bool update;
	bool axisx_update;
	bool axisy_update;
	int *series_type_;   //数列的显示类型
	float y_scale_;     //Y轴每刻度对应的实际值
	bool last_tick_; //是否显示横坐标轴最后一个主刻度的标注
	
	void init_axis(SAxis * axis, int size);
	void draw_axis(SAxis * axis, int pos, int type);
	void draw_staff(SAxis * axis, int type, int pos, int sn, int sign);
    void draw_data(int grps);
};

class CChartVector{
public:
	CChartVector(int num=0, int len=40);//num 矢量的个数
	~CChartVector();
	CChartVector & operator =(const CChartVector &other);

	int left;
	int top;
	int width_;
	int height_;
	int color;
	SFont font;
	SEdges border;
	
	bool series_arrow; //是否以箭形图显示矢量
	int bar_width; //箭杆的宽度
	int axis_len; //轴的长度
	float axis_scale; //轴长对应的值
	
	void add_data(int indx, float amp, float phs, int color);
	void draw();
	void draw_frame();
	void draw_data();
	void clear();
	int height(){return height_;};
	int width(){return width_;};
	void refresh();
	void set_visible(bool yn);
	void set_data_label(int indx, const char *str);
	void set_axis_title(int indx, const char *str);
	
protected:
private:
	bool visible;

	int data_num; //矢量的数目
	Vector *data_value; //每个矢量的值
	int *data_color; //每个矢量的颜色
	SLabel * data_label; //矢量标注
	
	bool update;
	SLabel axis_title[4]; //分别为正半横轴,正半竖轴,负半横轴,负半竖轴的标题
};

#endif /* CHART_H */
