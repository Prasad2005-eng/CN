#include <stdio.h>
#include <string.h>

#define MAX 1024

static int valid_bits(const char *s) {
    if (!*s) return 0;
    for (int i = 0; s[i]; i++)
        if (s[i] != '0' && s[i] != '1') return 0;
    return 1;
}

static void xor_division(char *work, const char *gen, int len, int glen) {
    for (int i = 0; i <= len - glen; i++) {
        if (work[i] == '1') {
            for (int j = 0; j < glen; j++)
                work[i+j] = (work[i+j] == gen[j]) ? '0' : '1';
        }
    }
}

int main(void) {
    char data[MAX], gen[MAX], work[MAX], recv[MAX];

    printf("============================================================\n");
    printf("                    CRC ERROR CONTROL\n");
    printf("============================================================\n");

    printf("Enter Data Bits: ");
    scanf("%1023s", data);
    printf("Enter Generator Bits: ");
    scanf("%1023s", gen);

    int dlen = strlen(data), glen = strlen(gen);
    if (!valid_bits(data) || !valid_bits(gen) || glen < 2 || gen[0] != '1') {
        printf("Invalid binary input/generator.\n");
        return 1;
    }

    strcpy(work, data);
    for (int i = 0; i < glen - 1; i++) strcat(work, "0");

    printf("\nCALCULATION\n------------------------------------------------------------\n");
    printf("Data length = %d\n", dlen);
    printf("Generator length = %d\n", glen);
    printf("Zeros appended = %d\n", glen - 1);
    printf("Extended Data = %s\n", work);

    xor_division(work, gen, strlen(work), glen);

    char crc[MAX], codeword[MAX];
    strcpy(crc, work + dlen);
    strcpy(codeword, data);
    strcat(codeword, crc);

    printf("CRC Remainder = %s\n", crc);
    printf("Transmitted Codeword = %s\n", codeword);

    printf("\nEnter Received Codeword to check (or enter transmitted codeword): ");
    scanf("%1023s", recv);

    if ((int)strlen(recv) != dlen + glen - 1 || !valid_bits(recv)) {
        printf("Invalid received codeword.\n");
        return 1;
    }

    char check[MAX];
    strcpy(check, recv);
    xor_division(check, gen, strlen(check), glen);

    int error = 0;
    for (int i = dlen; i < dlen + glen - 1; i++)
        if (check[i] == '1') error = 1;

    printf("\nCRC CHECK\n------------------------------------------------------------\n");
    printf("Remainder after division = %.*s\n", glen - 1, check + dlen);
    if (error)
        printf("RESULT: ERROR DETECTED\n");
    else
        printf("RESULT: NO ERROR DETECTED\n");

    printf("============================================================\n");
    return 0;
}

