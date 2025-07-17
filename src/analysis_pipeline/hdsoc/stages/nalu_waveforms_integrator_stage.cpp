#include "analysis_pipeline/hdsoc/stages/nalu_waveforms_integrator_stage.h"

#include <numeric>
#include <spdlog/spdlog.h>
#include <TList.h>

#include "unpacker/nalu/data_products/NaluWaveform.hh"
#include "analysis_pipeline/hdsoc/data_products/channel_integral.h"

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

    auto list = std::make_unique<TList>();
    list->SetOwner(kTRUE);  // Important: ensures deletion of contained objects

    try {
        auto lock = getDataProductManager()->checkoutRead(inputLabel_);
        const auto* waveformList = dynamic_cast<const TList*>(lock->getObject());

        if (!waveformList) {
            spdlog::error("[{}] Failed to cast input to TList", Name());
            return;
        }

        int count = 0;
        for (const TObject* obj : *waveformList) {
            auto* waveform = dynamic_cast<const NaluWaveform*>(obj);
            if (!waveform) continue;

            double sum = std::accumulate(waveform->trace.begin(), waveform->trace.end(), 0.0);
            auto* ci = new ChannelIntegral(waveform->channel_num, sum);
            list->Add(ci);
            ++count;
        }

        spdlog::debug("[{}] Integrated {} waveforms into TList", Name(), count);
    } catch (const std::exception& e) {
        spdlog::error("[{}] Exception while reading '{}': {}", Name(), inputLabel_, e.what());
        return;
    }

    auto pdp = std::make_unique<PipelineDataProduct>();
    pdp->setName(outputLabel_);
    pdp->setObject(std::move(list));
    pdp->addTag("HDSoC");
    pdp->addTag("integral");
    pdp->addTag("sum");
    pdp->addTag("integral_list");
    pdp->addTag("sum_list");
    pdp->addTag("built_by_nalu_waveforms_integrator");
    getDataProductManager()->addOrUpdate(outputLabel_, std::move(pdp));
}

