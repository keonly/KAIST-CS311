#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "cache.h"

#define BYTES_PER_WORD 4

void cdump(int, int, int);
void sdump(int, int, int, int, int, int, int);
void xdump(int, int, cache_block **);
int main(int, char **);

/***************************************************************
 *                                                             *
 * Procedure : cdump                                           *
 *                                                             *
 * Purpose   : Dump cache configuration                        *
 *                                                             *
 ***************************************************************/
void
cdump(int capacity, int assoc, int blocksize)
{

    printf("Cache Configuration:\n");
    printf("-------------------------------------\n");
    printf("Capacity: %dB\n", capacity);
    printf("Associativity: %dway\n", assoc);
    printf("Block Size: %dB\n", blocksize);
    printf("\n");
}

/***************************************************************
 *                                                             *
 * Procedure : sdump                                           *
 *                                                             *
 * Purpose   : Dump cache stat		                           *
 *                                                             *
 ***************************************************************/
void
sdump(int total_reads,
      int total_writes,
      int write_backs,
      int reads_hits,
      int write_hits,
      int reads_misses,
      int write_misses)
{
    printf("Cache Stat:\n");
    printf("-------------------------------------\n");
    printf("Total reads: %d\n", total_reads);
    printf("Total writes: %d\n", total_writes);
    printf("Write-backs: %d\n", write_backs);
    printf("Read hits: %d\n", reads_hits);
    printf("Write hits: %d\n", write_hits);
    printf("Read misses: %d\n", reads_misses);
    printf("Write misses: %d\n", write_misses);
    printf("\n");
}

/***************************************************************
 *                                                             *
 * Procedure : xdump                                           *
 *                                                             *
 * Purpose   : Dump current cache state                        *
 * 							                                   *
 * Cache Design						                           *
 *  							                               *
 * 	    cache[set][assoc][word per block]		               *
 *      						                               *
 *       +--------+--------+--------+--------+ 	               *
 *       |        |  way0  |  way1  |  way2  |                 *
 *       +--------+--------+--------+--------+                 *
 *       |        |  word0 |  word0 |  word0 |                 *
 *       |  set0  |  word1 |  word1 |  word1 |                 *
 *       |        |  word2 |  word2 |  word2 |                 *
 *       |        |  word3 |  word3 |  word3 |                 *
 *       +--------+--------+--------+--------+                 *
 *       |        |  word0 |  word0 |  word0 |                 *
 *       |  set1  |  word1 |  word1 |  word1 |                 *
 *       |        |  word2 |  word2 |  word2 |                 *
 *       |        |  word3 |  word3 |  word3 |                 *
 *       +--------+--------+--------+--------+                 *
 *                                                             *
 ***************************************************************/
void
xdump(int set, int way, cache_block **cache)
{
    int i, j, k = 0;

    printf("Cache Content:\n");
    printf("-------------------------------------\n");
    for(i = 0; i < way; i++)
    {
        if(i == 0)
            printf("    ");

        printf("      WAY[%d]", i);
    }
    printf("\n");

    for(i = 0; i < set; i++)
    {
        printf("SET[%d]:   ", i);
        for(j = 0; j < way; j++)
        {
            if(k != 0 && j == 0)
                printf("          ");

            printf("0x%08x  ", cache[i][j].content);
        }
        printf("\n");
    }
    printf("\n");
}

int
main(int argc, char *argv[])
{
    cache_block **cache;
    cache_spec *spec;
    cache_stat *stat;
    FILE *file;
    bool x_op;
    int set, words;
    uint32_t addr;

    spec  = init_cache_spec();
    stat  = init_cache_stat();
    x_op  = parse_args(argc, argv, spec);
    words = spec->blocksize / BYTES_PER_WORD;
    cache = cache_alloc_init(spec);
    file  = fopen(argv[argc - 1], "r");
    if(file == NULL)
        halt_simulator("File does not exist");

    read_file(file, cache, stat, spec);
    clear_tags(cache, spec);
    call_dump_functions(x_op, stat, spec, cache);

    free(cache);
    free(spec);
    free(stat);
    return 0;
}

void
call_dump_functions(bool option,
                    cache_stat *stat,
                    cache_spec *spec,
                    uint32_t **cache)
{
    cdump(spec->capacity, spec->way, spec->blocksize);

    sdump(stat->total_read,
          stat->total_write,
          stat->write_back,
          stat->read_hit,
          stat->write_hit,
          stat->read_miss,
          stat->write_miss);

    if(option)
        xdump(spec->set, spec->way, cache);
}