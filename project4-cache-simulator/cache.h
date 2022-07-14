#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <stdbool.h>

#define OP_C 0
#define OP_X 1

#define EMPTY_TAG 0x1
#define DIRTY_TAG 0x2

#define READ 0
#define WRITE 1

#define CACHE_MISS -1

typedef struct _cache_block
{
    uint32_t content;
    uint32_t last_used;
} cache_block;

typedef struct _cache_spec
{
    int capacity;
    int way;
    int blocksize;
    int set;
} cache_spec;

typedef struct _cache_stat
{
    int cycle;
    int total_read;
    int total_write;
    int write_back;
    int read_hit;
    int write_hit;
    int read_miss;
    int write_miss;
} cache_stat;

void halt_simulator(char **);
cache_spec *init_cache_spec(void);
cache_stat *init_cache_stat(void);
bool parse_args(int, char **, cache_spec *);
cache_block **cache_alloc_init(cache_spec *);
void read_file(FILE *, cache_block **, cache_stat *, cache_spec *);
void process(cache_block **, uint32_t, bool, cache_stat *, cache_spec *);
uint32_t evict(cache_block **, uint32_t, cache_stat *, cache_spec *);
int check_hit(cache_block **, uint32_t, cache_spec *);
void clear_tags(cache_block **, cache_spec *);