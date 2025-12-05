#include "basic.h"
#include "days.h"

#include "sv.h"
#include "stb_ds.h"

#include <stdint.h>
#include <string.h>

typedef struct {
    s64 begin;
    s64 end;
} Range;

char *
day5_part1(const char *input, usize input_length)
{
    // Split input up into one view of ranges, and one view of available ingredients
    StringView temp = sv_from_parts(input, input_length);
    while (sv_first(temp) != '\n') {
        sv_split_first(temp, '\n', NULL, &temp);
    }
    StringView fresh_id_ranges_sv = sv_from_parts(input, temp.begin - input);
    fresh_id_ranges_sv = sv_trim(fresh_id_ranges_sv);
    StringView available_ingredients_ids_sv = sv_from_parts(temp.begin, temp.length-1);
    available_ingredients_ids_sv = sv_trim(available_ingredients_ids_sv);


    // Build array of ranges of fresh ingredient IDs
    Range *fresh_id_ranges = NULL;
    temp = fresh_id_ranges_sv;
    while (!sv_is_empty(temp)) {
        StringView range_sv;
        sv_split_first(temp, '\n', &range_sv, &temp);
        StringView range_begin_sv;
        StringView range_end_sv;
        sv_split_first(range_sv, '-', &range_begin_sv, &range_end_sv);
        Range range;
        sv_to_int64(range_begin_sv, &range.begin);
        sv_to_int64(range_end_sv, &range.end);
        arrput(fresh_id_ranges, range);
    }

    // Count available ingredient IDs that are within fresh ranges
    s64 n_fresh_available_ingredients_ids = 0;
    temp = available_ingredients_ids_sv;
    while (!sv_is_empty(temp)) {
        StringView id_sv;
        sv_split_first(temp, '\n', &id_sv, &temp);
        s64 id;
        sv_to_int64(id_sv, &id);

        for (usize i = 0; i < arrlenu(fresh_id_ranges); ++i) {
            Range *range = &fresh_id_ranges[i];
            if (id >= range->begin && id <= range->end) {
                n_fresh_available_ingredients_ids += 1;
                break;
            }
        }
    }

    arrfree(fresh_id_ranges);
    return sprint("%lld", n_fresh_available_ingredients_ids);
}

char *
day5_part2(const char *input, usize input_length)
{
    return sprint("");
}
