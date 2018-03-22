//
// Created by Eduard Valeyev on 3/20/18.
//

#include "catch.hpp"

#include <iostream>
#include "../../src/SeQuant2/op.hpp"

TEST_CASE("Op", "[elements]") {

  using namespace sequant2;

  SECTION("constructors") {
    REQUIRE_NOTHROW(FOp{});

    REQUIRE_NOTHROW(FOp(Index(L"i_1"), Action::create));
    FOp o1(Index(L"i_1"), Action::create);
    REQUIRE(o1.statistics == Statistics::FermiDirac);
    REQUIRE(o1.index() == Index(L"i_1"));
    REQUIRE(o1.action() == Action::create);

    REQUIRE_NOTHROW(bann(Index(L"i_2")));
    auto o2 = bann(Index(L"i_2"));
    REQUIRE(o2.statistics == Statistics::BoseEinstein);
    REQUIRE(o2.index() == Index(L"i_2"));
    REQUIRE(o2.action() == Action::annihilate);

    REQUIRE_NOTHROW(bcre(L"i_3"));
    auto o3 = bcre(L"i_3");
    REQUIRE(o3.statistics == Statistics::BoseEinstein);
    REQUIRE(o3.index() == Index(L"i_3"));
    REQUIRE(o3.action() == Action::create);

    REQUIRE_NOTHROW(fann(L"i_4", {L"a_1", L"a_2"}));
    auto o4 = fann(L"i_4", {L"a_1", L"a_2"});
    REQUIRE(o4.statistics == Statistics::FermiDirac);
    REQUIRE(o4.index() == Index(L"i_4", {L"a_1", L"a_2"}));
    REQUIRE(o4.action() == Action::annihilate);

    REQUIRE_NOTHROW(FOperator{fcre(L"i_1"), fann(L"i_1")});
    auto oper1 = FOperator{fcre(L"i_1"), fann(L"i_1")};
    REQUIRE(oper1.statistics == Statistics::FermiDirac);
    REQUIRE(oper1.size() == 2);
    REQUIRE(oper1[0] == fcre(L"i_1"));
    REQUIRE(oper1[1] == fann(L"i_1"));

    REQUIRE_NOTHROW(FNOperator({L"i_1"}, {L"a_1"}));
    auto nop1 = FNOperator({L"i_1"}, {L"a_1"});
    REQUIRE(nop1.creators().size() == 1);
    REQUIRE(nop1.annihilators().size() == 1);
    REQUIRE(nop1.creators()[0] == fcre(L"i_1"));
    REQUIRE(nop1.annihilators()[0] == fann(L"a_1"));

    REQUIRE_NOTHROW(FNOperator({Index{L"i_1"}, Index{L"i_2"}},
                               {Index{L"a_1", {L"i_1", L"i_2"}}, Index{L"a_2", {L"i_1", L"i_2"}}}));
    auto nop2 =
        FNOperator({Index{L"i_1"}, Index{L"i_2"}}, {Index{L"a_1", {L"i_1", L"i_2"}}, Index{L"a_2", {L"i_1", L"i_2"}}});
    REQUIRE(nop2.creators().size() == 2);
    REQUIRE(nop2.annihilators().size() == 2);
    REQUIRE(nop2.creators()[0] == fcre(L"i_1"));
    REQUIRE(nop2.creators()[1] == fcre(L"i_2"));
    REQUIRE(nop2.annihilators()[0] == fann(Index{L"a_1", {L"i_1", L"i_2"}}));
    REQUIRE(nop2.annihilators()[1] == fann(Index{L"a_2", {L"i_1", L"i_2"}}));
  }

  SECTION("conversion") {
    auto nop1 =
        FNOperator({Index{L"i_1"}, Index{L"i_2"}}, {Index{L"a_1", {L"i_1", L"i_2"}}, Index{L"a_2", {L"i_1", L"i_2"}}});
    REQUIRE_NOTHROW(static_cast<FOperator>(nop1));
    FOperator op1(static_cast<FOperator>(nop1));
    REQUIRE(op1.size() == 4);
    REQUIRE(op1[0] == fcre(L"i_1"));
    REQUIRE(op1[1] == fcre(L"i_2"));
    REQUIRE(op1[2] == fann(Index{L"a_2", {L"i_1", L"i_2"}}));
    REQUIRE(op1[3] == fann(Index{L"a_1", {L"i_1", L"i_2"}}));
  }

  SECTION("latex") {
    FOp o1(Index(L"i_1"), Action::create);
    REQUIRE(to_latex(o1) == L"{a^{\\dagger}_{i_1}}");

    BOp o2(Index(L"a_1"), Action::create);
    REQUIRE(to_latex(o2) == L"{b^{\\dagger}_{a_1}}");

    auto oper1 = FOperator{fcre(L"i_1"), fann(L"i_1")};
    REQUIRE(to_latex(oper1) == L"{{a^{\\dagger}_{i_1}}{a_{i_1}}}");

    auto nop1 = FNOperator({L"i_1", L"i_2"}, {L"a_1", L"a_2"});
    REQUIRE(to_latex(nop1) == L"{\\tilde{a}^{{i_1}{i_2}}_{{a_1}{a_2}}}");

    auto nop2 =
        FNOperator({Index{L"i_1"}, Index{L"i_2"}}, {Index{L"a_1", {L"i_1", L"i_2"}}, Index{L"a_2", {L"i_1", L"i_2"}}});
    REQUIRE(to_latex(nop2) == L"{\\tilde{a}^{{i_1}{i_2}}_{{a_1^{{i_1}{i_2}}}{a_2^{{i_1}{i_2}}}}}");
  }

}  // TEST_CASE("Op")