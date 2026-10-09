#ifndef BYTESTUFF_COMMON_H
#define BYTESTUFF_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9999
#define BUFFER_SIZE 1024

#define FLAG 'F'
#define ESC 'E'


/*
 * Check whether the input data is valid.
 *
 * For this practical, we allow printable
 * characters except newline.
 */
int is_valid_data(const char *data)
{
    if (data == NULL || data[0] == '\0')
        return 0;

    return 1;
}


/*
 * Perform Byte Stuffing.
 *
 * Rules:
 *
 * F -> EF
 * E -> EE
 * Other characters remain unchanged.
 */
void byte_stuff(const char *input, char *output)
{
    int i = 0;
    int j = 0;

    while (input[i] != '\0')
    {
        if (input[i] == FLAG)
        {
            output[j++] = ESC;
            output[j++] = FLAG;
        }
        else if (input[i] == ESC)
        {
            output[j++] = ESC;
            output[j++] = ESC;
        }
        else
        {
            output[j++] = input[i];
        }

        i++;
    }

    output[j] = '\0';
}


/*
 * Perform Byte Destuffing.
 *
 * Rules:
 *
 * EF -> F
 * EE -> E
 *
 * Returns:
 * 1 = successful
 * 0 = malformed stuffed data
 */
int byte_destuff(const char *input, char *output)
{
    int i = 0;
    int j = 0;

    while (input[i] != '\0')
    {
        /*
         * If ESC is found, the next character
         * must exist.
         */
        if (input[i] == ESC)
        {
            if (input[i + 1] == '\0')
            {
                return 0;
            }

            /*
             * Copy the character after ESC.
             *
             * EF -> F
             * EE -> E
             */
            output[j++] = input[i + 1];

            /*
             * Skip both ESC and the escaped character.
             */
            i += 2;
        }
        else
        {
            /*
             * Normal character.
             */
            output[j++] = input[i];
            i++;
        }
    }

    output[j] = '\0';

    return 1;
}

#endif
