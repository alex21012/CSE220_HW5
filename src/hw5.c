#include "hw5.h"

typedef struct {
    char search_text[MAX_SEARCH_LEN];
    char replace_text[MAX_SEARCH_LEN];
    int start_line;
    int end_line;
    char *input_file;
    char *output_file;
} CmdArgs;

int parse_arguments(int argc, char *argv[], CmdArgs *args) {
    if (argc < 7) return MISSING_ARGUMENT;

    memset(args, 0, sizeof(CmdArgs));
    args->start_line = 1;
    args->end_line = -1;

    bool s_flag = false;
    bool r_flag = false;
    bool l_flag = false;
    bool w_flag = false;
    for (int opt; (opt = getopt(argc, argv, "s:r:l:w")) != -1;) {
        switch (opt) {
            case 's': 
                if (s_flag) return DUPLICATE_ARGUMENT;
                s_flag = true;
                strncpy(args->search_text, optarg, MAX_SEARCH_LEN - 1);
                break;
            case 'r': 
                if (r_flag) return DUPLICATE_ARGUMENT;
                r_flag = true;
                strncpy(args->replace_text, optarg, MAX_SEARCH_LEN - 1);
                break;
            case 'l': 
                if (l_flag) return DUPLICATE_ARGUMENT;
                l_flag = true;
                if (sscanf(optarg, "%d,%d", &args->start_line, &args->end_line) != 2 || args->start_line > args->end_line)
                    return L_ARGUMENT_INVALID;
                break;
            case 'w': 
                if (w_flag) return DUPLICATE_ARGUMENT;
                w_flag = true;
                break;
            default: 
                return 0;
        }
    }

    if (!(s_flag && r_flag)) {
        return s_flag ? R_ARGUMENT_MISSING : S_ARGUMENT_MISSING;
    }
    args->input_file = argv[optind++];
    args->output_file = argv[optind];
    return 0;
}

void search_and_replace(FILE *in, FILE *out, const CmdArgs *args) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    for (int line_number = 1; (read = getline(&line, &len, in)) != -1; ++line_number) {
        if (line_number < args->start_line || (args->end_line != -1 && line_number > args->end_line)) {
            fputs(line, out);
            continue;
        }

        for (char *pos = line; *pos;) {
            char *found = strstr(pos, args->search_text);
            if (!found) {
                fputs(pos, out);
                break;
            }
            fwrite(pos, sizeof(char), found - pos, out);
            fputs(args->replace_text, out);
            pos = found + strlen(args->search_text);
        }
    }
    free(line);
}

int main(int argc, char *argv[]) {
    CmdArgs args;
    int parse_result = parse_arguments(argc, argv, &args);
    if (parse_result != 0) return parse_result;

    FILE *input_file = fopen(args.input_file, "r");
    if (!input_file) return INPUT_FILE_MISSING;

    FILE *output_file = fopen(args.output_file, "w");
    if (!output_file) {
        fclose(input_file);
        return OUTPUT_FILE_UNWRITABLE;
    }

    search_and_replace(input_file, output_file, &args);

    fclose(input_file);
    fclose(output_file);

    return 0;
}
