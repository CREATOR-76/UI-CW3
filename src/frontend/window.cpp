#include "window.hpp"

#include "dataset.hpp"
#include "file_select_widget.hpp"
#include "fluorinated_compounds_page.hpp"
#include "pollutant_overview_page.hpp"
#include "pollutant_analysis_page.h"

#include <Qt>
#include <QtWidgets>

static const int MIN_WIDTH = 620;

WaterSampleWindow::WaterSampleWindow() : QMainWindow() {
  createMainWidget();

  setMinimumWidth(MIN_WIDTH);
  setWindowTitle("Cw3 Water Sample Analysis application - Group 15");
}

void WaterSampleWindow::createMainWidget() {
  toolbar = this->addToolBar("toolbar");
  createFileSelect();

  pages = new QTabWidget(this);
  pollutant_overview_page = new PollutantOverviewPage(this);
  fluorPage = new FluorinatedCompoundsPage(this);
  environmentalLitterPage = new EnvironmentalLitterPage();
  pollutantAnalysisPage = new PollutantAnalysisPage(this);

  pages->addTab(pollutantAnalysisPage, "Pollutant Analysis");
  pages->addTab(pollutant_overview_page, "Pollutants Overview Page");
  pages->addTab(fluorPage, "Fluorinated Compounds Page");
  pages->addTab(environmentalLitterPage, "Environmental Litter Page");

  setCentralWidget(pages);
}

void WaterSampleWindow::createFileSelect() {
  auto fileSelect = new FileSelectWidget(this);

  toolbar->addWidget(fileSelect);
  connect(fileSelect, &FileSelectWidget::fileSelected, this,
          &WaterSampleWindow::loadDataset);
  fileSelect->show();
}

void WaterSampleWindow::loadDataset(QString &filename) {
  try {
    dataset = new WaterDataset();
    dataset->loadData(filename);

    auto successmessage = new QLabel("csv loaded successfully");
    toolbar->addWidget(successmessage);
    QTimer::singleShot(5000, successmessage, &QLabel::hide);

    pollutant_overview_page->updateData(dataset);
    if (fluorPage) {
      fluorPage->updateData(dataset);
    }
    environmentalLitterPage->updateData(dataset);

  } catch (const std::exception &error) {
    QMessageBox::critical(this, "CSV File Error", error.what());
    return;
  }
}

void WaterSampleWindow::about() {
  QMessageBox::about(this, "About Water Analysis Tool",
                     "Water Analysis Tool displays and analyzes water quality "
                     "data loaded from "
                     "a CSV file.\n\n"
                     "Made by Group 15");
}
