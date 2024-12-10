#ifndef FLUORINATED_COMPOUNDS_PAGE_HPP
#define FLUORINATED_COMPOUNDS_PAGE_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QChart>
#include <QChartView>
#include <QScatterSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QComboBox>
#include "dataset.hpp"

class FluorinatedCompoundsPage : public QWidget {
    Q_OBJECT

public:
    explicit FluorinatedCompoundsPage(QWidget *parent = nullptr);
    void updateData(WaterDataset* dataset);

    private slots:
        void handlePointClicked(const QPointF &point);
    void showDataPointDetails(const QPointF &point, const QString &location,
                            double concentration, const QString &dateTime);
    void handleLocationChanged(const QString& location); // 新增：处理地点选择变化

private:
    void setupUI();
    void createChart();
    void updateChart(const QString& selectedLocation = QString()); // 新增：更新图表数据
    QChart *chart;
    QChartView *chartView;
    QScatterSeries *safePoints;
    QScatterSeries *warningPoints;
    QScatterSeries *dangerPoints;
    QVBoxLayout *mainLayout;
    QComboBox *locationComboBox;  // 新增：地点选择下拉框
    WaterDataset* currentDataset;

    bool isPFASCompound(const QString &determinandLabel, const QString &definition);
    QString getPFASImplications(double concentration);
    double getSafetyThreshold() const { return 0.1; }
};

#endif // FLUORINATED_COMPOUNDS_PAGE_HPP