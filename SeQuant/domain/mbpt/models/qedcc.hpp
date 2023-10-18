#ifndef SEQUANT_DOMAIN_MBPT_MODELS_QEDCC_HPP
#define SEQUANT_DOMAIN_MBPT_MODELS_QEDCC_HPP

#include <SeQuant/core/container.hpp>
#include <SeQuant/core/expr_fwd.hpp>
#include <SeQuant/core/index.hpp>
#include <SeQuant/core/timer.hpp>

namespace sequant::mbpt::sr {

/// derives equations of traditional coupled-cluster method
class QEDCC {
  size_t N, P, PMIN;

 public:
  /// @brief constructor for QEDCC class
  /// @param n coupled cluster excitation rank
  /// @param p projector excitation rank
  /// @param pmin minimum projector excitation rank
  QEDCC(size_t n, size_t p = std::numeric_limits<size_t>::max(), size_t pmin = 1);

  /// @brief derives similarity-transformed expressions of mpbt::Operators
  /// @param expr expression to be transformed
  /// @param r order of truncation
  /// @pre expr should be composed of mbpt::Operators
  /// @return transformed expression
  ExprPtr sim_tr(ExprPtr expr, size_t r);

  /// derives t amplitude equations
  std::vector<sequant::ExprPtr> t(bool screen = true, bool use_topology = true,
                                  bool use_connectivity = true,
                                  bool canonical_only = true);
  /// derives λ amplitude equations
  std::vector<sequant::ExprPtr> λ(bool screen = false, bool use_topology = true,
                                  bool use_connectivity = true,
                                  bool canonical_only = true);
};  // class QEDCC

}  // namespace sequant::mbpt::sr

#endif  // SEQUANT_DOMAIN_MBPT_MODELS_QEDCC_HPP
