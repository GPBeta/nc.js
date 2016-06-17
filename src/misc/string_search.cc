#include "string_search.h"

namespace node {
namespace stringsearch {

intptr_t StringSearchBase::kBadCharShiftTable[kUC16AlphabetSize];
intptr_t StringSearchBase::kGoodSuffixShiftTable[kBMMaxShift + 1];
intptr_t StringSearchBase::kSuffixTable[kBMMaxShift + 1];
}
}  // namespace node::stringsearch
