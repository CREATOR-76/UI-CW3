#include "fluorinated_compounds_page.hpp"
#include <QDateTime>
#include <set>

using namespace std;

FluorinatedCompoundsPage::FluorinatedCompoundsPage(QWidget *parent)
    : QWidget(parent), currentDataset(nullptr) {
    setupUI();
}

void FluorinatedCompoundsPage::setupUI() {
    mainLayout = new QVBoxLayout(this);

    // 创建地点选择控件
    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* locationLabel = new QLabel("Select Location:", this);
    locationComboBox = new QComboBox(this);
    locationComboBox->addItem("All Locations");  // 添加默认选项

    filterLayout->addWidget(locationLabel);
    filterLayout->addWidget(locationComboBox);
    filterLayout->addStretch();  // 添加弹性空间

    mainLayout->addLayout(filterLayout);

    // 创建图表
    createChart();
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(chartView);

    connect(locationComboBox, &QComboBox::currentTextChanged,
            this, &FluorinatedCompoundsPage::handleLocationChanged);

    setLayout(mainLayout);
}

void FluorinatedCompoundsPage::createChart() {
    chart = new QChart();

    safePoints = new QScatterSeries();
    warningPoints = new QScatterSeries();
    dangerPoints = new QScatterSeries();

    safePoints->setMarkerSize(10.0);
    warningPoints->setMarkerSize(10.0);
    dangerPoints->setMarkerSize(10.0);

    safePoints->setColor(QColor(0, 255, 0));     // 绿色
    warningPoints->setColor(QColor(255, 140, 0));   // 黄色
    dangerPoints->setColor(QColor(255, 0, 0));    // 红色

    safePoints->setName("Safe Level");
    warningPoints->setName("Warning Level");
    dangerPoints->setName("Danger Level");

    chart->setTitle("PFAS Concentration Over Time");
    chart->addSeries(safePoints);
    chart->addSeries(warningPoints);
    chart->addSeries(dangerPoints);

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("Date");

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Concentration (μg/L)");
    axisY->setMin(0);
    axisY->setLabelFormat("%.6f");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    safePoints->attachAxis(axisX);
    safePoints->attachAxis(axisY);
    warningPoints->attachAxis(axisX);
    warningPoints->attachAxis(axisY);
    dangerPoints->attachAxis(axisX);
    dangerPoints->attachAxis(axisY);

    connect(safePoints, &QScatterSeries::clicked, this, &FluorinatedCompoundsPage::handlePointClicked);
    connect(warningPoints, &QScatterSeries::clicked, this, &FluorinatedCompoundsPage::handlePointClicked);
    connect(dangerPoints, &QScatterSeries::clicked, this, &FluorinatedCompoundsPage::handlePointClicked);
}

bool FluorinatedCompoundsPage::isPFASCompound(const QString &determinandLabel, const QString &definition) {
    static const QStringList keywords = {"perfluoro", "Perfluoro", "pfas", "fluor", "Pfas", "Fluor"};

    QString label = determinandLabel.toLower();
    QString def = definition.toLower();

    for (const QString& keyword : keywords) {
        if (label.contains(keyword) || def.contains(keyword)) {
            return true;
        }
    }
    return false;
}

void FluorinatedCompoundsPage::handleLocationChanged(const QString& location) {
    updateChart(location);
}

void FluorinatedCompoundsPage::updateData(WaterDataset* dataset) {
    currentDataset = dataset;
    if (!dataset || !dataset->getData()) return;

    locationComboBox->clear();
    locationComboBox->addItem("All Locations");

    set<string> locations;  // 使用set去重

    // 只收集有效 PFAS 数据的地点（result > 0）
    for (const auto& samplingPoint : *dataset->getData()) {
        if (!samplingPoint.second->getSamples()) continue;

        bool hasPFAS = false;
        for (const auto& sample : *samplingPoint.second->getSamples()) {
            if (!sample->getDeterminands()) continue;

            for (const auto& determinand : *sample->getDeterminands()) {
                if (isPFASCompound(
                        QString::fromStdString(determinand->getLabel()),
                        QString::fromStdString(determinand->getDefinition()))) {
                    // 检查测定值是否大于0
                    if (determinand->getResult() > 0) {
                        locations.insert(samplingPoint.second->getLabel());
                        hasPFAS = true;
                        break;
                    }
                        }
            }
            if (hasPFAS) break;
        }
    }

    // 添加到下拉框
    for (const auto& location : locations) {
        locationComboBox->addItem(QString::fromStdString(location));
    }

    // 更新图表
    updateChart();
}

void FluorinatedCompoundsPage::updateChart(const QString& selectedLocation) {
    if (!currentDataset) return;

    QVector<QPointF> safeData;
    QVector<QPointF> warningData;
    QVector<QPointF> dangerData;

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    QDateTime firstDate, lastDate;

    double threshold = getSafetyThreshold();
    bool filterLocation = !selectedLocation.isEmpty() && selectedLocation != "All Locations";

    for (const auto& samplingPoint : *currentDataset->getData()) {
        // 地点过滤
        if (filterLocation &&
            QString::fromStdString(samplingPoint.second->getLabel()) != selectedLocation) {
            continue;
        }

        if (!samplingPoint.second->getSamples()) continue;

        for (const auto& sample : *samplingPoint.second->getSamples()) {
            if (!sample->getDeterminands()) continue;

            QDateTime dateTime = QDateTime::fromString(
                QString::fromStdString(sample->getDateTime()),
                Qt::ISODate
            );
            if (!dateTime.isValid()) continue;
            qint64 timestamp = dateTime.toMSecsSinceEpoch();

            for (const auto& determinand : *sample->getDeterminands()) {
                if (isPFASCompound(
                        QString::fromStdString(determinand->getLabel()),
                        QString::fromStdString(determinand->getDefinition()))) {

                    double value = determinand->getResult();
                    // 只处理大于0的值
                    if (value > 0) {
                        QPointF point(timestamp, value);
                        if (value <= threshold) {
                            safeData.append(point);
                        } else if (value <= threshold * 2) {
                            warningData.append(point);
                        } else {
                            dangerData.append(point);
                        }

                        minY = qMin(minY, value);
                        maxY = qMax(maxY, value);

                        if (firstDate.isNull() || dateTime < firstDate) firstDate = dateTime;
                        if (lastDate.isNull() || dateTime > lastDate) lastDate = dateTime;
                    }
                }
            }
        }
    }

    // 一次性替换所有点
    safePoints->replace(safeData);
    warningPoints->replace(warningData);
    dangerPoints->replace(dangerData);

    int totalPoints = safeData.size() + warningData.size() + dangerData.size();

    if (totalPoints > 0) {
        if (QDateTimeAxis *axisX = qobject_cast<QDateTimeAxis*>(chart->axes(Qt::Horizontal).first())) {
            axisX->setRange(firstDate, lastDate);
        }

        if (QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first())) {
            double margin = (maxY - minY) * 0.1;
            axisY->setRange(0, maxY + margin);
        }
    } else {
        if (QDateTimeAxis *axisX = qobject_cast<QDateTimeAxis*>(chart->axes(Qt::Horizontal).first())) {
            axisX->setRange(QDateTime::currentDateTime().addMonths(-1),
                           QDateTime::currentDateTime());
        }
        if (QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first())) {
            axisY->setRange(0, 1.0);
        }
    }
}

void FluorinatedCompoundsPage::handlePointClicked(const QPointF &point) {
    if (!currentDataset || !currentDataset->getData()) return;

    QDateTime clickedDateTime = QDateTime::fromMSecsSinceEpoch(point.x());
    double concentration = point.y();

    for (const auto& samplingPoint : *currentDataset->getData()) {
        if (!samplingPoint.second->getSamples()) continue;

        for (const auto& sample : *samplingPoint.second->getSamples()) {
            QDateTime sampleDateTime = QDateTime::fromString(
                QString::fromStdString(sample->getDateTime()),
                Qt::ISODate
            );

            if (sampleDateTime == clickedDateTime) {
                QString location = QString::fromStdString(samplingPoint.second->getLabel());
                showDataPointDetails(point, location, concentration,
                                   clickedDateTime.toString(Qt::ISODate));
                return;
            }
        }
    }
}

void FluorinatedCompoundsPage::showDataPointDetails(const QPointF &point,
                                                   const QString &location,
                                                   double concentration,
                                                   const QString &dateTime) {
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("PFAS Sample Details");

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    layout->addWidget(new QLabel(QString("Location: %1").arg(location)));
    layout->addWidget(new QLabel(QString("Date: %1").arg(dateTime)));
    layout->addWidget(new QLabel(QString("Concentration: %1 μg/L").arg(
        QString::number(concentration, 'f', 6))));

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    layout->addWidget(line);

    layout->addWidget(new QLabel("Implications:"));
    layout->addWidget(new QLabel(getPFASImplications(concentration)));

    QPushButton *closeButton = new QPushButton("Close");
    layout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);

    dialog->setLayout(layout);
    dialog->exec();
}

QString FluorinatedCompoundsPage::getPFASImplications(double concentration) {
    double threshold = getSafetyThreshold();

    if (concentration <= threshold) {
        return "Safe level: Below regulatory threshold.\n"
               "No immediate health concerns.";
    } else if (concentration <= threshold * 2) {
        return "Moderate level: Above regulatory threshold.\n"
               "Potential long-term health effects.\n"
               "Recommended for monitoring.";
    } else {
        return "High level: Significantly above threshold.\n"
               "Immediate action recommended.\n"
               "Potential health and environmental risks.";
    }
}