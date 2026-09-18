// Copyright 2026 Kenneth Looney
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#ifndef KASM_CHECKED_ARITHMETIC_H
#define KASM_CHECKED_ARITHMETIC_H

#include <limits.h>

static inline int checked_add(long long left, long long right,
                              long long *result)
{
    if ((right > 0 && left > LLONG_MAX - right) ||
        (right < 0 && left < LLONG_MIN - right))
        return 0;

    *result = left + right;
    return 1;
}

static inline int checked_subtract(long long left, long long right,
                                   long long *result)
{
    if ((right < 0 && left > LLONG_MAX + right) ||
        (right > 0 && left < LLONG_MIN + right))
        return 0;

    *result = left - right;
    return 1;
}

static inline int checked_multiply(long long left, long long right,
                                   long long *result)
{
    if (left == 0 || right == 0)
    {
        *result = 0;
        return 1;
    }

    if ((left == -1 && right == LLONG_MIN) ||
        (right == -1 && left == LLONG_MIN))
        return 0;

    if (left > 0)
    {
        if (right > 0 && left > LLONG_MAX / right)
            return 0;
        if (right < 0 && right < LLONG_MIN / left)
            return 0;
    }
    else
    {
        if (right > 0 && left < LLONG_MIN / right)
            return 0;
        if (right < 0 && left < LLONG_MAX / right)
            return 0;
    }

    *result = left * right;
    return 1;
}

#endif // KASM_CHECKED_ARITHMETIC_H