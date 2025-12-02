#include "aoc2025.h"
#include "days.h"

#include "raylib.h"
#include "clay.h"
#include "clay_renderer_raylib.c"

#include <stdio.h>

typedef char *(*ProblemFunc)(const char*, usize);

struct {
    const char *day_button_title;

    const char *title_file;
    Clay_String title;

    ProblemFunc  part1_func;
    const char  *part1_description_file;
    Clay_String  part1_description;
    const char  *part1_test_data_file;
    Clay_String  part1_test_data;
    const char  *part1_real_data_file;
    Clay_String  part1_real_data;
    char        *part1_test_result;
    char        *part1_real_result;

    ProblemFunc  part2_func;
    const char  *part2_description_file;
    Clay_String  part2_description;
    const char  *part2_test_data_file;
    Clay_String  part2_test_data;
    const char  *part2_real_data_file;
    Clay_String  part2_real_data;
    char        *part2_test_result;
    char        *part2_real_result;

} days[] = {
    { .day_button_title  = "Day 1",
      .title_file        = "data/day1/title.txt",
      .part1_description_file  = "data/day1/part1_description.txt",
      .part1_test_data_file = "data/day1/test_data.txt",
      .part1_real_data_file    = "data/day1/real_data.txt",
      .part1_func              = day1_part1,
      .part2_description_file  = "data/day1/part2_description.txt",
      .part2_test_data_file = "data/day1/test_data.txt",
      .part2_real_data_file    = "data/day1/real_data.txt",
      .part2_func              = day1_part2}
};



const uint32_t FONT_ID_BODY_24 = 0;
const uint32_t FONT_ID_BODY_16 = 1;
#define COLOR_ORANGE (Clay_Color){225, 138, 50, 255}
#define COLOR_BLUE   (Clay_Color){111, 173, 162, 255}

// COLORS
#define BG_COLOR                (Clay_Color){13, 17, 22, 255}
#define SURFACE_COLOR           (Clay_Color){22, 27, 34, 255}
#define SURFACE_HIGHLIGHT_COLOR (Clay_Color){30, 36, 45, 255}

#define PRIMARY_ACCENT_COLOR (Clay_Color){74, 179, 244, 255}
#define SECONDARY_ACCENT_COLOR (Clay_Color){107, 214, 255, 255}

#define PRIMARY_TEXT_COLOR (Clay_Color){229, 233, 240, 255}
#define SECONDARY_TEXT_COLOR (Clay_Color){169, 180, 198, 255}
#define ACCENT_TEXT_COLOR (Clay_Color){10, 14, 18, 255}

Texture2D aoc_icon;

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) (Clay_Vector2){.x = vector.x, .y = vector.y}

Clay_TextElementConfig headerTextConfig = {.fontId = 1, .letterSpacing = 5, .fontSize = 16, .textColor = {0,0,0,255}};

static usize active_day_index    = 0;
static u32   active_problem_part = 1;

static void
handle_day_button_interaction(Clay_ElementId   element_id,
                              Clay_PointerData pointer_info,
                              intptr_t         user_data)
{
    UNUSED(element_id);
    usize index = (usize)user_data;
    if (pointer_info.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (index <= ARRAY_LENGTH(days)) {
            active_day_index = index;
        }
    }
}

static void
handle_part_button_interaction(Clay_ElementId   element_id,
                               Clay_PointerData pointer_info,
                               intptr_t         user_data)
{
    UNUSED(element_id);
    u32 part = (u32)user_data;
    if (pointer_info.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (part == 1) active_problem_part = 1;
        else           active_problem_part = 2;
    }
}

static void
handle_run_button_interaction(Clay_ElementId   element_id,
                              Clay_PointerData pointer_info,
                              intptr_t         user_data)
{
    UNUSED(element_id);
    bool test = (bool)user_data;
    if (pointer_info.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if (test) {
            if (active_problem_part == 1) {
                free(days[active_day_index].part1_test_result);
                days[active_day_index].part1_test_result = days[active_day_index].part1_func(days[active_day_index].part1_test_data.chars, days[active_day_index].part1_test_data.length);
            } else {
                free(days[active_day_index].part2_test_result);
                days[active_day_index].part2_test_result = days[active_day_index].part2_func(days[active_day_index].part2_test_data.chars, days[active_day_index].part2_test_data.length);
            }
        } else {
            if (active_problem_part == 1) {
                free(days[active_day_index].part1_real_result);
                days[active_day_index].part1_real_result = days[active_day_index].part1_func(days[active_day_index].part1_real_data.chars, days[active_day_index].part1_real_data.length);
            } else {
                free(days[active_day_index].part2_real_result);
                days[active_day_index].part2_real_result = days[active_day_index].part2_func(days[active_day_index].part2_real_data.chars, days[active_day_index].part2_real_data.length);
            }
        }
    }
}

static Clay_RenderCommandArray
CreateLayout(void)
{
    Clay_BeginLayout();
    CLAY({.id = CLAY_ID("MasterContainer"),
          .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                     .padding = {16,16,16,16},
                     .childGap = 16},
          .backgroundColor = BG_COLOR})
    {
        CLAY({.id = CLAY_ID("SideBar"),
              .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                         .sizing = {.width = CLAY_SIZING_FIXED(300), .height = CLAY_SIZING_GROW(0)},
                         .padding = {16,16,16,16},
                         .childGap = 16},
              .backgroundColor = SURFACE_COLOR})
        {
            CLAY({.id = CLAY_ID("HeaderOuter"),
                  .layout = {.sizing = {.width = CLAY_SIZING_GROW(0)},
                             .padding = {8,8,8,8},
                             .childGap = 8,
                             .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}},
                  .backgroundColor = SURFACE_HIGHLIGHT_COLOR})
            {
                CLAY({.id = CLAY_ID("Header"),
                      .layout = {.sizing = {.width = CLAY_SIZING_FIXED(60), .height = CLAY_SIZING_FIXED(60)}},
                      .image = {.imageData = &aoc_icon}}) {}
                CLAY_TEXT(CLAY_STRING("Advent of Code 2025"), CLAY_TEXT_CONFIG({.fontSize = 24,
                                                                                .textColor = PRIMARY_TEXT_COLOR,
                                                                                .textAlignment = CLAY_TEXT_ALIGN_CENTER}));
            }
            CLAY({.id = CLAY_ID("DaysContainer"),
                  .layout = {.childGap = 2,
                             .sizing = {.width = CLAY_SIZING_GROW(0)},
                             .layoutDirection = CLAY_TOP_TO_BOTTOM},
                  .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()}})
            {
                for (usize i = 0; i < ARRAY_LENGTH(days); ++i) {
                    Clay_Color background_color = (i == active_day_index) ? PRIMARY_ACCENT_COLOR : SURFACE_HIGHLIGHT_COLOR;
                    Clay_Color text_color = (i == active_day_index) ? ACCENT_TEXT_COLOR : PRIMARY_TEXT_COLOR;
                    CLAY({.layout = {.padding = {40,40,20,20},
                                     .sizing = {.width = CLAY_SIZING_GROW()}},
                          .backgroundColor = Clay_Hovered() ? SECONDARY_ACCENT_COLOR : background_color})
                    {
                        Clay_OnHover(handle_day_button_interaction, (intptr_t)i);
                        Clay_String button_text = {.chars = days[i].day_button_title, .length = (s32)strlen(days[i].day_button_title)};
                        CLAY_TEXT(button_text, CLAY_TEXT_CONFIG({.fontSize = 24,
                                    .textColor = Clay_Hovered() ? ACCENT_TEXT_COLOR : text_color}));
                    }
                }
            }
        }

        CLAY({.id = CLAY_ID("RightPanel"),
              .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                         .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                         .childGap = 16}})
        {
            CLAY({.id = CLAY_ID("MainContent"),
                  .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                             .padding = {16,16,16,16},
                             .childGap = 16,
                             .sizing = {.width = CLAY_SIZING_GROW(0)}},
                  .backgroundColor = SURFACE_COLOR,
                  .clip = {.vertical = true,
                           .childOffset = Clay_GetScrollOffset()}})
            {
                CLAY({.id = CLAY_ID("PartSelector"),
                      .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                                 .sizing = {.width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_FIT()},
                                 .padding = {25, 25, 0, 0},
                                 .childGap = 25}})
                {
                    CLAY({.id = CLAY_ID("Part1Button"),
                          .layout = {.padding = {0, 0, 20, 20},
                                     .sizing = {.width = CLAY_SIZING_GROW()},
                                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER}},
                          .backgroundColor = Clay_Hovered() ? SECONDARY_ACCENT_COLOR
                                                            : ((active_problem_part == 1) ? PRIMARY_ACCENT_COLOR
                                                                                          : SURFACE_HIGHLIGHT_COLOR)})
                    {
                        Clay_OnHover(handle_part_button_interaction, 1);
                        CLAY_TEXT(CLAY_STRING("Part 1"), CLAY_TEXT_CONFIG({.fontSize = 24,
                                                                           .textColor =  Clay_Hovered() ? ACCENT_TEXT_COLOR :
                                                                                                        ((active_problem_part == 1) ? ACCENT_TEXT_COLOR
                                                                                                                                    : PRIMARY_TEXT_COLOR),
                                                                           .textAlignment = CLAY_TEXT_ALIGN_CENTER}));
                    }
                    CLAY({.id = CLAY_ID("Part2Button"),
                          .layout = {.padding = {0, 0, 20, 20},
                                     .sizing = {.width = CLAY_SIZING_GROW()},
                                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER}},
                          .backgroundColor = Clay_Hovered() ? SECONDARY_ACCENT_COLOR
                                                            : ((active_problem_part == 2) ? PRIMARY_ACCENT_COLOR
                                                                                          : SURFACE_HIGHLIGHT_COLOR)})
                    {
                        Clay_OnHover(handle_part_button_interaction, 2);
                        CLAY_TEXT(CLAY_STRING("Part 2"), CLAY_TEXT_CONFIG({.fontSize = 24,
                                                                           .textColor =  Clay_Hovered() ? ACCENT_TEXT_COLOR
                                                                                                        : ((active_problem_part == 1) ? ACCENT_TEXT_COLOR
                                                                                                                                      : PRIMARY_TEXT_COLOR),
                                                                           .textAlignment = CLAY_TEXT_ALIGN_CENTER}));
                    }
                }

                CLAY({.id = CLAY_ID("DayOuter"),
                      .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                                 .sizing = {.height = CLAY_SIZING_GROW(), .width = CLAY_SIZING_FIXED(1000)},
                                 .childGap = 30}})
                {
                    usize i = active_day_index;
                    CLAY_TEXT(days[i].title, CLAY_TEXT_CONFIG({.fontSize = 48,
                                                               .lineHeight = 60,
                                                               .textColor = PRIMARY_TEXT_COLOR,
                                                               .textAlignment = CLAY_TEXT_ALIGN_CENTER}));
                    CLAY({.id = CLAY_ID("DayDescription"),
                          .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                                     .sizing = {.height = CLAY_SIZING_GROW()}}})
                    {
                        Clay_String desc = (active_problem_part == 1) ? days[i].part1_description : days[i].part2_description;
                        CLAY_TEXT(desc, CLAY_TEXT_CONFIG({.fontSize = 24,
                                                          .textColor = SECONDARY_TEXT_COLOR,
                                                          .textAlignment = CLAY_TEXT_ALIGN_LEFT}));
                    }
                    CLAY({.id = CLAY_ID("RunSection"),
                          .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                                     .sizing = {.width = CLAY_SIZING_GROW()},
                                     .childGap = 50}})
                    {
                        CLAY({.id = CLAY_ID("RunTestWrapper"),
                              .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                                         .sizing = {.width = CLAY_SIZING_FIXED(400)},
                                         .childGap = 10}})
                        {
                            CLAY({.id = CLAY_ID("RunTestButtonWrapper"),
                                  .backgroundColor = Clay_Hovered() ? SECONDARY_ACCENT_COLOR : PRIMARY_ACCENT_COLOR,
                                  .layout = {.padding = {50,50,10,10}}})
                            {
                                Clay_OnHover(handle_run_button_interaction, (intptr_t)true);
                                CLAY_TEXT(CLAY_STRING("Run test"), CLAY_TEXT_CONFIG({.fontSize = 28,
                                                                                     .textColor = ACCENT_TEXT_COLOR}));
                            }
                            CLAY({.id = CLAY_ID("RunTestResultWrapper"),
                                  .layout = {.padding = {20,0,10,10},
                                  .sizing = {.width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW(200)}},
                                  .backgroundColor = SURFACE_HIGHLIGHT_COLOR})
                            {
                                char *result_cstr = NULL;
                                if (active_problem_part == 1) {
                                    result_cstr = days[i].part1_test_result;
                                } else {
                                    result_cstr = days[i].part2_test_result;
                                }
                                if (result_cstr) {
                                    Clay_String result = {.chars = result_cstr,
                                                          .length = (s32)strlen(result_cstr)};
                                    CLAY_TEXT(result, CLAY_TEXT_CONFIG({.fontSize = 24,
                                                                        .textColor = PRIMARY_TEXT_COLOR,
                                                                        .textAlignment = CLAY_TEXT_ALIGN_CENTER}));
                                }
                            }
                        }
                        CLAY({.id = CLAY_ID("RunRealWrapper"),
                              .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                                         .sizing = {.width = CLAY_SIZING_FIXED(400)},
                                         .childGap = 10}})
                        {
                            CLAY({.id = CLAY_ID("RunRealButtonWrapper"),
                                  .backgroundColor = Clay_Hovered() ? SECONDARY_ACCENT_COLOR : PRIMARY_ACCENT_COLOR,
                                  .layout = {.padding = {50,50,10,10}}})
                            {
                                Clay_OnHover(handle_run_button_interaction, (intptr_t)false);
                                CLAY_TEXT(CLAY_STRING("Run real"), CLAY_TEXT_CONFIG({.fontSize = 28,
                                                                                     .textColor = ACCENT_TEXT_COLOR}));
                            }
                            CLAY({.id = CLAY_ID("RunRealResultWrapper"),
                                  .layout = {.padding = {20,0,10,10},
                                             .sizing = {.width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW(200)}},
                                  .backgroundColor = SURFACE_HIGHLIGHT_COLOR})
                            {
                                char *result_cstr = NULL;
                                if (active_problem_part == 1) {
                                    result_cstr = days[i].part1_real_result;
                                } else {
                                    result_cstr = days[i].part2_real_result;
                                }
                                if (result_cstr) {
                                    Clay_String result = {.chars = result_cstr,
                                                          .length = (s32)strlen(result_cstr)};
                                    CLAY_TEXT(result, CLAY_TEXT_CONFIG({.fontSize = 24,
                                                                        .textColor = PRIMARY_TEXT_COLOR,
                                                                        .textAlignment = CLAY_TEXT_ALIGN_CENTER}));
                                }
                            }
                        }
                    }
                }
            }
        }

        Clay_ScrollContainerData main_scroll_data = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("MainContent")));
        if (main_scroll_data.found) {
            CLAY({.id = CLAY_ID("ScrollBar"),
                  .floating = {.attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                               .offset = {.y = -(main_scroll_data.scrollPosition->y / main_scroll_data.contentDimensions.height) * main_scroll_data.scrollContainerDimensions.height},
                               .zIndex = 1,
                               .parentId = Clay_GetElementId(CLAY_STRING("MainContent")).id,
                               .attachPoints = {.element = CLAY_ATTACH_POINT_RIGHT_TOP, .parent = CLAY_ATTACH_POINT_RIGHT_TOP}}})
            {
                CLAY({.id = CLAY_ID("ScrollBarButton"),
                      .layout = {.sizing = {CLAY_SIZING_FIXED(12), CLAY_SIZING_FIXED((main_scroll_data.scrollContainerDimensions.height / main_scroll_data.contentDimensions.height) * main_scroll_data.scrollContainerDimensions.height)}},
                      .backgroundColor = SURFACE_HIGHLIGHT_COLOR,
                      .cornerRadius = CLAY_CORNER_RADIUS(6)}) {}
            }
        }
    }

    return Clay_EndLayout();
}

typedef struct
{
    Clay_Vector2 click_origin;
    Clay_Vector2 position_origin;
    bool mouse_down;
} ScrollbarData;

ScrollbarData scrollbar_data = {0};

bool debug_enabled = false;

static void
update_draw_frame(Font* fonts)
{
    Vector2 mouse_wheel_delta = GetMouseWheelMoveV();
    float   mouse_wheel_x     = mouse_wheel_delta.x;
    float   mouse_wheel_y     = mouse_wheel_delta.y;

#ifndef NDEBUG
    if (IsKeyPressed(KEY_D)) {
        debug_enabled = !debug_enabled;
        Clay_SetDebugModeEnabled(debug_enabled);
    }
#endif

    Clay_Vector2 mouse_position = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(GetMousePosition());
    Clay_SetPointerState(mouse_position, IsMouseButtonDown(0) && !scrollbar_data.mouse_down);
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float)GetScreenWidth(), (float)GetScreenHeight() });
    if (!IsMouseButtonDown(0)) {
        scrollbar_data.mouse_down = false;
    }

    if (IsMouseButtonDown(0) && !scrollbar_data.mouse_down && Clay_PointerOver(Clay__HashString(CLAY_STRING("ScrollBar"), 0, 0))) {
        Clay_ScrollContainerData scroll_container_data = Clay_GetScrollContainerData(Clay__HashString(CLAY_STRING("MainContent"), 0, 0));
        scrollbar_data.click_origin                    = mouse_position;
        scrollbar_data.position_origin                 = *scroll_container_data.scrollPosition;
        scrollbar_data.mouse_down                      = true;
    } else if (scrollbar_data.mouse_down) {
        Clay_ScrollContainerData scroll_container_data = Clay_GetScrollContainerData(Clay__HashString(CLAY_STRING("MainContent"), 0, 0));
        if (scroll_container_data.contentDimensions.height > 0) {
            Clay_Vector2 ratio = (Clay_Vector2){scroll_container_data.contentDimensions.width / scroll_container_data.scrollContainerDimensions.width,
                                                scroll_container_data.contentDimensions.height / scroll_container_data.scrollContainerDimensions.height};
            if (scroll_container_data.config.vertical) {
                scroll_container_data.scrollPosition->y = scrollbar_data.position_origin.y + (scrollbar_data.click_origin.y - mouse_position.y) * ratio.y;
            }
            if (scroll_container_data.config.horizontal) {
                scroll_container_data.scrollPosition->x = scrollbar_data.position_origin.x + (scrollbar_data.click_origin.x - mouse_position.x) * ratio.x;
            }
        }
    }

    Clay_UpdateScrollContainers(true, (Clay_Vector2) {mouse_wheel_x, mouse_wheel_y}, GetFrameTime());
    Clay_RenderCommandArray render_commands = CreateLayout();
    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(render_commands, fonts);
    EndDrawing();
}

bool reinitialize_clay = false;
static Font fonts[2];

static void
handle_clay_errors(Clay_ErrorData errorData)
{
    printf("%s", errorData.errorText.chars);
    if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
        reinitialize_clay = true;
        Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
    } else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
        reinitialize_clay = true;
        Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
    }
}


int
aoc2025_entry(void)
{
    for (usize i = 0; i < ARRAY_LENGTH(days); ++i) {
        char *title               = read_entire_file(days[i].title_file);
        days[i].title             = (Clay_String){.chars = title, .length = (s32)strlen(title)};
        char *part1_description   = read_entire_file(days[i].part1_description_file);
        days[i].part1_description = (Clay_String){.chars = part1_description, .length = (s32)strlen(part1_description)};
        char *part1_test_data     = read_entire_file(days[i].part1_test_data_file);
        days[i].part1_test_data   = (Clay_String){.chars = part1_test_data, .length = (s32)strlen(part1_test_data)};
        char *part1_real_data     = read_entire_file(days[i].part1_real_data_file);
        days[i].part1_real_data   = (Clay_String){.chars = part1_real_data, .length = (s32)strlen(part1_real_data)};
        char *part2_description   = read_entire_file(days[i].part2_description_file);
        days[i].part2_description = (Clay_String){.chars = part2_description, .length = (s32)strlen(part2_description)};
        char *part2_test_data     = read_entire_file(days[i].part2_test_data_file);
        days[i].part2_test_data   = (Clay_String){.chars = part2_test_data, .length = (s32)strlen(part2_test_data)};
        char *part2_real_data     = read_entire_file(days[i].part2_real_data_file);
        days[i].part2_real_data   = (Clay_String){.chars = part2_real_data, .length = (s32)strlen(part2_real_data)};
    }


    u64 total_memory_size = Clay_MinMemorySize();
    Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(total_memory_size, malloc(total_memory_size));
    Clay_Initialize(clay_memory,
                    (Clay_Dimensions){(f32)GetScreenWidth(), (f32)GetScreenHeight()},
                    (Clay_ErrorHandler){handle_clay_errors, 0});
    Clay_Raylib_Initialize(1920, 1080, "Advent of Code 2025",
                           FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    aoc_icon = LoadTexture("data/aoc_icon.png");
    set_native_window_icon();

    fonts[FONT_ID_BODY_24] = LoadFontEx("data/Roboto-Regular.ttf", 48, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY_24].texture, TEXTURE_FILTER_BILINEAR);
    fonts[FONT_ID_BODY_16] = LoadFontEx("data/Roboto-Regular.ttf", 32, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);


    // Main loop
    while (!WindowShouldClose()) {
        if (reinitialize_clay) {
            Clay_SetMaxElementCount(8192);
            total_memory_size = Clay_MinMemorySize();
            clay_memory = Clay_CreateArenaWithCapacityAndMemory(total_memory_size, malloc(total_memory_size));
            Clay_Initialize(clay_memory, (Clay_Dimensions) { (float)GetScreenWidth(), (float)GetScreenHeight() }, (Clay_ErrorHandler) { handle_clay_errors, 0 });
            reinitialize_clay = false;
        }
        update_draw_frame(fonts);
    }
    Clay_Raylib_Close();



    for (usize i = 0; i < ARRAY_LENGTH(days); ++i) {
        free((char *)days[i].title.chars);
        free((char *)days[i].part1_description.chars);
        free((char *)days[i].part1_test_data.chars);
        free((char *)days[i].part1_real_data.chars);
        free(days[i].part1_test_result);
        free(days[i].part1_real_result);
        free((char *)days[i].part2_description.chars);
        free((char *)days[i].part2_test_data.chars);
        free((char *)days[i].part2_real_data.chars);
        free(days[i].part2_test_result);
        free(days[i].part2_real_result);

    }

    return 0;
}
