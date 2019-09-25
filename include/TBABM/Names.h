#pragma once

#include <RNG.h>

using std::vector;
using std::string;

class Names {
  public:
    Names(vector<string> names_ = {}) {
      if (names_.size() != 0)
        names = names_;
    };

    string getName(RNG &rng) {
      size_t length = names.size();
      auto num = rng.mt_();

      return names.at(static_cast<size_t>(num) % length);
    }

  private:
    vector<string> names = 
#include "Names.inc"
};
