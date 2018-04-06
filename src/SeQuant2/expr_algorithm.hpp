//
// Created by Eduard Valeyev on 3/30/18.
//

#ifndef SEQUANT2_EXPR_ALGORITHM_HPP
#define SEQUANT2_EXPR_ALGORITHM_HPP

namespace sequant2 {

/// Recursively canonicalizes an Expr and replaces it as needed
/// @param[in,out] expr expression to be canonicalized; will be replaced if canonicalization is impure
inline void canonicalize(ExprPtr& expr) {
  const auto biproduct = expr->canonicalize();
  if (biproduct && biproduct->type_id() == Expr::get_type_id<Constant>()) {
    const auto constant_ptr = std::static_pointer_cast<Constant>(biproduct);
    expr = biproduct * expr;
  }
}

/// Simplifies an Expr by applying all known transformations (e.g. eliminating trivial math, CSE, etc.)
/// @param[in,out] expr expression to be simplified
inline void simplify(ExprPtr& expr) {
  // not yet implemented
}

namespace detail {
struct expand_visitor {
  void operator()(ExprPtr& expr) {
    if (debug) std::wcout << "expand_visitor received " << to_latex(expr) << std::endl;
    // apply expand() iteratively until done
    while(expand(expr)) {
      if (debug)  std::wcout << "after 1 round of expansion have " << to_latex(expr) << std::endl;
    }
    if (debug) std::wcout << "expansion result = " << to_latex(expr) << std::endl;
    // now need to flatten!
    // and simplify?! e.g. extract constants out of products, etc.
  }

  /// expands the first Sum in a Product
  /// @param[in,out] expr (shared_ptr to ) a Product whose first Sum gets expanded; on return @c expr contains the result
  bool expand_product(ExprPtr& expr) {
    auto& expr_ref = *expr;
    std::shared_ptr<Sum> result;
    const auto nsubexpr = ranges::size(*expr);
    for(std::size_t i=0; i != nsubexpr; ++i) {
      if (expr_ref[i]->type_id() == Expr::get_type_id<Sum>()) {
        // allocate the result, if not done yet
        if (!result)
          result = std::make_shared<Sum>();
        ExprPtr subexpr_to_expand = expr_ref[i];
        for(auto& subsubexpr: *subexpr_to_expand) {
          auto expr_clone = expr->clone();
          (*expr_clone)[i] = subsubexpr;
          result->append(std::move(expr_clone));
        }
        expr = std::static_pointer_cast<Expr>(result); // expanded one Sum, return
        return true;
      }
    }
    return false;
  }

  /// expands a Sum
  bool expand_sum(ExprPtr& expr) {
    auto& expr_ref = *expr;
    std::shared_ptr<Sum> result;  // will keep the result if one or more summands is expanded
    const auto nsubexpr = ranges::size(*expr);
    if (debug) std::wcout << "in expand_sum: expr = " << to_latex(expr) << std::endl;
    for(std::size_t i=0; i != nsubexpr; ++i) {
      if (expr_ref[i]->type_id() == Expr::get_type_id<Product>()) {
        const auto this_term_expanded = expand_product(expr_ref[i]);
        // if this is the first term that was expanded, create a result and copy all preceeding subexpressions into it
        if (!result && this_term_expanded) {
          result = std::make_shared<Sum>();
          for(std::size_t j=0; j != i; ++j)
            result->append(expr_ref[j]);
        }
        // if expr != expanded result append current subexpr
        if (result)
          result->append(expr_ref[i]);
        if (debug) std::wcout << "in expand_sum: after expand_product(" << (this_term_expanded ? "true)" : "false)") << " result = " << to_latex(result ? result : expr) << std::endl;
      }
    }
    if (result) { // if any summand was expanded, copy result into expr, else expr was unchanged, nothing to do
      expr = std::static_pointer_cast<Expr>(result);
      return true;
    }
    else
      return false;
  }

  // @return true if expanded Product of Sum into Sum of Product
  bool expand(ExprPtr& expr) {
    const auto type_id = expr->type_id();
    if (type_id == Expr::get_type_id<Product>()) {
      return expand_product(expr);
    }
    else if (type_id == Expr::get_type_id<Sum>()) {
      return expand_sum(expr);
    } else
      return false;
  }

  bool debug = false;
};
};  // namespace detail

/// Recursively expands products of sums
inline void expand(ExprPtr& expr) {
  detail::expand_visitor expander{};
  expr->visit(expander);
  expander(expr);
}

}  // namespace sequant2

#endif //SEQUANT2_EXPR_ALGORITHM_HPP
