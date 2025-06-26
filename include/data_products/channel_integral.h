#ifndef CHANNEL_INTEGRAL_H
#define CHANNEL_INTEGRAL_H

#include "common/data_products/DataProduct.hh"
#include <cstdint>
#include <string>
#include <vector>

namespace dataProducts {

class ChannelIntegral : public DataProduct {
public:
    ChannelIntegral() = default;
    ChannelIntegral(uint64_t channel, double integral);

    ~ChannelIntegral() override = default;

    void Show() const override;
    std::string String() const;

    uint64_t channel_num = 0;
    double integral_value = 0.0;

    ClassDefOverride(ChannelIntegral, 1);
};

} // namespace dataProducts

#endif // CHANNEL_INTEGRAL_H
