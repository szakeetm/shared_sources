
#ifndef _CONVERGENCEPLOTTERH_
#define _CONVERGENCEPLOTTERH_

#include "GLApp/GLWindow.h"
#include "GLApp/GLChart/GLChartConst.h"
#include <vector>
#include <map>
#include <memory>

class GLChart;
class GLLabel;
class GLCombo;
class GLButton;
class GLFormula;
class GLDataView;
class GLToggle;
class GLTextField;
class GLTitledPanel;
class Worker;
class InterfaceGeometry;
struct Formulas;

class ConvergencePlotter : public GLWindow {

public:

  // Construction
  ConvergencePlotter(Worker *appWorker, std::shared_ptr<Formulas> formulas);
  ConvergencePlotter(const ConvergencePlotter& copy);

  // Component method
  void Display(Worker *w);
  void Refresh();
  void Update(float appTime);
  void Reset();

    // Implementation
  void ProcessMessage(GLComponent *src,int message) override;
  void SetBounds(int x,int y,int w,int h);
  int addView(int formulaHash);
  std::vector<int> GetViews() const;
  void SetViews(const std::vector<int> &updatedViews);
  bool IsLogScaled();
  void SetLogScaled(bool logScale);
  void SetWorker(Worker *w);

private:  
  int remView(int formulaHash);
  void refreshViews();

  Worker      *worker;
  GLButton    *dismissButton;
  GLChart     *chart;
  GLCombo     *profCombo;
  //GLToggle    *showAllMoments;

    GLButton    *pruneEveryNButton;
    GLButton    *pruneFirstNButton;
    GLButton    *addButton;
  GLButton    *removeButton;
  GLButton    *removeAllButton;
  GLTextField *formulaText;
  GLButton    *plotExpressionBtn;
  GLToggle    *logYToggle;

    GLToggle    *colorToggle;
    GLLabel    *fixedLineWidthText;
    GLButton    *fixedLineWidthButton;
    GLTextField    *fixedLineWidthField;

    GLTitledPanel *panel2;
    GLLabel    *convEpsText;
    GLTextField    *convEpsField;
    GLLabel    *convBandLenText;
    GLTextField    *convBandLenField;
    GLTextField    *shapeParamField;
    GLTextField    *shapeParamField2;
    GLButton    *convApplyButton;
    GLToggle* absOrRelToggle;
    GLDataView  *views[MAX_VIEWS];
  int          nbView;
  float        lastUpdate;

  std::shared_ptr<Formulas> appFormulas;

};

#endif /* _CONVERGENCEPLOTTERH_ */
