#ifndef CHANNEL_INTEGRAL_COLLECTION_H
#define CHANNEL_INTEGRAL_COLLECTION_H

#include <vector>
#include "data_products/channel_integral.h"
#include "common/data_products/DataProduct.hh"

namespace dataProducts {

class ChannelIntegralCollection : public DataProduct {
public:
    ChannelIntegralCollection();
    ~ChannelIntegralCollection() override;

    void Clear(Option_t* option = "") override;  // Clear usually has Option_t*, it's fine
    void Show() const override;                   // Show without option param for override

    // Container interface
    void emplace_back(const ChannelIntegral& ci);
    size_t size() const;
    const ChannelIntegral& operator[](size_t idx) const;
    ChannelIntegral& operator[](size_t idx);
    const std::vector<ChannelIntegral>& getIntegrals() const;

private:
    std::vector<ChannelIntegral> integrals_;

    ClassDefOverride(ChannelIntegralCollection, 1);
};

} // namespace dataProducts

#endif // CHANNEL_INTEGRAL_COLLECTION_H
