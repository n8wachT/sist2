#include "walk.h"
#include "src/ctx.h"

__always_inline
parse_job_t *create_fs_parse_job(const char *filepath, const struct stat *info, int base) {
    int len = (int) strlen(filepath);
    parse_job_t *job = malloc(sizeof(parse_job_t) + len);

    strcpy(job->filepath, filepath);
    job->base = base;
    char *p = strrchr(filepath + base, '.');
    if (p != NULL) {
        job->ext = (int) (p - filepath + 1);
    } else {
        job->ext = len;
    }

    job->info = *info;

    memset(job->parent, 0, 16);

    job->vfile.filepath = job->filepath;
    job->vfile.read = fs_read;
    job->vfile.close = fs_close;
    job->vfile.fd = -1;
    job->vfile.is_fs_file = TRUE;

    return job;
}

int handle_entry(const char *filepath, const struct stat *info, int typeflag, struct FTW *ftw) {
    if (ftw->level <= ScanCtx.depth && typeflag == FTW_F && S_ISREG(info->st_mode)) {
        parse_job_t *job = create_fs_parse_job(filepath, info, ftw->base);
        tpool_add_work(ScanCtx.pool, parse, job);
    }

    return 0;
}

int walk_directory_tree(const char *dirpath) {
    return nftw(dirpath, handle_entry, 15, FTW_PHYS);
}
