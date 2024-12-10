#pragma once

#include "dataset.hpp"
#include <QtCharts>
#include <QtWidgets>

class EnvironmentalLitterPage : public QWidget {
  Q_OBJECT

public:
  explicit EnvironmentalLitterPage(QWidget *parent = nullptr);
  void updateData(WaterDataset *newDataset);

private:
  QComboBox *locationFilter;
  QComboBox *litterTypeFilter;
  QChart *chart;
  QPieSeries *pieSeries;
  QBarSeries *barSeries;
  QMap<QString, QMap<QString, int>> litterData;

  QLabel *complianceSummaryLabel;
  QMap<QString, int> totalDeterminands; // Total determinands for each location
  QMap<QString, QString>
      complianceStatus; // Compliance status for each location

  void setupFilters(QVBoxLayout *mainLayout);
  void setupChart(QVBoxLayout *mainLayout);
  void updateChart();
  void aggregateData(WaterDataset &dataset);
  void populatePieChart(QMap<QString, int> aggregatedCounts);
  void calculateCompliance();
  void updateComplianceSummary();
  void updateFilters();

  void addComplianceIndicator(QVBoxLayout *mainLayout, const QString &status);
  QString getComplianceStyle(const QString &status);
};
