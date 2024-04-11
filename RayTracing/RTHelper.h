#pragma once

struct FacetHitDetail {
    // Temporary var (used in Intersect for collision)

    FacetHitDetail() = default;
    FacetHitDetail(double d, double u, double v, bool hit) :colDistTranspPass(d), colU(u), colV(v) {};

    double colDistTranspPass = 1.0E99;
    double colU = 0.0;
    double colV = 0.0;
};
