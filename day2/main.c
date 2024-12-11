#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <minos/status.h>
#include <minos/sysstd.h>
#include <assert.h>
#include <strinternal.h>
#include <stdbool.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define eprintfln(...) (eprintf(__VA_ARGS__), fputs("\n", stderr))
typedef struct {
    char* data;
    size_t len;
} Buf;

intptr_t read_exact(uintptr_t file, void* bytes, size_t amount) {
    while(amount) {
        size_t rb = read(file, bytes, amount);
        if(rb < 0) return rb;
        if(rb == 0) return -PREMATURE_EOF;
        amount-=rb;
        bytes+=rb;
    }
    return 0;
}
intptr_t load(const char* path, Buf* res) {
    intptr_t e;
    uintptr_t fd;
    if((e=open(path, MODE_READ, 0)) < 0) {
        fprintf(stderr, "Failed to open `%s`: %s\n", path, status_str(e));
        return e;
    }
    fd=e;
    if((e=seek(fd, 0, SEEK_EOF)) < 0) {
        fprintf(stderr, "Failed to seek: %s\n", status_str(e));
        goto err_seek;
    }
    size_t size = e;
    if((e=seek(fd, 0, SEEK_START)) < 0) {
        fprintf(stderr, "Failed to seek back: %s\n", status_str(e));
        goto err_seek;
    }
    char* buf = malloc(size);
    if(!buf) {
        fprintf(stderr, "Failed: Not enough memory :(\n");
        goto err;
    }
    if((e=read_exact(fd, buf, size)) < 0) {
        fprintf(stderr, "Failed to read: %s\n", status_str(e));
        goto err;
    }
    res->data = buf;
    res->len = size;
    close(fd);
    return 0;
err:
    free(buf);
err_seek:
    close(fd);
    return e;
}
const char* load_cstr(const char* path) {
    intptr_t e;
    Buf buf;
    if((e=load(path, &buf)) < 0) {
        eprintfln("Failed to load `%s`: %s", path, status_str(e));
        exit(1);
    }
    char* data = realloc(buf.data, buf.len+1);
    assert(data && "Ran out of memory");
    data[buf.len] = '\0';
    return data;
}
void dump_nums(int* nums, size_t nums_count) {
    printf("{ ");
    for(size_t i = 0; i < nums_count; ++i) {
        if(i > 0) printf(", ");
        printf("%d", nums[i]);
    } 
    printf(" }");
}
typedef struct {
    int *data;
    size_t len, cap;
} IVec;
void ireserve(IVec* ivec, size_t extra) {
    if(ivec->len+extra > ivec->cap) {
        size_t ncap = ivec->cap * 2 + extra;
        ivec->data = realloc(ivec->data, ncap*sizeof(ivec->data[0]));
        assert(ivec->data && "Just buy more ram");
        ivec->cap = ncap;
    }
}
static void ipush(IVec* ivec, int data) {
    ireserve(ivec, 1);
    ivec->data[ivec->len++] = data;
}
static void iinsert(IVec* ivec, size_t i, int data) {
    assert(i <= ivec->len);
    ireserve(ivec, 1);
    memmove(&ivec->data[i+1], &ivec->data[i], (ivec->len-i)*sizeof(int));
    ivec->data[i] = data;
    ivec->len++;
}
static int iiremove(IVec* ivec, size_t i) {
    assert(i < ivec->len);
    int d = ivec->data[i];
    memmove(&ivec->data[i], &ivec->data[i+1], (ivec->len-1-i)*sizeof(int));
    ivec->len--;
    return d;
}
const char* ltrim(const char* head) {
    while(head[0] && (head[0] == ' ' || head[0] == '\n' || head[0] == '\r')) head++;
    return head;
}
static int signi(int a) {
    return (a & (1<<31)) >> 31;
}
bool verify(int* data, size_t len) {
    if(len <= 1) return true;
    int sign = signi(data[0]-data[1]);
    for(size_t i = 0; i < len-1; ++i) {
        int a = data[i] - data[i+1];
        if(sign != signi(a)) return false;
        if(abs(a) < 1 || abs(a) > 3) return false;
    }
    return true;
}
static bool verify_part2(IVec* row) {
    if(verify(row->data, row->len)) return true;
    for(size_t i = 0; i < row->len; ++i) {
        int d = iiremove(row, i);
        if(verify(row->data, row->len)) {
            iinsert(row, i, d);
            return true;
        }
        iinsert(row, i, d);
    }
    return false;
}
int part1() {
    const char* head = load_cstr("/inputs/day2_input.txt");
    size_t count=0;
    IVec row={0};
    while((head=ltrim(head))[0]) {
        while(head[0] != '\n' && head[0]) {
            while(head[0] == ' ' && head[0]) head++;
            ipush(&row, atoi_internal(head, &head));
        }
        count += verify(row.data, row.len);
        // dump_nums(row.data, row.len);
        // printf(" = %s\n", verify(row.data, row.len) ? "true" : "false");
        row.len = 0;
    }
    printf("count = %zu\n", count);
    return 0;
}
int part2() {
    const char* head = load_cstr("/inputs/day2_input.txt");
    size_t count=0;
    IVec row={0};
    while((head=ltrim(head))[0]) {
        while(head[0] != '\n' && head[0]) {
            while(head[0] == ' ' && head[0]) head++;
            if(head[0] == '\n') break;
            ipush(&row, atoi_internal(head, &head));
        }
        bool v = verify_part2(&row);
        count += v;
        // printf("%s\n", v ? "true" : "false");
        // dump_nums(row.data, row.len);
        // printf(" = %s\n", v ? "true" : "false");
        row.len = 0;
    }
    printf("count = %zu\n", count);
    return 0;
}
int main(void) {
#if 0
    return part1();
#else
    return part2();
#endif
}
