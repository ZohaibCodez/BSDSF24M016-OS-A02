/*
* Programming Assignment 02: ls
* Version 1.3.0: horizontal (across) display with -x
* Version 1.2.0: default output in columns, down then across
* Version 1.1.0: adds the long listing format (-l)
* Usage:
*       $ ls
*       $ ls -l
*       $ ls -l /home /etc
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/ioctl.h>

extern int errno;

/* display mode selected by the command-line options */
typedef enum
{
    MODE_DEFAULT,       /* columns, down then across */
    MODE_LONG,          /* -l */
    MODE_HORIZONTAL     /* -x */
} display_mode_t;

void do_ls(const char *dir, display_mode_t mode);
void display_horizontal(char **names, int count, int maxlen);
char **read_names(const char *dir, int *count, int *maxlen);
int get_terminal_width(void);
void display_columns(char **names, int count, int maxlen);
void do_ls_long(const char *dir);
void mode_to_string(mode_t mode, char *str);
void print_long_entry(const char *path, const char *name, const struct stat *st,
                      int w_link, int w_user, int w_group, int w_size);
int num_width(unsigned long n);

int main(int argc, char *argv[])
{
    display_mode_t mode = MODE_DEFAULT;
    int opt;

    while ((opt = getopt(argc, argv, "lx")) != -1)
    {
        switch (opt)
        {
        case 'l':
            mode = MODE_LONG;       /* the last option given wins */
            break;
        case 'x':
            mode = MODE_HORIZONTAL;
            break;
        default:
            fprintf(stderr, "Usage: %s [-l | -x] [dir...]\n", argv[0]);
            return 1;
        }
    }

    if (optind == argc)
    {
        do_ls(".", mode);
    }
    else
    {
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s : \n", argv[i]);
            do_ls(argv[i], mode);
            puts("");
        }
    }
    return 0;
}

/* Decide which display function to use. */
void do_ls(const char *dir, display_mode_t mode)
{
    if (mode == MODE_LONG)
    {
        do_ls_long(dir);
        return;
    }

    /* read all names first, then hand them to the chosen display function */
    int count = 0, maxlen = 0;
    char **names = read_names(dir, &count, &maxlen);
    if (names == NULL)
        return;

    if (mode == MODE_HORIZONTAL)
        display_horizontal(names, count, maxlen);
    else
        display_columns(names, count, maxlen);

    for (int i = 0; i < count; i++)
        free(names[i]);
    free(names);
}

/* Read all non-hidden entry names into a dynamically grown array.
   Returns NULL if the directory cannot be opened. */
char **read_names(const char *dir, int *count, int *maxlen)
{
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory : %s\n", dir);
        return NULL;
    }

    int capacity = 16;
    int n = 0;
    int longest = 0;
    char **names = malloc(capacity * sizeof(char *));
    if (names == NULL)
    {
        perror("malloc");
        closedir(dp);
        return NULL;
    }

    struct dirent *entry;
    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (n == capacity)
        {
            capacity *= 2;
            char **tmp = realloc(names, capacity * sizeof(char *));
            if (tmp == NULL)
            {
                perror("realloc");
                break;
            }
            names = tmp;
        }

        names[n] = strdup(entry->d_name);
        if (names[n] == NULL)
        {
            perror("strdup");
            break;
        }

        int len = (int)strlen(names[n]);
        if (len > longest)
            longest = len;
        n++;
    }
    if (errno != 0)
        perror("readdir failed");

    closedir(dp);
    *count = n;
    *maxlen = longest;
    return names;
}

/* Terminal width via ioctl(TIOCGWINSZ); falls back to 80 columns
   (e.g. when output is piped to a file or another program). */
int get_terminal_width(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
        return 80;
    return ws.ws_col;
}

/* "Across": print left to right, wrap when the next item would not fit. */
void display_horizontal(char **names, int count, int maxlen)
{
    int term_width = get_terminal_width();
    int col_width = maxlen + 2;
    int pos = 0;                                /* current horizontal position */

    for (int i = 0; i < count; i++)
    {
        /* wrap if this column would run past the right edge */
        if (pos > 0 && pos + col_width > term_width)
        {
            putchar('\n');
            pos = 0;
        }
        printf("%-*s", col_width, names[i]);
        pos += col_width;
    }
    if (count > 0)
        putchar('\n');
}

/* "Down then across": fill each column top to bottom, then move right. */
void display_columns(char **names, int count, int maxlen)
{
    if (count == 0)
        return;

    int term_width = get_terminal_width();
    int col_width = maxlen + 2;                 /* name + 2 spaces gap */

    int cols = term_width / col_width;
    if (cols < 1)
        cols = 1;
    if (cols > count)
        cols = count;

    int rows = (count + cols - 1) / cols;       /* ceiling division */

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int idx = r + c * rows;
            if (idx >= count)
                break;
            /* pad every item except the last one on the line */
            if (c == cols - 1 || idx + rows >= count)
                printf("%s", names[idx]);
            else
                printf("%-*s", col_width, names[idx]);
        }
        putchar('\n');
    }
}

/* Number of decimal digits in n (used for column alignment). */
int num_width(unsigned long n)
{
    int w = 1;
    while (n >= 10)
    {
        n /= 10;
        w++;
    }
    return w;
}

/* Convert st_mode into a string like "drwxr-xr-x" (str must hold 11 bytes). */
void mode_to_string(mode_t mode, char *str)
{
    /* file type: the S_IFMT mask isolates the type bits */
    if (S_ISREG(mode))       str[0] = '-';
    else if (S_ISDIR(mode))  str[0] = 'd';
    else if (S_ISLNK(mode))  str[0] = 'l';
    else if (S_ISCHR(mode))  str[0] = 'c';
    else if (S_ISBLK(mode))  str[0] = 'b';
    else if (S_ISFIFO(mode)) str[0] = 'p';
    else if (S_ISSOCK(mode)) str[0] = 's';
    else                     str[0] = '?';

    /* owner / group / other permission bits */
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';

    /* special bits replace the execute character */
    if (mode & S_ISUID)
        str[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID)
        str[6] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX)
        str[9] = (mode & S_IXOTH) ? 't' : 'T';
}

/* Print one line of the long listing. */
void print_long_entry(const char *path, const char *name, const struct stat *st,
                      int w_link, int w_user, int w_group, int w_size)
{
    char perms[11];
    mode_to_string(st->st_mode, perms);

    struct passwd *pw = getpwuid(st->st_uid);
    struct group *gr = getgrgid(st->st_gid);

    char userbuf[32], groupbuf[32];
    const char *user = pw ? pw->pw_name : (snprintf(userbuf, sizeof userbuf, "%u", st->st_uid), userbuf);
    const char *group = gr ? gr->gr_name : (snprintf(groupbuf, sizeof groupbuf, "%u", st->st_gid), groupbuf);

    /* ctime() gives "Wed Jun 30 21:49:08 1993\n"; skip the weekday and
       keep "Jun 30 21:49" (12 chars) */
    time_t mtime = st->st_mtime;
    char *t = ctime(&mtime);
    char timebuf[13] = "";
    if (t != NULL)
    {
        strncpy(timebuf, t + 4, 12);
        timebuf[12] = '\0';
    }

    printf("%s %*lu %-*s %-*s %*ld %s %s",
           perms,
           w_link, (unsigned long)st->st_nlink,
           w_user, user,
           w_group, group,
           w_size, (long)st->st_size,
           timebuf, name);

    /* for symbolic links show "name -> target" */
    if (S_ISLNK(st->st_mode))
    {
        char target[PATH_MAX];
        ssize_t n = readlink(path, target, sizeof(target) - 1);
        if (n >= 0)
        {
            target[n] = '\0';
            printf(" -> %s", target);
        }
    }
    putchar('\n');
}

/* Long listing: two passes so every column can be aligned. */
void do_ls_long(const char *dir)
{
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory : %s\n", dir);
        return;
    }

    struct dirent *entry;
    int w_link = 1, w_user = 1, w_group = 1, w_size = 1;
    long total_blocks = 0;
    char path[PATH_MAX];
    struct stat st;

    /* pass 1: find the widest value in each column and the block total */
    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        snprintf(path, sizeof path, "%s/%s", dir, entry->d_name);
        if (lstat(path, &st) == -1)
        {
            perror(path);
            continue;
        }
        total_blocks += st.st_blocks;

        int w = num_width(st.st_nlink);
        if (w > w_link) w_link = w;

        struct passwd *pw = getpwuid(st.st_uid);
        w = pw ? (int)strlen(pw->pw_name) : num_width(st.st_uid);
        if (w > w_user) w_user = w;

        struct group *gr = getgrgid(st.st_gid);
        w = gr ? (int)strlen(gr->gr_name) : num_width(st.st_gid);
        if (w > w_group) w_group = w;

        w = num_width((unsigned long)st.st_size);
        if (w > w_size) w_size = w;
    }

    /* st_blocks counts 512-byte blocks; ls prints 1K blocks */
    printf("total %ld\n", total_blocks / 2);

    /* pass 2: print */
    rewinddir(dp);
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        snprintf(path, sizeof path, "%s/%s", dir, entry->d_name);
        if (lstat(path, &st) == -1)
            continue;
        print_long_entry(path, entry->d_name, &st, w_link, w_user, w_group, w_size);
    }

    closedir(dp);
}
