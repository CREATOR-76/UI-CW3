#include "water_sample.hpp"
#include <algorithm>
#include <vector>

using namespace std;

Determinand::Determinand(string label, string definition, string notation,
                         string unit_label, double result)
    : label(label), definition(definition), notation(notation),
      unit_label(unit_label), result(result) {}

Sample::Sample(bool isComplianceSample, string purpose, string dateTime,
               string sampledMaterialType)
    : isComplianceSample(isComplianceSample), purpose(purpose),
      dateTime(dateTime), sampledMaterialType(sampledMaterialType) {
  determinands = new vector<Determinand *>();
}

SamplingPoint::SamplingPoint(string notation, int northing, int easting,
                             string label)
    : notation(notation), northing(northing), easting(easting), label(label) {
  samples = new vector<Sample *>();
}

Sample *SamplingPoint::getSampleFromDateTime(const string &dateTime) {
  if (!hasSamples())
    return nullptr;

  auto sample =
      find_if(samples->begin(), samples->end(),
              [&dateTime](Sample *s) { return s->getDateTime() == dateTime; });

  if (sample != samples->end())
    return *sample;
  return nullptr;
}

double Sample::getResultFromLabel(std::string label) {
  auto det =
      find_if(determinands->begin(), determinands->end(),
              [&label](Determinand *d) { return d->getLabel() == label; });

  if (det != determinands->end()) {
    return (*det)->getResult();
  }
  return -1;
}
