// Copyright 2022 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "xls/ir/call_graph.h"

#include "absl/container/flat_hash_set.h"
#include "xls/ir/op.h"

namespace xls {

std::optional<Function*> CalledFunction(Node* node) {
  switch (node->op()) {
    case Op::kCountedFor:
      return node->As<CountedFor>()->body();
    case Op::kDynamicCountedFor:
      return node->As<DynamicCountedFor>()->body();
    case Op::kMap:
      return node->As<Map>()->to_apply();
    case Op::kInvoke:
      return node->As<Invoke>()->to_apply();
    default:
      return std::nullopt;
  }
}

std::vector<Function*> CalledFunctions(FunctionBase* function_base) {
  absl::flat_hash_set<Function*> called_set;
  std::vector<Function*> called;
  for (Node* node : function_base->nodes()) {
    if (std::optional<Function*> callee = CalledFunction(node)) {
      auto [_, inserted] = called_set.insert(callee.value());
      if (inserted) {
        called.push_back(callee.value());
      }
    }
  }
  return called;
}

// Recursive DFS visitor of the call graph induced by invoke
// instructions. Builds a post order of functions in the post_order vector.
static void DfsVisit(FunctionBase* f,
                     absl::flat_hash_set<FunctionBase*>* visited,
                     std::vector<FunctionBase*>* post_order) {
  visited->insert(f);
  for (FunctionBase* callee : CalledFunctions(f)) {
    if (!visited->contains(callee)) {
      DfsVisit(callee, visited, post_order);
    }
  }
  post_order->push_back(f);
}

std::vector<FunctionBase*> GetDependentFunctions(FunctionBase* function_base) {
  absl::flat_hash_set<FunctionBase*> visited;
  std::vector<FunctionBase*> post_order;
  DfsVisit(function_base, &visited, &post_order);

  if (XLS_VLOG_IS_ON(2)) {
    XLS_VLOG(2) << absl::StreamFormat("DependentFunctions(%s):",
                                      function_base->name());
    for (FunctionBase* f : post_order) {
      XLS_VLOG(2) << "  " << f->name();
    }
  }
  return post_order;
}

// Returns the functions which are roots in the call graph, that is, the
// functions which are not called by any other functions.
static std::vector<FunctionBase*> GetRootFunctions(Package* p) {
  absl::flat_hash_set<FunctionBase*> called_functions;
  for (FunctionBase* f : p->GetFunctionBases()) {
    for (FunctionBase* callee : CalledFunctions(f)) {
      called_functions.insert(callee);
    }
  }
  std::vector<FunctionBase*> roots;
  for (FunctionBase* f : p->GetFunctionBases()) {
    if (!called_functions.contains(f)) {
      roots.push_back(f);
    }
  }
  return roots;
}

std::vector<FunctionBase*> FunctionsInPostOrder(Package* p) {
  absl::flat_hash_set<FunctionBase*> visited;
  std::vector<FunctionBase*> post_order;
  for (FunctionBase* f : GetRootFunctions(p)) {
    DfsVisit(f, &visited, &post_order);
  }
  return post_order;
}

}  // namespace xls
