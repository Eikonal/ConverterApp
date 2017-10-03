#include <vector>
#include <algorithm>

extern "C" {

const char* pluginName() {
    return "Sine converter";
}

void applyConversion(const std::vector<int>& input, std::vector<int>& output)
{
    // check input/output size match
    if (output.size() != input.size()) {
        output.resize(input.size());
    }

    // apply transform
    std::transform(
                input.begin(), input.end(),
                output.begin(),
                [](const int& value) { return 3*value; });
}

}
