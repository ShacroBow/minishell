#include <stdio.h>
#include <stdlib.h>

void filter_content(const char *input_file, const char *output_file) {
    FILE *infile = fopen(input_file, "r");
    FILE *outfile = fopen(output_file, "w");
    
    if (infile == NULL) {
        perror("Error opening input file");
        return;
    }
    if (outfile == NULL) {
        perror("Error opening output file");
        fclose(infile);
        return;
    }

    int ch;
    int inside_braces = 0;

    while ((ch = fgetc(infile)) != EOF) {
        if (ch == '{') {
            inside_braces = 1; // Start capturing content
            fputc(ch, outfile); // Write the opening brace to output
        } else if (ch == '}') {
            inside_braces = 0; // Stop capturing content
            fputc(ch, outfile); // Write the closing brace to output
        } else if (inside_braces) {
            fputc(ch, outfile); // Write content inside braces to output
        }
    }

    fclose(infile);
    fclose(outfile);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    filter_content(argv[1], argv[2]);

    return EXIT_SUCCESS;
}
