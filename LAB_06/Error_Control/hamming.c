#include <stdio.h>
#include <string.h>

#define MAX 512

static int is_power2(int x) { return x && !(x & (x - 1)); }

int main(void) {
    char data[MAX], received[MAX];
    printf("============================================================\n");
    printf("                    HAMMING CODE\n");
    printf("============================================================\n");
    printf("Enter Data Bits: ");
    scanf("%511s", data);

    int m = strlen(data), r = 0;
    while ((1 << r) < m + r + 1) r++;

    printf("\nCALCULATION\n------------------------------------------------------------\n");
    printf("Data bits (m) = %d\n", m);
    printf("Find r such that 2^r >= m + r + 1\n", r);
    for (int x = 1; x <= r; x++)
        printf("r=%d: 2^%d=%d, m+r+1=%d -> %s\n",
               x, x, 1<<x, m+x, ((1<<x) >= m+x+1) ? "VALID" : "not enough");
    printf("Parity bits required = %d\n", r);

    int n = m + r;
    int code[MAX] = {0};
    int di = 0;

    /* Positions are counted from right to left: position 1 is the rightmost bit. */
    for (int pos = 1; pos <= n; pos++) {
        if (!is_power2(pos)) {
            code[pos] = data[m - 1 - di] - '0';
            di++;
        }
    }

    for (int p = 1; p <= n; p <<= 1) {
        int parity = 0;
        for (int pos = 1; pos <= n; pos++)
            if ((pos & p) && pos != p) parity ^= code[pos];
        code[p] = parity;
    }

    printf("\nPOSITION (right-to-left numbering)\n");
    for (int pos = n; pos >= 1; pos--) printf("%d ", pos);
    printf("\nCODE\n");
    for (int pos = n; pos >= 1; pos--) printf("%d ", code[pos]);
    printf("\n");

    char generated[MAX];
    for (int i = 0; i < n; i++) generated[i] = code[n-i] + '0';
    generated[n] = '\0';
    printf("Generated Hamming Code = %s\n", generated);

    printf("\nEnter Received Code (same length): ");
    scanf("%511s", received);
    if ((int)strlen(received) != n) {
        printf("Invalid received code length.\n");
        return 1;
    }

    int recv[MAX];
    for (int i = 1; i <= n; i++)
        recv[i] = received[n-i] - '0';

    int syndrome = 0;
    printf("\nSYNDROME CALCULATION\n------------------------------------------------------------\n");
    for (int p = 1; p <= n; p <<= 1) {
        int parity = 0;
        for (int pos = 1; pos <= n; pos++)
            if (pos & p) parity ^= recv[pos];
        printf("S%d = %d\n", p, parity);
        if (parity) syndrome += p;
    }

    printf("Syndrome = %d\n", syndrome);

    if (syndrome == 0) {
        printf("RESULT: No single-bit error detected.\n");
    } else if (syndrome <= n) {
        printf("Error Position = %d (counted from right)\n", syndrome);
        recv[syndrome] ^= 1;

        char corrected[MAX];
        for (int i = 0; i < n; i++) corrected[i] = recv[n-i] + '0';
        corrected[n] = '\0';

        printf("Corrected Code = %s\n", corrected);
    } else {
        printf("Syndrome is outside the code length; unable to correct.\n");
    }

    printf("============================================================\n");
    return 0;
}

