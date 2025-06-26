#ifndef CHANNEL_INTEGRAL_HISTOGRAM_STAGE_HH
#define CHANNEL_INTEGRAL_HISTOGRAM_STAGE_HH

#include "stages/base_stage.h"

#include <string>

/**
 * @class ChannelIntegralHistogramStage
 * @brief A pipeline stage that reads a TList of ChannelIntegral objects
 *        and accumulates one TH1D histogram per channel number.
 *
 * Each histogram is stored in a TList under a configurable output label.
 */
class ChannelIntegralHistogramStage : public BaseStage {
public:
    ChannelIntegralHistogramStage() = default;
    ~ChannelIntegralHistogramStage() override = default;

    void OnInit() override;
    void Process() override;

    std::string Name() const override { return "ChannelIntegralHistogramStage"; }

private:
    std::string inputLabel_;   ///< Name of the input data product (TList of ChannelIntegral)
    std::string outputLabel_;  ///< Name of the output data product (TList of TH1D)
    std::string titlePrefix_;  ///< Histogram title prefix
    int bins_ = 100;
    double min_ = 0.0;
    double max_ = 10000.0;

    void FillHistograms(TList* outputList, const TList* inputList);

    ClassDefOverride(ChannelIntegralHistogramStage, 1);
};

#endif // CHANNEL_INTEGRAL_HISTOGRAM_STAGE_HH
