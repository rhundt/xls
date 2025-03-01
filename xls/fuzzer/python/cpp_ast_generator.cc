// Copyright 2021 The XLS Authors
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

// Exposes AST generation capability (as is needed for fuzzing) to Python code
// (which currently drives the sampling / running process).

#include <memory>

#include "absl/base/casts.h"
#include "absl/status/statusor.h"
#include "pybind11/functional.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "xls/common/status/ret_check.h"
#include "xls/common/status/status_macros.h"
#include "xls/common/status/statusor_pybind_caster.h"
#include "xls/dslx/ast.h"
#include "xls/fuzzer/ast_generator.h"
#include "xls/fuzzer/sample_generator.h"

namespace py = pybind11;

namespace xls::dslx {

PYBIND11_MODULE(cpp_ast_generator, m) {
  ImportStatusModule();
  py::module::import("xls.dslx.python.interp_value");

  py::class_<RngState>(m, "RngState")
      .def(py::init([](int64_t seed) { return RngState{std::mt19937(seed)}; }))
      .def("random", &RngState::RandomDouble)
      .def("randrange", &RngState::RandRange)
      .def("randrange_biased_towards_zero",
           &RngState::RandRangeBiasedTowardsZero);

  py::class_<AstGeneratorOptions>(m, "AstGeneratorOptions")
      .def(py::init([](std::optional<bool> emit_loops,
                       std::optional<int64_t> max_width_bits_types,
                       std::optional<int64_t> max_width_aggregate_types,
                       std::optional<bool> emit_gate,
                       std::optional<bool> generate_proc) {
             AstGeneratorOptions options;
             if (max_width_bits_types.has_value()) {
               options.max_width_bits_types = max_width_bits_types.value();
             }
             if (max_width_aggregate_types.has_value()) {
               options.max_width_aggregate_types =
                   max_width_aggregate_types.value();
             }
             if (emit_gate.has_value()) {
               options.emit_gate = emit_gate.value();
             }
             if (generate_proc.has_value()) {
               options.generate_proc = generate_proc.value();
             }
             return options;
           }),
           py::arg("emit_loops") = absl::nullopt,
           py::arg("max_width_bits_types") = absl::nullopt,
           py::arg("max_width_aggregate_types") = absl::nullopt,
           py::arg("emit_gate") = absl::nullopt,
           py::arg("generate_proc") = absl::nullopt)
      // Pickling is required by the multiprocess fuzzer which pickles options
      // to send to the separate worker process.
      .def(py::pickle(
          [](const AstGeneratorOptions& o) {
            return py::make_tuple(o.emit_signed_types, o.max_width_bits_types,
                                  o.max_width_aggregate_types, o.emit_loops,
                                  o.emit_gate, o.generate_proc);
          },
          [](py::tuple t) {
            return AstGeneratorOptions{
                .emit_signed_types = t[0].cast<bool>(),
                .max_width_bits_types = t[1].cast<int64_t>(),
                .max_width_aggregate_types = t[2].cast<int64_t>(),
                .emit_loops = t[3].cast<bool>(),
                .emit_gate = t[4].cast<bool>(),
                .generate_proc = t[5].cast<bool>()};
          }));

  m.def("generate",
        [](const AstGeneratorOptions& options,
           RngState& state) -> absl::StatusOr<std::string> {
          AstGenerator g(options, &state.rng());
          XLS_ASSIGN_OR_RETURN(std::unique_ptr<Module> module,
                               g.Generate("main", "test"));
          return module->ToString();
        });

  m.def("choose_bit_pattern", [](int64_t bit_count, RngState& state) -> Bits {
    AstGenerator g(AstGeneratorOptions(), &state.rng());
    return g.ChooseBitPattern(bit_count);
  });

  m.def("generate_arguments",
        [](const std::vector<const ConcreteType*>& arg_types,
           RngState* rng) -> absl::StatusOr<py::tuple> {
          XLS_ASSIGN_OR_RETURN(auto arguments,
                               GenerateArguments(arg_types, rng));
          py::tuple result(arguments.size());
          for (int64_t i = 0; i < arguments.size(); ++i) {
            result[i] = arguments[i];
          }
          return result;
        });
  m.def("generate_sample", GenerateSample, py::arg("options"),
        py::arg("default_options"), py::arg("rng"));
}

}  // namespace xls::dslx
