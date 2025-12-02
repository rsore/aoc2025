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
            if ((strlen(id) & 1) != 0) continue; // Ignore if digit count is odd
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
            usize id_len = strlen(id);
            usize half_id_len = id_len/2;

            bool id_invalid = false;
            for (size_t len = 1; len <= half_id_len; ++len) {
                if ((id_len % len) != 0) continue;
                StringView sub_id = sv_from_parts(id, len);
                if (sub_id.begin[0] == '0') continue;

                bool sub_invalid = true;
                usize next_begin = len;
                while (next_begin < id_len) {
                    StringView comp_id = sv_from_parts(id+next_begin, len);
                    if (!sv_eq(sub_id, comp_id)) {
                        sub_invalid = false;
                        break;
                    }
                    next_begin += len;
                }
                if (sub_invalid) id_invalid = true;
            }
            if (id_invalid) invalid_sum += id_n;

            free(id);
        }
    }

    return sprint("%lld", invalid_sum);
}
