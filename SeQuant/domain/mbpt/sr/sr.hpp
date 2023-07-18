//
// Created by Eduard Valeyev on 2019-02-19.
//

#ifndef SEQUANT_SRCC_HPP
#define SEQUANT_SRCC_HPP

#include <limits>
#include <utility>

#include "SeQuant/core/expr_fwd.hpp"
#include "SeQuant/core/sequant.hpp"
#include "SeQuant/core/space.hpp"
#include "SeQuant/domain/mbpt/op.hpp"

namespace sequant {
namespace mbpt {
namespace sr {

struct qns_tag;

// clang-format off
/// single reference operator algebra can be screened by tracking the number of creators and annihilators in the occupied and unoccupied space
/// the order of of elements is {# of occupied creators, # of occupied annihilators, # of unoccupied creators, # of unoccupied annihilators}
/// \note use signed integer, although could use unsigned in this case, so that can represent quantum numbers and their changes by the same type
// clang-format on
using qns_t = mbpt::QuantumNumberChange<4, qns_tag, std::int64_t>;
/// changes in quantum number represented by quantum numbers themselves
using qnc_t = qns_t;
using op_t = mbpt::Operator<qnc_t>;

// clang-format off
/// @return the number of creators in \p qns acting on space \p s
/// @pre `(s.type()==IndexSpace::Type::active_occupied || s.type()==IndexSpace::Type::active_unoccupied)&&s.qns()==IndexSpace::null_qns`
// clang-format on
qninterval_t ncre(qns_t qns, const IndexSpace& s);

// clang-format off
/// @return the number of creators in \p qns acting on space \p s
/// @pre `s==IndexSpace::Type::active_occupied || s==IndexSpace::Type::active_unoccupied`
// clang-format on
qninterval_t ncre(qns_t qns, const IndexSpace::Type& s);

// clang-format off
/// @return the number of creators in \p qns acting on the occupied space
// clang-format on
qninterval_t ncre_occ(qns_t qns);

// clang-format off
/// @return the number of creators in \p qns acting on the unoccupied space
// clang-format on
qninterval_t ncre_uocc(qns_t qns);

// clang-format off
/// @return the total number of creators in \p qns
// clang-format on
qninterval_t ncre(qns_t qns);

// clang-format off
/// @return the number of annihilators in \p qns acting on space \p s
/// @pre `(s.type()==IndexSpace::Type::active_occupied || s.type()==IndexSpace::Type::active_unoccupied)&&s.qns()==IndexSpace::null_qns`
// clang-format on
qninterval_t nann(qns_t qns, const IndexSpace& s);

// clang-format off
/// @return the number of annihilators in \p qns acting on space \p s
/// @pre `s==IndexSpace::Type::active_occupied || s==IndexSpace::Type::active_unoccupied`
// clang-format on
qninterval_t nann(qns_t qns, const IndexSpace::Type& s);

// clang-format off
/// @return the number of annihilators in \p qns acting on the occupied space
// clang-format on
qninterval_t nann_occ(qns_t qns);

// clang-format off
/// @return the number of annihilators in \p qns acting on the unoccupied space
// clang-format on
qninterval_t nann_uocc(qns_t qns);

// clang-format off
/// @return the total number of annihilators in \p qns
// clang-format on
qninterval_t nann(qns_t qns);

/// combines 2 sets of quantum numbers using Wick's theorem
qns_t combine(qns_t, qns_t);

}  // namespace sr
}  // namespace mbpt

/// @param qns the quantum numbers to adjoint
/// @return the adjoint of \p qns
mbpt::sr::qns_t adjoint(mbpt::sr::qns_t);

namespace mbpt {
namespace sr {

// clang-format off
/// @brief makes a tensor-level fermionic many-body operator for use in single-reference methods

/// A many-body operator has the following generic form:
/// \f$ \frac{1}{P} T_{b_1 b_2 \dots b_B}^{k_1 k_2 \dots k_K} A^{b_1 b_2 \dots b_B}_{k_1 k_2 \dots k_K} \f$
/// where \f$ \{B,K\} \f$ are number of bra/ket indices of \f$ T \f$ or, equivalently, the number of creators/annihilators
/// of normal-ordered (w.r.t. the default, not necessarily Fermi vacuum) operator \f$ A \f$.
/// Indices \f$ \{ b_i \} \f$ / \f$ \{ k_i \} \f$ are (active) unoccupied/occupied for (pure) excitation operators,
/// are occupied/unoccupied for deexcitation operators; for general operators complete basis indices are assumed by default,
/// unless overridden by user manually. \f$ P \f$ is the "normalization" factor and depends on the vacuum used to define \f$ A \f$,
/// and indices \f$ \{ b_i \} \f$ / \f$ \{ k_i \} \f$.
/// @note The choice of unoccupied indices/spaces can be controlled by the default Formalism:
/// - if `get_default_formalism().sum_over_uocc() == SumOverUocc::Complete` IndexSpace::complete_unoccupied will be used instead of IndexSpace::active_unoccupied
/// - if `get_default_formalism().csv_formalism() == CSVFormalism::CSV` will use cluster-specific (e.g., PNO) unoccupied indices
/// @warning Tensor \f$ T \f$ will be antisymmetrized if `get_default_context().spbasis() == SPBasis::spinorbital`, else it will be particle-symmetric; the latter is only valid if # of bra and ket indices coincide.
// clang-format on
class make_op {
  OpType op_;
  std::vector<IndexSpace::Type> bra_spaces_;
  std::vector<IndexSpace::Type> ket_spaces_;

  const auto nbra() const { return bra_spaces_.size(); };
  const auto nket() const { return ket_spaces_.size(); };

 public:
  // clang-format off
  /// @param[in] op the operator type:
  /// - if @p op is a (pure) excitation operator bra/ket indices
  ///   will be IndexSpace::active_unoccupied/IndexSpace::active_occupied,
  /// - for (pure) deexcitation @p op bra/ket will be IndexSpace::active_occupied/IndexSpace::active_unoccupied
  /// - for general @p op bra/ket will be IndexSpace::complete
  /// @param[in] nbra number of bra indices/creators
  /// @param[in] nket number of ket indices/annihilators; if not specified, will be set to @p nbra
  // clang-format on
  make_op(OpType op, std::size_t nbra,
          std::size_t nket = std::numeric_limits<std::size_t>::max());

  /// @param[in] bras the bra indices/creators
  /// @param[in] kets the ket indices/annihilators
  /// @param[in] op the operator type
  /// @warning this operator is only to be used for general @p op
  make_op(OpType op, std::initializer_list<IndexSpace::Type> bras,
          std::initializer_list<IndexSpace::Type> kets);

  ExprPtr operator()() const;
}; // class make_op

#include "sr_op.impl.hpp"

ExprPtr H1();

ExprPtr H2();

ExprPtr H0mp();
ExprPtr H1mp();

ExprPtr F();
ExprPtr W();

ExprPtr H();

/// computes the vacuum expectation value (VEV)

/// @param[in] expr input expression
/// @param[in] nop_connections specifies the pairs of normal operators to be
/// connected
/// @param[in] use_top if true, topological equivalence will be utilized
/// @return the VEV
ExprPtr vac_av(ExprPtr expr,
               std::vector<std::pair<int, int>> nop_connections = {},
               bool use_top = true);


// these produce operator-level expressions
namespace op {

ExprPtr H1();

ExprPtr H2();
ExprPtr H2_oo_vv();
ExprPtr H2_vv_vv();
ExprPtr H();

/// makes particle-conserving excitation operator of rank \p K
ExprPtr T_(std::size_t K);

/// makes sum of particle-conserving excitation operators of all ranks up to \p
/// K
ExprPtr T(std::size_t K);

/// makes particle-conserving deexcitation operator of rank \p K
ExprPtr Lambda_(std::size_t K);

/// makes sum of particle-conserving deexcitation operators of all ranks up to
/// \p
/// K
ExprPtr Lambda(std::size_t K);

/// makes deexcitation operator of rank \p K
ExprPtr A(std::size_t K);

/// @return true if \p op_or_op_product can produce determinant of excitation
/// rank \p k when applied to reference
bool raises_vacuum_to_rank(const ExprPtr& op_or_op_product,
                           const unsigned long k);

/// @return true if \p op_or_op_product can produce determinant of excitation
/// rank up to \p k when applied to vacuum
bool raises_vacuum_up_to_rank(const ExprPtr& op_or_op_product,
                              const unsigned long k);

/// @return true if \p op_or_op_product can produce vacuum from determinant of
/// excitation rank \p k
bool lowers_rank_to_vacuum(const ExprPtr& op_or_op_product,
                           const unsigned long k);

/// @return true if \p op_or_op_product can produce vacuum from determinant of
/// excitation rank up to \p k
bool lowers_rank_or_lower_to_vacuum(const ExprPtr& op_or_op_product,
                                    const unsigned long k);

/// computes the vacuum expectation value (VEV)

/// @param[in] expr input expression
/// @param[in] op_connections list of pairs of labels of operators to be
/// connected
/// @return the VEV
ExprPtr vac_av(
    ExprPtr expr,
    std::vector<std::pair<std::wstring, std::wstring>> op_connections = {
        {L"h", L"t"}, {L"f", L"t"}, {L"g", L"t"}});

}  // namespace op

}  // namespace sr

extern template class Operator<sr::qns_t, Statistics::FermiDirac>;
extern template class Operator<sr::qns_t, Statistics::BoseEinstein>;

}  // namespace mbpt
}  // namespace sequant

#endif  // SEQUANT_SRCC_HPP
