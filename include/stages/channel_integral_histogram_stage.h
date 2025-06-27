#ifndef CHANNEL_INTEGRAL_HISTOGRAM_STAGE_HH
#define CHANNEL_INTEGRAL_HISTOGRAM_STAGE_HH

#include "stages/base_stage.h"
#include <string>
#include <unordered_map>
#include <vector>

class TList;
class TH1D;

class ChannelIntegralHistogramStage : public BaseStage {
public:
    ChannelIntegralHistogramStage() = default;
    ~ChannelIntegralHistogramStage() override = default;

    void OnInit() override;
    void Process() override;

    std::string Name() const override { return "ChannelIntegralHistogramStage"; }

private:
    std::string inputLabel_;
    std::string outputLabel_;
    std::string titlePrefix_;

    int bins_ = 100;
    double min_ = 0.0;
    double max_ = 10000.0;

    bool autoMinMax_ = false;
    bool autoBinCount_ = false;

    void FillHistograms(TList* outputList, const TList* inputList);

    ClassDefOverride(ChannelIntegralHistogramStage, 1);
};

#endif // CHANNEL_INTEGRAL_HISTOGRAM_STAGE_HH
