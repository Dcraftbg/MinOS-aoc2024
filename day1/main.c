#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <minos/status.h>
#include <minos/sysstd.h>
#include <assert.h>
#include <strinternal.h>
#define HASHMAP_DEFINE
#include <collections/hashmap.h>
#define ihash(x) ((size_t)x)
#define ieq(a, b) ((a) == (b))
#define ialloc malloc
#define ifree(ptr, s) free(ptr)
MAKE_HASHMAP_EX(IMap, imap, size_t, int, ihash, ieq, ialloc, ifree);


#define hashmap_foreach(pair_t, bi, pair, map) \
    for(size_t bi = 0; bi < (map).buckets.len; ++bi) \
        for(pair_t* pair = (map).buckets.items[bi].first; pair; pair = pair->next)
#define imap_foreach(bi, pair, imap)\
    hashmap_foreach(Pair_IMap, bi, pair, imap)
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
int int_cmp(const void* a_void, const void* b_void) {
    int a = *((const int*)a_void);
    int b = *((const int*)b_void);
    return a-b;
}
void dump_nums(int* nums, size_t nums_count) {
    printf("{ ");
    for(size_t i = 0; i < nums_count; ++i) {
        if(i > 0) printf(", ");
        printf("%d", nums[i]);
    } 
    printf(" }\n");
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
const char* ltrim(const char* head) {
    while(head[0] && (head[0] == ' ' || head[0] == '\n' || head[0] == '\r')) head++;
    return head;
}
int part1() {
    intptr_t e;
    Buf buf;
    const char* input_path = "/inputs/day1_input.txt";
    if((e=load(input_path, &buf)) < 0) {
        eprintfln("Failed to load `%s`: %s", input_path, status_str(e));
        return 1;
    }
    char* data = realloc(buf.data, buf.len+1);
    assert(data && "Ran out of memory");
    data[buf.len] = '\0';
    const char* head = data;
    IVec a;
    IVec b;
    while((head = ltrim(head))[0]) {
        ipush(&a, atoi_internal(head, &head));
        head = ltrim(head);
        ipush(&b, atoi_internal(head, &head));
    }
    qsort(a.data, a.len, sizeof(a.data[0]), int_cmp);
    qsort(b.data, b.len, sizeof(b.data[0]), int_cmp);
    // dump_nums(a.data, a.len);
    // dump_nums(b.data, b.len);
    assert(a.len == b.len);
    size_t sum = 0;
    for(size_t i = 0; i < a.len; ++i) {
        sum += abs(a.data[i]-b.data[i]);
    }
    eprintfln("Sum: %zu", sum);
    return 0;
}
void imap_insert_or_inc(IMap* imap, int v) {
    size_t *t;
    if((t=imap_get(imap, v))) {
        (*t)++;
        return;
    }
    imap_insert(imap, v, 1);
}
int part2() {
    intptr_t e;
    Buf buf;
    const char* input_path = "/inputs/day1_input.txt";
    if((e=load(input_path, &buf)) < 0) {
        eprintfln("Failed to load `%s`: %s", input_path, status_str(e));
        return 1;
    }
    char* data = realloc(buf.data, buf.len+1);
    assert(data && "Ran out of memory");
    data[buf.len] = '\0';
    const char* head = data;
    IVec left={0};
    IMap right={0};
    while((head = ltrim(head))[0]) {
        ipush(&left, atoi_internal(head, &head));
        head = ltrim(head);
        imap_insert_or_inc(&right, atoi_internal(head, &head));
    }
    size_t sum=0;
    for(size_t i = 0; i < left.len; ++i) {
        int a = left.data[i];
        size_t count=0;
        size_t* t;
        if((t=imap_get(&right, a)))
            count = *t;
        sum += a*count;
    }
    eprintfln("Sum: %zu", sum);
    return 0;
}
int main(void) {
    return 
        1 ? 
            part2() :
            part1();
}
