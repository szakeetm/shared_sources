//
// Created by pbahr on 18/06/2020.
//

#include "GraphicsHelper.h"
#include <algorithm> // min, max
#include <cmath> // fmod

void RGBtoHSV(float& R, float& G, float& B) {
    float H = 0.0f;
    float S = 0.0f;
    float V = 0.0f;

    float maxColor = std::max(std::max(R, G), B);
    float minColor = std::min(std::min(R, G), B);
    float colorDelta = maxColor - minColor;

    if(colorDelta > 0) {
        if(maxColor == R) {
            H = 60.0f * (std::fmod(((G - B) / colorDelta), 6.0f));
        } else if(maxColor == G) {
            H = 60.0f * (((B - R) / colorDelta) + 2.0f);
        } else {
            H = 60.0f * (((R - G) / colorDelta) + 4.0f);
        }

        if(maxColor > 0.0) {
            S = colorDelta / maxColor;
        } else {
            S = 0.0;
        }

        V = maxColor;
    } else {
        H = 0.0;
        S = 0.0;
        V = maxColor;
    }

    if(H < 0.0) {
        H += 360.0f;
    }
    R=H;
    G=S;
    B=V;
}

void HSVtoRGB(float& H, float& S, float& V) {
    float R = 0.0f;
    float G = 0.0f;
    float B = 0.0f;

    float chromaVal = V * S;
    float hVal = std::fmod(H / 60.0f, 6.0f);
    float xVal = chromaVal * (1.0f - std::fabs(std::fmod(hVal, 2.0f) - 1.0f));
    float modVal = V - chromaVal;

    if(0.0f <= hVal && hVal < 1.0f) {
        R = chromaVal;
        G = xVal;
        B = 0.0f;
    } else if(hVal < 2.0f) {
        R = xVal;
        G = chromaVal;
        B = 0.0f;
    } else if(hVal < 3.0f) {
        R = 0.0f;
        G = chromaVal;
        B = xVal;
    } else if(hVal < 4.0f) {
        R = 0.0f;
        G = xVal;
        B = chromaVal;
    } else if(hVal < 5.0f) {
        R = xVal;
        G = 0.0f;
        B = chromaVal;
    } else if(hVal < 6.0f) {
        R = chromaVal;
        G = 0.0f;
        B = xVal;
    } else {
        R = 0.0f;
        G = 0.0f;
        B = 0.0f;
    }

    R += modVal;
    G += modVal;
    B += modVal;

    H = R;
    S = G;
    V = B;
}

void modifyRGBColor(float& R, float& G, float& B, float saturityMod, float brightnessMod){
    RGBtoHSV(R, G, B);
    G = std::clamp(G*saturityMod,0.0f,1.0f); // change Saturity
    B = std::clamp(B*brightnessMod,0.0f,1.0f); // change brighness Value
    HSVtoRGB(R, G, B);
    G = std::clamp(G,0.0f,1.0f); // account for conversion overflow
    B = std::clamp(B,0.0f,1.0f);

    R = R / 1.0f;
    G = G / 1.0f;
    B = B / 1.0f;
}