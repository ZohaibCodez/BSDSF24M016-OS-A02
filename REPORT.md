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
