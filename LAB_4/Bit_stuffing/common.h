#ifndef BITSTUFF_COMMON_H
#define BITSTUFF_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9999
#define BUFFER_SIZE 1024
#define FLAG "01111110"

/*
 * Validates whether the input contains only
 * binary characters: 0 and 1.
 */
int is_binary(const char *data)
{
    if (data == NULL || data[0] == '\0')
        return 0;

    for (int i = 0; data[i] != '\0'; i++)
    {
        if (data[i] != '0' && data[i] != '1')
            return 0;
    }

    return 1;
}

/*
 * Performs bit stuffing.
 *
 * Rule:
 * Insert 0 after every five consecutive 1s.
 */
void bit_stuff(const char *input, char *output)
{
    int count = 0;
    int j = 0;

    for (int i = 0; input[i] != '\0'; i++)
    {
        output[j++] = input[i];

        if (input[i] == '1')
        {
            count++;

            if (count == 5)
            {
                output[j++] = '0';
                count = 0;
            }
        }
        else
        {
            count = 0;
        }
    }

    output[j] = '\0';
}

/*
 * Performs bit destuffing.
 *
 * Rule:
 * After five consecutive 1s, remove the
 * following stuffed 0.
 */
int bit_destuff(const char *input, char *output)
{
    int count = 0;
    int j = 0;

    for (int i = 0; input[i] != '\0'; i++)
    {
        if (input[i] != '0' && input[i] != '1')
            return 0;

        output[j++] = input[i];

        if (input[i] == '1')
        {
            count++;

            if (count == 5)
            {
                /*
                 * The next bit must be the
                 * stuffed zero.
                 */
                if (input[i + 1] != '0')
                    return 0;

                i++;
                count = 0;
            }
        }
        else
        {
            count = 0;
        }
    }

    output[j] = '\0';

    return 1;
}

#endif
