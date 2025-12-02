#include "days.h"
#include "basic.h"

#include "sv.h"

#include <string.h>

char *
day2_part1(const char *input, usize input_length)
{
    StringView sv = sv_from_parts(input, input_length);
    sv = sv_trim(sv);

    u64 invalid_sum = 0;
    while (!sv_is_empty(sv)) {
        StringView range;
        sv_split_first(sv, ',', &range, &sv);

        StringView begin_sv;
        StringView end_sv;
        sv_split_first(range, '-', &begin_sv, &end_sv);

        s64 begin;
        sv_to_int64(begin_sv, &begin);
        s64 end;
        sv_to_int64(end_sv, &end);

        for (s64 id_n = begin; id_n <= end; ++id_n) {
            char *id = sprint("%lld", id_n);
            if ((strlen(id) & 1) != 0) continue; // Ignore digit counts
            usize half_len = strlen(id)/2;
            char *first = id;
            char *second = id+half_len;
            if (memcmp(first, second, half_len) == 0) {
                invalid_sum += id_n;
            }
            free(id);
        }
    }

    return sprint("%lld", invalid_sum);
}

char *
day2_part2(const char *input, usize input_length)
{
    return sprint("");
}
