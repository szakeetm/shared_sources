

#include "Formulas.h"
#include "FormulaEvaluator.h"
#include "Helper/ConsoleLogger.h"
#include "GLTypes.h" //Error()
#include "Worker.h"
#include <sstream>

// convergence constants
constexpr size_t max_vector_size = 65536;

//! Add a formula to the formula storage
void Formulas::AddFormula(const std::string& name, const std::string& expression) {
    GLFormula p;
    p.SetExpression(expression);
    p.SetName(name);
    p.Parse();

    formulas.push_back(std::move(p)); //copy constructing is not permissible as evalTree would be destroyed
    UpdateVectorSize();
}

//! Clear formula storage
void Formulas::ClearFormulas() {
    formulas.clear();
    UpdateVectorSize();
}

/**
* \brief Resets convergence data e.g. when simulation resets
*/
void Formulas::ResetConvergenceData() {
    for (auto& convVec : convergenceData) {
        convVec.clear();
    }
}


//! Initialize formulas by parsing string to values
void Formulas::EvaluateFormulaVariables(size_t formulaIndex, const std::vector <std::pair<std::string, std::optional<double>>>& aboveFormulaValues){
    auto& formula = formulas[formulaIndex];
	
	size_t nbVar = formula.GetNbVariable();
	bool ok = true; //for each formula, stop at first variable that can't be evaluated
	for (size_t j = 0; j < nbVar && ok; j++) {
		auto varIterator = formula.GetVariableAt(j);
        formula.evalErrorMsg = "";
        try {
            ok = evaluator->EvaluateVariable(varIterator, aboveFormulaValues);
            if (!ok) formula.SetEvalError(fmt::format("Unknown variable \"{}\"", varIterator->varName));
        }
        catch (const Error& err) {//Specific evaluation error message to display to user, currently used for "formula not found" and "formula not yet evaluated"
            formula.SetEvalError(err.what());
            ok = false;
        }
	}

	if (ok) {
		formula.hasEvalError = false;
	}
};

/**
* \brief Resizes Vector storing convergence values if needed (change of expressions, etc.)
*/
void Formulas::UpdateVectorSize() {
    //Rebuild vector size
    size_t nbFormulas = formulas.size();
    if (convergenceData.size() != nbFormulas) {
        convergenceData.resize(nbFormulas);
    }
    if (formulaValueCache.size() != nbFormulas) {
        formulaValueCache.resize(nbFormulas);
    }
}

//Calculate formula values for later usage in corresponding windows
void Formulas::EvaluateFormulas(size_t nbDesorbed) {

    std::vector <std::pair<std::string, std::optional<double>>> aboveFormulaValues; //Formulas can refer to previous formulas (above) by number or by name. If evaluation was successful, they can see their value

	// Evaluate each formula and cache values for later usage
	// in FormulaEditor, ConvergencePlotter
	for (size_t i = 0; i < formulas.size(); ++i) {
        // First evaluate variable values
        EvaluateFormulaVariables(i,aboveFormulaValues);
        try {
            double value = formulas[i].Evaluate();       
            formulaValueCache[i] = FormulaHistoryDatapoint(nbDesorbed, value);
            aboveFormulaValues.push_back({ formulas[i].GetName(),value });
        }
        catch (...) {
            aboveFormulaValues.push_back({ formulas[i].GetName(),std::nullopt });
        }
	}
	if (recordConvergence) RecordNewConvergenceDataPoint();
}

//! Add new value to convergence vector
bool Formulas::RecordNewConvergenceDataPoint() {
	bool hasChanged = false;
	if (formulas.empty()) return false;
	UpdateVectorSize();
	for (int formulaId = 0; formulaId < formulas.size(); ++formulaId) {
		if (formulas[formulaId].hasParseError || formulas[formulaId].hasEvalError) {
			continue;
		}

		auto& lastValue_local = formulaValueCache[formulaId];
		// Check against potential nan values to skip
		if (std::isnan(lastValue_local.value))
			continue;

		auto& convergenceData_local = convergenceData[formulaId];
		if (convergenceData_local.size() >= max_vector_size) {
			removeEveryNth(4, formulaId, 1000); // delete every 4th element for now
			hasChanged = true;
		}

		// Insert new value when completely new value pair inserted
		if (convergenceData_local.empty() || (lastValue_local.nbDes != convergenceData_local.back().nbDes && lastValue_local.value != convergenceData_local.back().value)) {
			convergenceData_local.emplace_back(lastValue_local);
			hasChanged = true;
		}
		else if (lastValue_local.nbDes == convergenceData_local.back().nbDes && lastValue_local.value != convergenceData_local.back().value) {
			// if (for some reason) the nbDesorptions dont change but the formula value, only update the latter
			convergenceData_local.back().value = lastValue_local.value;
			hasChanged = true;
		}
		else if (lastValue_local.value == convergenceData_local.back().value) {
			if (convergenceData_local.size() > 1 && lastValue_local.value == (convergenceData_local.rbegin()[1]).value) {
				// if the value remains constant, just update the nbDesorbptions
				convergenceData_local.back().nbDes = lastValue_local.nbDes;
				hasChanged = true;
			}
			else {
				// if there was no previous point with the same value, add a new one (only a second one to save space)
				convergenceData_local.emplace_back(lastValue_local);
				hasChanged = true;
			}
		}
	}

	return hasChanged;
}

std::string Formulas::GetFormulaValue(int index)
{
    std::string out;
    if (formulas[index].hasParseError) {
        out = (formulas[index].GetParseErrorMsg());
    }
    else if (formulas[index].hasEvalError) {
        out = (formulas[index].GetEvalErrorMsg());
    }
    else {
        std::stringstream tmp; // ugly solution copied from legacy gui
        tmp << formulaValueCache[index].value; //not elegant but converts 12.100000000001 to 12.1 etc., fmt::format doesn't
        out.append(tmp.str());
    }
    return out;
}

std::string Formulas::ExportCurrentFormulas()
{
    std::string out;
    size_t formulasSize = formulas.size();
    out.append("Expression\tName\tValue\n"); // Headers
    for (int i = 0; i < formulasSize; i++) {
        out.append(formulas[i].GetExpression() + '\t');
        out.append(formulas[i].GetName() + '\t');
        out.append(GetFormulaValue(i) + '\n');
    }
    return out;
}
std::string Formulas::ExportFormulasAtAllMoments(Worker* worker)
{
    size_t nMoments = worker->interfaceMomentCache.size();
    if (nMoments == 0) return ExportCurrentFormulas();
    std::string out;
    size_t formulasSize = formulas.size();
    size_t selectedMomentSave = worker->displayedMoment;

    //need to store results to only run calculation m times instead of e*m times 
    std::vector<std::vector<std::string>> expressionMomentTable;
    expressionMomentTable.resize(formulasSize);
    for (int m = 0; m <= nMoments; m++) {
        /*
        Calculation results for moments are not stored anywhere, only the 'current' value
        of an expression is available so in order to export values at all moments, all those
        values need to be calculated now
        */
        worker->displayedMoment = m;
        {
            worker->Update(0.0f);
        }
        EvaluateFormulas(worker->globalStatCache.globalHits.nbDesorbed);
        for (int e = 0; e < formulasSize; e++) {
            expressionMomentTable[e].push_back(GetFormulaValue(e));
        }
    }
    // restore moment from before starting
    worker->displayedMoment = selectedMomentSave;
    {
        worker->Update(0.0f);
    }
    EvaluateFormulas(worker->globalStatCache.globalHits.nbDesorbed);
    // headers
    out.append("Expression\tName\tConst.flow\t");
    for (int i = 0; i < nMoments; ++i) {
        out.append("Moment " + std::to_string(i + 1) + "\t");
    }
    out.erase(out.length() - 1);
    out.append("\n\t\t\t");
    for (int i = 0; i < nMoments; ++i) {
        out.append(std::to_string(worker->interfaceMomentCache[i].time));
        out.append("\t");
    }
    out.erase(out.length() - 1);
    out.append("\n");

    for (int e = 0; e < formulasSize; e++) {
        out.append(formulas[e].GetExpression() + '\t');
        out.append(formulas[e].GetName() + '\t');
        for (int m = 0; m < expressionMomentTable[e].size(); m++) {
            out.append(expressionMomentTable[e][m] + '\t');
        }
        out.erase(out.length() - 1);
        out.append("\n");
    }
    return out;
}
;

/**
* \brief Removes every everyN-th element from the convergence vector in case the max size has been reached
* \param everyN specifies stepsize for backwards delete
* \param formulaId formula whose convergence values shall be pruned
* \param skipLastN skips last N data points e.g. to retain a sharp view, while freeing other data points
*/
void Formulas::removeEveryNth(size_t everyN, int formulaId, size_t skipLastN) {
    auto &conv_vec = convergenceData[formulaId];
    for (int i = conv_vec.size() - everyN - skipLastN; i > 0; i = i - everyN)
        conv_vec.erase(conv_vec.begin() + i);
}

/**
* \brief Removes first n elements from the convergence vector
 * \param n amount of elements that should be removed from the front
 * \param formulaId formula whose convergence values shall be pruned
*/
void Formulas::removeFirstN(size_t n, int formulaId) {
    auto &conv_vec = convergenceData[formulaId];
    conv_vec.erase(conv_vec.begin(), conv_vec.begin() + std::min(n, conv_vec.size()));
}

/*
double Formulas::GetConvRate(int formulaId) {

    auto &conv_vec = convergenceValues[formulaId].valueHistory;
    if(conv_vec.empty()) return 0.0;

    double sumValSquared = 0.0;
    double sumVal = 0.0;
    for(auto& convVal : conv_vec){
        sumValSquared += convVal.value * convVal.value;
        sumVal += convVal.value;
    }
    double invN = 1.0 / conv_vec.size();
    double var = invN * sumValSquared - std::pow(invN * sumVal,2);
    double convDelta = 3.0 * std::sqrt(var) / (conv_vec.size() * conv_vec.size());

    if(convDelta / conv_vec.back().value < 1.0e-6) Log::console_msg(2, "[1] Sufficient convergent reached: {:e}\n", convDelta / conv_vec.back().value);

    double xmean = sumVal * invN;
    double varn = 0.0;
    for(auto& convVal : conv_vec){
        varn += std::pow(convVal.value - xmean,2);
    }
    varn *= 1.0 / (conv_vec.size() - 1.0);
    double quant95Val = 1.96 * std::sqrt(varn) / std::sqrt(conv_vec.size());
    double quant99Val = 2.576 * std::sqrt(varn) / std::sqrt(conv_vec.size());
    double quant995Val = 3.0 * std::sqrt(varn) / std::sqrt(conv_vec.size());
    if(xmean - quant95Val <= conv_vec.back().value && xmean + quant95Val >= conv_vec.back().value) Log::console_msg(2, "[95.0] Sufficient convergent reached: {:e} in [{:e} , {:e}]\n", conv_vec.back().value, xmean - quant95Val, xmean + quant95Val);
    else if(xmean - quant99Val <= conv_vec.back().value && xmean + quant99Val >= conv_vec.back().value) Log::console_msg(2, "[99.0] Sufficient convergent reached: {:e} in [{:e} , {:e}]\n", conv_vec.back().value, xmean - quant99Val, xmean + quant99Val);
    else if(xmean - quant995Val <= conv_vec.back().value && xmean + quant995Val >= conv_vec.back().value) Log::console_msg(2, "[99.5] Sufficient convergent reached: {:e} in [{:e} , {:e}]\n", conv_vec.back().value, xmean - quant995Val, xmean + quant995Val);
    else {
        double dist = std::min(conv_vec.back().value - xmean + quant995Val, conv_vec.back().value + xmean - quant995Val);
        Log::console_msg(2, "[4] Convergence distance to a995: {:e} --> {:e} close to [{:e} , {:e}]\n", dist, conv_vec.back().value, xmean - quant995Val, xmean + quant995Val);
        Log::console_msg(2, "[4] Abs to a95: {:e} --> Rel to a95: {:e} / {:e}\n", quant95Val, (conv_vec.back().value - xmean) / xmean, (quant95Val / conv_vec.back().value));
    }
    return convDelta;
}
*/

/**
* \brief Restart values for ASCBR in case method parameters changed
*/
/*
void Formulas::RestartASCBR(int formulaId){
    auto& convData = convergenceValues[formulaId];
    convData.chain_length = 0;
    convData.upper_bound = convData.lower_bound = 0.0;

    freq_accum.clear();
    freq_accum.resize(cb_length);
}
*/

/**
* \brief Check stopping criterium based on *acceptable shifting convergence band rule* (ASCBR)
 * \param formulaId formula whose conv values should be checked for
 * \return true = end condition met: run algorithm until (chain length == cb_length)
*/
/*
bool Formulas::CheckASCBR(int formulaId) {
    // Initialize
    auto& convData = convergenceValues[formulaId];
    if(convData.valueHistory.empty()) return false;

    const size_t cb_len = cb_length; // convergence band length

    // Step 1: increment step, +1 MC event
    // done outside

    // Step 2: Increment value for variable total T
    // done outside in RecordNewConvergenceDataPoint()

    // & calculate new mean
    //double conv_mean_local = convData.conv_total / convData.n_samples;
    const double conv_mean_local = convData.valueHistory.back().value;

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
*/
