#include "analysis_pipeline/hdsoc/data_products/channel_integral.h"
#include <iostream>
#include <sstream>

using namespace dataProducts;

ClassImp(ChannelIntegral)

ChannelIntegral::ChannelIntegral(uint64_t channel, double integral)
    : channel_num(channel), integral_value(integral) {}

void ChannelIntegral::Show() const {
    std::cout << String() << std::endl;
}

std::string ChannelIntegral::String() const {
    std::ostringstream oss;
    oss << "ChannelIntegral { channel = " << channel_num
        << ", integral = " << integral_value << " }";
    return oss.str();
}
