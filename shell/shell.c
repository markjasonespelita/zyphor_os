// zyshell - Zyphor OS Shell
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <glob.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define MAX_ARGS    64
#define MAX_HISTORY 100

// ── History ───────────────────────────────────────────────────────────────────
static char  *history[MAX_HISTORY];
static int    history_len = 0;

void history_add(const char *line)
{
    if (line[0] == '\0') return;
    if (history_len > 0 && strcmp(history[history_len - 1], line) == 0) return;

    if (history_len == MAX_HISTORY) {
        free(history[0]);
        memmove(history, history + 1, (MAX_HISTORY - 1) * sizeof(char *));
        history_len--;
    }
    history[history_len++] = strdup(line);
}

// ── Raw-mode helpers ──────────────────────────────────────────────────────────
static struct termios orig_termios;

void raw_enable(void)
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void raw_disable(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Returns the visible (non-ANSI) length of a string.
int visible_len(const char *s)
{
    int len = 0;
    while (*s) {
        if (*s == '\033') {
            s++;
            if (*s == '[') {
                s++;
                while (*s && !(*s >= 'A' && *s <= 'Z') && !(*s >= 'a' && *s <= 'z'))
                    s++;
                if (*s) s++;
            }
        } else {
            len++;
            s++;
        }
    }
    return len;
}

// Returns the current terminal width (columns), defaulting to 80.
int term_width(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;
    return 80;
}

// Tracks the maximum height reached during drawing loops to guarantee precise vertical rewinds
static int last_drawn_rows = 0;

// Redraw the current input line, handling multi-line wrapping correctly.
void redraw_line(const char *prompt_str, const char *buf)
{
    int cols       = term_width();
    int prompt_vis = visible_len(prompt_str);
    int buf_len    = (int)strlen(buf);
    int total_len  = prompt_vis + buf_len;

    if (last_drawn_rows > 0) {
        printf("\033[%dA", last_drawn_rows);
    }

    printf("\r\033[J");
    printf("%s%s", prompt_str, buf);

    last_drawn_rows = total_len / cols;

    if (total_len > 0 && total_len % cols == 0) {
        printf(" \b");
    }
    fflush(stdout);
}

// Reposition the terminal cursor to the logical cursor offset within the input.
void reposition_cursor(const char *prompt_str, int cursor, int buf_len)
{
    int cols       = term_width();
    int prompt_vis = visible_len(prompt_str);

    int total_len  = prompt_vis + buf_len;
    int target_pos = prompt_vis + cursor;

    int total_rows = total_len / cols;
    int target_row = target_pos / cols;
    int target_col = target_pos % cols;

    int move_up = total_rows - target_row;
    if (move_up > 0) {
        printf("\033[%dA", move_up);
    }

    printf("\r");
    if (target_col > 0) {
        printf("\033[%dC", target_col);
    }
    fflush(stdout);
}

// ── Git branch helper ─────────────────────────────────────────────────────────
int get_git_branch(char *branch, size_t bsize)
{
    char path[1035];
    char cwd[1024];

    if (!getcwd(cwd, sizeof(cwd)))
        return 0;

    char dir[1024];
    strncpy(dir, cwd, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';

    while (1) {
        snprintf(path, sizeof(path), "%s/.git/HEAD", dir);

        FILE *f = fopen(path, "r");
        if (f) {
            char line[256];
            if (fgets(line, sizeof(line), f)) {
                line[strcspn(line, "\n")] = '\0';

                const char *prefix = "ref: refs/heads/";
                if (strncmp(line, prefix, strlen(prefix)) == 0) {
                    strncpy(branch, line + strlen(prefix), bsize - 1);
                    branch[bsize - 1] = '\0';
                } else {
                    strncpy(branch, line, 8);
                    branch[8] = '\0';
                }
            }
            fclose(f);
            return 1;
        }

        char *slash = strrchr(dir, '/');
        if (!slash || slash == dir)
            break;
        *slash = '\0';
    }

    return 0;
}

// ── Prompt builder ────────────────────────────────────────────────────────────
char *build_prompt(void)
{
    char  hostname[256];
    char  cwd[1024];
    char *username;
    char  branch[256];
    char  branch_part[300] = "";
    char *buf;

    username = getenv("USER");
    if (!username) username = "unknown";
    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));

    char *home = getenv("HOME");
    char display_cwd[1024];
    if (home && strncmp(cwd, home, strlen(home)) == 0
             && (cwd[strlen(home)] == '/' || cwd[strlen(home)] == '\0')) {
        snprintf(display_cwd, sizeof(display_cwd), "~%s", cwd + strlen(home));
    } else {
        strncpy(display_cwd, cwd, sizeof(display_cwd) - 1);
        display_cwd[sizeof(display_cwd) - 1] = '\0';
    }

    if (get_git_branch(branch, sizeof(branch))) {
        snprintf(branch_part, sizeof(branch_part),
            " \033[1;33m(\033[1;36m%s\033[1;33m)\033[0m", branch);
    }

    size_t len = 128 + strlen(username) + strlen(hostname) + strlen(display_cwd) + strlen(branch_part);
    buf = malloc(len);
    snprintf(buf, len,
        "\033[1;35m%s\033[0m@\033[1;36m%s\033[0m:\033[1;34m%s\033[0m%s$ ",
        username, hostname, display_cwd, branch_part);
    return buf;
}

// ── Line reader with arrow-key history ───────────────────────────────────────
int read_line(char *out)
{
    char  buf[BUFFER_SIZE] = {0};
    int   len    = 0;
    int   cursor = 0;
    int   hidx   = history_len;
    char  saved[BUFFER_SIZE] = {0};

    char *prompt = build_prompt();

    last_drawn_rows = 0;

    fputs(prompt, stdout);
    fflush(stdout);

    raw_enable();

    while (1) {
        unsigned char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) {
            raw_disable();
            free(prompt);
            return -1;
        }

        if (c == '\r' || c == '\n') {
            putchar('\n');
            break;

        } else if (c == 127 || c == '\b') {
            if (cursor > 0) {
                memmove(&buf[cursor - 1], &buf[cursor], len - cursor);
                len--;
                cursor--;
                buf[len] = '\0';
                redraw_line(prompt, buf);
                reposition_cursor(prompt, cursor, len);
            }

        } else if (c == '\x1b') {
            unsigned char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) <= 0) continue;
            if (read(STDIN_FILENO, &seq[1], 1) <= 0) continue;
            if (seq[0] != '[') continue;

            if (seq[1] == 'A') {
                // ↑
                if (hidx == history_len)
                    strncpy(saved, buf, BUFFER_SIZE - 1);
                if (hidx > 0) {
                    hidx--;
                    strncpy(buf, history[hidx], BUFFER_SIZE - 1);
                    buf[BUFFER_SIZE - 1] = '\0';
                    len = cursor = strlen(buf);
                    redraw_line(prompt, buf);
                    reposition_cursor(prompt, cursor, len);
                }

            } else if (seq[1] == 'B') {
                // ↓
                if (hidx < history_len) {
                    hidx++;
                    const char *src = (hidx == history_len) ? saved : history[hidx];
                    strncpy(buf, src, BUFFER_SIZE - 1);
                    buf[BUFFER_SIZE - 1] = '\0';
                    len = cursor = strlen(buf);
                    redraw_line(prompt, buf);
                    reposition_cursor(prompt, cursor, len);
                }

            } else if (seq[1] == 'C') {
                // →
                if (cursor < len) {
                    cursor++;
                    redraw_line(prompt, buf);
                    reposition_cursor(prompt, cursor, len);
                }

            } else if (seq[1] == 'D') {
                // ←
                if (cursor > 0) {
                    cursor--;
                    redraw_line(prompt, buf);
                    reposition_cursor(prompt, cursor, len);
                }
            }

        } else if (c >= 32 && c < 127) {
            if (len < BUFFER_SIZE - 1) {
                memmove(&buf[cursor + 1], &buf[cursor], len - cursor);
                buf[cursor++] = c;
                len++;
                buf[len] = '\0';
                redraw_line(prompt, buf);
                reposition_cursor(prompt, cursor, len);
            }
        }
    }

    raw_disable();
    free(prompt);
    strncpy(out, buf, BUFFER_SIZE - 1);
    out[BUFFER_SIZE - 1] = '\0';
    return 0;
}

// ── Command helpers ───────────────────────────────────────────────────────────

// Expands a leading ~/ or bare ~ in a token to the home directory.
char *expand_tilde(const char *token)
{
    const char *home = getenv("HOME");
    if (!home) home = "/";

    if (token[0] == '~' && (token[1] == '/' || token[1] == '\0')) {
        size_t len = strlen(home) + strlen(token + 1) + 1;
        char *expanded = malloc(len);
        snprintf(expanded, len, "%s%s", home, token + 1);
        return expanded;
    }
    return strdup(token);
}

static char *expanded_args[MAX_ARGS];
static int   expanded_count = 0;

void free_expanded(void)
{
    for (int j = 0; j < expanded_count; j++) {
        free(expanded_args[j]);
        expanded_args[j] = NULL;
    }
    expanded_count = 0;
}

// Quote-aware argument parsing to prevent splitting quoted strings.
void parse_command(char *input, char *args[])
{
    free_expanded();

    int i = 0;
    char *p = input;

    while (*p != '\0' && i < MAX_ARGS - 1) {
        while (*p == ' ' || *p == '\t' || *p == '\n')
            p++;
        if (*p == '\0') break;

        char *token_start;
        if (*p == '"') {
            p++;
            token_start = p;
            while (*p != '\0' && *p != '"')
                p++;
            if (*p == '"') { *p = '\0'; p++; }
        } else {
            token_start = p;
            while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '\n')
                p++;
            if (*p != '\0') { *p = '\0'; p++; }
        }

        char *expanded = expand_tilde(token_start);
        glob_t g;
        int ret = glob(expanded, GLOB_NOCHECK | GLOB_TILDE, NULL, &g);
        free(expanded);

        if (ret == 0) {
            for (size_t k = 0; k < g.gl_pathc && i < MAX_ARGS - 1; k++) {
                expanded_args[i] = strdup(g.gl_pathv[k]);
                args[i] = expanded_args[i];
                i++;
                expanded_count++;
            }
            globfree(&g);
        }
    }

    args[i] = NULL;
}

// ── I/O Redirection ───────────────────────────────────────────────────────────
typedef struct {
    char *in_file;
    char *out_file;
    int   append;
} Redirection;

void free_redirection(Redirection *r)
{
    free(r->in_file);
    free(r->out_file);
    r->in_file  = NULL;
    r->out_file = NULL;
}

void parse_redirection(const char *segment, Redirection *r, char *clean)
{
    r->in_file  = NULL;
    r->out_file = NULL;
    r->append   = 0;

    char tmp[BUFFER_SIZE];
    strncpy(tmp, segment, BUFFER_SIZE - 1);
    tmp[BUFFER_SIZE - 1] = '\0';

    clean[0] = '\0';

    char *tok = strtok(tmp, " \t");
    while (tok != NULL) {
        if (strcmp(tok, ">>") == 0) {
            tok = strtok(NULL, " \t");
            if (tok) { free(r->out_file); r->out_file = strdup(tok); r->append = 1; }

        } else if (strcmp(tok, ">") == 0) {
            tok = strtok(NULL, " \t");
            if (tok) { free(r->out_file); r->out_file = strdup(tok); r->append = 0; }

        } else if (strcmp(tok, "<") == 0) {
            tok = strtok(NULL, " \t");
            if (tok) { free(r->in_file); r->in_file = strdup(tok); }

        } else {
            char *dbl = strstr(tok, ">>");
            char *sng = strchr(tok, '>');
            char *inp = strchr(tok, '<');

            char *op     = NULL;
            int   optype = 0;

            if (dbl)                                          { op = dbl; optype = 1; }
            if (sng && sng != dbl && (!op || sng < op))      { op = sng; optype = 2; }
            if (inp && (!op || inp < op))                     { op = inp; optype = 3; }

            if (op) {
                if (op > tok) {
                    char word[BUFFER_SIZE];
                    size_t wlen = op - tok;
                    strncpy(word, tok, wlen);
                    word[wlen] = '\0';
                    if (clean[0]) strncat(clean, " ", BUFFER_SIZE - strlen(clean) - 1);
                    strncat(clean, word, BUFFER_SIZE - strlen(clean) - 1);
                }
                int oplen = (optype == 1) ? 2 : 1;
                char *filename = op + oplen;
                if (*filename == '\0')
                    filename = strtok(NULL, " \t");
                if (filename && *filename != '\0') {
                    if      (optype == 1) { free(r->out_file); r->out_file = strdup(filename); r->append = 1; }
                    else if (optype == 2) { free(r->out_file); r->out_file = strdup(filename); r->append = 0; }
                    else                  { free(r->in_file);  r->in_file  = strdup(filename); }
                }
            } else {
                if (clean[0]) strncat(clean, " ", BUFFER_SIZE - strlen(clean) - 1);
                strncat(clean, tok, BUFFER_SIZE - strlen(clean) - 1);
            }
        }
        tok = strtok(NULL, " \t");
    }
}

int apply_redirection(const Redirection *r)
{
    if (r->in_file) {
        int fd = open(r->in_file, O_RDONLY);
        if (fd < 0) { perror("zyshell: open"); return -1; }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (r->out_file) {
        int flags = O_WRONLY | O_CREAT | (r->append ? O_APPEND : O_TRUNC);
        int fd = open(r->out_file, flags, 0644);
        if (fd < 0) { perror("zyshell: open"); return -1; }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    return 0;
}

// ── ls color injection ────────────────────────────────────────────────────────
// Rewrites args in-place to insert --color=always after "ls".
// colored must be at least MAX_ARGS + 1 in size.
void inject_ls_color(char *args[], char *colored[])
{
    colored[0] = "ls";
    colored[1] = "--color=always";
    int j = 2;
    for (int k = 1; args[k] != NULL && j < MAX_ARGS; k++, j++)
        colored[j] = args[k];
    colored[j] = NULL;
}

// ── OS screen ─────────────────────────────────────────────────────────────────
void show_os_screen(void)
{
    char pretty_name[256] = "Zyphor OS";
    char version_id[64]   = "unknown";

    FILE *f = fopen("/etc/os-release", "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\n")] = '\0';
            if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
                char *val = line + 12;
                if (val[0] == '"') { val++; val[strcspn(val, "\"")] = '\0'; }
                strncpy(pretty_name, val, sizeof(pretty_name) - 1);
            }
            if (strncmp(line, "VERSION_ID=", 11) == 0) {
                char *val = line + 11;
                if (val[0] == '"') { val++; val[strcspn(val, "\"")] = '\0'; }
                strncpy(version_id, val, sizeof(version_id) - 1);
            }
        }
        fclose(f);
    }

    printf("\n\033[1m%s\033[0m  \033[2m%s\033[0m\n", pretty_name, "(ZyShell)");
}

// ── Command execution ─────────────────────────────────────────────────────────
int execute_command_redir(char *args[], const Redirection *r)
{
    if (args[0] == NULL)
        return 0;

    if (strcmp(args[0], "exit") == 0)
        return -2;

    if (strcmp(args[0], "cd") == 0) {
        const char *dir = args[1];
        if (dir == NULL) {
            dir = getenv("HOME");
            if (dir == NULL) dir = "/";
        }
        if (chdir(dir) != 0) {
            perror("zyshell");
            return 1;
        }
        return 0;
    }

    // Resolve the actual argv to execute (inject --color=always for ls).
    char *exec_args[MAX_ARGS + 1];
    char **argv = args;
    if (strcmp(args[0], "ls") == 0) {
        inject_ls_color(args, exec_args);
        argv = exec_args;
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (apply_redirection(r) < 0) _exit(1);
        execvp(argv[0], argv);
        perror("zyshell");
        _exit(127);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    } else {
        perror("fork");
        return -1;
    }
}

// ── Pipe execution ────────────────────────────────────────────────────────────
int execute_pipeline(const char *segment)
{
    char buf[BUFFER_SIZE];
    strncpy(buf, segment, BUFFER_SIZE - 1);
    buf[BUFFER_SIZE - 1] = '\0';

    char *stages[MAX_ARGS];
    int   nstages = 0;

    char *tok = strtok(buf, "|");
    while (tok != NULL && nstages < MAX_ARGS - 1) {
        while (*tok == ' ' || *tok == '\t') tok++;
        char *end = tok + strlen(tok) - 1;
        while (end >= tok && (*end == ' ' || *end == '\t')) *end-- = '\0';
        if (*tok != '\0')
            stages[nstages++] = tok;
        tok = strtok(NULL, "|");
    }

    if (nstages <= 1) {
        if (nstages == 0) {
            char *args[MAX_ARGS]; args[0] = NULL;
            Redirection r = {NULL, NULL, 0};
            return execute_command_redir(args, &r);
        }

        Redirection r;
        char clean[BUFFER_SIZE];
        parse_redirection(stages[0], &r, clean);
        char *args[MAX_ARGS];
        parse_command(clean, args);
        int status = execute_command_redir(args, &r);
        free_redirection(&r);
        return status;
    }

    int   prev_fd  = -1;
    pid_t last_pid = -1;

    for (int i = 0; i < nstages; i++) {
        int pipefd[2] = {-1, -1};
        int is_last   = (i == nstages - 1);

        Redirection r;
        char clean[BUFFER_SIZE];
        parse_redirection(stages[i], &r, clean);

        char *args[MAX_ARGS];
        parse_command(clean, args);

        if (!is_last) {
            if (pipe(pipefd) < 0) {
                perror("zyshell: pipe");
                if (prev_fd != -1) close(prev_fd);
                free_redirection(&r);
                return 1;
            }
        }

        if (args[0] == NULL) {
            if (prev_fd != -1) close(prev_fd);
            if (!is_last) { close(pipefd[0]); close(pipefd[1]); }
            free_redirection(&r);
            continue;
        }

        // Resolve argv (inject --color=always for ls).
        char *exec_args[MAX_ARGS + 1];
        char **argv = args;
        if (strcmp(args[0], "ls") == 0) {
            inject_ls_color(args, exec_args);
            argv = exec_args;
        }

        pid_t pid = fork();
        if (pid == 0) {
            if (prev_fd != -1 && r.in_file == NULL) {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            } else if (prev_fd != -1) {
                close(prev_fd);
            }

            if (!is_last && r.out_file == NULL) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            } else if (!is_last) {
                close(pipefd[0]);
                close(pipefd[1]);
            }

            if (apply_redirection(&r) < 0) _exit(1);

            execvp(argv[0], argv);
            perror("zyshell");
            _exit(1);
        } else if (pid < 0) {
            perror("zyshell: fork");
        }

        if (is_last) last_pid = pid;

        if (prev_fd != -1) close(prev_fd);
        if (!is_last) {
            close(pipefd[1]);
            prev_fd = pipefd[0];
        }

        free_redirection(&r);
    }

    int last_status = 0;
    int status;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0) {
        if (wpid == last_pid)
            last_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }

    return last_status;
}

// ── Command chaining (&&, ||, ;) ──────────────────────────────────────────────
int run_chain(const char *input)
{
    char buf[BUFFER_SIZE];
    strncpy(buf, input, BUFFER_SIZE - 1);
    buf[BUFFER_SIZE - 1] = '\0';

    int   last_status = 0;
    char *p           = buf;

    while (*p != '\0') {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        char *and_op  = NULL;
        char *or_op   = NULL;
        char *semi_op = NULL;

        for (char *q = p; *q != '\0'; q++) {
            if (!and_op  && q[0] == '&' && q[1] == '&') and_op  = q;
            if (!or_op   && q[0] == '|' && q[1] == '|') or_op   = q;
            if (!semi_op && q[0] == ';')                 semi_op = q;
        }

        char *op      = NULL;
        int   op_type = 0;

        if (and_op)                             { op = and_op;  op_type = 1; }
        if (or_op   && (!op || or_op   < op))   { op = or_op;   op_type = 2; }
        if (semi_op && (!op || semi_op < op))   { op = semi_op; op_type = 3; }

        char segment[BUFFER_SIZE];
        if (op) {
            size_t seg_len = op - p;
            strncpy(segment, p, seg_len);
            segment[seg_len] = '\0';
            p = op + (op_type == 3 ? 1 : 2);
        } else {
            strncpy(segment, p, BUFFER_SIZE - 1);
            segment[BUFFER_SIZE - 1] = '\0';
            p += strlen(p);
        }

        char *end = segment + strlen(segment) - 1;
        while (end >= segment && (*end == ' ' || *end == '\t')) *end-- = '\0';

        if (segment[0] == '\0') continue;

        int status = execute_pipeline(segment);

        if (status == -2)
            return -2;

        last_status = status;

        if (op_type == 1 && last_status != 0) {
            while (*p != '\0' && *p != ';') p++;
            if (*p == ';') p++;
        } else if (op_type == 2 && last_status == 0) {
            while (*p != '\0' && *p != ';') p++;
            if (*p == ';') p++;
        }
    }

    return last_status;
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main(void)
{
    show_os_screen();

    char input[BUFFER_SIZE];

    while (1) {
        if (read_line(input) < 0)
            break;

        if (input[0] == '\0')
            continue;

        history_add(input);

        if (run_chain(input) == -2)
            break;
    }

    printf("Exiting zyshell...\n");
    return 0;
}