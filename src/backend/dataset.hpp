// COMP2811 Coursework 1 sample solution: QuakeDataset class

#pragma once

#include "water_sample.hpp"
#include <QtWidgets>
#include <unordered_map>

class WaterDataset {
public:
  WaterDataset();
  WaterDataset(const QString &filename);
  void loadData(const QString &);
  int size() const { return data->size(); }
  bool hasElements() { return size() > 0; }
  const std::unordered_map<std::string, SamplingPoint *> *getData() {
    return data;
  }

  SamplingPoint *getFromNotation(string &notation);
  const std::unordered_map<std::string, SamplingPoint *> *getData() const {
    return data;
  }
  SamplingPoint *getFromLabel(std::string &label);

private:
  std::unordered_map<std::string, SamplingPoint *> *data;
};
