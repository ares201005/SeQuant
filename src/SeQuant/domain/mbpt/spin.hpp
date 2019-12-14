//
// Created by Eduard Valeyev on 2019-02-27.
//

#ifndef SEQUANT_SPIN_HPP
#define SEQUANT_SPIN_HPP

#include <algorithm>

#include <codecvt>
#include <map>
#include <string>
#include <tuple>

#include "SeQuant/core/expr.hpp"
#include "SeQuant/core/tensor.hpp"

#define SPINTRACE_PRINT 0
#define PRINT_PERMUTATION_LIST 0

namespace sequant {

class spinIndex : public Index {
 private:
};

struct zero_result : public std::exception {};

ExprPtr remove_spin(ExprPtr &expr, std::map<Index, Index> &replacement_map){

  // std::cout << "remove_spin_replacements:\n";
  // ranges::for_each(replacement_map, [&](std::pair<Index, Index> i) {std::wcout << (i.first).label() << " -> " << (i.second).label() << std::endl;} );

  Sum expr_sum{};
  for(auto&& summand : *expr){
    Product new_tensor_product{};
    const auto scalar_factor = summand->as<Product>().scalar().real();
    for(auto&& product : *summand){
       Tensor tensor = product->as<Tensor>();
       // TODO: why do tags need to be reset?
       ranges::for_each(tensor.const_braket(), [&] (const Index& idx) {idx.reset_tag();} );
       auto mutated = tensor.transform_indices(replacement_map, false);
       auto tensor_ptr = std::make_shared<Tensor>(tensor);
      new_tensor_product.append(1,tensor_ptr);
    }
    new_tensor_product.scale(scalar_factor);
    ExprPtr product_ptr = std::make_shared<Product>(new_tensor_product);
    expr_sum.append(product_ptr);
  }
  ExprPtr result = std::make_shared<Sum>(expr_sum);
  std::wcout << result->to_latex() << std::endl;
  return result;
}

inline bool tensor_symm(const Tensor& tensor) {
  // TODO: IF tensor, evaluate; ELSE iterate over terms to extract tensors
  bool result = false;
  assert(tensor.bra().size() == tensor.ket().size());
  // For each index check if QNS match.
  // return FALSE if one of the pairs does not match.
  auto iter_ket = tensor.ket().begin();
  for (auto&& index : tensor.bra()) {
    if (IndexSpace::instance(index.label()).qns() ==
        IndexSpace::instance(iter_ket->label()).qns()) {
      result = true;
    } else {
      result = false;
      return result;
    }
    iter_ket++;
  }
  return result;
}
//================================================
#if 1
// These functions are from:
// https://www.geeksforgeeks.org/number-swaps-sort-adjacent-swapping-allowed/
// They are required to calculate the number of inversions or adjacent swaps
// performed to get a (-1)^n sign for a permuted tensor
// TODO: Use STL containers and simplify for current application
int merge(int arr[], int temp[], int left, int mid, int right) {
  auto inv_count = 0;

  auto i = left; /* i is index for left subarray*/
  auto j = mid;  /* i is index for right subarray*/
  auto k = left; /* i is index for resultant merged subarray*/
  while ((i <= mid - 1) && (j <= right)) {
    if (arr[i] <= arr[j])
      temp[k++] = arr[i++];
    else {
      temp[k++] = arr[j++];

      /* this is tricky -- see above explanation/
        diagram for merge()*/
      inv_count = inv_count + (mid - i);
    }
  }

  /* Copy the remaining elements of left subarray
   (if there are any) to temp*/
  while (i <= mid - 1) temp[k++] = arr[i++];

  /* Copy the remaining elements of right subarray
   (if there are any) to temp*/
  while (j <= right) temp[k++] = arr[j++];

  /*Copy back the merged elements to original array*/
  for (i = left; i <= right; i++) arr[i] = temp[i];

  return inv_count;
}

int _mergeSort(int arr[], int temp[], int left, int right) {
  int mid, inv_count = 0;
  if (right > left) {
    /* Divide the array into two parts and call
      _mergeSortAndCountInv() for each of the parts */
    mid = (right + left) / 2;

    /* Inversion count will be sum of inversions in
       left-part, right-part and number of inversions
       in merging */
    inv_count = _mergeSort(arr, temp, left, mid);
    inv_count += _mergeSort(arr, temp, mid + 1, right);

    /*Merge the two parts*/
    inv_count += merge(arr, temp, left, mid + 1, right);
  }

  return inv_count;
}

int countSwaps(int arr[], int n) {
  int temp[n];
  return _mergeSort(arr, temp, 0, n - 1);
}
#endif
//================================================
/// expand an antisymmetric Tensor
/// @param tensor a tensor from a product
/// @return expression pointer containing the sum of expanded terms
ExprPtr expand_antisymm(const Tensor& tensor) {
  // bool result = false;
  assert(tensor.bra().size() == tensor.ket().size());

  // Generate a sum of asymmetric tensors if the input tensor is antisymmetric
  // AND more than one body otherwise, return the tensor
  if ((tensor.symmetry() == Symmetry::antisymm) && (tensor.bra().size() > 1)) {
    // auto n = tensor.ket().size();

    container::set<Index> bra_list;
    for (auto&& bra_idx : tensor.bra()) bra_list.insert(bra_idx);
    const auto const_bra_list = bra_list;

    container::set<Index> ket_list;
    for (auto&& ket_idx : tensor.ket()) ket_list.insert(ket_idx);

    Sum expr_sum{};
    auto p_count = 0;
    do {
      // Generate tensor with new labels
      auto new_tensor = Tensor(tensor.label(), bra_list, ket_list);

      if(tensor_symm(new_tensor)) {
        int permutation_int_array[bra_list.size()];
        auto counter_for_array = 0;

        // Store distance of each index in an array
        ranges::for_each(const_bra_list, [&](Index i) {
          auto pos = std::find(bra_list.begin(), bra_list.end(), i);
          int dist = std::distance(bra_list.begin(), pos);
          permutation_int_array[counter_for_array] = dist;
          counter_for_array++;
        });

        // Call function to count number of pair swaps
        auto permutation_count =
            countSwaps(permutation_int_array, sizeof(permutation_int_array) /
                sizeof(*permutation_int_array));

        ExprPtr new_tensor_ptr = std::make_shared<Tensor>(new_tensor);

        // Tensor as product with (-1)^n, where n is number of adjacent swaps
        Product new_tensor_product{};
        new_tensor_product.append(std::pow(-1, permutation_count),
                                  new_tensor_ptr);

        ExprPtr new_tensor_product_ptr =
            std::make_shared<Product>(new_tensor_product);
        expr_sum.append(new_tensor_product_ptr);
      }
      p_count++;
    } while (std::next_permutation(bra_list.begin(), bra_list.end()));

    ExprPtr result = std::make_shared<Sum>(expr_sum);
    // std::wcout << tensor.to_latex() << ":\n" << result->to_latex() << "\n" << std::endl;
    return result;
  } else {
    ExprPtr result = std::make_shared<Tensor>(tensor);
    // std::wcout << tensor.to_latex() << ":\n" << result->to_latex() << "\n" << std::endl;
    return result;
  }
}

ExprPtr expand_antisymm(const ExprPtr& expr){
  Sum expanded_sum{};
  for(auto &&summand: *expr){
    const auto scalar_factor = summand->as<Product>().scalar().real();
    Product expanded_product{};
    for(auto &&expr_product: *summand ){
       auto tensor = expr_product->as<sequant::Tensor>();
       auto expanded_ptr = expand_antisymm(tensor);
       expanded_product.append(1, expanded_ptr);
    }
    expanded_product.scale(scalar_factor);
    ExprPtr expanded_product_ptr = std::make_shared<Product>(expanded_product);
    expanded_sum.append(expanded_product_ptr);
  }
  ExprPtr result = std::make_shared<Sum>(expanded_sum);
  return result;
}

/// Check if the number of alpha spins in bra and ket are equal
/// beta spins will match if total number of indices is the same
/// @param any tensor
/// @return true if number of alpha spins match in bra and ket
bool can_expand(const Tensor& tensor){
  assert(tensor.bra_rank() == tensor.ket_rank());
  auto result = false;
  auto alpha_in_bra = 0;
  auto alpha_in_ket = 0;
  ranges::for_each(tensor.bra(),[&](Index i){
     if(IndexSpace::instance(i.label()).qns() == IndexSpace::alpha)
       ++alpha_in_bra;
  });
  ranges::for_each(tensor.ket(),[&](Index i){
    if(IndexSpace::instance(i.label()).qns() == IndexSpace::alpha)
      ++alpha_in_ket;
  });
  if(alpha_in_bra == alpha_in_ket)
    result = true;
  return result;
}

/// @param expr input expression
/// @param ext_index_groups groups of external indices
/// @return the expression with spin integrated out
ExprPtr spintrace(ExprPtr expr,
                  std::initializer_list<IndexList> ext_index_groups = {{}}) {
  // SPIN TRACE DOES NOT SUPPORT PROTO INDICES YET.
  auto check_proto_index = [&](const ExprPtr& expr) {
    if (expr->is<Tensor>()) {
      ranges::for_each(
          expr->as<Tensor>().const_braket(),
          [&](const Index& idx) { assert(!idx.has_proto_indices()); });
    }
  };
  expr->visit(check_proto_index);

  container::set<Index, Index::LabelCompare> grand_idxlist;
  auto collect_indices = [&](const ExprPtr& expr) {
    if (expr->is<Tensor>()) {
      ranges::for_each(expr->as<Tensor>().const_braket(),
                       [&](const Index& idx) { grand_idxlist.insert(idx); });
    }
  };
  expr->visit(collect_indices);

  container::set<Index> ext_idxlist;
  for (auto&& idxgrp : ext_index_groups) {
    for (auto&& idx : idxgrp) {
      auto result = ext_idxlist.insert(idx);
      assert(result.second);
    }
  }

  container::set<Index> int_idxlist;
  for (auto&& gidx : grand_idxlist) {
    if (ext_idxlist.find(gidx) == ext_idxlist.end()) {
      int_idxlist.insert(gidx);
    }
  }

  // EFV: generate the grand list of index groups by concatenating list of
  // external index EFV: groups with the groups of internal indices (each
  // internal index = 1 group)
  using IndexGroup = container::svector<Index>;
  container::svector<IndexGroup> index_groups;

  for (auto&& i : int_idxlist) {
    index_groups.emplace_back(IndexGroup(1, i));
  }

  index_groups.insert(index_groups.end(), ext_index_groups.begin(),
                      ext_index_groups.end());

  // EFV: for each spincase (loop over integer from 0 to 2^(n)-1, n=#of index
  // groups)

  // const uint64_t nspincases = std::pow(2, index_groups.size());
  const uint64_t nspincases = 1;  // TODO: remove this line before release

  // Sum spin_expr_sum{};
  auto total_terms = 0;
  const auto tstart = std::chrono::high_resolution_clock::now();
  for (uint64_t spincase_bitstr = 0; spincase_bitstr != nspincases;
       ++spincase_bitstr) {
    std::cout << "\nPermutation: " << spincase_bitstr << ":\n";
    std::wcout << "expr: " << expr->to_latex() << "\n";

    // EFV:  assign spin to each index group => make a replacement list
    std::map<Index, Index> index_replacements;
    std::map<Index, Index> remove_spin_replacements;

    int64_t index_group_count = 0;
    for (auto&& index_group : index_groups) {
      auto spin_bit = (spincase_bitstr << (64 - index_group_count - 1)) >> 63;
      assert((spin_bit == 0) || (spin_bit == 1));

      for (auto&& index : index_group) {
        IndexSpace space;
        if (spin_bit == 0) {
          space = IndexSpace::instance(
              IndexSpace::instance(index.label()).type(), IndexSpace::alpha);
        } else {
          space = IndexSpace::instance(
              IndexSpace::instance(index.label()).type(), IndexSpace::beta);
        }
        auto subscript_label =
            index.label().substr(index.label().find(L'_') + 1);
        std::wstring subscript_label_ws(subscript_label.begin(),
                                        subscript_label.end());

        Index spin_index = Index::make_label_index(space, subscript_label_ws);

        index_replacements.emplace(std::make_pair(index, spin_index));
        remove_spin_replacements.emplace(std::make_pair(spin_index, index));
      }
      ++index_group_count;
    }

    // sequant::Tensor tensor;
    auto summand_count = 0;
    Sum temp_sum;

    // For each term in the sum
    for (auto&& summand : *expr) {

      // Get scaling factor for the summand
      const auto scalar_factor = summand->as<Product>().scalar().real();

      sequant::Tensor tensor;
      Product temp_product{};

      // For each product in the sum
      for (auto&& expr_product : *summand) {

        // get tensors from products
        tensor = expr_product->as<Tensor>();

        // apply index replacement on tensor
        auto pass_mutated =
            tensor.transform_indices(index_replacements, false);

        if(can_expand(tensor)){
          // TODO: Use canonicalizer
          auto antisymm_expansion = expand_antisymm(tensor);
          temp_product.append(1,antisymm_expansion);
          // std::wcout << "antisymmetrized: " << antisymm_expansion->to_latex() << "\n";
          ExprPtr temp_tensor = std::make_shared<Tensor>(tensor);
        }

        // ExprPtr tensor_ptr = std::make_shared<Tensor>(tensor);
        // temp_product.append(1.0, tensor_ptr);
      }
      // std::wcout << "temp_product: " << temp_product.to_latex() << std::endl;

      if ((*summand).size() == temp_product.size()) {
        temp_product.scale(scalar_factor);
        ExprPtr tensor_product_ptr = std::make_shared<Product>(temp_product);
        expand(tensor_product_ptr);
        temp_sum.append(tensor_product_ptr);
      }
    }
    // std::wcout << "temp_sum_: " << temp_sum.to_latex() << std::endl;
    ExprPtr tensor_sum_ptr = std::make_shared<Sum>(temp_sum);
    // std::wcout << "tensor_sum_ptr: " << tensor_sum_ptr->to_latex() << std::endl;

//     std::cout << "remove_spin_replacements:\n";
//     ranges::for_each(remove_spin_replacements, [&](std::pair<Index, Index> i) {std::wcout << (i.first).label() << " -> " << (i.second).label() << std::endl;} );

    auto no_spin_ptr = remove_spin(tensor_sum_ptr, remove_spin_replacements);
    expand(no_spin_ptr);
    std::wcout << " no_spin_ptr: " << no_spin_ptr->to_latex() << std::endl;

    // spin_expr_sum.append(tensor_sum_ptr);
    total_terms += summand_count;

  }  // Permutation FOR loop

  const auto tstop = std::chrono::high_resolution_clock::now();
  auto time_elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(tstop - tstart);
  std::cout << "Time: " << time_elapsed.count() << " milli sec; " << total_terms
            << " terms after expansion." << std::endl;

  // std::wcout << spin_expr_sum.to_latex() << std::endl;
  return nullptr;
}

}  // namespace sequant

#endif  // SEQUANT_SPIN_HPP
