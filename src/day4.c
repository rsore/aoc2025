#include "days.h"
#include "basic.h"

#include "sv.h"

#include <stdint.h>
#include <string.h>

char *
day4_part1(const char *input, usize input_length)
{
    StringView sv = sv_from_parts(input, input_length);

    intptr_t grid_width = 0;
    while (sv_at(sv, grid_width) != '\n') grid_width += 1;
    intptr_t grid_pitch = grid_width + 1;

    s64 accessible = 0;

    for (intptr_t current_index = 0; current_index < (intptr_t)sv.length; ++current_index) {
        if (sv_at(sv, current_index) != '@') continue;

        s32 adjacent_rolls_of_paper = 0;

        intptr_t left_index        = current_index - 1;
        intptr_t right_index       = current_index + 1;
        intptr_t above_index       = current_index - grid_pitch;
        intptr_t below_index       = current_index + grid_pitch;
        intptr_t above_left_index  = above_index - 1;
        intptr_t above_right_index = above_index + 1;
        intptr_t below_left_index  = below_index - 1;
        intptr_t below_right_index = below_index + 1;
        if ((left_index        >= 0) && (left_index        < (intptr_t)sv.length) && (sv_at(sv, left_index)        == '@')) adjacent_rolls_of_paper += 1;
        if ((right_index       >= 0) && (right_index       < (intptr_t)sv.length) && (sv_at(sv, right_index)       == '@')) adjacent_rolls_of_paper += 1;
        if ((above_index       >= 0) && (above_index       < (intptr_t)sv.length) && (sv_at(sv, above_index)       == '@')) adjacent_rolls_of_paper += 1;
        if ((below_index       >= 0) && (below_index       < (intptr_t)sv.length) && (sv_at(sv, below_index)       == '@')) adjacent_rolls_of_paper += 1;
        if ((above_left_index  >= 0) && (above_left_index  < (intptr_t)sv.length) && (sv_at(sv, above_left_index)  == '@')) adjacent_rolls_of_paper += 1;
        if ((above_right_index >= 0) && (above_right_index < (intptr_t)sv.length) && (sv_at(sv, above_right_index) == '@')) adjacent_rolls_of_paper += 1;
        if ((below_left_index  >= 0) && (below_left_index  < (intptr_t)sv.length) && (sv_at(sv, below_left_index)  == '@')) adjacent_rolls_of_paper += 1;
        if ((below_right_index >= 0) && (below_right_index < (intptr_t)sv.length) && (sv_at(sv, below_right_index) == '@')) adjacent_rolls_of_paper += 1;

        if (adjacent_rolls_of_paper >= 4) continue;

        accessible += 1;
    }

    return sprint("%lld", accessible);
}

char *
day4_part2(const char *input, usize input_length)
{
    StringView sv = sv_from_parts(input, input_length);

    char *backbuf = (char *)malloc(input_length);
    char *to_free = backbuf;

    intptr_t grid_width = 0;
    while (sv_at(sv, grid_width) != '\n') grid_width += 1;
    intptr_t grid_pitch = grid_width + 1;

    s64 removable = 0;
    s64 removable_before = -1;
    while (removable != removable_before) {
        removable_before = removable;
        memcpy(backbuf, sv.begin, sv.length);

        s64 accessible = 0;
        for (intptr_t current_index = 0; current_index < (intptr_t)sv.length; ++current_index) {
            if (sv_at(sv, current_index) != '@') continue;

            s32 adjacent_rolls_of_paper = 0;

            intptr_t left_index        = current_index - 1;
            intptr_t right_index       = current_index + 1;
            intptr_t above_index       = current_index - grid_pitch;
            intptr_t below_index       = current_index + grid_pitch;
            intptr_t above_left_index  = above_index - 1;
            intptr_t above_right_index = above_index + 1;
            intptr_t below_left_index  = below_index - 1;
            intptr_t below_right_index = below_index + 1;
            if ((left_index        >= 0) && (left_index        < (intptr_t)sv.length) && (sv_at(sv, left_index)        == '@')) adjacent_rolls_of_paper += 1;
            if ((right_index       >= 0) && (right_index       < (intptr_t)sv.length) && (sv_at(sv, right_index)       == '@')) adjacent_rolls_of_paper += 1;
            if ((above_index       >= 0) && (above_index       < (intptr_t)sv.length) && (sv_at(sv, above_index)       == '@')) adjacent_rolls_of_paper += 1;
            if ((below_index       >= 0) && (below_index       < (intptr_t)sv.length) && (sv_at(sv, below_index)       == '@')) adjacent_rolls_of_paper += 1;
            if ((above_left_index  >= 0) && (above_left_index  < (intptr_t)sv.length) && (sv_at(sv, above_left_index)  == '@')) adjacent_rolls_of_paper += 1;
            if ((above_right_index >= 0) && (above_right_index < (intptr_t)sv.length) && (sv_at(sv, above_right_index) == '@')) adjacent_rolls_of_paper += 1;
            if ((below_left_index  >= 0) && (below_left_index  < (intptr_t)sv.length) && (sv_at(sv, below_left_index)  == '@')) adjacent_rolls_of_paper += 1;
            if ((below_right_index >= 0) && (below_right_index < (intptr_t)sv.length) && (sv_at(sv, below_right_index) == '@')) adjacent_rolls_of_paper += 1;

            if (adjacent_rolls_of_paper >= 4) continue;

            accessible += 1;
            backbuf[current_index] = '.';
        }

        removable += accessible;

        // Swap buffers
        char *temp = (char *)sv.begin;
        sv.begin = backbuf;
        backbuf = temp;
    }

    free(to_free);
    return sprint("%lld", removable);
}
