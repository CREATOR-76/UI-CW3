// COMP2811 Coursework 1 sample solution: QuakeDataset class

#include "dataset.hpp"
#include "csv.hpp"
#include "water_sample.hpp"
#include <QWidget>
#include <string>
#include <unordered_map>

using namespace std;

WaterDataset::WaterDataset() {
  data = new std::unordered_map<string, SamplingPoint *>;
}
WaterDataset::WaterDataset(const QString &filename) {
  data = new std::unordered_map<string, SamplingPoint *>;
  loadData(filename);
}

SamplingPoint *WaterDataset::getFromNotation(string &notation) {
  auto p = (*data).find(notation);
  if (p == data->end())
    return nullptr;
  else
    return p->second;
}

SamplingPoint *WaterDataset::getFromLabel(string &label) {
  for (auto p : *data) {
    if (p.second->getLabel() == label) {
      return p.second;
    }
  }
  return nullptr;
}

void WaterDataset::loadData(const QString &filename) {
  csv::CSVReader reader(filename.toStdString());

  data->clear();
  int sum = 0;
  int sum_points = 0;
  int samples = 0;
  for (const auto &row : reader) {
    auto samplingPoint = row["sample.samplingPoint.notation"].get<>();
    auto northing = row["sample.samplingPoint.northing"].get<int>();
    auto easting = row["sample.samplingPoint.easting"].get<int>();
    auto samplingPointLabel = row["sample.samplingPoint.label"].get<>();

    auto samplePurposeLabel = row["sample.purpose.label"].get<>();
    auto materialType = row["sample.sampledMaterialType.label"].get<>();
    auto datetime = row["sample.sampleDateTime"].get<>();

    auto determinandLabel = row["determinand.label"].get<>();
    auto determinandDef = row["determinand.definition"].get<>();
    auto determinandNotation = row["determinand.notation"].get<>();
    auto determinandUnitLabel = row["determinand.unit.label"].get<>();
    auto result = row["result"].get<double>();

    bool isComp;
    if (row["sample.isComplianceSample"].get<>() == "true") {
      isComp = true;
    } else {
      isComp = false;
    }

    auto p = getFromNotation(samplingPoint);
    if (p == nullptr) {
      p = new SamplingPoint(samplingPoint, northing, easting,
                            samplingPointLabel);
      (*data)[samplingPoint] = p;
    }

    Sample *s = p->getSampleFromDateTime(datetime);
    if (!s) {
      s = new Sample(isComp, samplePurposeLabel, datetime, materialType);
      p->addSample(s);
    }

    Determinand *d =
        new Determinand(determinandLabel, determinandDef, determinandNotation,
                        determinandUnitLabel, result);
    s->addDeterminand(d);
    sum++;
  }
}
