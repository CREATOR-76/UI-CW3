#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "dataset.hpp"
#include "environmental_litter_page.hpp"
#include "fluorinated_compounds_page.hpp"
#include "pollutant_overview_page.hpp"
#include "pollutant_analysis_page.h"
#include <QtWidgets>

class WaterSampleWindow : public QMainWindow {
  Q_OBJECT

public:
  WaterSampleWindow();

private:
  void createMainWidget();
  void createFileSelect();

  WaterDataset *dataset;
  PollutantOverviewPage *pollutant_overview_page;
  FluorinatedCompoundsPage *fluorPage;
  EnvironmentalLitterPage *environmentalLitterPage;
  PollutantAnalysisPage* pollutantAnalysisPage;
  QTabWidget *pages;
  QToolBar *toolbar;

private slots:
  void about();
  void loadDataset(QString &);
};

#endif // WINDOW_HPP
