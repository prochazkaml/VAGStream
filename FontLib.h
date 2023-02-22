#include <SYS/TYPES.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBGS.H>

void Font_ChangeColor(int r, int g, int b);
void Font_ChangeColorWithOpacity(int r, int g, int b, int bg_r, int bg_g, int bg_b, int op);
void Font_ChangePosition(int x, int y);
void Font_ResetPos();
void Font_PutChar(char c);
void Font_PrintString(char *str);
void Font_PrintStringCentered(char *str);
void Font_PrintStringRTL(char *str);
