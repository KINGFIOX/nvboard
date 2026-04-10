#include "src/internal/nvboard_internal.h"

#include "absl/time/clock.h"
#include "absl/time/time.h"

namespace nvboard {
namespace {

absl::Time boot_time;

}  // namespace

uint64_t GetTime() {
  return absl::ToInt64Microseconds(absl::Now() - boot_time);
}

void InitTimer() { boot_time = absl::Now(); }

}  // namespace nvboard
