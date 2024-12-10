#pragma once

#include "dataset.hpp"
#include <QtCharts>
#include <QtWidgets>

class PollutantOverviewPage : public QWidget {
  Q_OBJECT

public:
  explicit PollutantOverviewPage(QWidget *parent = nullptr);
  void updateData(WaterDataset *dataset);

private:
  WaterDataset *dataset;
  SamplingPoint *current_point;

  QGridLayout *layout;

  QComboBox *pollutant_select;
  QComboBox *location_select;
  QChartView *chart;
  QChart *current_chart;
  QLineSeries *time_series;

  void create_layout();
  void create_widgets();

private slots:
  void locationSet();
  void pollutantSet();
};
