#include "util.h"
#include "days.h"

#include "cap.h"

#include <stdio.h>

typedef void (*DayFunc)(void);

struct Cli {
    CapContext *ctx;
    int day;
} static cli;

static void
init_cli(int argc, char **argv)
{
    cli.ctx = cap_context_new();

    cap_set_program_description(cli.ctx, "Advent Of Code 2025");

    // Default to 0 -> all days
    cap_option_int(cli.ctx, &cli.day)
        ->long_name("day")
        ->description("Specify one day to run, value of 0 or less imply all.")
        ->metavar("number")
        ->done();

    int exit_code;
    if (cap_parse_and_handle(cli.ctx, argc, argv, &exit_code) == CAP_EXIT) {
        exit(exit_code);
    }
}

static void
deinit_cli()
{
    cap_context_free(cli.ctx);
}

int
main(int argc, char **argv)
{
    init_cli(argc, argv);

    static const DayFunc days[] = {
      day1
    };

    if (cli.day <= 0) cli.day = ARRAY_LENGTH(days);
    if (cli.day > ARRAY_LENGTH(days)) {
        fprintf(stderr, "Day %d is out of range, max is %zu\n", cli.day, ARRAY_LENGTH(days));
        return 1;
    }

    if (cli.day <= 0) {
        // Run all days
        for (size_t i = 0; i < ARRAY_LENGTH(days); ++i) {
            printf("=== Day %zu ===\n", i+1);
            days[i]();
            printf("\n\n");
        }
    } else {
        // Run specific day
        printf("=== Day %d ===\n", cli.day);
        days[(usize)cli.day-1]();
    }

    deinit_cli();

    return 0;
}
