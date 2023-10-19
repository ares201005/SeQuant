#ifndef SEQUANT_PYTHON_MBPT_H
#define SEQUANT_PYTHON_MBPT_H

#include <SeQuant/domain/mbpt/convention.hpp>
#include <SeQuant/domain/mbpt/sr.hpp>

#include "python.h"

#include <cstdint>

namespace sequant::python::mbpt {

template <class F>
auto make_sr_op(F f) {
  auto op = [f](std::int64_t Rank) { return f(Rank); };
  return op;
}

template <class... Args>
ExprPtr VacuumAverage(const ExprPtr& e, const Args&... args) {
  return sequant::mbpt::sr::vac_av(e, args...);
}

#define SR_OP(OP)                                                           \
#OP, [](std::int64_t Rank) { return sequant::mbpt::sr::OP(Rank); },       \
                                                                   py::arg( \
                                                                       "Bra")

inline void __init__(py::module m) {
  sequant::mbpt::set_default_convention();
  sequant::set_default_context(
      Context(Vacuum::SingleProduct, IndexSpaceMetric::Unit,
              BraKetSymmetry::conjugate, SPBasis::spinorbital));
  sequant::TensorCanonicalizer::register_instance(
      std::make_shared<DefaultTensorCanonicalizer>());

  m.def("F", &sequant::mbpt::sr::F);
  m.def("H", &sequant::mbpt::sr::H,
        "H(k = 2) returns a Hamiltonian operator with up to k-body terms",
        py::arg("k") = 2);

  m.def("opH", &sequant::mbpt::sr::op::H,
        "oH(k = 2) returns a Hamiltonian operator with up to k-body terms",
        py::arg("k") = 2);

  m.def("Hw", &sequant::mbpt::sr::Hw,
        "Hw(k = 2) returns a bosonic Hamiltonian operator with up to k-body terms",
        py::arg("k") = 2);
  m.def("Hep", &sequant::mbpt::sr::Hep);

  m.def(SR_OP(A));
  m.def(SR_OP(T));
  m.def(SR_OP(T_));

  m.def("VacuumAverage", &VacuumAverage<>);
  m.def("VacuumAverage", &VacuumAverage<std::vector<std::pair<int, int> > >);
}

}  // namespace sequant::python::mbpt

#endif /* SEQUANT_PYTHON_MBPT_H */
