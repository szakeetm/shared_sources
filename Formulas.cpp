//
// Created by pbahr on 09/10/2020.
//

#include "Formulas.h"
#include <cmath>

// convergence constants
constexpr size_t max_vector_size() { return 65536; };
constexpr size_t d_precision() { return 5; };

bool Formulas::InitializeFormulas(){
    bool allOk = true;
    for (size_t i = 0; i < formulas_n.size(); i++) {

        // Evaluate variables
        int nbVar = formulas_n.at(i)->GetNbVariable();
        bool ok = true;
        for (int j = 0; j < nbVar && ok; j++) {
            VLIST *v = formulas_n.at(i)->GetVariableAt(j);
            ok = evaluator->EvaluateVariable(v);
        }

        if (ok) {
            formulas_n.at(i)->hasVariableEvalError = false;
        }
        else{
            allOk = false;
        }
    }
    return allOk;
};

/**
* \brief Resizes Vector storing convergence values if needed (change of expressions, etc.)
*/
void Formulas::UpdateVectorSize() {
    //Rebuild vector size
    size_t nbFormulas = formulas_n.size();
    if (convergenceValues.size() != nbFormulas) {
        convergenceValues.resize(nbFormulas);
    }
}

bool Formulas::FetchNewConvValue(size_t nbDesorbed){
    bool hasChanged = false;
    if (!formulas_n.empty()) {
        UpdateVectorSize();
        for (int formulaId = 0; formulaId < formulas_n.size(); ++formulaId) {
            // TODO: Cross check integrity of formula with editor!?
            auto& conv_vec = convergenceValues[formulaId].conv_vec;
            if (conv_vec.size() >= max_vector_size()) {
                pruneEveryN(4, formulaId, 1000); // delete every 4th element for now
                hasChanged = true;
            }
            double r;
            formulas_n[formulaId]->hasVariableEvalError = false;
            if (formulas_n[formulaId]->Evaluate(&r)) {
                // TODO: This could lead to false negatives for stop criterion when we don't generate more sample points
                if(conv_vec.empty() || (conv_vec.back().first != nbDesorbed && conv_vec.back().second != r)) {
                    conv_vec.emplace_back(std::make_pair(nbDesorbed, r));
                    convergenceValues[formulaId].n_samples += 1;
                    convergenceValues[formulaId].conv_total += r;
                    hasChanged = true;
                }
            }
        }
    }

    return hasChanged;
};

/**
* \brief Removes every everyN-th element from the convergence vector in case the max size has been reached
* \param everyN specifies stepsize for backwards delete
* \param formulaId formula whose convergence values shall be pruned
* \param skipLastN skips last N data points e.g. to retain a sharp view, while freeing other data points
*/
void Formulas::pruneEveryN(size_t everyN, int formulaId, size_t skipLastN) {
    auto &conv_vec = convergenceValues[formulaId].conv_vec;
    for (int i = conv_vec.size() - everyN - skipLastN; i > 0; i = i - everyN)
        conv_vec.erase(conv_vec.begin() + i);
}

/**
* \brief Removes first n elements from the convergence vector
 * \param n amount of elements that should be removed from the front
 * \param formulaId formula whose convergence values shall be pruned
*/
void Formulas::pruneFirstN(size_t n, int formulaId) {
    auto &conv_vec = convergenceValues[formulaId].conv_vec;
    conv_vec.erase(conv_vec.begin(), conv_vec.begin() + std::min(n, conv_vec.size()));
}

/**
* \brief Removes first n elements from the convergence vector
 * \param formulaId formula whose convergence values shall be pruned
*/
double Formulas::GetConvRate(int formulaId) {

    auto &conv_vec = convergenceValues[formulaId].conv_vec;
    if(conv_vec.empty()) return 0.0;

    double sumValSquared = 0.0;
    double sumVal = 0.0;
    for(auto& convVal : conv_vec){
        sumValSquared += convVal.second * convVal.second;
        sumVal += convVal.second;
    }
    double invN = 1.0 / conv_vec.size();
    double var = invN * sumValSquared - std::pow(invN * sumVal,2);
    double convDelta = 3.0 * std::sqrt(var) / (conv_vec.size() * conv_vec.size());

    if(convDelta / conv_vec.back().second < 1.0e-6) printf("[1] Sufficient convergent reached: %e\n", convDelta / conv_vec.back().second);

    double xmean = sumVal * invN;
    double varn = 0.0;
    for(auto& convVal : conv_vec){
        varn += std::pow(convVal.second - xmean,2);
    }
    varn *= 1.0 / (conv_vec.size() - 1.0);
    double quant95Val = 1.96 * std::sqrt(varn) / std::sqrt(conv_vec.size());
    double quant99Val = 2.576 * std::sqrt(varn) / std::sqrt(conv_vec.size());
    double quant995Val = 3.0 * std::sqrt(varn) / std::sqrt(conv_vec.size());
    if(xmean - quant95Val <= conv_vec.back().second && xmean + quant95Val >= conv_vec.back().second) printf("[95.0] Sufficient convergent reached: %e in [%e , %e]\n", conv_vec.back().second, xmean - quant95Val, xmean + quant95Val);
    else if(xmean - quant99Val <= conv_vec.back().second && xmean + quant99Val >= conv_vec.back().second) printf("[99.0] Sufficient convergent reached: %e in [%e , %e]\n", conv_vec.back().second, xmean - quant99Val, xmean + quant99Val);
    else if(xmean - quant995Val <= conv_vec.back().second && xmean + quant995Val >= conv_vec.back().second) printf("[99.5] Sufficient convergent reached: %e in [%e , %e]\n", conv_vec.back().second, xmean - quant995Val, xmean + quant995Val);
    else {
        double dist = std::min(conv_vec.back().second - xmean + quant995Val, conv_vec.back().second + xmean - quant995Val);
        printf("[4] Convergence distance to a995: %e --> %e close to [%e , %e]\n", dist, conv_vec.back().second, xmean - quant995Val, xmean + quant995Val);
        printf("[4] Abs to a95: %e --> Rel to a95: %e / %e\n", quant95Val, (conv_vec.back().second - xmean) / xmean, (quant95Val / conv_vec.back().second));
    }
    return convDelta;
}

/**
* \brief Restart values for ASCBR in case method parameters changed
*/
void Formulas::RestartASCBR(int formulaId){
    auto& convData = convergenceValues[formulaId];
    convData.chain_length = 0;
    convData.upper_bound = convData.lower_bound = 0.0;

    freq_accum.clear();
    freq_accum.resize(cb_length);
}

/**
* \brief Check stopping criterium based on *acceptable shifting convergence band rule* (ASCBR)
 * \param formulaId formula whose conv values should be checked for
 * \return true = end condition met: run algorithm until (chain length == cb_length)
*/
bool Formulas::CheckASCBR(int formulaId) {
    // Initialize
    auto& convData = convergenceValues[formulaId];
    if(convData.conv_vec.empty()) return false;

    const size_t cb_len = cb_length; // convergence band length

    // Step 1: increment step, +1 MC event
    // done outside

    // Step 2: Increment value for variable total T
    // done outside in FetchNewConvValue()

    // & calculate new mean
    //double conv_mean_local = convData.conv_total / convData.n_samples;
    const double conv_mean_local = convData.conv_vec.back().second;

    // Step 3: Check if mean still within bounds
    bool withinBounds = (conv_mean_local >= convData.lower_bound && conv_mean_local <= convData.upper_bound) ? true : false;

    if(!withinBounds){
        // half-width of convergence band
        const double eps = useAbsEps ? 0.5 * std::pow(10.0, -1.0*epsilon) : 0.5 * conv_mean_local * std::pow(10.0, -1.0*epsilon);
        // absolute: exponent d = significant digits after decimal point

        // Step 4: Update bounds
        convData.upper_bound = conv_mean_local + eps;
        convData.lower_bound = conv_mean_local - eps;
        // Step 5: Update length of inbound "chain"
        ++freq_accum[(convData.chain_length>=cb_length)?0:convData.chain_length];
        convData.chain_length = 0;
    }
    else{
        // Step 5: Update length of inbound "chain"
        ++convData.chain_length;
    }

    return convData.chain_length > cb_len;
}

double Formulas::ApproxShapeParameter() {
    // Initialize
    double shape_param = 0.0;
    double den = 0.0;
    for(int i = 1; i < cb_length; ++i){
        den += (double) i * freq_accum[i];
    }
    if(den <= 1e-8) den = 1.0;
    shape_param = 1.0 - freq_accum[1] / den;

    return shape_param;
}

double Formulas::ApproxShapeParameter2() {
    // Initialize
    double shape_param = 0.0;
    double den = 0.0;
    for(int i = 1; i < cb_length; ++i){
        den += (double) i * freq_accum[i];
    }
    if(den <= 1e-8) den = 1.0;
    shape_param = 1.0 - freq_accum[0] / den;

    return shape_param;
}