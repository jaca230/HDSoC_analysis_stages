#include "data_products/channel_integral_collection.h"
#include <iostream>

namespace dataProducts {

ChannelIntegralCollection::ChannelIntegralCollection() = default;

ChannelIntegralCollection::~ChannelIntegralCollection() = default;

void ChannelIntegralCollection::Clear(Option_t* option) {
    integrals_.clear();
}

void ChannelIntegralCollection::Show() const {
    std::cout << "ChannelIntegralCollection with " << integrals_.size() << " entries:\n";
    for (const auto& integral : integrals_) {
        integral.Show();
    }
}

void ChannelIntegralCollection::emplace_back(const ChannelIntegral& ci) {
    integrals_.emplace_back(ci);
}

size_t ChannelIntegralCollection::size() const {
    return integrals_.size();
}

const ChannelIntegral& ChannelIntegralCollection::operator[](size_t idx) const {
    return integrals_[idx];
}

ChannelIntegral& ChannelIntegralCollection::operator[](size_t idx) {
    return integrals_[idx];
}

const std::vector<ChannelIntegral>& ChannelIntegralCollection::getIntegrals() const {
    return integrals_;
}

} // namespace dataProducts

ClassImp(ChannelIntegralCollection)
