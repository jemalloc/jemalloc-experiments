#include "Allocation.h"
#include <jemalloc/jemalloc.h>

bool Allocation::operator<(const Allocation &that) const {
  return this->freeAfterAbsolute < that.freeAfterAbsolute;
}

bool Allocation::operator>(const Allocation &that) const {
  return this->freeAfterAbsolute > that.freeAfterAbsolute;
}

bool Allocation::isEmpty() const { return this->toFree_.size() == 0; }

Allocation::Allocation(std::vector<void *> toFree, int freeAfterArg)
    : toFree_(toFree), freeAfterRelative(freeAfterArg), freeAfterAbsolute(0) {}

void Allocation::clear() const {
  for (auto &ptr : this->toFree_) {
    free(ptr);
  }
}
