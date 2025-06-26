#ifndef EXAMPLE_PIPELINE_PLUGIN_CONSTANT_VALUE_STAGE_H
#define EXAMPLE_PIPELINE_PLUGIN_CONSTANT_VALUE_STAGE_H

#include "stages/base_stage.h"

class ConstantValueStage : public BaseStage {
public:
    ConstantValueStage();
    ~ConstantValueStage() override = default;

    void Process() override;
    std::string Name() const override { return "ConstantValueStage"; }

protected:
    void OnInit() override;

private:
    std::string productName_;
    double value_ = 42.0;

    ClassDefOverride(ConstantValueStage, 1);
};

#endif // EXAMPLE_PIPELINE_PLUGIN_CONSTANT_VALUE_STAGE_H
