#include <SeQuant/domain/mbpt/context.hpp>

namespace sequant::mbpt {

Context::Context(CSV csv) noexcept : csv_(csv) {}

bool operator==(Context const& left, Context const& right) {
  return left.csv() == right.csv();
}

bool operator!=(Context const& left, Context const& right) {
  return !(left == right);
}

namespace detail {

Context& default_formalism_instance() {
  static Context instance_;
  return instance_;
}

}  // namespace detail

const Context& get_default_formalism() {
  return sequant::detail::get_implicit_context<Context>();
}

void set_default_formalism(const Context& ctx) {
  sequant::detail::set_implicit_context(ctx);
}

void reset_default_formalism() {
  sequant::detail::reset_implicit_context<Context>();
}

[[nodiscard]] sequant::detail::ImplicitContextResetter<Context>
set_scoped_default_formalism(const Context& f) {
  return sequant::detail::set_scoped_implicit_context(f);
}

}  // namespace sequant::mbpt
