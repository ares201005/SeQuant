//
// Created by Eduard Valeyev on 3/20/18.
//

#ifndef SEQUANT_OP_H
#define SEQUANT_OP_H

#include <numeric>

#include <range/v3/all.hpp>

#include "attr.hpp"
#include "container.hpp"
#include "expr.hpp"
#include "hugenholtz.hpp"
#include "index.hpp"
#include "ranges.hpp"
#include "sequant.hpp"

namespace sequant {

/// @brief Op is a creator/annihilator operator
///
/// Op = Index + Action
/// @tparam S specifies the particle statistics
template<Statistics S = Statistics::FermiDirac>
class Op {
 public:
  static constexpr Statistics statistics = S;

  Op() = default;
  Op(Index index, Action action) noexcept : index_(std::move(index)), action_(action) {}

  const Index &index() const { return index_; }
  const Action &action() const { return action_; }
  /// applies (Hermitian) adjoint to this operator
  Op &adjoint() {
    action_ = sequant::adjoint(action_);
    return *this;
  }

  /// @return the string representation of @c this in LaTeX format
  std::wstring to_latex() const {
    std::wstring result;
    result = L"{";
    result += (S == Statistics::FermiDirac ? L"a" : L"b");
    result += (action() == Action::create ? L"^{\\dagger}_" : L"_");
    result += index().to_latex();
    result += L"}";
    return result;
  }

  /// compares 2 Op objects while ignoring Index label (see Index::TypeCompare)
  struct TypeCompare {
    bool operator()(const Op<S> &first, const Op<S> &second) {
      bool result;
      if (first.action() == second.action()) {
        result = Index::TypeCompare{}(first.index(), second.index());
      } else
        result = first.action() < second.action();
      return result;
    }
  };

  /// tests equality of 2 Op objects while ignoring Index label (see
  /// Index::TypeEquality)
  struct TypeEquality {
    bool operator()(const Op<S> &first, const Op<S> &second) {
      bool result = (first.action() == second.action()) &&
                    Index::TypeEquality{}(first.index(), second.index());
      return result;
    }
  };

 private:
  Index index_;
  Action action_;
};

template<Statistics S>
bool operator==(const Op<S> &op1, const Op<S> &op2) {
  return op1.index() == op2.index() && op1.action() == op2.action();
}

template<Statistics S>
bool operator!=(const Op<S> &op1, const Op<S> &op2) {
  return !(op1 == op2);
}

using BOp = Op<Statistics::BoseEinstein>;
using FOp = Op<Statistics::FermiDirac>;

inline BOp bcre(Index i) { return BOp(i, Action::create); }
template <typename I>
inline BOp bcre(Index i, std::initializer_list<I> pi) {
  return BOp(Index(i, pi), Action::create);
}
inline BOp bann(Index i) { return BOp(i, Action::annihilate); }
template <typename I>
inline BOp bann(Index i, std::initializer_list<I> pi) {
  return BOp(Index(i, pi), Action::annihilate);
}
inline FOp fcre(Index i) { return FOp(i, Action::create); }
template <typename I>
inline FOp fcre(Index i, std::initializer_list<I> pi) {
  return FOp(Index(i, pi), Action::create);
}
inline FOp fann(Index i) { return FOp(i, Action::annihilate); }
template <typename I>
inline FOp fann(Index i, std::initializer_list<I> pi) {
  return FOp(Index(i, pi), Action::annihilate);
}

/// @return true if this is a pure quasdiparticle creator with respect to the
/// given vacuum, false otherwise
template <Statistics S>
bool is_pure_qpcreator(const Op<S> &op,
                       Vacuum vacuum = get_default_context().vacuum()) {
  switch (vacuum) {
    case Vacuum::Physical:
      return op.action() == Action::create;
    case Vacuum::SingleProduct: {
      const auto occ_class = occupancy_class(op.index().space());
      return (occ_class < 0 && op.action() == Action::annihilate) ||
             (occ_class > 0 && op.action() == Action::create);
    }
    default:
      throw std::logic_error(
          "is_pure_qpcreator: cannot handle MultiProduct vacuum");
  }
};

/// @return true if this is a quasdiparticle creator with respect to the given
/// vacuum, false otherwise
template <Statistics S>
bool is_qpcreator(const Op<S> &op,
                  Vacuum vacuum = get_default_context().vacuum()) {
  switch (vacuum) {
    case Vacuum::Physical:
      return op.action() == Action::create;
    case Vacuum::SingleProduct: {
      const auto occ_class = occupancy_class(op.index().space());
      return (occ_class <= 0 && op.action() == Action::annihilate) ||
             (occ_class >= 0 && op.action() == Action::create);
    }
    default:
      throw std::logic_error("is_qpcreator: cannot handle MultiProduct vacuum");
  }
};

template <Statistics S>
IndexSpace qpcreator_space(const Op<S> &op,
                           Vacuum vacuum = get_default_context().vacuum()) {
  switch (vacuum) {
    case Vacuum::Physical:
      return op.action() == Action::create ? op.index().space()
                                           : IndexSpace::null_instance();
    case Vacuum::SingleProduct:
      return op.action() == Action::annihilate
                 ? intersection(op.index().space(),
                                IndexSpace::instance(IndexSpace::occupied))
                 : intersection(
                       op.index().space(),
                       IndexSpace::instance(IndexSpace::complete_unoccupied));
    default:
      throw std::logic_error(
          "qpcreator_space: cannot handle MultiProduct vacuum");
  }
}

/// @return true if this is a pure quasdiparticle annihilator with respect to
/// the given vacuum, false otherwise
template <Statistics S>
bool is_pure_qpannihilator(const Op<S> &op,
                           Vacuum vacuum = get_default_context().vacuum()) {
  switch (vacuum) {
    case Vacuum::Physical:
      return op.action() == Action::annihilate;
    case Vacuum::SingleProduct: {
      const auto occ_class = occupancy_class(op.index().space());
      return (occ_class > 0 && op.action() == Action::annihilate) ||
             (occ_class < 0 && op.action() == Action::create);
    }
    default:
      throw std::logic_error(
          "is_pure_qpannihilator: cannot handle MultiProduct vacuum");
  }
};

/// @return true if this is a quasdiparticle annihilator with respect to the
/// given vacuum, false otherwise
template <Statistics S>
bool is_qpannihilator(const Op<S> &op,
                      Vacuum vacuum = get_default_context().vacuum()) {
  switch (vacuum) {
    case Vacuum::Physical:
      return op.action() == Action::annihilate;
    case Vacuum::SingleProduct: {
      const auto occ_class = occupancy_class(op.index().space());
      return (occ_class >= 0 && op.action() == Action::annihilate) ||
             (occ_class <= 0 && op.action() == Action::create);
    }
    default:
      throw std::logic_error(
          "is_qpannihilator: cannot handle MultiProduct vacuum");
  }
};

template <Statistics S>
IndexSpace qpannihilator_space(const Op<S> &op,
                               Vacuum vacuum = get_default_context().vacuum()) {
  switch (vacuum) {
    case Vacuum::Physical:
      return op.action() == Action::annihilate ? op.index().space()
                                               : IndexSpace::null_instance();
    case Vacuum::SingleProduct:
      return op.action() == Action::create
                 ? intersection(op.index().space(),
                                IndexSpace::instance(IndexSpace::occupied))
                 : intersection(
                       op.index().space(),
                       IndexSpace::instance(IndexSpace::complete_unoccupied));
    default:
      throw std::logic_error(
          "qpcreator_space: cannot handle MultiProduct vacuum");
  }
}

template <Statistics S = Statistics::FermiDirac>
class NormalOperator;

/// @brief Operator is a sequence of Op objects
///
/// @tparam S specifies the particle statistics
template<Statistics S = Statistics::FermiDirac>
class Operator : public container::svector<Op<S>>, public Expr {
 public:
  using base_type = container::svector<Op<S>>;
  static constexpr Statistics statistics = S;

  // iterate over this using the base_type
  using iterator = typename base_type::iterator;
  using const_iterator = typename base_type::const_iterator;

  using base_type::begin;
  using base_type::end;
  using base_type::cbegin;
  using base_type::cend;
  using base_type::empty;
  using base_type::size;
  using base_type::at;
  using base_type::operator[];

  Operator() = default;
  explicit Operator(std::initializer_list<Op<S>> ops)
      : base_type(ops) {}
  explicit Operator(base_type &&ops)
      : base_type(std::move(ops)) {}
  template <typename I>
  Operator(Action action, std::initializer_list<I> indices)
      : base_type(make_ops(action, indices)) {}

  operator base_type &() const & { return *this; }
  operator base_type &&() && { return *this; }

  /// applies (Hermitian) adjoint operation to this
  /// @return reference to @c *this , for daisy-chaining
  Operator &adjoint() {
    std::reverse(this->begin(), this->end());
    std::for_each(this->begin(), this->end(), [](Op<S> &op) { op.adjoint(); });
    return *this;
  }

  /// @return the string representation of @c this in LaTeX format
  std::wstring to_latex() const override {
    std::wstring result;
    result = L"{";
    for (const auto &o : *this)
      result += o.to_latex();
    result += L"}";
    return result;
  }

  type_id_type type_id() const override {
    return get_type_id<Operator>();
  };

  std::shared_ptr<Expr> clone() const override {
    return std::make_shared<Operator>(*this);
  }

 private:
  base_type make_ops(Action action,
                     IndexList indices) {
    base_type result;
    result.reserve(indices.size());
    for (const auto &idx : indices)
      result.emplace_back(idx, action);
    return result;
  }

  base_type
  make_ops(Action action,
           WstrList index_labels) {
    base_type result;
    result.reserve(index_labels.size());
    for (const auto &idx_label : index_labels)
      result.emplace_back(Index{idx_label}, action);
    return result;
  }

  bool static_equal(const Expr &that) const override {
    const auto &that_cast = static_cast<const Operator &>(that);
    if (this->size() == that_cast.size()) {
      if (this->empty()) return true;
      // compare hash values first
      if (this->hash_value() ==
          that.hash_value())  // hash values agree -> do full comparison
        return static_cast<const base_type &>(*this) ==
               static_cast<const base_type &>(that_cast);
      else
        return false;
    } else
      return false;
  }

  bool is_cnumber() const override {
    return false;
  }

  bool commutes_with_atom(const Expr& that) const override {
    bool result = true;
    /// does not commute with Operator<S>
    /// TODO implement checks of commutativity with Operator<S>
    if (that.is<Operator<S>>()) {
      result = false;
    }
    else if (that.is<NormalOperator<S>>()) {
      result = that.as<NormalOperator<S>>().commutes_with_atom(*this);
    }
    return result;
  }
};

/// @brief NormalOperator is an Operator normal-ordered with respect to vacuum.

/// @note Normal ordering means all creators are to the left of all
/// annihilators. It is natural to express at least number-conserving normal
/// operators (i.e. those with equal number of creators and annihilators) as
/// tensors with creators as superscripts and annihilators as subscripts.
/// Operator cre(p1) cre(p2) ... cre(pN) ann(qN) ... ann(q2) ann(q1) is
/// represented in such notation as a^{p1 p2 ... pN}_{q1 q2 ... qN}, hence it is
/// natural to specify annihilators in the order of their particle index (i.e.
/// as q1 q2, etc.) which is reverse of the order of their appearance in
/// Operator.
///
/// @note The tensor notation becomes less intuitive for number non-conserving
/// operators, e.g. cre(p1) cre(p2) ann(q2) could be represented as a^{p1
/// p2}_{q2 ⎵} or a^{p1 p2}_{⎵ q2}. To make it explicit that ann(q2) acts on
/// same particle as cre(p2) the latter notation is used; similarly, cre(p1)
/// ann(q1) ann(q2) is represented as a^{⎵ p1}_{q1 q2}.
///
/// @tparam S specifies the particle statistics
template<Statistics S>
class NormalOperator : public Operator<S> {
 public:
  static constexpr Statistics statistics = S;
  using base_type = Operator<S>;

  // iterate over this using the base_type
  using iterator = typename Operator<S>::iterator;
  using const_iterator = typename Operator<S>::const_iterator;

  using base_type::begin;
  using base_type::end;
  using base_type::cbegin;
  using base_type::cend;
  using base_type::empty;
  using base_type::size;
  using base_type::at;
  using base_type::operator[];

  /// constructs an identity operator
  NormalOperator(Vacuum v = get_default_context().vacuum()) {}

  /// @param creators sequence of creators
  /// @param annihilators sequence of annihilators (in order of particle indices, see the class documentation for more info).
  template <
      typename IndexContainer,
      typename = std::enable_if_t<
          !meta::is_initializer_list_v<std::decay_t<IndexContainer>> &&
          std::is_same_v<typename std::decay_t<IndexContainer>::value_type, Index>>>
  NormalOperator(IndexContainer &&creator_indices,
                 IndexContainer &&annihilator_indices,
                 Vacuum v = get_default_context().vacuum())
      : Operator<S>{}, vacuum_(v), ncreators_(creator_indices.size()) {
    this->reserve(creator_indices.size() + annihilator_indices.size());
    for (const auto &i : creator_indices) {
      this->emplace_back(i, Action::create);
    }
    for (const auto &i : annihilator_indices | ranges::view::reverse) {
      this->emplace_back(i, Action::annihilate);
    }
  }

  /// @param creators sequence of creators
  /// @param annihilators sequence of annihilators (in order of particle indices, see the class documentation for more info).
  NormalOperator(std::initializer_list<Op<S>> creators,
                 std::initializer_list<Op<S>> annihilators,
                 Vacuum v = get_default_context().vacuum())
      : Operator<S>{}, vacuum_(v), ncreators_(size(creators)) {
    for (const auto &op: creators) {
      assert(op.action() == Action::create);
    }
    for (const auto &op: annihilators) {
      assert(op.action() == Action::annihilate);
    }
    this->reserve(size(creators) + size(annihilators));
    this->insert(this->end(), cbegin(creators), cend(creators));
    this->insert(this->end(), crbegin(annihilators), crend(annihilators));
  }

//  /// @param creator_indices sequence of creator indices
//  /// @param annihilator_indices sequence of annihilator indices (in order of particle indices, see the class documentation for more info).
  template <typename I, typename = std::enable_if_t<!std::is_same_v<std::decay_t<I>,Op<S>>>>
  NormalOperator(std::initializer_list<I> creator_indices,
                 std::initializer_list<I> annihilator_indices,
                 Vacuum v = get_default_context().vacuum())
      : Operator<S>{}, vacuum_(v), ncreators_(creator_indices.size()) {
    this->reserve(creator_indices.size() + annihilator_indices.size());
    for (const auto &i: creator_indices) {
      this->emplace_back(i, Action::create);
    }
    for (const auto &i : annihilator_indices | ranges::view::reverse) {
      this->emplace_back(i, Action::annihilate);
    }
  }

  NormalOperator(const NormalOperator &other)
      : Operator<S>(other),
        vacuum_(other.vacuum_),
        ncreators_(other.ncreators_),
        hug_(other.hug_ ? std::make_unique<hug_type>(*other.hug_) : nullptr) {}
  NormalOperator(NormalOperator &&) = default;
  NormalOperator &operator=(NormalOperator &&) = default;
  NormalOperator &operator=(const NormalOperator &other) {
    static_cast<base_type &>(*this) = static_cast<const base_type &>(other);
    vacuum_ = other.vacuum_;
    ncreators_ = other.ncreators_;
    hug_ = other.hug_ ? std::make_unique<hug_type>(*other.hug_) : nullptr;
    return *this;
  }

  /// @return the vacuum state with respect to which the operator is normal-ordered.
  Vacuum vacuum() const { return vacuum_; }
  /// @return the range of creators
  auto creators() const { return ranges::view::counted(this->cbegin(), ncreators()); }
  /// @return the range of annihilators
  auto annihilators() const { return ranges::view::counted(this->crbegin(), nannihilators()); }
  /// @return the number of creators
  auto ncreators() const { return ncreators_; }
  /// @return the number of annihilators
  auto nannihilators() const { return this->size() - ncreators(); }
  /// @return view of creators and annihilators as a single range
  auto creann() const {
    return ranges::view::concat(creators(), annihilators());
  }

  /// @return number of creators/annihilators
  /// @throw std::logic_error if the operator is not particle number conserving (i.e. if ncreators() != nannihilators() )
  auto rank() const {
    if (ncreators() != nannihilators()) {
      throw std::logic_error("NormalOperator::rank(): ncreators != nannihilators");
    }
    return ncreators();
  }


  /// @return the representation of @c *this as a Hugenholtz vertex
  /// @sa HugenholtzVertex
  const auto &hug() const {
    if (!hug_) {
      hug_ = std::make_unique<hug_type>(
          static_cast<const typename base_type::base_type &>(*this));
    }
    return hug_;
  }

  NormalOperator &adjoint() {
    static_cast<Operator<S> &>(*this).adjoint();
    ncreators_ = this->size() - ncreators_;
    // TODO rebuild the Hug
    hug_.reset();
    return *this;
  }

  std::wstring to_latex() const override {
    std::wstring result;
    result = L"{";
    result += (S == Statistics::FermiDirac
               ? (vacuum() == Vacuum::Physical ? L"a" : L"\\tilde{a}")
               : (vacuum() == Vacuum::Physical ? L"b" : L"\\tilde{b}"));
    result += L"^{";
    const auto ncreators = this->ncreators();
    const auto nannihilators = this->nannihilators();
    if (ncreators <
        nannihilators) {  // pad on the left with square underbrackets, i.e. ⎵
      const auto iend = nannihilators - ncreators;
      if (iend > 0) result += L"\\textvisiblespace";
      for (size_t i = 1; i != iend; ++i) {
        result += L"\\,\\textvisiblespace";
      }
      if (ncreators > 0) {
        result += L"\\,";
      }
    }
    for (const auto &o : creators())
      result += o.index().to_latex();
    result += L"}_{";
    if (ncreators >
        nannihilators) {  // pad on the left with square underbrackets, i.e. ⎵
      const auto iend = ncreators - nannihilators;
      if (iend > 0) result += L"\\textvisiblespace";
      for (size_t i = 1; i != iend; ++i) {
        result += L"\\,\\textvisiblespace";
      }
      if (nannihilators > 0) {
        result += L"\\,";
      }
    }
    for (const auto &o : annihilators())
      result += o.index().to_latex();
    result += L"}}";
    return result;
  }

  /// overload base_type::erase
  iterator erase(const_iterator it) {
    if (it->action() == Action::create) --ncreators_;
    if (hug_) hug_->erase(it - begin(), *it);
    return Operator<S>::erase(it);
  }

  /// overload base_type::insert
  template <typename T>
  iterator insert(const_iterator it, T &&value) {
    if (value.action() == Action::create) ++ncreators_;
    auto result = Operator<S>::insert(it, std::forward<T>(value));
    if (hug_) hug_->insert(result - begin(), *result);
    return result;
  }

  Expr::type_id_type type_id() const override {
    return Expr::get_type_id<NormalOperator>();
  };

  std::shared_ptr<Expr> clone() const override {
    return std::make_shared<NormalOperator>(*this);
  }

 private:
  Vacuum vacuum_;
  std::size_t ncreators_ = 0;
  using hug_type = HugenholtzVertex<Op<S>, typename Op<S>::TypeEquality>;
  mutable std::unique_ptr<hug_type> hug_;  // only created if needed

  bool static_equal(const Expr &that) const override {
    const auto &that_cast = static_cast<const NormalOperator &>(that);
    if (this->vacuum() == that_cast.vacuum() &&
        this->ncreators() == that_cast.ncreators()) {
      return static_cast<const base_type &>(*this) ==
             static_cast<const base_type &>(*this);
    } else
      return false;
  }

  template <Statistics>
  friend class Operator;
  bool commutes_with_atom(const Expr &that) const override {
    // same as WickTheorem::can_contract
    auto can_contract = [this](const Op<S> &left, const Op<S> &right) {
      if (is_qpannihilator<S>(left, vacuum_) &&
          is_qpcreator<S>(right, vacuum_)) {
        const auto qpspace_left = qpannihilator_space<S>(left, vacuum_);
        const auto qpspace_right = qpcreator_space<S>(right, vacuum_);
        const auto qpspace_common = intersection(qpspace_left, qpspace_right);
        if (qpspace_common != IndexSpace::null_instance()) return true;
      }
      return false;
    };

    bool result = true;
    /// does not commute with Operator<S>
    /// TODO implement checks of commutativity with Operator<S>
    if (that.is<Operator<S>>()) {
      result = false;
    } else if (that.is<NormalOperator<S>>()) {
      const auto& op_that = that.as<NormalOperator<S>>();
      if (vacuum() ==
          op_that.vacuum()) {  // can only check commutativity for same vacua
        for (auto &&op_l : *this) {
          for (auto &&op_r : op_that) {
            if (can_contract(op_l, op_r)) {
              return false;
            }
          }
        }
      } else {  // NormalOperators for different vacua do not commute
        result = false;
      }
    }
    return result;
  }
};

template<Statistics S>
bool operator==(const NormalOperator<S> &op1, const NormalOperator<S> &op2) {
  return op1.vacuum() == op2.vacuum() && ranges::equal(op1, op2);
}

/// @brief NormalOperatorSequence is a sequence NormalOperator objects, all
/// ordered with respect to same vacuum
///
/// @tparam S specifies the particle statistics
template<Statistics S = Statistics::FermiDirac>
class NormalOperatorSequence : public container::svector<NormalOperator<S>>, public Expr {
 public:
  using base_type = container::svector<NormalOperator<S>>;
  static constexpr Statistics statistics = S;

  using base_type::begin;
  using base_type::end;
  using base_type::cbegin;
  using base_type::cend;
  using base_type::empty;
  using base_type::size;
  using base_type::at;
  using base_type::operator[];

  /// constructs an empty sequence
  NormalOperatorSequence() : vacuum_(get_default_context().vacuum()) {}

  NormalOperatorSequence(std::initializer_list<NormalOperator<S>> operators)
      : base_type(operators) {
    check_vacuum();
  }

  Vacuum vacuum() const { return vacuum_; }

  operator const base_type &() const & { return *this; }
  operator base_type &&() && { return *this; }

  /// @return the total number of Op<S> objects in this
  /// @warning not to be confused with NormalOperatorSequence::size() that
  /// returns the number of NormalOperator<S> objects
  auto opsize() const {
    size_t opsz = 0;
    for (auto &&nop : *this) {
      opsz += nop.size();
    }
    return opsz;
  }

  /// applies (Hermitian) adjoint operation to this
  /// @return reference to @c *this , for daisy-chaining
  NormalOperatorSequence &adjoint() {
    std::reverse(this->begin(), this->end());
    std::for_each(this->begin(), this->end(), [](NormalOperator<S> &op) { op.adjoint(); });
    return *this;
  }

  std::wstring to_latex() const override {
    std::wstring result;
    result = L"{";
    for (const auto &op: *this)
      result += op.to_latex();
    result += L"}";
    return result;
  }

  Expr::type_id_type type_id() const override {
    return Expr::get_type_id<NormalOperatorSequence>();
  };

 private:
  Vacuum vacuum_ = Vacuum::Invalid;
  /// ensures that all operators use same vacuum, and sets vacuum_
  void check_vacuum() {
    vacuum_ =
        std::accumulate(this->cbegin(), this->cend(), Vacuum::Invalid, [](Vacuum v1, const NormalOperator<S> &v2) {
          if (v1 == Vacuum::Invalid) {
            return v2.vacuum();
          } else {
            if (v1 != v2.vacuum())
              throw std::invalid_argument(
                  "NormalOperatorSequence expects all constituent NormalOperator objects to use same vacuum");
            else
              return v1;
          }
        });
  }

  bool static_equal(const Expr &that) const override {
    const auto& that_cast = static_cast<const NormalOperatorSequence&>(that);
    if (this->vacuum() == that_cast.vacuum()) {
      if (this->empty()) return true;
      if (this->hash_value() == that.hash_value())
        return static_cast<const base_type&>(*this) == static_cast<const base_type&>(*this);
       else return false;
    } else return false;
  }

};

using BOperator = Operator<Statistics::BoseEinstein>;
using BNOperator = NormalOperator<Statistics::BoseEinstein>;
using BNOperatorSeq = NormalOperatorSequence<Statistics::BoseEinstein>;
using FOperator = Operator<Statistics::FermiDirac>;
using FNOperator = NormalOperator<Statistics::FermiDirac>;
using FNOperatorSeq = NormalOperatorSequence<Statistics::FermiDirac>;

template<Statistics S>
std::wstring to_latex(const Op<S> &op) {
  return op.to_latex();
}

template<Statistics S>
std::wstring to_latex(const Operator<S> &op) {
  return op.to_latex();
}

template<Statistics S>
std::wstring to_latex(const NormalOperator<S> &op) {
  return op.to_latex();
}

template<Statistics S>
std::wstring to_latex(const NormalOperatorSequence<S> &opseq) {
  return opseq.to_latex();
}

/// @name customization points to support generic algorithms on NormalOperator objects as Tensors
/// @{
template <Statistics S>
auto bra(const NormalOperator<S>& nop) {
  return ranges::view::counted(ranges::crbegin(nop.annihilators()),
                               nop.nannihilators()) |
         ranges::view::transform(
             [](auto &&op) -> const Index & { return op.index(); });
}
template <Statistics S>
auto bra(NormalOperator<S>& nop) {
  return ranges::view::counted(ranges::rbegin(nop.annihilators()),
                               nop.nannihilators()) |
      ranges::view::transform(
          [](auto &&op) -> const Index & { return op.index(); }) |
      ranges::view::reverse;
}
template <Statistics S>
auto ket(const NormalOperator<S>& nop) {
  return ranges::view::counted(ranges::cbegin(nop.creators()),
                               nop.ncreators()) |
      ranges::view::transform(
          [](auto &&op) -> const Index & { return op.index(); });
}
template <Statistics S>
auto ket(NormalOperator<S>& nop) {
  return ranges::view::counted(ranges::begin(nop.creators()),
                               nop.ncreators()) |
      ranges::view::transform(
          [](auto &&op) -> const Index & { return op.index(); });
}
template <Statistics S>
auto braket(const NormalOperator<S>& nop) {
  return ranges::view::join(bra(nop),ket(nop));
}
template <Statistics S>
auto braket(NormalOperator<S>& nop) {
  return ranges::view::join(bra(nop),ket(nop));
}
///@}

namespace detail {
  struct OpIdRegistrar {
    OpIdRegistrar();
  };
}  // namespace detail
}  // namespace sequant

#endif // SEQUANT_OP_H