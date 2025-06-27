#include "stages/channel_integral_histogram_stage.h"

#include <TList.h>
#include <TH1D.h>
#include <TObject.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>

#include "data_products/channel_integral.h"

using namespace dataProducts;

ClassImp(ChannelIntegralHistogramStage)

void ChannelIntegralHistogramStage::OnInit() {
    inputLabel_ = parameters_.value("input_product", "ChannelIntegralCollection");
    outputLabel_ = parameters_.value("product_name", "ChannelIntegralHistogramCollection");
    titlePrefix_ = parameters_.value("title_prefix", "Integral");

    autoMinMax_ = !(parameters_.contains("min") && parameters_.contains("max"));
    autoBinCount_ = !parameters_.contains("bins");

    if (!autoMinMax_) {
        min_ = parameters_.value("min", 0.0);
        max_ = parameters_.value("max", 10000.0);
    }

    if (!autoBinCount_) {
        bins_ = parameters_.value("bins", 100);
    }

    spdlog::debug("[{}] Initialized with input '{}', output '{}', autoMinMax={}, autoBinCount={}",
                  Name(), inputLabel_, outputLabel_, autoMinMax_, autoBinCount_);
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
        PipelineDataProductLock outHandle = getDataProductManager()->checkoutWrite(outputLabel_);
        TList* outputList = dynamic_cast<TList*>(outHandle->getObject());

        if (!outputList) {
            spdlog::error("[{}] Output product '{}' exists but is not a TList", Name(), outputLabel_);
            return;
        }

        FillHistograms(outputList, inputList);

        spdlog::debug("[{}] Processed {} entries into histogram list '{}'",
                      Name(), inputList->GetSize(), outputLabel_);
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

        PipelineDataProductLock outHandle = getDataProductManager()->checkoutWrite(outputLabel_);
        TList* outputList = dynamic_cast<TList*>(outHandle->getObject());

        if (!outputList) {
            spdlog::error("[{}] Newly created output product '{}' is not a TList", Name(), outputLabel_);
            return;
        }

        FillHistograms(outputList, inputList);

        spdlog::debug("[{}] Processed {} entries into newly created histogram list '{}'",
                      Name(), inputList->GetSize(), outputLabel_);
    }
}

void ChannelIntegralHistogramStage::FillHistograms(TList* outputList, const TList* inputList) {
    std::unordered_map<int, std::vector<double>> channelValues;

    // First pass: collect values
    for (const TObject* obj : *inputList) {
        auto* ci = dynamic_cast<const ChannelIntegral*>(obj);
        if (!ci) continue;
        channelValues[ci->channel_num].push_back(ci->integral_value);
    }

    for (const auto& [channel, values] : channelValues) {
        std::string histName = "channel_" + std::to_string(channel);

        // Always recompute and remake if auto-anything is active
        bool mustRebuild = autoMinMax_ || autoBinCount_;

        // Remove existing hist if needed
        if (mustRebuild) {
            TObject* existing = outputList->FindObject(histName.c_str());
            if (existing) {
                outputList->Remove(existing);
                delete existing;
            }
        }

        TH1D* hist = dynamic_cast<TH1D*>(outputList->FindObject(histName.c_str()));

        if (!hist) {
            double min = autoMinMax_
                ? *std::min_element(values.begin(), values.end())
                : min_;
            double max = autoMinMax_
                ? *std::max_element(values.begin(), values.end())
                : max_;
            if (min == max) max += 1.0;

            int bins = autoBinCount_
                ? std::max(10, static_cast<int>(std::sqrt(values.size())))
                : bins_;

            hist = new TH1D(histName.c_str(),
                            (titlePrefix_ + " - Ch " + std::to_string(channel)).c_str(),
                            bins, min, max);
            hist->SetDirectory(nullptr);
            outputList->Add(hist);
        }

        for (double val : values) {
            hist->Fill(val);
        }
    }
}

