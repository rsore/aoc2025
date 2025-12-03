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
    s64 globally_total_joltage = 0;

    StringView sv = sv_from_parts(input, input_length);
    sv = sv_trim(sv);
    while (!sv_is_empty(sv)) {
        StringView bank;
        sv_split_first(sv, '\n', &bank, &sv);
        bank = sv_trim(bank);
        if (bank.length == 0) break;

        char max_joltage_in_bank_buffer[12] = {0};
        usize filled = 0;
        usize start = 0;
        while (filled < 12) {
            usize left = 12 - filled;
            usize end = bank.length - left + 1;

            s32 best_digit = -1;
            usize best_index = start;

            for (usize i = start; i < end; ++i) {
                s32 digit = sv_at(bank, i) - '0';
                if (digit > best_digit) {
                    best_digit = digit;
                    best_index = i;
                }
            }

            max_joltage_in_bank_buffer[filled++] = (char)(best_digit + '0');
            start = best_index + 1;
        }

        StringView max_joltage_in_bank_sv = sv_from_parts(max_joltage_in_bank_buffer, 12);
        s64 max_joltage_in_bank;
        sv_to_int64(max_joltage_in_bank_sv, &max_joltage_in_bank);
        globally_total_joltage += max_joltage_in_bank;
    }

    return sprint("%lld", globally_total_joltage);
}
