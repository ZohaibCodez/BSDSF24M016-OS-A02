/*
* Programming Assignment 02: ls
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

extern int errno;

void do_ls(const char *dir, int long_flag);
void do_ls_long(const char *dir);
void mode_to_string(mode_t mode, char *str);
void print_long_entry(const char *path, const char *name, const struct stat *st,
                      int w_link, int w_user, int w_group, int w_size);
int num_width(unsigned long n);

int main(int argc, char *argv[])
{
    int long_flag = 0;
    int opt;

    while ((opt = getopt(argc, argv, "l")) != -1)
    {
        switch (opt)
        {
        case 'l':
            long_flag = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-l] [dir...]\n", argv[0]);
            return 1;
        }
    }

    if (optind == argc)
    {
        do_ls(".", long_flag);
    }
    else
    {
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s : \n", argv[i]);
            do_ls(argv[i], long_flag);
            puts("");
        }
    }
    return 0;
}

/* Decide which display function to use. */
void do_ls(const char *dir, int long_flag)
{
    if (long_flag)
    {
        do_ls_long(dir);
        return;
    }

    /* original simple display */
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory : %s\n", dir);
        return;
    }
    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        printf("%s\n", entry->d_name);
    }
    if (errno != 0)
        perror("readdir failed");

    closedir(dp);
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
