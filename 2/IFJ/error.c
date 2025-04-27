/**
 * @file error.c
 *
 * Error handling functions.
 *
 * IFJ Project 2024, Team 'xstepa77'
 *
 * @author <xlitvi02> Gleb Litvinchuk
 * @author <xstepa77> Pavel Stepanov
 * @author <xkovin00> Viktoriia Kovina
 * @author <xshmon00> Gleb Shmonin
 */

#include "error.h"
#include "utils.h"

/**
 * Print an error message and exit the program with the given error code.
 */
void error_exit(int error_code, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "ERROR %i: ", error_code);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    cleanup_pointers_storage();
    exit(error_code);
}
