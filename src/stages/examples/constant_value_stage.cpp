#include "stages/examples/constant_value_stage.h"

#include <TParameter.h>
#include <spdlog/spdlog.h>

ClassImp(ConstantValueStage)

ConstantValueStage::ConstantValueStage() = default;

void ConstantValueStage::OnInit() {
    productName_ = parameters_.value("product_name", "constant_value");
    value_ = parameters_.value("value", 42.0);

    spdlog::debug("[{}] Initialized with name='{}', value={}", Name(), productName_, value_);
}

void ConstantValueStage::Process() {
    auto param = std::make_unique<TParameter<double>>(productName_.c_str(), value_);

    getDataProductManager()->withProducts([&](auto& products) {
        auto pdp = std::make_unique<PipelineDataProduct>();
        pdp->setName(productName_);
        pdp->setObject(std::move(param));
        products[productName_] = std::move(pdp);
    });

    spdlog::debug("[{}] Produced constant value {} for '{}'", Name(), value_, productName_);
}
