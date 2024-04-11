#pragma once

struct FacetHitDetails {
    // Temporary var (used in Intersect for collision)

    FacetHitDetails() = default;
    FacetHitDetails(double d, double u, double v, bool hit) :colDistTranspPass(d), colU(u), colV(v), isHit(hit) {};

    double colDistTranspPass = 1.0E99;
    double colU = 0.0;
    double colV = 0.0;
    bool   isHit = false;
};
