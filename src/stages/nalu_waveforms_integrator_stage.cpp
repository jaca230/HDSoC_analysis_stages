#include "stages/nalu_waveforms_integrator_stage.h"

#include <numeric>
#include <spdlog/spdlog.h>

#include "nalu/data_products/NaluWaveform.hh"
#include "data_products/channel_integral.h"

using namespace dataProducts;

ClassImp(NaluWaveformsIntegratorStage)

NaluWaveformsIntegratorStage::NaluWaveformsIntegratorStage() = default;

void NaluWaveformsIntegratorStage::OnInit() {
    inputLabel_ = parameters_.value("input_product", "NaluWaveformCollection");
    outputLabel_ = parameters_.value("product_name", "ChannelIntegralCollection");

    spdlog::debug("[{}] Initialized with input='{}', output='{}'",
                  Name(), inputLabel_, outputLabel_);
}

void NaluWaveformsIntegratorStage::Process() {
    if (!getDataProductManager()->hasProduct(inputLabel_)) {
        spdlog::warn("[{}] Input product '{}' not found", Name(), inputLabel_);
        return;
    }

    auto integrals = std::make_unique<ChannelIntegralCollection>();

    try {
        auto lock = getDataProductManager()->checkoutRead(inputLabel_);
        const auto* waveforms = dynamic_cast<const NaluWaveformCollection*>(lock->getObject());

        if (!waveforms) {
            spdlog::error("[{}] Failed to cast object to NaluWaveformCollection", Name());
            return;
        }

        for (const auto& waveform : *waveforms) {
            double sum = std::accumulate(waveform.trace.begin(), waveform.trace.end(), 0.0);
            ChannelIntegral ci(waveform.channel_num, sum);
            integrals->emplace_back(ci);
        }

        spdlog::debug("[{}] Integrated {} waveforms", Name(), waveforms->size());
    } catch (const std::exception& e) {
        spdlog::error("[{}] Exception while reading '{}': {}", Name(), inputLabel_, e.what());
        return;
    }

    auto pdp = std::make_unique<PipelineDataProduct>();
    pdp->setName(outputLabel_);
    pdp->setObject(std::move(integrals));
    getDataProductManager()->addOrUpdate(outputLabel_, std::move(pdp));
}
