#include "cache.h"

static inline uint32_t
get_set_bits(cache_block **cache, uint32_t addr, cache_spec *spec)
{
    return ((addr / spec->blocksize) & (spec->set - 1));
}

static inline uint32_t
get_tag_bits(cache_block **cache, uint32_t addr, cache_spec *spec)
{
    return (addr / (spec->blocksize * spec->set));
}

void
halt_simulator(char **str)
{
    printf("Unexpected simulator behaviour detected:\n");
    printf("%s\n", str);
    exit(1);
}

cache_spec *
init_cache_spec(void)
{
    cache_spec *spec;

    spec = (cache_spec *) malloc(sizeof(cache_spec));

    spec->capacity  = 256;
    spec->way       = 4;
    spec->blocksize = 8;

    return spec;
}

cache_stat *
init_cache_stat(void)
{
    cache_stat *stat;

    stat = (cache_stat *) malloc(sizeof(cache_stat));

    stat->cycle       = 0;
    stat->total_read  = 0;
    stat->total_write = 0;
    stat->write_back  = 0;
    stat->read_hit    = 0;
    stat->write_hit   = 0;
    stat->read_miss   = 0;
    stat->write_miss  = 0;

    return stat;
}

bool
parse_args(int argc, char *argv[], cache_spec *spec)
{
    char *token;
    bool option;

    option = false;

    if(argc < 2)
        halt_simulator("Wrong input (1)");

    int i = 1;
    while(i != argc - 1)
    {
        if(strcmp(argv[i], "-c") == 0)
        {
            token           = strtok(argv[i + 1], ":");
            spec->capacity  = atoi(token);
            token           = strtok(NULL, ":");
            spec->way       = atoi(token);
            token           = strtok(NULL, "\0");
            spec->blocksize = atoi(token);
            spec->set       = spec->capacity / spec->way / spec->blocksize;
        }

        else if(strcmp(argv[i], "-x") == 0)
            option = true;

        i++;
    }

    return option;
}

cache_block **
cache_alloc_init(cache_spec *spec)
{
    cache_block **cache;

    cache = (cache_block **) malloc(sizeof(cache_block *) * spec->set);

    for(int i = 0; i < spec->set; i++)
    {
        cache[i] = (cache_block *) malloc(sizeof(cache_block) * spec->way);
    }

    for(int i = 0; i < spec->set; i++)
    {
        for(int j = 0; j < spec->way; j++)
        {
            cache[i][j].content   = EMPTY_TAG;
            cache[i][j].last_used = 0;
        }
    }

    return cache;
}

void
read_file(FILE *file, cache_block **cache, cache_stat *stat, cache_spec *spec)
{
    char *buffer;
    size_t len;
    uint32_t addr;
    bool rw;

    while(getline(&buffer, &len, file) != -1)
    {
        switch(buffer[0])
        {
        case 'R':
            rw = READ;
            break;
        case 'W':
            rw = WRITE;
            break;
        default:
            halt_simulator("Wrong R/W option in file");
        }
        addr = (uint32_t) strtol(buffer + 2, NULL, 0);
        process(cache, addr, rw, stat, spec);
    }
}

void
process(cache_block **cache,
        uint32_t addr,
        bool rw,
        cache_stat *stat,
        cache_spec *spec)
{
    uint32_t ofs, idx;

    stat->cycle++;

    addr &= ~(spec->blocksize - 1);
    ofs = get_set_bits(cache, addr, spec);
    idx = check_hit(cache, addr, spec);

    switch(rw)
    {
    case READ:
        stat->total_read++;
        if(idx == CACHE_MISS)
        {
            stat->read_miss++;
            idx = evict(cache, ofs, stat, spec);

            cache[ofs][idx].content = addr;
        }
        else
        {
            stat->read_hit++;
        }
        break;

    case WRITE:
        stat->total_write++;
        idx = check_hit(cache, addr, spec);
        if(idx == CACHE_MISS)
        {
            stat->write_miss++;
            idx = evict(cache, ofs, stat, spec);

            cache[ofs][idx].content = addr;
        }
        else
        {
            stat->write_hit++;
        }
        cache[ofs][idx].content |= DIRTY_TAG;
        break;
    }

    cache[ofs][idx].last_used = stat->cycle;
}

uint32_t
evict(cache_block **cache, uint32_t ofs, cache_stat *stat, cache_spec *spec)
{
    uint32_t idx;

    idx = 0;
    for(int i = 0; i < spec->way; i++)
    {
        if(cache[ofs][i].last_used < cache[ofs][idx].last_used)
        {
            idx = i;
        }
    }

    if(cache[ofs][idx].content & DIRTY_TAG)
    {
        stat->write_back++;
    }
    return idx;
}

int
check_hit(cache_block **cache, uint32_t addr, cache_spec *spec)
{
    uint32_t ofs, tag;

    ofs = get_set_bits(cache, addr, spec);

    for(int i = 0; i < spec->way; i++)
    {
        if(cache[ofs][i].content != EMPTY_TAG)
        {
            tag = get_tag_bits(cache, cache[ofs][i].content, spec);
            if(tag == get_tag_bits(cache, addr, spec))
                return i;
        }
    }

    return -1;
}

void
clear_tags(cache_block **cache, cache_spec *spec)
{
    for(int i = 0; i < spec->set; i++)
    {
        for(int j = 0; j < spec->way; j++)
        {
            cache[i][j].content &= ~(EMPTY_TAG | DIRTY_TAG);
        }
    }
}
