#ifndef HD_SOC_ANALYSIS_STAGES_NALU_WAVEFORMS_INTEGRATOR_STAGE_H
#define HD_SOC_ANALYSIS_STAGES_NALU_WAVEFORMS_INTEGRATOR_STAGE_H

#include "analysis_pipeline_core/stages/base_stage.h"

class NaluWaveformsIntegratorStage : public BaseStage {
public:
    NaluWaveformsIntegratorStage();
    ~NaluWaveformsIntegratorStage() override = default;

    void Process() override;
    std::string Name() const override { return "NaluWaveformsIntegratorStage"; }

protected:
    void OnInit() override;

private:
    std::string inputLabel_;
    std::string outputLabel_;

    ClassDefOverride(NaluWaveformsIntegratorStage, 1);
};

#endif // HD_SOC_ANALYSIS_STAGES_NALU_WAVEFORMS_INTEGRATOR_STAGE_H
