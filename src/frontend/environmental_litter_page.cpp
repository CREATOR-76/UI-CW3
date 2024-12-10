#include "environmental_litter_page.hpp"
#include <iostream>

EnvironmentalLitterPage::EnvironmentalLitterPage(QWidget *parent) {

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QLabel *titleLabel = new QLabel("Environmental Litter Indicators");
  titleLabel->setAlignment(Qt::AlignCenter);
  titleLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
  mainLayout->addWidget(titleLabel);

  setupFilters(mainLayout);
  setupChart(mainLayout);
}

void EnvironmentalLitterPage::updateData(WaterDataset *newDataset) {
  // Clear existing data
  if (!litterData.empty())
    litterData.clear();
  if (!totalDeterminands.empty())
    totalDeterminands.clear();
  if (!complianceStatus.empty())
    complianceStatus.clear();

  // Re-aggregate the data with the new dataset
  aggregateData(*newDataset);
  calculateCompliance();

  // **Update UI elements**
  updateComplianceSummary(); // Update compliance banner
  updateFilters();           // Update location and litter type filters
  updateChart();             // Update chart data
}

void EnvironmentalLitterPage::updateFilters() {
  // **Update Location Filter**
  locationFilter->clear(); // Clear existing options
  locationFilter->addItem("All Locations", "All Locations"); // Default option

  for (const auto &location : litterData.keys()) {
    QString displayLocation = location;
    locationFilter->addItem(displayLocation, location);
  }

  // **Update Litter Type Filter**
  litterTypeFilter->clear();                     // Clear existing options
  litterTypeFilter->addItem("All Litter Types"); // Default option

  if (!litterData.isEmpty()) {
    // Populate with the first available location's litter types
    for (const auto &type : litterData.values().first().keys()) {
      litterTypeFilter->addItem(type);
    }
  } else {
    qDebug() << "Warning: No data found for Litter Types. Defaulting to 'All "
                "Litter Types'.";
  }
}

void EnvironmentalLitterPage::aggregateData(WaterDataset &dataset) {
  for (const auto &pointPair : *(dataset.getData())) {
    SamplingPoint *samplingPoint = pointPair.second;
    const auto *samples = samplingPoint->getSamples();
    QString locationLabel = QString::fromStdString(samplingPoint->getLabel());

    for (Sample *sample : *samples) {
      const auto *determinands = sample->getDeterminands();

      if (!totalDeterminands.contains(locationLabel))
        totalDeterminands[locationLabel] = 0;

      totalDeterminands[locationLabel] += determinands->size();

      for (Determinand *determinand : *determinands) {
        if (determinand->getUnitLabel() == "garber c") {
          QString determinandLabel =
              QString::fromStdString(determinand->getDefinition());
          determinandLabel.remove("Bathing Water Profile : ");

          if (!litterData[locationLabel].contains(determinandLabel))
            litterData[locationLabel][determinandLabel] = 0;

          litterData[locationLabel][determinandLabel]++;
        }
      }
    }
  }
}

void EnvironmentalLitterPage::updateComplianceSummary() {
  if (complianceStatus.empty()) {
    return;
  }

  if (!complianceSummaryLabel)
    delete complianceSummaryLabel; // Delete old label to avoid duplication

  int totalLocations = complianceStatus.size();
  int compliantLocations = 0;

  for (const auto &status : complianceStatus.values()) {
    if (status == "Compliant")
      compliantLocations++;
  }

  QString complianceSummary =
      QString("Compliance: %1 of %2 locations are compliant.")
          .arg(compliantLocations)
          .arg(totalLocations);

  QString complianceStatus = (compliantLocations == totalLocations)
                                 ? "Compliant"
                             : (compliantLocations > 0) ? "Caution"
                                                        : "Non-Compliant";

  complianceSummaryLabel = new QLabel(complianceSummary);
  complianceSummaryLabel->setAlignment(Qt::AlignCenter);
  complianceSummaryLabel->setStyleSheet(getComplianceStyle(complianceStatus));

  layout()->addWidget(
      complianceSummaryLabel); // Add the label to the main layout
}

void EnvironmentalLitterPage::calculateCompliance() {
  for (const auto &location : totalDeterminands.keys()) {
    int total = totalDeterminands[location];
    int litterTotal = 0;

    if (litterData.contains(location)) {
      for (const auto &count : litterData[location]) {
        litterTotal += count;
      }
    }

    double percentage =
        (total > 0) ? (static_cast<double>(litterTotal) / total * 100) : 0.0;

    if (percentage < 5.0) {
      complianceStatus[location] = "Compliant";
    } else if (percentage >= 5.0 && percentage <= 10.0) {
      complianceStatus[location] = "Caution";
    } else {
      complianceStatus[location] = "Non-Compliant";
    }
  }
}

void EnvironmentalLitterPage::addComplianceIndicator(QVBoxLayout *mainLayout,
                                                     const QString &status) {
  QLabel *complianceBox = new QLabel(status);
  complianceBox->setAlignment(Qt::AlignCenter);
  complianceBox->setStyleSheet(getComplianceStyle(status));
  mainLayout->addWidget(complianceBox);
}

void EnvironmentalLitterPage::setupFilters(QVBoxLayout *mainLayout) {
  QHBoxLayout *filterLayout = new QHBoxLayout;

  // Location Filter
  QLabel *locationFilterLabel = new QLabel("Filter by Location:");
  locationFilter = new QComboBox();
  locationFilter->addItem("All Locations", "All Locations");

  // for (const auto &location : litterData.keys()) {
  //   QString displayLocation = location;
  //   displayLocation.remove("<CSVField> ");
  //   locationFilter->addItem(displayLocation, location);
  // }
  filterLayout->addWidget(locationFilterLabel);
  filterLayout->addWidget(locationFilter);

  // Litter Type Filter
  QLabel *litterTypeFilterLabel = new QLabel("Filter by Litter Type:");
  litterTypeFilter = new QComboBox();
  litterTypeFilter->addItem("All Litter Types");

  // ** FIXED THIS PART **
  // if (!litterData.isEmpty()) {
  //   for (const auto &type : litterData.values().first().keys()) {
  //     litterTypeFilter->addItem(type);
  //   }
  // } else {
  //   qDebug() << "Warning: No data found for Litter Types. Defaulting to 'All
  //   "
  //               "Litter Types'.";
  // }

  filterLayout->addWidget(litterTypeFilterLabel);
  filterLayout->addWidget(litterTypeFilter);

  mainLayout->addLayout(filterLayout);

  // Connect both filters to the updateChart method
  connect(locationFilter, &QComboBox::currentIndexChanged, this,
          &EnvironmentalLitterPage::updateChart);
  connect(litterTypeFilter, &QComboBox::currentIndexChanged, this,
          &EnvironmentalLitterPage::updateChart);
}

void EnvironmentalLitterPage::setupChart(QVBoxLayout *mainLayout) {
  chart = new QChart();
  chart->setTitle("Litter Types Distribution");
  chart->setAnimationOptions(QChart::AllAnimations);

  pieSeries = new QPieSeries();
  barSeries = new QBarSeries();

  chart->addSeries(pieSeries);

  QChartView *chartView = new QChartView(chart);
  chartView->setRenderHint(QPainter::Antialiasing);
  mainLayout->addWidget(chartView);
}

void EnvironmentalLitterPage::populatePieChart(
    QMap<QString, int> aggregatedCounts) {
  // Remove X and Y axes for Pie Chart
  if (!chart->axes(Qt::Horizontal).isEmpty()) {
    chart->removeAxis(chart->axes(Qt::Horizontal).first());
  }
  if (!chart->axes(Qt::Vertical).isEmpty()) {
    chart->removeAxis(chart->axes(Qt::Vertical).first());
  }

  // Calculate total count
  int totalCount = 0;
  for (const auto &type : aggregatedCounts.keys()) {
    totalCount += aggregatedCounts[type];
  }

  // Add the count and percentage to the pie series
  for (const auto &type : aggregatedCounts.keys()) {
    int count = aggregatedCounts[type];

    // Calculate percentage
    double percentage =
        (totalCount > 0) ? (static_cast<double>(count) / totalCount * 100) : 0;

    // Format the label for the pie chart
    QString sliceLabel =
        QString("%1 (Count: %2, %3%)")
            .arg(type)  // Litter type
            .arg(count) // Count
            .arg(QString::number(percentage, 'f',
                                 1)); // Percentage with 1 decimal place

    QPieSlice *slice = new QPieSlice(type, count);
    slice->setLabel(sliceLabel); // Add labels like "Plastic (Count: 34, 25.4%)"
                                 // to the chart
    slice->setLabelVisible(true);

    pieSeries->append(slice);
  }

  chart->addSeries(pieSeries);

  // Customize the legend to display only the litter type names
  auto legendMarkers = chart->legend()->markers(pieSeries);
  int i = 0;
  for (auto *slice : pieSeries->slices()) {
    legendMarkers.at(i++)->setLabel(
        slice->label().split('(').first().trimmed()); // Set legend to only the
                                                      // litter type
  }

  pieSeries->setLabelsVisible(true);
  pieSeries->setLabelsPosition(QPieSlice::LabelOutside);
}

QString EnvironmentalLitterPage::getComplianceStyle(const QString &status) {
  if (status == "Compliant")
    return "background-color: #28a745; color: white; padding: 10px; font-size: "
           "16px; font-weight: bold; border-radius: 8px; text-align: center;";
  else if (status == "Caution")
    return "background-color: #ffc107; color: black; padding: 10px; font-size: "
           "16px; font-weight: bold; border-radius: 8px; text-align: center;";
  else
    return "background-color: #dc3545; color: white; padding: 10px; font-size: "
           "16px; font-weight: bold; border-radius: 8px; text-align: center;";
}

void EnvironmentalLitterPage::updateChart() {
  QString selectedLocation = locationFilter->currentData().toString();
  QString selectedLitterType = litterTypeFilter->currentText();

  if (chart->series().contains(pieSeries)) {
    chart->removeSeries(pieSeries);
  }
  if (chart->series().contains(barSeries)) {
    chart->removeSeries(barSeries);
  }

  delete pieSeries;
  delete barSeries;

  pieSeries = new QPieSeries(this);
  barSeries = new QBarSeries(this);

  if (selectedLitterType == "All Litter Types") {
    QMap<QString, int> aggregatedCounts;

    if (selectedLocation == "All Locations") {
      for (const auto &location : litterData.keys()) {
        for (const auto &type : litterData[location].keys()) {
          aggregatedCounts[type] += litterData[location][type];
        }
      }

      chart->setTitle("Litter Types Distribution (All Locations)");
    } else if (litterData.contains(selectedLocation)) {
      aggregatedCounts = litterData[selectedLocation];

      // Format the location name
      QString formattedLocation = selectedLocation;
      formattedLocation = formattedLocation.left(1).toUpper() +
                          formattedLocation.mid(1).toLower();

      chart->setTitle("Litter Types Distribution (" + formattedLocation + ")");
    }

    populatePieChart(aggregatedCounts);
  } else if (selectedLocation == "All Locations") {
    QBarSet *barSet = new QBarSet(selectedLitterType);
    QStringList categories; // X-axis categories (location names)

    for (const auto &location : litterData.keys()) {
      if (litterData[location].contains(selectedLitterType)) {
        *barSet << litterData[location][selectedLitterType];
      } else {
        *barSet << 0;
      }

      QString formattedLocation = location;
      formattedLocation = formattedLocation.left(1).toUpper() +
                          formattedLocation.mid(1).toLower();
      categories << formattedLocation;
    }

    barSeries->append(barSet);

    if (!chart->axes(Qt::Horizontal).isEmpty()) {
      chart->removeAxis(chart->axes(Qt::Horizontal).first());
    }
    if (!chart->axes(Qt::Vertical).isEmpty()) {
      chart->removeAxis(chart->axes(Qt::Vertical).first());
    }

    // Set up Y-axis
    QValueAxis *axisY = new QValueAxis(this);
    qreal maxY = 0;
    for (int i = 0; i < barSet->count(); ++i) {
      maxY = std::max(maxY, barSet->at(i));
    }
    qreal paddedMaxY = (maxY == 0) ? 1 : maxY + maxY * 0.1; // Add 10% padding
    axisY->setRange(0, paddedMaxY);
    axisY->setTitleText("Count");
    chart->addAxis(axisY, Qt::AlignLeft);

    // Set up X-axis
    QBarCategoryAxis *axisX = new QBarCategoryAxis(this);
    axisX->append(categories);
    axisX->setLabelsAngle(90);
    chart->addAxis(axisX, Qt::AlignBottom);

    chart->addSeries(barSeries);
    barSeries->attachAxis(axisY);
    barSeries->attachAxis(axisX);

    chart->setTitle("Distribution of " + selectedLitterType +
                    " Across Locations");
  } else {
    QMap<QString, int> aggregatedCounts;

    if (litterData.contains(selectedLocation) &&
        litterData[selectedLocation].contains(selectedLitterType)) {
      aggregatedCounts[selectedLitterType] =
          litterData[selectedLocation][selectedLitterType];
    }

    QString formattedLocation = selectedLocation;
    formattedLocation = formattedLocation.left(1).toUpper() +
                        formattedLocation.mid(1).toLower();

    chart->setTitle("Distribution of " + selectedLitterType + " in " +
                    formattedLocation);

    populatePieChart(aggregatedCounts);
  }
}
