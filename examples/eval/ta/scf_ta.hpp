//
// Created by Bimal Gaudel on 7/18/21.
//

#ifndef SEQUANT_EVAL_SCF_TA_HPP
#define SEQUANT_EVAL_SCF_TA_HPP

#include <SeQuant/domain/eval/eval.hpp>

#include <SeQuant/core/container.hpp>
#include <SeQuant/core/eval_node.hpp>
#include <SeQuant/core/parse_expr.hpp>

#include "examples/eval/calc_info.hpp"
#include "examples/eval/data_info.hpp"
#include "examples/eval/scf.hpp"
#include "examples/eval/ta/data_world_ta.hpp"

#include <chrono>

namespace sequant::eval {

template <typename Tensor_t>
class SequantEvalScfTA final : public SequantEvalScf {
 public:
  using ExprT = EvalExprTA;
  using EvalNodeTA = EvalNode<ExprT>;

 private:
  container::vector<EvalNodeTA> nodes_;
  CacheManager<Tensor_t const> cman_;
  DataWorldTA<Tensor_t> data_world_;

  Tensor_t const& f_vo() const {
    static Tensor_t tnsr =
        data_world_(parse_expr(L"f{a1;i1}", Symmetry::nonsymm)->as<Tensor>());
    return tnsr;
  }

  Tensor_t const& g_vvoo() const {
    static Tensor_t tnsr = data_world_(
        parse_expr(L"g{a1,a2;i1,i2}", Symmetry::nonsymm)->as<Tensor>());
    return tnsr;
  }

  double energy_spin_orbital() const {
    auto const& T1 = data_world_.amplitude(1);
    auto const& T2 = data_world_.amplitude(2);
    auto const& G_vvoo = g_vvoo();
    auto const& F_vo = f_vo();

    Tensor_t tau_scaled{};
    tau_scaled("a,b,i,j") = 0.25 * T2("a,b,i,j") + 0.5 * T1("a,i") * T1("b,j");

    return TA::dot(F_vo("a,i"), T1("a,i")) +
           TA::dot(G_vvoo("a,b,i,j"), tau_scaled("a,b,i,j"));
  }

  double energy_spin_free_orbital() const {
    auto const& T1 = data_world_.amplitude(1);
    auto const& T2 = data_world_.amplitude(2);
    auto const& G_vvoo = g_vvoo();
    auto const& F_vo = f_vo();

    Tensor_t tau{};
    tau("a,b,i,j") = T2("a,b,i,j") + T1("a,i") * T1("b,j");

    return 2.0 * TA::dot(F_vo("a,i"), T1("a,i")) +
           TA::dot(2 * G_vvoo("a,b,i,j") - G_vvoo("a,b,j,i"), tau("a,b,i,j"));
  }

  void reset_cache() override { cman_.reset(); }

  double norm() const override {
    // todo use all Ts instead of only T2
    return data_world_.amplitude(2)("0,1,2,3").norm().get();
  }

  double solve() override {
    using ranges::views::concat;
    using ranges::views::intersperse;
    using ranges::views::join;
    using ranges::views::repeat_n;
    using ranges::views::transform;
    using ranges::views::zip;

    auto tnsr_to_bk_labels_sorted = [](Tensor const& tnsr) -> std::string {
      auto bra_sorted = tnsr.bra() | ranges::to_vector;
      auto ket_sorted = tnsr.ket() | ranges::to_vector;
      ranges::sort(bra_sorted, Index::LabelCompare{});
      ranges::sort(ket_sorted, Index::LabelCompare{});
      return concat(bra_sorted, ket_sorted) | transform(&Index::label) |
             transform([](auto&& lbl) { return sequant::to_string(lbl); }) |
             intersperse(",") | join | ranges::to<std::string>;
    };

    auto rs = repeat_n(Tensor_t{}, info_.eqn_opts.excit) | ranges::to_vector;
    for (auto&& [r, n] : zip(rs, nodes_)) {
      std::string const target_indices = tnsr_to_bk_labels_sorted(n->tensor());
      auto st = info_.eqn_opts.spintrace;
      auto cm = info_.optm_opts.reuse_imeds;
      if (st && cm) {
        r = eval::evaluate_symm(n, target_indices, data_world_, cman_);
      } else if (st && !cm) {
        r = eval::evaluate_symm(n, target_indices, data_world_);
      } else if (!st && cm) {
        r = eval::evaluate_antisymm(n, target_indices, data_world_, cman_);
      } else {
        r = eval::evaluate_antisymm(n, target_indices, data_world_);
      }
    }
    data_world_.update_amplitudes(rs);
    return info_.eqn_opts.spintrace ? energy_spin_free_orbital()
                                    : energy_spin_orbital();
  }

 public:
  SequantEvalScfTA(TA::World& ta_world, CalcInfo const& calc_info)
      : SequantEvalScf{calc_info},
        cman_{{}},
        data_world_{ta_world, calc_info.fock_eri, calc_info.eqn_opts.excit} {
    assert(info_.eqn_opts.excit >= 2 &&
           "At least double excitation (CCSD) is required!");

    using HRC = std::chrono::high_resolution_clock;

    auto const exprs = info_.exprs();

    auto ns = info_.nodes<ExprT>(exprs);

    cman_ = info_.cache_manager_scf<Tensor_t const>(ns);
    nodes_ = std::move(ns);
  }
};

}  // namespace sequant::eval

#endif  // SEQUANT_EVAL_SCF_TA_HPP
