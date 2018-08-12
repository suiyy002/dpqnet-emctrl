#ifndef _VGA_COLOR_H_
#define _VGA_COLOR_H_

enum VGAColorType {
    kVGA_Black, kVGA_White,

    kVGA_Red, kVGA_HiRed, kVGA_Orange, kVGA_HiOrange,
    kVGA_Yellow, kVGA_HiYellow, kVGA_Green, kVGA_HiGreen,
    kVGA_Cyan, kVGA_HiCyan, kVGA_Blue, kVGA_HiBlue,
    kVGA_Purple, kVGA_HiPurple,
    
    kVGA_HiWhite,
    kVGA_Grey, kVGA_HiGrey,
    kVGA_LightYellow, kVGA_Blue1, kVGA_Green1,
    kVGA_Grey1, kVGA_BlackGrey,
    kVGA_Default
};

class VGAColor {
public:
	VGAColor() { IniColorBit32(); };
	~VGAColor() {};
	
	void SetColorSys(int bits);
	
	int color(int type) { return color_[type]; };
protected:
private:
    void IniColorBit32();
	int color_[kVGA_Default+1];
};

VGAColor &vga_color();

int vgacolor(int type);

#endif  //_VGA_COLOR_H_


