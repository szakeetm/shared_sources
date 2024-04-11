

#ifndef MOLFLOW_PROJ_GRAPHICSHELPER_H
#define MOLFLOW_PROJ_GRAPHICSHELPER_H

void RGBtoHSV(float& R, float& G, float& B);
void HSVtoRGB(float& H, float& S, float& V);
void modifyRGBColor(float& R, float& G, float& B, float saturityMod, float brightnessMod);
#endif //MOLFLOW_PROJ_GRAPHICSHELPER_H
