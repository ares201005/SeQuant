//
// Created by Eduard Valeyev on 2019-03-26.
//

#ifndef SEQUANT_OP_HPP
#define SEQUANT_OP_HPP

#include <string>
#include <vector>

namespace sequant {
namespace mbpt {

enum class OpType { f, g, t, l, A, L, R };
enum class OpClass { ex, deex, gen };

const static std::vector<std::wstring> cardinal_tensor_labels = {
    L"A", L"L", L"λ", L"f", L"g", L"t", L"R", L"S", L"a", L"ã", L"b", L"ᵬ"};

std::wstring to_wstring(OpType op);
OpClass to_class(OpType op);

}  // namespace mbpt
}  // namespace sequant

#endif //SEQUANT_OP_HPP