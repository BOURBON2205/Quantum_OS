#ifndef KERNEL_H
#define KERNEL_H

/* ============================================================================
 * kernel.h
 * Shared kernel types, constants, hardware helpers, and subsystem APIs.
 * ========================================================================== */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

/* ---------- OS identity ---------- */
#define OS_NAME "QuantumOS"
#define OS_VER  "1.0"

/* ---------- scheduler ---------- */
#define TICK_HZ      100
#define MAX_THREADS  16
#define QUANTUM      3       /* 3 ticks = 30 ms */

/* ---------- kernel heap ---------- */
#define KERNEL_HEAP_SIZE (512 * 1024)

/* ---------- users ---------- */
#define UID_ROOT  0
#define UID_GUEST 1

/* ---------- hardware helpers ---------- */
static inline void outb(u16 port, u8 value) {
    __asm__ volatile("outb %0,%1" :: "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 value;
    __asm__ volatile("inb %1,%0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline u32 irq_save(void) {
    u32 flags;
    __asm__ volatile("pushf; pop %0; cli" : "=r"(flags) :: "memory");
    return flags;
}

static inline void irq_restore(u32 flags) {
    __asm__ volatile("push %0; popf" :: "r"(flags) : "memory", "cc");
}

/* ---------- string.c ---------- */
size_t strlen(const char *s);
int    strcmp(const char *a, const char *b);
int    strncmp(const char *a, const char *b, size_t n);
char  *strncpy(char *dst, const char *src, size_t n);
void  *memset(void *dst, int c, size_t n);
void  *memcpy(void *dst, const void *src, size_t n);
void  *memmove(void *dst, const void *src, size_t n);
int    atoi(const char *s);

/* ---------- console.c ---------- */
void console_init(void);
void kputc(char c);
void kputs(const char *s);
void kprintf(const char *fmt, ...);
void console_clear(void);
void console_color(u8 color);
void console_set_header_end(void);
void console_get_cursor(int *x, int *y);
int  console_get_mouse_click(int *x, int *y);

#define KEY_UP    0x101
#define KEY_DOWN  0x102
#define KEY_LEFT  0x103
#define KEY_RIGHT 0x104
#define KEY_HOME  0x105
#define KEY_END   0x106
#define KEY_DEL   0x107

int console_getc_nb(void);
int readline(char *buf, int max);

/* ---------- interrupts.c ---------- */
void interrupts_init(void);
extern volatile u32 ticks;

/* ---------- mem.c ---------- */
void mem_init(void);
void *kmalloc(u32 size);
void kfree(void *ptr);

struct meminfo {
    u32 total;
    u32 used;
    u32 free;
    u32 nblocks;
    u32 nalloc;
    u32 nfree;
    u32 nfail;
};

void mem_info(struct meminfo *mi);

/* ---------- thread.c ---------- */
enum tstate {
    T_UNUSED,
    T_NEW,
    T_READY,
    T_RUNNING,
    T_WAITING,
    T_TERMINATED
};

struct mutex;

typedef struct thread {
    int           tid;
    char          name[16];
    enum tstate   state;
    u32           esp;
    u8           *stack;
    void        (*fn)(void *);
    void         *arg;
    u32           wake_tick;
    int           sleeping;
    struct mutex *wait_mutex;
    struct thread *wnext;
    u32           runtime;
} thread_t;

void      threads_init(void);
thread_t *thread_create(const char *name, void (*fn)(void *), void *arg);
void      thread_exit(void);
void      thread_yield(void);
void      thread_sleep(u32 ticks);
void      thread_block(void);
void      thread_wake(thread_t *thread);
int       thread_kill(int tid);
void      schedule(void);
void      timer_tick(void);
void      threads_dump(void);
const char *state_name(enum tstate state);
thread_t *thread_table(int index);

extern thread_t *current;
extern int trace_on;
extern u32 ctx_switches;

int others_ready(void);

/* ---------- sync.c ---------- */
struct wq {
    thread_t *head;
    thread_t *tail;
};

typedef struct mutex {
    int locked;
    thread_t *owner;
    struct wq q;
    const char *name;
} mutex_t;

typedef struct {
    int count;
    struct wq q;
} sem_t;

void mutex_init(mutex_t *mutex, const char *name);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);
int  mutex_cancel_wait(thread_t *thread);

void sem_init(sem_t *sem, int count);
void sem_wait(sem_t *sem);
void sem_post(sem_t *sem);

int detect_deadlock(void);

/* ---------- fs.c ---------- */
#define E_NOENT (-1)
#define E_EXIST (-2)
#define E_PERM  (-3)
#define E_NOMEM (-4)
#define E_FULL  (-5)

#define P_R 4
#define P_W 2

void        fs_init(void);
int         fs_create(const char *name);
int         fs_write(const char *name, const char *text, int append);
int         fs_read(const char *name, const char **data, u32 *size);
int         fs_delete(const char *name);
int         fs_chmod(const char *name, int owner, int other);
void        fs_list(void);
const char *fs_err(int error);

/* ---------- users / access control ---------- */
extern int cur_uid;

const char *uid_name(int uid);
int user_switch(const char *name, const char *password);

/* ---------- demos and shell ---------- */
void demo_threads(void);
void demo_race(void);
void demo_deadlock(int fix);
void demo_memtest(void);

void shell_run(void);
void banner(void);

#endif
