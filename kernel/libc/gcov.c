#include <config.h>

#include <libc.h>
#include <gcov.h>

#ifdef POK_NEEDS_GCOV

/**
 * store_gcov_u32 - store 32 bit number in gcov format to buffer
 * @buffer: target buffer or NULL
 * @off: offset into the buffer
 * @v: value to be stored
 *
 * Number format defined by gcc: numbers are recorded in the 32 bit
 * unsigned binary form of the endianness of the machine generating the
 * file. Returns the number of bytes stored. If @buffer is %NULL, doesn't
 * store anything.
 */
static size_t store_gcov_u32(void *buffer, size_t off, uint32_t v)
{
    uint32_t *data;

    if (buffer) {
        data = buffer + off;
        *data = v;
    }

    return sizeof(*data);
}

/**
 * store_gcov_u64 - store 64 bit number in gcov format to buffer
 * @buffer: target buffer or NULL
 * @off: offset into the buffer
 * @v: value to be stored
 *
 * Number format defined by gcc: numbers are recorded in the 32 bit
 * unsigned binary form of the endianness of the machine generating the
 * file. 64 bit numbers are stored as two 32 bit numbers, the low part
 * first. Returns the number of bytes stored. If @buffer is %NULL, doesn't store
 * anything.
 */
static size_t store_gcov_u64(void *buffer, size_t off, uint64_t v)
{
    uint32_t *data;

    if (buffer) {
        data = buffer + off;

        data[0] = (v & 0xffffffffUL);
        data[1] = (v >> 32);
    }

    return sizeof(*data) * 2;
}

/*
 * Determine whether a counter is active. Doesn't change at run-time.
 */
static int counter_active(struct gcov_info *info, unsigned int type)
{
    return info->merge[type] ? 1 : 0;
}

/**
 * convert_to_gcda - convert profiling data set to gcda file format
 * @buffer: the buffer to store file data or %NULL if no data should be stored
 * @info: profiling data set to be converted
 *
 * Returns the number of bytes that were/would have been stored into the buffer.
 */
static size_t convert_to_gcda(unsigned char *buffer, struct gcov_info *info)
{
    struct gcov_fn_info *fi_ptr;
    struct gcov_ctr_info *ci_ptr;
    unsigned int fi_idx;
    unsigned int ct_idx;
    unsigned int cv_idx;
    size_t pos = 0;

    /* File header. */
    pos += store_gcov_u32(buffer, pos, GCOV_DATA_MAGIC);
    pos += store_gcov_u32(buffer, pos, info->version);
    pos += store_gcov_u32(buffer, pos, info->stamp);

    for (fi_idx = 0; fi_idx < info->n_functions; fi_idx++) {
        fi_ptr = info->functions[fi_idx];

        /* Function record. */
        pos += store_gcov_u32(buffer, pos, GCOV_TAG_FUNCTION);
        pos += store_gcov_u32(buffer, pos, GCOV_TAG_FUNCTION_LENGTH);
        pos += store_gcov_u32(buffer, pos, fi_ptr->ident);
        pos += store_gcov_u32(buffer, pos, fi_ptr->lineno_checksum);
        pos += store_gcov_u32(buffer, pos, fi_ptr->cfg_checksum);

        ci_ptr = fi_ptr->ctrs;

        for (ct_idx = 0; ct_idx < GCOV_COUNTERS; ct_idx++) {
            if (!counter_active(info, ct_idx))
                continue;

            /* Counter record. */
            pos += store_gcov_u32(buffer, pos,
                          GCOV_TAG_FOR_COUNTER(ct_idx));
            pos += store_gcov_u32(buffer, pos, ci_ptr->num * 2);

            for (cv_idx = 0; cv_idx < ci_ptr->num; cv_idx++) {
                pos += store_gcov_u64(buffer, pos,
                              ci_ptr->values[cv_idx]);
            }

            ci_ptr++;
        }
    }

    return pos;
}

#define DEFAULT_GCOV_ENTRY_COUNT 200
#define GCOV_HEXDUMP_BUF_SIZE 10000

#define GCOV_MAX_DATA_SIZE 50000

unsigned char data[GCOV_MAX_DATA_SIZE];

struct gcov_entry_t {
    const char *filename;
    uint32_t data_start;
    uint32_t data_end;
};

struct gcov_entry_t entry[DEFAULT_GCOV_ENTRY_COUNT];

static struct gcov_info *gcov_info_head[DEFAULT_GCOV_ENTRY_COUNT];
static size_t num_used_gcov_entries = 0;

static size_t dump_gcov_entry(unsigned char *to_buffer, struct gcov_info *info)
{
    if (info == NULL) {
        return 0;
    }

    size_t sz = convert_to_gcda(NULL, info);
    if (sz >= GCOV_HEXDUMP_BUF_SIZE) {
        printf("%s: size for '%s' is %zd, limit %d\n",
                __func__, info->filename, sz,
                GCOV_HEXDUMP_BUF_SIZE);
        return sz;
    }

    sz = convert_to_gcda(to_buffer, info);
    return sz;
}

void gcov_dump(void)
{
    size_t i, sz;
    size_t size = 0;

    if (gcov_info_head == NULL) {
        return;
    }

    for (i = 0; i < num_used_gcov_entries; i++) {
        struct gcov_info *info = gcov_info_head[i];
        entry[i].filename = info->filename;

        // offset in common buffer
        entry[i].data_start = (uint32_t)data + size;

        sz = dump_gcov_entry(data + size, info);
        size += sz; // total size
        entry[i].data_end = entry[i].data_start + sz;

        //printf("dump binary memory %s 0x%lx 0x%lx\n", entry[i].filename,
        //        entry[i].data_start, entry[i].data_end);
    }
    printf("total size:   %zu\n", size);
    printf("entries used: %zu\n", num_used_gcov_entries);
}

/*
* __gcov_init is called by gcc-generated constructor code for each object
* file compiled with -fprofile-arcs.
*/
void __gcov_init(struct gcov_info *info)
{
    if (info == NULL) {
        printf("%s: NULL info\n", __func__);
        return;
    }
    //printf("%s filename '%s'\n", __func__, info->filename);

    if (num_used_gcov_entries >= DEFAULT_GCOV_ENTRY_COUNT) {
        printf("%s: gcov_info_head is full, all %zd entries used\n",
                __func__, num_used_gcov_entries);
        return;
    }

    gcov_info_head[num_used_gcov_entries++] = info;
}

/* Call the coverage initializers if not done by startup code */
void pok_gcov_init(void) {
    extern uint32_t __CTOR_START__, __CTOR_END__; // linker defined symbols
    uint32_t start = (uint32_t)(&__CTOR_START__ + 1);
    uint32_t end = (uint32_t)(&__CTOR_END__ - 1);
    printf("start 0x%lx, end 0x%lx\n", start, end);
    void (**p)(void);

    while(start < end) {
        p = (void(**)(void))start; // get function pointer
        (*p)(); // call constructor
        start += sizeof(p);
    }
}

#endif /* POK_NEEDS_GCOV */
