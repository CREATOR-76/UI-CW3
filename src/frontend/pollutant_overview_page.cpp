#include "pollutant_overview_page.hpp"
#include <algorithm>
#include <qdatetimeaxis.h>
#include <qnamespace.h>
#include <qvalueaxis.h>
#include <set>

using namespace std;

PollutantOverviewPage::PollutantOverviewPage(QWidget *parent)
    : QWidget(parent) {
  create_layout();
  create_widgets();
}

void PollutantOverviewPage::create_layout() {
  layout = new QGridLayout();
  this->setLayout(layout);
}

void PollutantOverviewPage::create_widgets() {
  pollutant_select = new QComboBox(this);
  location_select = new QComboBox(this);

  current_chart = new QChart();
  chart = new QChartView(current_chart);

  chart->setRenderHint(QPainter::Antialiasing);
  time_series = new QLineSeries(current_chart);
  current_chart->addSeries(time_series);

  time_series->setMarkerSize(10);
  time_series->setColor(QColor(0, 0, 0));

  layout->addWidget(location_select, 0, 0);
  layout->addWidget(pollutant_select, 0, 1);
  layout->addWidget(chart, 1, 0, 2, 2);

  // Connect signals to slots
  connect(location_select, &QComboBox::currentTextChanged, this,
          &PollutantOverviewPage::locationSet);
  connect(pollutant_select, &QComboBox::currentTextChanged, this,
          &PollutantOverviewPage::pollutantSet);
}

void PollutantOverviewPage::updateData(WaterDataset *dataset_in) {
  location_select->clear();
  dataset = dataset_in;
  if (!dataset)
    return;

  set<string> locations;
  for (const auto &point : *dataset->getData()) {
    if (!point.second || !point.second->getSamples())
      continue;

    locations.insert(point.second->getLabel());
  }

  for (const string &location : locations) {
    location_select->addItem(QString::fromStdString(location));
  }
}

void PollutantOverviewPage::locationSet() {
  auto location = location_select->currentText().toStdString();
  current_point = dataset->getFromLabel(location);
  if (!current_point)
    return;

  pollutant_select->clear();
  set<string> pollutants;

  for (const auto &sample : *current_point->getSamples()) {
    if (!sample->getDeterminands())
      continue;

    for (const auto &determinand : *sample->getDeterminands()) {
      pollutants.insert(determinand->getLabel());
    }
  }

  for (const auto &pollutant : pollutants) {
    pollutant_select->addItem(QString::fromStdString(pollutant));
  }
}

void PollutantOverviewPage::pollutantSet() {
  if (!current_point)
    return;

  auto determinand_label = pollutant_select->currentText().toStdString();

  double minY = numeric_limits<double>::max();
  double maxY = numeric_limits<double>::lowest();
  QDateTime firstDate, lastDate;

  QVector<QPointF> points;

  for (const auto &sample : *current_point->getSamples()) {
    double res = sample->getResultFromLabel(determinand_label);

    QDateTime dateTime = QDateTime::fromString(
        QString::fromStdString(sample->getDateTime()), Qt::ISODateWithMs);

    if (res == -1)
      continue;
    points.push_back(QPointF(dateTime.toMSecsSinceEpoch(), res));

    minY = min(minY, res);
    maxY = max(maxY, res);

    if (firstDate.isNull() || dateTime < firstDate)
      firstDate = dateTime;
    if (lastDate.isNull() || dateTime > lastDate)
      lastDate = dateTime;
  }

  time_series->replace(points);

  auto axisX = new QDateTimeAxis();
  axisX->setTitleText("Date");
  if (!firstDate.isNull() && !lastDate.isNull()) {
    axisX->setRange(firstDate, lastDate);
  }

  auto axisY = new QValueAxis();
  axisY->setTitleText("Value");
  axisY->setRange(minY, maxY);

  current_chart->setAxisX(axisX);
  current_chart->setAxisY(axisY);

  time_series->attachAxis(axisX);
  time_series->attachAxis(axisY);

  current_chart->setTitle(QString::fromStdString(determinand_label));
}
