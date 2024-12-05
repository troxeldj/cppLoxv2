#ifndef LOXRETURN_H
#define LOXRETURN_H

#include <any>

struct LoxReturn {
  std::any value;
  LoxReturn(std::any value) : value{std::move(value)} {}
};

#endif