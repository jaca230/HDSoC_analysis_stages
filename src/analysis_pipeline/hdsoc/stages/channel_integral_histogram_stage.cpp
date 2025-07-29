#include "analysis_pipeline/hdsoc/stages/channel_integral_histogram_stage.h"

#include <TList.h>
#include <TH1D.h>
#include <TObject.h>
#include <spdlog/spdlog.h>
#include <algorithm>

#include "analysis_pipeline/hdsoc/data_products/channel_integral.h"

using namespace dataProducts;

ClassImp(ChannelIntegralHistogramStage)

void ChannelIntegralHistogramStage::OnInit() {
    inputLabel_ = parameters_.value("input_product", "ChannelIntegralCollection");
    outputLabel_ = parameters_.value("product_name", "ChannelIntegralHistogramCollection");
    titlePrefix_ = parameters_.value("title_prefix", "Integral");
    bins_ = parameters_.value("bins", 100);

    // Check if relative range params are present
    bool hasRelMin = parameters_.contains("relative_min");
    bool hasRelMax = parameters_.contains("relative_max");
    useRelativeRange_ = hasRelMin && hasRelMax;

    if (useRelativeRange_) {
        relativeMin_ = parameters_.value("relative_min", -1000.0);
        relativeMax_ = parameters_.value("relative_max", 1000.0);
        spdlog::debug("[{}] Using relative range offsets: min={} max={}", Name(), relativeMin_, relativeMax_);
    } else {
        min_ = parameters_.value("min", 0.0);
        max_ = parameters_.value("max", 10000.0);
        spdlog::debug("[{}] Using fixed range: min={} max={}", Name(), min_, max_);
    }

    spdlog::debug("[{}] Initialized with input '{}', output '{}', bins={}", Name(), inputLabel_, outputLabel_, bins_);
}

void ChannelIntegralHistogramStage::Process() {
    if (!getDataProductManager()->hasProduct(inputLabel_)) {
        spdlog::warn("[{}] Input '{}' not found", Name(), inputLabel_);
        return;
    }

    auto inputHandle = getDataProductManager()->checkoutRead(inputLabel_);
    const auto* inputList = dynamic_cast<const TList*>(inputHandle->getObject());

    if (!inputList) {
        spdlog::error("[{}] Input product '{}' is not a TList", Name(), inputLabel_);
        return;
    }

    if (getDataProductManager()->hasProduct(outputLabel_)) {
        auto outHandle = getDataProductManager()->checkoutWrite(outputLabel_);
        TList* outputList = dynamic_cast<TList*>(outHandle->getObject());

        if (!outputList) {
            spdlog::error("[{}] Output product '{}' exists but is not a TList", Name(), outputLabel_);
            return;
        }

        FillHistograms(outputList, inputList);

        spdlog::debug("[{}] Processed {} entries into histogram list '{}'", Name(), inputList->GetSize(), outputLabel_);
    } else {
        auto newList = std::make_unique<TList>();
        newList->SetOwner(kTRUE);

        auto pdp = std::make_unique<PipelineDataProduct>();
        pdp->setName(outputLabel_);
        pdp->setObject(std::move(newList));
        pdp->addTag("HDSoC");
        pdp->addTag("histogram");
        pdp->addTag("histogram_list");
        pdp->addTag("built_by_channel_integral_histogram");
        getDataProductManager()->addOrUpdate(outputLabel_, std::move(pdp));

        auto outHandle = getDataProductManager()->checkoutWrite(outputLabel_);
        TList* outputList = dynamic_cast<TList*>(outHandle->getObject());

        if (!outputList) {
            spdlog::error("[{}] Newly created output product '{}' is not a TList", Name(), outputLabel_);
            return;
        }

        FillHistograms(outputList, inputList);

        spdlog::debug("[{}] Processed {} entries into newly created histogram list '{}'", Name(), inputList->GetSize(), outputLabel_);
    }
}

void ChannelIntegralHistogramStage::FillHistograms(TList* outputList, const TList* inputList) {
    // We need to know the first value per channel if relative range is used.
    // Let's store the first value seen per channel.
    std::unordered_map<int, double> firstValuePerChannel;

    for (const TObject* obj : *inputList) {
        auto* ci = dynamic_cast<const ChannelIntegral*>(obj);
        if (!ci) continue;

        std::string histName = "channel_" + std::to_string(ci->channel_num);
        auto* hist = dynamic_cast<TH1D*>(outputList->FindObject(histName.c_str()));

        if (!hist) {
            // Determine histogram range
            double histMin, histMax;
            if (useRelativeRange_) {
                // If first value for this channel not stored, store it now
                if (firstValuePerChannel.find(ci->channel_num) == firstValuePerChannel.end()) {
                    firstValuePerChannel[ci->channel_num] = ci->integral_value;
                }
                double base = firstValuePerChannel[ci->channel_num];
                histMin = base + relativeMin_;
                histMax = base + relativeMax_;
                if (histMin == histMax) histMax = histMin + 1.0;
            } else {
                histMin = min_;
                histMax = max_;
            }

            hist = new TH1D(histName.c_str(),
                            (titlePrefix_ + " - Ch " + std::to_string(ci->channel_num)).c_str(),
                            bins_, histMin, histMax);
            hist->SetDirectory(nullptr);
            outputList->Add(hist);
        }

        // Fill histogram
        hist->Fill(ci->integral_value);
    }
}
