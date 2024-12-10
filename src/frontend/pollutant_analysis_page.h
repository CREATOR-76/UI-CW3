#ifndef POLLUTANT_ANALYSIS_PAGE_H
#define POLLUTANT_ANALYSIS_PAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QVector>
#include <QComboBox>
#include <QDateTime>
#include <QLineEdit>
#include "dataset.hpp"

class PollutantAnalysisPage : public QWidget {
    Q_OBJECT

public:
    explicit PollutantAnalysisPage(QWidget *parent = nullptr);
    void updateData(WaterDataset* newDataset);

private slots:
    void handleTimeFilterChange(const QString &period);
    void handleLocationFilterChange(const QString &location);

    void onFileSelected(QString filename);

    void performSearch(const QString &searchTerm);

private:
    void setupDashboard();
    void setupTimeRangeSelector();
    void setupSearch();
    void createPollutantCard(const QString &title, const QString &summary,
                            QLayout *parentLayout, const QVector<QPointF> &dataPoints);
    void updatePollutantCard(int index, const QVector<QPointF> &dataPoints);
    void updateCards();
    void toggleSearchChartVisibility(bool visible);
    QVector<QPointF> filterDataByTimeRange(const QVector<QPointF> &data);
    void applyTimeRangeFilter(const QString &timeRange);

    WaterDataset* dataset;
    QVBoxLayout *cardsLayout;
    QVector<QChartView*> chartViews;
    QComboBox *timeFilter;
    QComboBox *locationFilter;
    QChartView *searchChartView;
    QLineEdit *searchBar;
    QComboBox *timeRangeComboBox;
};

#endif // POLLUTANT_ANALYSIS_PAGE_H