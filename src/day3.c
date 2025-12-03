#include "days.h"
#include "basic.h"

#include "sv.h"

#include <string.h>

char *
day3_part1(const char *input, usize input_length)
{
    s64 globally_total_joltage = 0;

    StringView sv = sv_from_parts(input, input_length);
    sv = sv_trim(sv);
    while (!sv_is_empty(sv)) {
        StringView bank;
        sv_split_first(sv, '\n', &bank, &sv);
        bank = sv_trim(bank);

        s64 max_joltage_in_bank = 0;
        for (usize i = 0; i < bank.length - 1; ++i) {
            for (usize j = i+1; j < bank.length; ++j) {
                char joltage_cstr[2] = {sv_at(bank, i), sv_at(bank, j)};
                StringView joltage_sv = sv_from_parts(joltage_cstr, 2);
                s64 joltage;
                sv_to_int64(joltage_sv, &joltage);
                max_joltage_in_bank = max_s64(max_joltage_in_bank,
                                              joltage);
            }
        }
        globally_total_joltage += max_joltage_in_bank;
    }

    return sprint("%lld", globally_total_joltage);
}

char *
day3_part2(const char *input, usize input_length)
{
    return sprint("");
}
