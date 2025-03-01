// Copyright 2020 The XLS Authors
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

#ifndef XLS_INTERPRETER_PROC_NETWORK_INTERPRETER_H_
#define XLS_INTERPRETER_PROC_NETWORK_INTERPRETER_H_

#include <memory>
#include <vector>

#include "absl/status/statusor.h"
#include "xls/interpreter/channel_queue.h"
#include "xls/interpreter/proc_interpreter.h"
#include "xls/ir/events.h"
#include "xls/ir/package.h"

namespace xls {

// Class for interpreting a network of procs. Simultaneously interprets all
// procs in a package handling all interproc communication via a channel queues.
// ProcNetworkInterpreters are thread-compatible, but not thread-safe.
class ProcNetworkInterpreter {
 public:
  // Creates and returns an proc network interpreter for the given
  // package. user_defined_queues must contain a queue for each receive-only
  // channel in the package.
  static absl::StatusOr<std::unique_ptr<ProcNetworkInterpreter>> Create(
      Package* package,
      std::vector<std::unique_ptr<ChannelQueue>>&& user_defined_queues);

  // Execute (up to) a single iteration of every proc in the package. In a
  // round-robin fashion each proc is executed until no further progress can be
  // made. If no conditional send/receive nodes exist in the package then
  // calling Tick will execute exactly one iteration for all procs in the
  // package. If conditional send/receive nodes do exist, then some procs may be
  // blocked in a state where the iteration is partially complete. In this case,
  // the call to Tick() will not execute a complete iteration of the
  // proc. Calling Tick() again will resume these procs from their partially
  // executed state. Returns an error if no progress can be made due to a
  // deadlock.
  absl::Status Tick();

  // Tick the proc network until some output channels have produced at least a
  // specified number of outputs as indicated by `output_counts`.
  // `output_counts` must only contain output channels and need not contain all
  // output channels. Returns the number of ticks executed before the conditions
  // were met. `max_ticks` is the maximum number of ticks of the proc network
  // before returning an error.
  absl::StatusOr<int64_t> TickUntilOutput(
      absl::flat_hash_map<Channel*, int64_t> output_counts,
      std::optional<int64_t> max_ticks = absl::nullopt);

  // Tick until all procs are blocked on receive operations. `max_ticks` is the
  // maximum number of ticks of the proc network before returning an
  // error. Note: some proc networks are not guaranteed to block even if given
  // no inputs. `max_ticks` is the maximum number of ticks of the proc network
  // before returning an error.
  absl::StatusOr<int64_t> TickUntilBlocked(
      std::optional<int64_t> max_ticks = absl::nullopt);

  ChannelQueueManager& queue_manager() { return *queue_manager_; }

  // Returns the state values for each proc in the network.
  absl::flat_hash_map<Proc*, absl::StatusOr<std::vector<Value>>> ResolveState()
      const;

  void ResetState();

  // Returns the events for each proc in the network.
  absl::flat_hash_map<Proc*, InterpreterEvents> GetInterpreterEvents() const {
    absl::flat_hash_map<Proc*, InterpreterEvents> result;
    for (const auto& interp : proc_interpreters_) {
      result[interp->proc()] = interp->GetInterpreterEvents();
    }
    return result;
  }

 private:
  ProcNetworkInterpreter(Package* package,
                         std::unique_ptr<ChannelQueueManager>&& queue_manager)
      : package_(package), queue_manager_(std::move(queue_manager)) {}

  // Execute (up to) a single iteration of every proc in the package. Returns
  // whether any progress was made. If no progress was made, then
  // `blocked_channels` will contain the set of channels on which receives are
  // blocked.
  absl::StatusOr<bool> TickInternal(std::vector<Channel*>* blocked_channels);

  Package* package_;
  std::unique_ptr<ChannelQueueManager> queue_manager_;

  // The vector of interpreters for each proc in the package.
  std::vector<std::unique_ptr<ProcInterpreter>> proc_interpreters_;
};

}  // namespace xls

#endif  // XLS_INTERPRETER_PROC_NETWORK_INTERPRETER_H_
