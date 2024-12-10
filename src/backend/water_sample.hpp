#pragma once

#include <string>
#include <vector>

using namespace std;

/*
 *
 * csv contains:
 * sample.samplingPoint,
 * sample.samplingPoint.notation,
 * sample.samplingPoint.label,
 * sample.samplingPoint.easting,
 * sample.samplingPoint.northing
 *
 * sample.sampleDateTime,
 *
 * determinand.label,
 * determinand.definition,
 * determinand.notation,
 * determinand.unit.label,
 *
 * resultQualfier.notation,
 * result,
 * codedResultInterpretation.interpretation,
 * sample.sampledMaterialType.label,
 * sample.isComplianceSample,
 * sample.purpose.label,
 *
 *
 */

/*
 *
 * need a samplingPoint class
 * sampling point contains samples
 * samples contain determinands
 *
 */

class Determinand
{
public:
  // default constructor for a determinand to parse the information from csv
  // into class
  Determinand(string label, string definition, string notation,
              string unit_label, double result);
  // getter method
  string getLabel() const { return label; }
  string getDefinition() const { return definition; }
  string getNotation() const { return notation; }
  string getUnitLabel() const { return unit_label; }
  double getResult() const { return result; }

private:
  string label;
  string definition;
  string notation;
  string unit_label;
  double result;
};

class Sample
{
public:
  // default cosntructor defining the information shared between samples
  Sample(bool isComplianceSample, string purpose, string dateTime,
         string sampledMaterialType);
  // getter mehtods
  bool getIsComplianceSample() const { return isComplianceSample; }
  string getPurpose() const { return purpose; }
  string getDateTime() const { return dateTime; }
  string getSampledMaterialType() { return sampledMaterialType; }

  bool hasElements() { return determinands->size() > 0;}
  std::vector<Determinand*> *getDeterminands() { return determinands;}
  double getResultFromLabel(std::string label);

  void addDeterminand(Determinand *d) { determinands->push_back(d); }
  const std::vector<Determinand *> *getDeterminands() const { return determinands; }

private:
  bool isComplianceSample;
  string purpose;
  string dateTime;
  string sampledMaterialType;
  vector<Determinand *> *determinands;
};

class SamplingPoint
{
public:
  SamplingPoint(string notation, int northing, int easting, string label);
  // default getters to return point information
  string getNotation() const { return notation; }
  int getNorthing() const { return northing; }
  int getEasting() const { return easting; }
  string getLabel() const { return label; }
  int getNoSamples() const { return samples->size(); }
  bool hasSamples() const { return samples->size() > 0; }

  const vector<Sample *> *getSamples() { return samples; }

  Sample *getSampleFromDateTime(const string &dateTime);
  const std::vector<Sample *> *getSamples() const { return samples; }

  void addSample(Sample *s) { samples->push_back(s); }

private:
  string notation;
  int northing;
  int easting;
  string label;
  vector<Sample *> *samples;
};
