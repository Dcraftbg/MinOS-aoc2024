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
const char* parse_mul(const char* head, int* a, int* b, bool* valid) {
    *valid = false;
    if(!head) return NULL;
    head+=3;
    if(head[0] != '(') return head;
    head++;
    *a = atoi_internal(head, &head);
    if(head[0] != ',') return head;
    head++;
    *b = atoi_internal(head, &head);
    if(head[0] != ')') return head;
    head++;
    *valid = true;
    return head;
}
bool strstarts(const char* a, const char* needle) {
    return strncmp(a, needle, strlen(needle)) == 0;
}
int part1() {
    const char* head = load_cstr("/inputs/day3_input.txt");
    int a;
    int b;
    bool valid;
    int sum=0;
    while(head && (head=ltrim(head))[0]) {
        head = parse_mul(strstr(head, "mul"), &a, &b, &valid);
        if(valid) {
            sum += a*b;
        }
    }
    printf("sum: %d\n", sum);
    return 0;
}
typedef struct {
    const char* head;
} Lexer;
const char* lex(Lexer* lexer) {
    while(lexer->head[0] && lexer->head[0] != 'm' && lexer->head[0] != 'd') {
        lexer->head++;
    }
    if(!lexer->head[0]) return NULL;
    return lexer->head;
}
int part2() {
    const char* head = load_cstr("/inputs/day3_input.txt");
    int a;
    int b;
    bool valid;
    int sum=0;
    Lexer lexer = {0};
    lexer.head = head;
    const char* lexem; 
    bool enabled=true;
    while((lexem = lex(&lexer))) {
        if(strstarts(lexem, "mul")) {
            lexer.head = parse_mul(lexer.head, &a, &b, &valid);
            if(enabled && valid) {
                sum += a*b;
            }
        } else if (strstarts(lexem, "do()")) {
            enabled = true;
            lexer.head += 4;
        } else if (strstarts(lexem, "don't()")) {
            enabled = false;
            lexer.head += 7;
        } else {
            lexer.head++;
        }
    }
    printf("sum: %d\n", sum);
    return 0;
}
int main(void) {
#if 0
    return part1();
#else
    return part2();
#endif
}
