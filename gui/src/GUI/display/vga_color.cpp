#include "vga_color.h"

VGAColor &vga_color()
{
	static VGAColor colr;
	return colr;
};

int vgacolor(int type)
{
    return vga_color().color(type);
}

/*!
Initialize bit32 color 
*/
void VGAColor::IniColorBit32()
{
    color_[kVGA_Black] = 0;
    color_[kVGA_White] = 0xC0C0C0;
    color_[kVGA_Red] = 0x800000;
    color_[kVGA_HiRed] = 0xFF0000;
    color_[kVGA_Orange] = 0xFFA000;
    color_[kVGA_HiOrange] = 0xFFA000;
    color_[kVGA_Yellow] = 0x808000;
    color_[kVGA_HiYellow] = 0xE0C000;
    color_[kVGA_Green] = 0x008000;
    color_[kVGA_HiGreen] = 0x00F000;
    color_[kVGA_Cyan] = 0x008080;
    color_[kVGA_HiCyan] = 0x00FFFF;
    color_[kVGA_Blue] = 0x000080;
    color_[kVGA_HiBlue] = 0x0000FF;
    color_[kVGA_Purple] = 0x800080;
    color_[kVGA_HiPurple] = 0xFF00FF;

    color_[kVGA_HiWhite] = 0xFAFAFA;
    color_[kVGA_Grey] = 0x808080;
    color_[kVGA_HiGrey] = 0x888888;
    color_[kVGA_LightYellow] = 0xE2E2A7;
    color_[kVGA_Blue1] = 0x0070C0;
    color_[kVGA_Green1] = 0x006000;
    color_[kVGA_Grey1] = 0xA0A0A0;
    color_[kVGA_BlackGrey] = 0x404040;
    color_[kVGA_Default] = color_[kVGA_White];
}

/*!

    Input:  bits -- bits per pixel
*/
void VGAColor::SetColorSys(int bits)
{
    int red, green, blue, i, n;
    
    if (bits==8) {
        for (i=1; i<=kVGA_Default; i++) {
            n = color_[i]>>16;
            red = n*0x7/0xff;
            n = (color_[i]>>8)&0xff;
            green = n*0x7/0xff;
            n = color_[i]&0xff;
            blue = n*0x3/0xff;

            color_[i] = (red<<5) | (green<<2) | blue;
        }
    } else if (bits==16) {
        for (i=1; i<=kVGA_Default; i++) {
            n = color_[i]>>16;
            red = n*0x1f/0xff;
            n = (color_[i]>>8)&0xff;
            green = n*0x3f/0xff;
            n = color_[i]&0xff;
            blue = n*0x1f/0xff;

            color_[i] = (red<<11) | (green<<5) | blue;
        }
    } else if (bits==24) {
    }
}

