#include "pollutant_analysis_page.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QDateTime>
#include <QPushButton>
#include <QMessageBox>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QDebug>
#include <QtCharts/QScatterSeries>

PollutantAnalysisPage::PollutantAnalysisPage(QWidget *parent)
    : QWidget(parent), dataset(nullptr) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Initialize time range selector
    setupTimeRangeSelector();

    // Setup the dashboard
    setupDashboard();

    // Setup search functionality
    setupSearch();

    setLayout(mainLayout);
}

void PollutantAnalysisPage::setupTimeRangeSelector() {
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout *>(layout());

    QHBoxLayout *timeRangeLayout = new QHBoxLayout();
    QLabel *timeRangeLabel = new QLabel("Select Time Range:", this);
    timeRangeComboBox = new QComboBox(this);
    timeRangeComboBox->addItem("All", "all");
    timeRangeComboBox->addItem("Last Month", "last_month");
    timeRangeComboBox->addItem("Last half year", "last_half_year");

    timeRangeLayout->addWidget(timeRangeLabel);
    timeRangeLayout->addWidget(timeRangeComboBox);
    mainLayout->addLayout(timeRangeLayout);

    connect(timeRangeComboBox, &QComboBox::currentTextChanged, this, [this]() {
        QString selectedRange = timeRangeComboBox->currentData().toString();
        applyTimeRangeFilter(selectedRange);
    });
}

void PollutantAnalysisPage::setupDashboard() {
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout *>(layout());

    QLabel *titleLabel = new QLabel("Water Quality Dashboard", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    QWidget *cardsContainer = new QWidget(this);
    cardsLayout = new QVBoxLayout(cardsContainer);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(cardsContainer);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea);

    createPollutantCard("Pollutant Overview", "General water quality metrics", cardsLayout, {});
    createPollutantCard("Pops", "Persistent Organic Pollutants levels", cardsLayout, {});
    createPollutantCard("Litter Indicators", "Water litter indicators", cardsLayout, {});
    createPollutantCard("Fluorinated Compounds", "Fluorinated compounds concentration", cardsLayout, {});
}

void PollutantAnalysisPage::createPollutantCard(const QString &title, const QString &summary,
                                     QLayout *parentLayout, const QVector<QPointF> &dataPoints) {
    QFrame *card = new QFrame(this);
    card->setFrameStyle(QFrame::Box | QFrame::Raised);
    card->setLineWidth(1);
    card->setStyleSheet("QFrame { background-color: white; border-radius: 8px; padding: 10px; margin: 5px; }");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);

    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    cardLayout->addWidget(titleLabel);

    QLabel *summaryLabel = new QLabel(summary, card);
    summaryLabel->setWordWrap(true);
    cardLayout->addWidget(summaryLabel);

    QChart *chart = new QChart();
    chart->setTitle(title);

    QChartView *chartView = new QChartView(chart, card);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(200);

    cardLayout->addWidget(chartView);
    parentLayout->addWidget(card);

    chartViews.append(chartView);
}

void PollutantAnalysisPage::updateData(WaterDataset* newDataset) {
    dataset = newDataset;
    if (!dataset) {
        qDebug() << "No dataset provided to update.";
        return;
    }

    qDebug() << "Dataset updated in PollutantAnalysisPage";
    updateCards();
}

void PollutantAnalysisPage::handleTimeFilterChange(const QString &period) {
    qDebug() << "Time filter changed to:" << period;
    updateCards();
}

void PollutantAnalysisPage::handleLocationFilterChange(const QString &location) {
    qDebug() << "Location filter changed to:" << location;
    updateCards();
}

void PollutantAnalysisPage::onFileSelected(QString filename) {
    qDebug() << "File selected:" << filename;

    if (!filename.isEmpty()) {
        if (dataset) {
            delete dataset; // Clear the old dataset
            dataset = nullptr;
        }

        dataset = new WaterDataset();
        dataset->loadData(filename);

        updateData(dataset);

        QMessageBox::information(this, "File Loaded", "Dataset successfully loaded!");
    } else {
        QMessageBox::warning(this, "File Selection", "No file selected.");
    }
}

void PollutantAnalysisPage::updateCards() {
    if (!dataset) {
        qDebug() << "No dataset available for updating cards.";
        return;
    }

    QVector<QPointF> overviewData;
    QVector<QPointF> popsData;
    QVector<QPointF> litterData;
    QVector<QPointF> fluorinatedData;

    // Traverse dataset and aggregate data
    for (const auto &entry : *dataset->getData()) {
        SamplingPoint *point = entry.second;

        for (const auto &sample : *point->getSamples()) {
            QDateTime sampleDateTime = QDateTime::fromString(
                QString::fromStdString(sample->getDateTime()), Qt::ISODate);
            if (!sampleDateTime.isValid()) continue; // Skip invalid dates

            for (const auto &determinand : *sample->getDeterminands()) {
                QString label = QString::fromStdString(determinand->getLabel());
                double result = determinand->getResult();

                overviewData.append(QPointF(sampleDateTime.toMSecsSinceEpoch(), result));

                if (label.contains("Phenoxy", Qt::CaseInsensitive) ||
                    label.contains("Endrin", Qt::CaseInsensitive)) {
                    popsData.append(QPointF(sampleDateTime.toMSecsSinceEpoch(), result));
                }

                if ((determinand->getUnitLabel() == "garber c") ||  // Check for specific unit label
                    label.contains("Plastic", Qt::CaseInsensitive) ||
                    label.contains("Microplastic", Qt::CaseInsensitive)) {
                    litterData.append(QPointF(sampleDateTime.toMSecsSinceEpoch(), result));
                }

                if (label.contains("Fluorinated", Qt::CaseInsensitive) ||
                    label.contains("Fluoride", Qt::CaseInsensitive)) {
                    fluorinatedData.append(QPointF(sampleDateTime.toMSecsSinceEpoch(), result));
                }
            }
        }
    }

    // Sort data by time
    auto sortByTime = [](const QPointF &a, const QPointF &b) {
        return a.x() < b.x();
    };

    std::sort(overviewData.begin(), overviewData.end(), sortByTime);
    std::sort(popsData.begin(), popsData.end(), sortByTime);
    std::sort(litterData.begin(), litterData.end(), sortByTime);
    std::sort(fluorinatedData.begin(), fluorinatedData.end(), sortByTime);

    // Update pollutant cards
    updatePollutantCard(0, overviewData);
    updatePollutantCard(1, popsData);
    updatePollutantCard(2, litterData);
    updatePollutantCard(3, fluorinatedData);
}

void PollutantAnalysisPage::updatePollutantCard(int index, const QVector<QPointF> &dataPoints) {
    if (index < 0 || index >= chartViews.size())
        return;

    QChartView *chartView = chartViews[index];
    QChart *chart = chartView->chart();

    // Clear existing chart data and axes
    chart->removeAllSeries();
    QList<QAbstractAxis *> oldAxes = chart->axes();
    for (QAbstractAxis *axis : oldAxes) {
        chart->removeAxis(axis);
        delete axis;
    }

    // Create a new line series
    QLineSeries *series = new QLineSeries();

    // Add point data
    for (const auto &point : dataPoints) {
        series->append(point);
    }

    chart->addSeries(series);

    // Create and set the date axis
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("yyyy-MM-dd"); // Date format
    axisX->setTitleText("Date");
    axisX->setTickCount(10);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Create and set the value axis
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Value");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->setTitle(chart->title());
}

void PollutantAnalysisPage::setupSearch() {
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout *>(layout());

    // Search bar
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Search for a pollutant...");
    QPushButton *searchButton = new QPushButton("Search", this);

    searchLayout->addWidget(searchBar);
    searchLayout->addWidget(searchButton);
    mainLayout->addLayout(searchLayout);

    connect(searchButton, &QPushButton::clicked, this, [this]() {
        performSearch(searchBar->text());
    });

    // Search result chart
    QFrame *searchCard = new QFrame(this);
    searchCard->setFrameStyle(QFrame::Box | QFrame::Raised);
    searchCard->setLineWidth(1);
    searchCard->setStyleSheet("QFrame { background-color: white; border-radius: 8px; padding: 10px; margin: 5px; }");

    QVBoxLayout *searchCardLayout = new QVBoxLayout(searchCard);
    QLabel *searchTitle = new QLabel("Search Results", searchCard);
    searchTitle->setStyleSheet("font-weight: bold; font-size: 16px;");
    searchCardLayout->addWidget(searchTitle);

    QChart *searchChart = new QChart();
    searchChart->setTitle("Search Results");

    searchChartView = new QChartView(searchChart, searchCard);
    searchChartView->setRenderHint(QPainter::Antialiasing);
    searchChartView->setMinimumHeight(200);

    searchCardLayout->addWidget(searchChartView);
    mainLayout->addWidget(searchCard);

    // Hide search chart by default
    searchCard->setVisible(false);
}

void PollutantAnalysisPage::performSearch(const QString &searchTerm) {
    if (!dataset) {
        QMessageBox::warning(this, "No Data", "Please load a dataset before searching.");
        toggleSearchChartVisibility(false); // Hide search chart
        return;
    }

    QVector<QPointF> searchData;

    // Traverse the dataset and filter matching pollutants
    for (const auto &entry : *dataset->getData()) {
        SamplingPoint *point = entry.second;

        for (const auto &sample : *point->getSamples()) {
            QDateTime sampleDateTime = QDateTime::fromString(
                QString::fromStdString(sample->getDateTime()), Qt::ISODate);
            if (!sampleDateTime.isValid()) continue; // Skip invalid dates

            for (const auto &determinand : *sample->getDeterminands()) {
                QString label = QString::fromStdString(determinand->getLabel());
                double result = determinand->getResult();

                // Match search term
                if (label.contains(searchTerm, Qt::CaseInsensitive)) {
                    searchData.append(QPointF(sampleDateTime.toMSecsSinceEpoch(), result));
                }
            }
        }
    }

    // Sort by time
    std::sort(searchData.begin(), searchData.end(), [](const QPointF &a, const QPointF &b) {
        return a.x() < b.x();
    });

    // Update the search chart
    if (searchData.isEmpty()) {
        QMessageBox::information(this, "No Results", "No data found for the specified pollutant.");
        toggleSearchChartVisibility(false); // Hide search chart
    } else {
        toggleSearchChartVisibility(true); // Show search chart

        QChart *chart = searchChartView->chart();

        // **Clear all old Series and Axes**
        chart->removeAllSeries();
        QList<QAbstractAxis *> oldAxes = chart->axes();
        for (QAbstractAxis *axis : oldAxes) {
            chart->removeAxis(axis);
            delete axis;
        }

        // Create new line chart
        QLineSeries *series = new QLineSeries();
        for (const auto &point : searchData) {
            series->append(point); // Add points in sorted order
        }

        chart->addSeries(series);

        // Create and set date axis
        QDateTimeAxis *axisX = new QDateTimeAxis();
        axisX->setFormat("yyyy-MM-dd");
        axisX->setTitleText("Date");
        axisX->setTickCount(10);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        // Create and set value axis
        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Value");
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        chart->setTitle(QString("Search Results for '%1'").arg(searchTerm));
    }
}

void PollutantAnalysisPage::toggleSearchChartVisibility(bool visible) {
    if (searchChartView && searchChartView->parentWidget()) {
        searchChartView->parentWidget()->setVisible(visible);
        if (!visible) {
            QChart *chart = searchChartView->chart();
            if (chart) {
                // **清理所有旧的 Series 和 Axes**
                chart->removeAllSeries();
                QList<QAbstractAxis *> oldAxes = chart->axes();
                for (QAbstractAxis *axis : oldAxes) {
                    chart->removeAxis(axis);
                    delete axis;
                }
                chart->setTitle(""); // 清除图表标题
            }
        }
    }
}

QVector<QPointF> PollutantAnalysisPage::filterDataByTimeRange(const QVector<QPointF> &data) {
    QString timeRange = timeRangeComboBox->currentData().toString(); // 获取用户选择的时间范围
    if (timeRange == "all") {
        return data; // 不过滤，返回所有数据
    }

    QDateTime now = QDateTime::currentDateTime();
    QDateTime cutoffDate;

    if (timeRange == "last_month") {
        cutoffDate = now.addMonths(-1); // 最近一个月的截止日期
    } else if (timeRange == "last_year") {
        cutoffDate = now.addYears(-1); // 最近一年的截止日期
    }

    QVector<QPointF> filteredData;
    for (const auto &point : data) {
        QDateTime pointDate = QDateTime::fromMSecsSinceEpoch(point.x());
        if (pointDate >= cutoffDate) { // 只保留在截止日期之后的数据点
            filteredData.append(point);
        }
    }

    return filteredData; // 返回过滤后的数据
}

void PollutantAnalysisPage::applyTimeRangeFilter(const QString &timeRange) {
    if (!dataset) {
        qDebug() << "No dataset available for filtering.";
        return;
    }

    // Find the latest timestamp in the dataset
    QDateTime latestTime;
    for (const auto &entry : *dataset->getData()) {
        SamplingPoint *point = entry.second;

        for (const auto &sample : *point->getSamples()) {
            QDateTime sampleDateTime = QDateTime::fromString(
                QString::fromStdString(sample->getDateTime()), Qt::ISODate);
            if (sampleDateTime.isValid() && sampleDateTime > latestTime) {
                latestTime = sampleDateTime;
            }
        }
    }

    if (!latestTime.isValid()) {
        QMessageBox::warning(this, "No Data", "Dataset contains no valid timestamps.");
        return;
    }

    qDebug() << "Latest time in dataset:" << latestTime;

    // Determine the start time based on the selected range
    QDateTime startTime;
    if (timeRange == "last_month") {
        startTime = latestTime.addMonths(-1); // Go back one month
    } else if (timeRange == "last_half_year") {
        startTime = latestTime.addMonths(-6); // Go back one season (3 months)
    } else {
        startTime = QDateTime(); // Include all data
    }

    qDebug() << "Time range filter applied:" << startTime << "to" << latestTime;

    // Initialize data vectors for all categories
    QVector<QPointF> overviewData;
    QVector<QPointF> popsData;
    QVector<QPointF> litterData;
    QVector<QPointF> fluorinatedData;

    // Filter the data based on the time range
    for (const auto &entry : *dataset->getData()) {
        SamplingPoint *point = entry.second;

        for (const auto &sample : *point->getSamples()) {
            QDateTime sampleDateTime = QDateTime::fromString(
                QString::fromStdString(sample->getDateTime()), Qt::ISODate);

            if (!sampleDateTime.isValid() || sampleDateTime < startTime || sampleDateTime > latestTime)
                continue; // Skip invalid or out-of-range timestamps

            for (const auto &determinand : *sample->getDeterminands()) {
                QString label = QString::fromStdString(determinand->getLabel());
                double result = determinand->getResult();

                // Aggregate data into the correct category
                overviewData.append(QPointF(sampleDateTime.toMSecsSinceEpoch(), result));

                if (label.contains("Phenoxy", Qt::CaseInsensitive) ||
                    label.contains("Endrin", Qt::CaseInsensitive)) {
                    popsData.append(QPointF(sampleDateTime.toMSecsSinceEpoch(), result));
                }

                if ((determinand->getUnitLabel() == "garber c") ||  // Check for specific unit label
                    label.contains("Plastic", Qt::CaseInsensitive) ||
                    label.contains("Microplastic", Qt::CaseInsensitive)) {
                    litterData.append(QPointF(sampleDateTime.toMSecsSinceEpoch(), result));
                }

                if (label.contains("Fluorinated", Qt::CaseInsensitive) ||
                    label.contains("Fluoride", Qt::CaseInsensitive)) {
                    fluorinatedData.append(QPointF(sampleDateTime.toMSecsSinceEpoch(), result));
                }
            }
        }
    }

    // Sort the data by time (ascending order)
    auto sortByTime = [](const QPointF &a, const QPointF &b) {
        return a.x() < b.x();
    };

    std::sort(overviewData.begin(), overviewData.end(), sortByTime);
    std::sort(popsData.begin(), popsData.end(), sortByTime);
    std::sort(litterData.begin(), litterData.end(), sortByTime);
    std::sort(fluorinatedData.begin(), fluorinatedData.end(), sortByTime);

    // Update all pollutant cards
    updatePollutantCard(0, overviewData);
    updatePollutantCard(1, popsData);
    updatePollutantCard(2, litterData);
    updatePollutantCard(3, fluorinatedData);
}