# REPORT.md: OS Assignment 02 (ls utility)

Roll No: BSDSF24M016

---

## Feature 2: ls-v1.1.0 (Long Listing Format)

### Q1. What is the crucial difference between `stat()` and `lstat()`? When is `lstat()` more appropriate in `ls`?

Both calls fill a `struct stat` with a file's metadata (mode, owner, size, times, link count). They differ only when the path is a **symbolic link**:

- `stat(path, &st)` **follows** the link and returns information about the file the link points to.
- `lstat(path, &st)` does **not** follow it and returns information about the link itself (its own type, size and permissions).

`lstat()` is the right choice in `ls -l`, because `ls` must show what is actually in the directory. For a symlink it has to print the type `l`, the link's own size (the length of the target path) and its own timestamp. With `stat()` the entry would look like the target file, and a dangling link would make `stat()` fail with `ENOENT`, so the entry could not be listed at all. In my code I use `lstat()` and then `readlink()` to print `name -> target`. The same reasoning applies to the recursive `-R` feature, where `lstat()` stops the program from following a symlink to a directory and looping forever.

### Q2. How do bitwise operators and macros extract the file type and permissions from `st_mode`?

`st_mode` packs several things into one integer: the **file type** in the high bits, the **special bits** (setuid, setgid, sticky) and the **permission bits** (rwx for owner, group and others) in the low bits. Each piece is isolated with a bit mask and the `&` operator.

**File type.** The mask `S_IFMT` selects only the type bits. Comparing the result with a type constant tells the type:

```c
if ((st.st_mode & S_IFMT) == S_IFDIR)   /* it is a directory */
```

The macros `S_ISDIR(m)`, `S_ISREG(m)`, `S_ISLNK(m)` and so on do this same test. I use them in `mode_to_string()` to choose `d`, `-`, `l`, `c`, `b`, `p` or `s`.

**Permissions.** Each permission has its own single-bit mask: `S_IRUSR`, `S_IWUSR`, `S_IXUSR` for the owner, `S_IRGRP`, `S_IWGRP`, `S_IXGRP` for the group and `S_IROTH`, `S_IWOTH`, `S_IXOTH` for others. ANDing the mode with a mask gives non-zero if the bit is set:

```c
str[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
```

**Special bits.** `S_ISUID`, `S_ISGID` and `S_ISVTX` are tested the same way. When set, they replace the execute character with `s`/`S` (setuid or setgid) or `t`/`T` (sticky). The lowercase letter means the execute bit is also set, and the uppercase letter means it is not.

---

## Feature 3: ls-v1.2.0 (Column Display, Down Then Across)

### Q1. Explain the logic for printing in a "down then across" format. Why is a single loop insufficient?

First all names are read into an array, and the longest name is recorded. With the terminal width `W` and a column width of `maxlen + 2`, the number of columns is `cols = W / col_width`, and the number of rows is `rows = ceil(count / cols)`.

The names are then **sorted into columns top to bottom**, but the terminal can only print **one row at a time, left to right**. So the program loops over rows, and inside each row loops over columns. The item for row `r` and column `c` is `names[r + c * rows]`. For example, with 10 names and 3 rows the first line prints `names[0]`, `names[3]`, `names[6]`, `names[9]`.

A single loop `for i in 0..count` would print `names[0], names[1], names[2], ...` in order. That gives "across then down" (the layout of `ls -x`), where consecutive names sit next to each other on one line. Down-then-across needs names that are `rows` apart in the array to sit on the same line, which requires the index to jump by `rows`. That can only be done with a nested loop that computes the index from the row and column.

### Q2. What is the purpose of `ioctl` here? What are the limits of a fixed 80-column fallback?

`ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)` asks the terminal driver for the current window size and fills a `struct winsize`. `ws.ws_col` is the number of columns, so the program can work out how many columns of names fit on the screen.

With only a fixed width of 80:
- On a **wider** terminal the output would waste space and use fewer columns than it could.
- On a **narrower** terminal, lines would be longer than the window and the terminal would wrap them mid-name, which breaks the alignment.
- The layout would not follow the user resizing the window.

The fixed 80 is still useful as a **fallback** when `ioctl` fails, such as when the output is redirected to a file or a pipe, where there is no terminal to ask.

---

## Feature 4: ls-v1.3.0 (Horizontal Column Display, -x)

### Q1. Compare the complexity of the "down then across" and "across" printing logic. Which needs more pre-calculation, and why?

**Down then across** needs the most pre-calculation. Before printing anything the program must know the number of columns (`term_width / col_width`) and then the number of rows (`ceil(count / cols)`), because the index of each item is `r + c * rows` and depends on `rows`. The nested loop has to handle a last column that is only partly filled (`idx >= count`). The list has to be fully known up front.

**Across** needs almost none. It only needs the column width and the terminal width. The program walks the array once in order, prints each name padded to the column width, keeps a running horizontal position, and prints a newline when the next item would go past the edge. There is no row count and no index arithmetic, so it is a single loop. It is simpler because the order of printing is the same as the order of the array. Down-then-across has to re-map the array into a grid.

### Q2. How did you manage the display modes (-l, -x, default)? How does the program decide which function to call?

I defined an enum `display_mode_t` with the values `MODE_DEFAULT`, `MODE_LONG` and `MODE_HORIZONTAL`. In `main()` the `getopt()` loop (option string `"lx"`) sets one variable `mode` when it sees `-l` or `-x`. If both are given, the last one wins, like the real `ls`. The mode is passed to `do_ls(dir, mode)`.

Inside `do_ls`:
- `MODE_LONG` calls `do_ls_long()`, which does its own two-pass reading because it needs `lstat` data.
- Otherwise it reads all names with `read_names()` and calls `display_horizontal()` for `MODE_HORIZONTAL`, or `display_columns()` for the default.

One variable and one dispatch point means a new mode only needs a new enum value and a new `if` branch.

---

## Feature 5: ls-v1.4.0 (Alphabetical Sort)

### Q1. Why must all entries be read into memory before sorting? What are the drawbacks for directories with millions of files?

`readdir()` returns entries in whatever order the file system stores them, one at a time, and there is no way to ask for "the next smallest name". A sort can only decide where an item belongs by comparing it with all the others, so the whole list has to exist in memory before the first line can be printed. The same applies to the column layout, which needs the longest name and the total count first.

For a directory with millions of entries this has costs:
- **Memory:** every name is stored (`strdup`) plus a pointer for each, so memory grows with the number of files, and `-l` would need metadata as well.
- **Latency:** nothing appears until the whole directory is read and sorted. Sorting is `O(n log n)`.
- **Failure risk:** `malloc`/`realloc` can fail on a very large directory.

Real `ls` has the same drawback, which is why it offers the unsorted `-f` option for huge directories (it prints entries as they are read).

### Q2. What is the purpose and signature of the `qsort()` comparison function? Why `const void *`?

`qsort(base, n, size, cmp)` is a generic sort that works on any array type. It knows only the element size, not the type, so it cannot compare elements itself. The caller supplies `cmp`, which tells `qsort` how to order two elements:

```c
int compare_names(const void *a, const void *b);
```

It returns a negative number if `a` sorts before `b`, zero if they are equal and a positive number if `a` sorts after `b`.

The parameters are `const void *` because `qsort` must accept comparison functions for any element type, and `void *` is the only pointer type that can hold the address of any type. `const` promises the comparison will not modify the elements. Inside the function we cast back to the real type. `qsort` passes the **address of each array element**, and each element is a `char *`, so the arguments are really `char **`:

```c
const char *s1 = *(const char *const *)a;
return strcmp(s1, s2);
```

Note: `strcmp` compares byte values (ASCII order), so uppercase letters sort before lowercase ones. The real `ls` uses locale-aware collation, so its order can differ for mixed case.
