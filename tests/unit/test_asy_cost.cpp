#include "catch.hpp"

#include <SeQuant/core/asy_cost.hpp>
#include <sstream>

struct MatFlops {
  size_t occ_range_size;
  size_t virt_range_size;
  long long int operator()(unsigned short nocc, unsigned short nvirt) const {
    long long int ops = 1;
    if (nocc > 0) ops *= static_cast<int>(std::pow(occ_range_size, nocc));
    if (nvirt > 0) ops *= static_cast<int>(std::pow(virt_range_size, nvirt));
    return ops > 1 ? 2 * ops : 0;
  }
};

TEST_CASE("TEST ASY_COST", "[AsyCost]") {
  using sequant::AsyCost;

  std::wostringstream oss{};
  auto clear = [&oss]() { oss.str(std::wstring{}); };

  SECTION("to_text") {
    oss << AsyCost{0, 0};
    REQUIRE(oss.str() == L"0");

    clear();
    oss << AsyCost{};
    REQUIRE(oss.str() == L"0");

    clear();
    oss << AsyCost{1, 0};
    REQUIRE(oss.str() == L"O");

    clear();
    oss << AsyCost{0, 1};
    REQUIRE(oss.str() == L"V");

    clear();
    oss << AsyCost{1, 1};
    REQUIRE(oss.str() == L"OV");

    clear();
    oss << AsyCost{2, 1};
    REQUIRE(oss.str() == L"O^2V");

    clear();
    oss << AsyCost{1, 2};
    REQUIRE(oss.str() == L"OV^2");

    clear();
    oss << AsyCost{2, 2};
    REQUIRE(oss.str() == L"O^2V^2");

    clear();
    oss << AsyCost{2, 2} + AsyCost{3, 2} + AsyCost{2, 3} + AsyCost{3, 3};
    REQUIRE(oss.str() == L"O^3V^3 + O^2V^3 + O^3V^2 + O^2V^2");

    clear();
    oss << AsyCost{1, 1} - AsyCost{2, 3} + AsyCost{2, 2};
    REQUIRE(oss.str() == L"- O^2V^3 + O^2V^2 + OV");

    clear();
    oss << AsyCost{1, 1, 20};

    REQUIRE(oss.str() == L"20*OV");
    REQUIRE(AsyCost{0, 0} == AsyCost::zero());
    REQUIRE(AsyCost{1, 1, 0} == AsyCost::zero());
  }

  SECTION("Comparisons") {
    auto const c1 = AsyCost{0, 0};
    auto const c2 = AsyCost{0, 1};
    auto const c3 = AsyCost{0, 2};
    auto const c4 = AsyCost{0, 2};

    REQUIRE(c1 == AsyCost::zero());
    REQUIRE(c1 < c2);
    REQUIRE(c2 > c1);
    REQUIRE(c2 < c3);
    REQUIRE(c3 > c2);
    REQUIRE(c3 == c4);

    auto const cc1 = AsyCost{4, 1} + AsyCost{3, 2} + AsyCost{4, 2};
    auto const cc2 = AsyCost{2, 2} + (AsyCost{2, 4} + AsyCost{2, 4});
    REQUIRE(cc1 < cc2);
    REQUIRE_FALSE(cc2 < cc1);
    REQUIRE(cc2 > cc1);
  }

  SECTION("Ops count") {
    const size_t nocc = 2;
    const size_t nvirt = 20;

    auto flops = MatFlops{nocc, nvirt};
    REQUIRE(AsyCost::zero().ops(nocc, nvirt) == 0);

    REQUIRE(AsyCost{2, 3}.ops(nocc, nvirt) == flops(2, 3));

    auto const cost = AsyCost{3, 1} + AsyCost{2, 1};
    REQUIRE(cost.ops(nocc, nvirt) == flops(3, 1) + flops(2, 1));
  }

  SECTION("Fractional costs") {
    clear();
    oss << AsyCost{2,4,{1,2}};
    REQUIRE(oss.str() == L"1/2*O^2V^4");

    auto const c1 = AsyCost{1,2} * boost::rational<int>{2, 3};
    clear();
    oss << c1 ;
    REQUIRE(oss.str() == L"2/3*OV^2");

    auto const c2 = AsyCost{1,2} / boost::rational<int>{2, 3};
    clear();
    oss << c2 ;
    REQUIRE(oss.str() == L"3/2*OV^2");

    auto const c3 = (AsyCost{1, 2} + AsyCost{2, 4}) * 2;
    clear();
    oss << c3;
    REQUIRE(oss.str() == L"2*O^2V^4 + 2*OV^2");

    clear();
  }
}