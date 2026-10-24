#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/ref_cpu.h"
#include "lib/tester.h"

static void bad_init(size_t size, u8 *memory) { (void)size; (void)memory; }
static void bad_set_state(struct state *state) { (void)state; }
static int bad_step(void) { return 0; }
static void bad_get_state(struct state *state) {
    memset(state, 0, sizeof(*state));
    state->PC = 0xFFFF; /* Deliberately wrong, including for NOP. */
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    if (!strcmp(argv[1], "stop")) {
        for (unsigned padding = 0; padding < 256; ++padding) {
            u8 code[] = {0x10, (u8)padding, 0};
            struct state input = {0}, output = {0};
            rcpu_init(sizeof(code), code);
            rcpu_reset(&input);
            if (rcpu_step() < 0) return 1;
            rcpu_get_state(&output);
            if (output.PC != 2 || !output.halted) {
                fprintf(stderr, "normal STOP must consume its padding byte\n");
                return 1;
            }
        }
    } else if (!strcmp(argv[1], "failure-status")) {
        struct tester_flags flags = {0};
        struct tester_operations ops = {bad_init, bad_set_state, bad_get_state, bad_step};
        flags.keep_going_on_mismatch = true;
        flags.enable_cb_instruction_testing = true;
        if (tester_run(&flags, &ops) == 0) {
            fprintf(stderr, "keep-going mismatches must return failure\n");
            return 1;
        }
    } else {
        return 2;
    }
    return 0;
}
