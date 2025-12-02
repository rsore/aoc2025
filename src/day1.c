#include "days.h"
#include "basic.h"

#include "sv.h"

char *
day1_part1(const char *input, usize input_length)
{
    StringView sv = sv_from_parts(input, input_length);
    sv = sv_trim(sv);

    u32 n_0 = 0;
    s32 dial = 50;
    while (sv.length) {
        StringView this_line;
        sv_split_first(sv, '\n', &this_line, &sv);
        this_line = sv_trim(this_line);

        StringView dir_sv = sv_take_and_consume(&this_line, 1);
        s32 dir = sv_eq(dir_sv, SV_LIT("R")) ? 1 : -1;
        s64 scalar;
        sv_to_int64(this_line, &scalar);
        dial += 100; // Offset to make mod of negative safe
        dial += dir * scalar;
        dial %= 100;
        if (dial == 0) n_0 += 1;
    }

    char *result = sprint("%u", n_0);

    return result;
}

char *
day1_part2(const char *input, usize input_length)
{
    StringView sv = sv_from_parts(input, input_length);
    sv = sv_trim(sv);

    u32 n_0 = 0;
    s32 dial = 50;
    while (sv.length) {
        StringView this_line;
        sv_split_first(sv, '\n', &this_line, &sv);
        this_line = sv_trim(this_line);

        StringView dir_sv = sv_take_and_consume(&this_line, 1);
        s32        dir    = sv_eq(dir_sv, SV_LIT("R")) ? 1 : -1;
        s64 scalar;
        sv_to_int64(this_line, &scalar);

        s64  value    = dir * scalar;
        bool was_zero = dial == 0;
        dial += value;

        if (dial < 0 && !was_zero) {
            n_0 += 1;
        }

        n_0 += absolute_value(dial) / 100;

        if (dial == 0) {
            n_0 += 1;
        }

        dial = ((dial % 100) + 100) % 100;
    }

    char *result = sprint("%u", n_0);

    return result;
}
