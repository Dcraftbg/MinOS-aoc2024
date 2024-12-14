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
intptr_t readline(char* buf, size_t bufmax, int fd) {
    intptr_t e;
    size_t i = 0;
    while(i < bufmax) {
        if((e = read(fd, buf+i, 1)) <= 0) {
            return e == 0 ? -PREMATURE_EOF : e;
        }
        if(buf[i++] == '\n') return i;
    }
    return -BUFFER_OVEWFLOW;
}
typedef struct {
    const char* data;
    int width, height;
} CharMap;
static char cm_get(CharMap* map, unsigned int x, unsigned int y) {
    if(x >= map->width || y >= map->height) return 0;
    return map->data[y*map->width+x];
}
typedef struct {
    char* data;
    size_t len, cap;
} Bytes;
void bytes_reserve(Bytes* vec, size_t extra) {
    if(vec->len+extra > vec->cap) {
        size_t ncap = vec->cap * 2 + extra;
        vec->data = realloc(vec->data, ncap*sizeof(vec->data[0]));
        assert(vec->data && "Just buy more ram");
        vec->cap = ncap;
    }
}
intptr_t bytes_readline(Bytes* bytes, int fd) {
    intptr_t e;
    bytes_reserve(bytes, 128);
    while((e=readline(bytes->data+bytes->len, bytes->cap-bytes->len, fd)) == -BUFFER_OVEWFLOW) {
        bytes->len += 128;
        bytes_reserve(bytes, 128);
    }
    if(e <= 0) return e;
    bytes->len += e-1;
    return 0;
}
int cm_load(CharMap* map, const char* path) {
    int fd;
    intptr_t e;
    if((e=open(path, MODE_READ, 0)) < 0) return e;
    fd = e;
    Bytes bytes={0};
    if((e=bytes_readline(&bytes, fd)) < 0) return e;
    map->width = bytes.len;
    map->height = 1;
    while(((e=bytes_readline(&bytes, fd)) >= 0)) {
        map->height++;
    }
    if(e != -PREMATURE_EOF) return e;
    bytes_reserve(&bytes, 1);
    bytes.data[bytes.len++] = '\0';
    map->data = bytes.data;
    return 0;
}

bool map_load(CharMap* map, char* buf, size_t count, int x, int y, int dx, int dy) {
    memset(buf, 0, count);
    size_t i;
    for(i = 0; i < count && (buf[i]=cm_get(map, x, y)); ++i, x+=dx, y+=dy);
    return i == count;
}
static bool check(CharMap* map, const char* needle, size_t needlelen, int x, int y, int dx, int dy) {
    char buf[120];
    if(!map_load(map, buf, needlelen, x, y, dx, dy)) return false;
    return strcmp(buf, needle) == 0;
}
int part1() {
    CharMap map;
    assert(cm_load(&map, "/inputs/day4_input.txt") == 0);
    size_t count=0;
    for(int y = 0; y < map.height; ++y) {
        for(int x = 0; x < map.width; ++x) {
            count += check(&map, "XMAS", 4, x, y, -1, -1);
            count += check(&map, "XMAS", 4, x, y,  0, -1);
            count += check(&map, "XMAS", 4, x, y,  1, -1);
            count += check(&map, "XMAS", 4, x, y, -1,  0);
            count += check(&map, "XMAS", 4, x, y,  1,  0);
            count += check(&map, "XMAS", 4, x, y, -1,  1);
            count += check(&map, "XMAS", 4, x, y,  0,  1);
            count += check(&map, "XMAS", 4, x, y,  1,  1);
        }
    }
    printf("%zu\n", count);
    return 0;
}

int part2() {
    CharMap map;
    assert(cm_load(&map, "/inputs/day4_input.txt") == 0);
    size_t count=0;
    for(int y = 0; y < map.height; ++y) {
        for(int x = 0; x < map.width; ++x) {
            count += 
                (check(&map, "MAS", 3, x, y  ,  1,  1) || check(&map, "SAM", 3, x, y  ,  1,  1)) &&
                (check(&map, "MAS", 3, x, y+2,  1, -1) || check(&map, "SAM", 3, x, y+2,  1, -1));
        }
    }
    printf("%zu\n", count);
    return 0;
}
int main(void) {
#if 1
    return part1();
#else
    return part2();
#endif
}
