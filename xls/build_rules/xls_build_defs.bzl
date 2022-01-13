# Copyright 2021 The XLS Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
This module contains build rules for XLS.

This module is the primary module and **must** contain all the XLS build rules
exposed to the user. This module is created for convenience.
"""

load(
    "//xls/build_rules:xls_dslx_rules.bzl",
    _xls_dslx_library = "xls_dslx_library",
    _xls_dslx_module_library = "xls_dslx_module_library",
    _xls_dslx_test = "xls_dslx_test",
)
load(
    "//xls/build_rules:xls_ir_macros.bzl",
    _xls_dslx_ir_macro = "xls_dslx_ir_macro",
    _xls_ir_opt_ir_macro = "xls_ir_opt_ir_macro",
)
load(
    "//xls/build_rules:xls_ir_rules.bzl",
    _get_mangled_ir_symbol = "get_mangled_ir_symbol",
    _xls_benchmark_ir = "xls_benchmark_ir",
    _xls_eval_ir_test = "xls_eval_ir_test",
    _xls_ir_equivalence_test = "xls_ir_equivalence_test",
)
load(
    "//xls/build_rules:xls_jit_wrapper_rules.bzl",
    _cc_xls_ir_jit_wrapper = "cc_xls_ir_jit_wrapper",
    _xls_ir_jit_wrapper = "xls_ir_jit_wrapper",
)
load(
    "//xls/build_rules:xls_rules.bzl",
    _xls_dslx_opt_ir_test = "xls_dslx_opt_ir_test",
)
load(
    "//xls/build_rules:xls_codegen_macros.bzl",
    _xls_ir_verilog_macro = "xls_ir_verilog_macro",
)
load(
    "//xls/build_rules:xls_macros.bzl",
    _xls_dslx_cpp_type_library = "xls_dslx_cpp_type_library",
    _xls_dslx_opt_ir_macro = "xls_dslx_opt_ir_macro",
    _xls_dslx_verilog_macro = "xls_dslx_verilog_macro",
    _xls_verify_checksum = "xls_verify_checksum",
)

# XLS Rules
xls_dslx_library = _xls_dslx_library
xls_dslx_module_library = _xls_dslx_module_library
xls_dslx_test = _xls_dslx_test

get_mangled_ir_symbol = _get_mangled_ir_symbol
xls_benchmark_ir = _xls_benchmark_ir
xls_ir_equivalence_test = _xls_ir_equivalence_test
xls_eval_ir_test = _xls_eval_ir_test

cc_xls_ir_jit_wrapper = _cc_xls_ir_jit_wrapper
xls_ir_jit_wrapper = _xls_ir_jit_wrapper

xls_dslx_opt_ir_test = _xls_dslx_opt_ir_test

# XLS Macros
# TODO (vmirian) 1-10-2022 Do not expose xls_dslx_ir to user. Prefer to simply
# have an opt ir generated from a DSLX file.
xls_dslx_ir = _xls_dslx_ir_macro

# TODO (vmirian) 1-10-2022 Do not expose xls_ir_opt_ir to user.
xls_ir_opt_ir = _xls_ir_opt_ir_macro
xls_ir_verilog = _xls_ir_verilog_macro
xls_dslx_opt_ir = _xls_dslx_opt_ir_macro
xls_dslx_verilog = _xls_dslx_verilog_macro
xls_dslx_cpp_type_library = _xls_dslx_cpp_type_library
xls_verify_checksum = _xls_verify_checksum
