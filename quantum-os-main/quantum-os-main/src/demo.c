/* ============================================================================
 * demo.c
 * Educational demonstrations for threads, synchronization, deadlock, and
 * kernel memory management.
 * ========================================================================== */
#include "kernel.h"

static sem_t done;

static void busy_ticks(u32 n) { u32 end = ticks + n; while ((int)(ticks - end) < 0) ; }

/* ---------- 1. Thread lifecycle and state transitions ---------- */
static void worker_a(void *a) {
    (void)a;
    for (int i = 1; i <= 3; i++) { kprintf("  [worker-A] CPU work, iteration %d\n", i); busy_ticks(4); }
    sem_post(&done);
}
static void worker_b(void *a) {
    (void)a;
    for (int i = 1; i <= 3; i++) { kprintf("  [worker-B] sleeping 50 ms, iteration %d\n", i); thread_sleep(5); }
    sem_post(&done);
}

void demo_threads(void) {
    int old = trace_on; trace_on = 1;
    sem_init(&done, 0);
    kprintf("  [DEMO] creating 2 kernel threads with different workloads...\n");
    thread_create("worker-A", worker_a, 0);
    thread_create("worker-B", worker_b, 0);
    sem_wait(&done); sem_wait(&done);        /* The shell thread blocks until both workers finish. */
    trace_on = old;
    kprintf("  [ OK ] thread lifecycle demo complete.\n");
    threads_dump();
}

/* ---------- 2. Race condition versus mutex ---------- */
#define RACE_N 1000000
static volatile int shared;
static int use_lock;
static mutex_t cmu;

static void race_worker(void *a) {
    (void)a;
    for (int i = 0; i < RACE_N; i++) {
        if (use_lock) mutex_lock(&cmu);
        int tmp = shared;                              /* read  */
        for (volatile int d = 0; d < 20; d++) ;         /* Simulated work inside the critical section. */
        shared = tmp + 1;                              /* write */
        if (use_lock) mutex_unlock(&cmu);
    }
    sem_post(&done);
}

static void race_once(int lock) {
    use_lock = lock; shared = 0;
    mutex_init(&cmu, "counter-lock"); sem_init(&done, 0);
    u32 c0 = ctx_switches;
    thread_create("race-1", race_worker, 0);
    thread_create("race-2", race_worker, 0);
    sem_wait(&done); sem_wait(&done);
    kprintf("  [%s] result=%d expected=%d switches=%u -> %s\n",
            lock ? "LOCK" : "RACE", shared, 2 * RACE_N, ctx_switches - c0,
            shared == 2 * RACE_N ? "CONSISTENT" : "RACE CONDITION");
}

void demo_race(void) {
    kprintf("  [DEMO] two threads increment a shared counter %d times each.\n", RACE_N);
    race_once(0);
    race_once(1);
}

/* ---------- 3. Deadlock ---------- */
static mutex_t ma, mb;
static int ordered;

static void dl_1(void *a) {
    (void)a;
    mutex_lock(&ma); kprintf("  [dl-1] holds A\n");
    thread_sleep(10);
    kprintf("  [dl-1] requests B...\n");
    mutex_lock(&mb); kprintf("  [dl-1] holds A dan B, complete\n");
    mutex_unlock(&mb); mutex_unlock(&ma);
    sem_post(&done);
}
static void dl_2(void *a) {
    (void)a;
    mutex_t *first = ordered ? &ma : &mb, *second = ordered ? &mb : &ma;
    mutex_lock(first); kprintf("  [dl-2] holds %s\n", first->name);
    thread_sleep(10);
    kprintf("  [dl-2] requests %s...\n", second->name);
    mutex_lock(second); kprintf("  [dl-2] holds A dan B, complete\n");
    mutex_unlock(second); mutex_unlock(first);
    sem_post(&done);
}

void demo_deadlock(int fix) {
    for (int i = 2; i < MAX_THREADS; i++) {              /* Remove blocked threads left by a previous deadlock demonstration. */
        thread_t *t = thread_table(i);
        if (t->state == T_WAITING && !strncmp(t->name, "dl-", 3)) thread_kill(t->tid);
    }
    ordered = fix;
    mutex_init(&ma, "mutex-A"); mutex_init(&mb, "mutex-B"); sem_init(&done, 0);
    kprintf(fix ? "  [DEMO] FIX: both threads acquire A then B.\n"
                : "  [DEMO] DEADLOCK: dl-1 uses A->B while dl-2 uses B->A.\n");
    thread_create("dl-1", dl_1, 0);
    thread_create("dl-2", dl_2, 0);
    if (fix) {
        sem_wait(&done); sem_wait(&done);
        kprintf("  [ OK ] both threads completed; circular wait was prevented.\n");
    } else {
        thread_sleep(60);                                 /* Allow both workers time to acquire their first lock and block. */
        if (!detect_deadlock()) kprintf("  [ OK ] no deadlock detected.\n");
        kprintf("  [ .. ] use 'ps' to inspect WAITING threads; clean them with 'kill <tid>'.\n");
    }
}

/* ---------- 4. Memory ---------- */
static void print_mem(void) {
    struct meminfo m; mem_info(&m);
    kprintf("  total=%u  used=%u  available=%u  blocks=%u  (alloc=%u free=%u failed=%u)\n",
            m.total, m.used, m.free, m.nblocks, m.nalloc, m.nfree, m.nfail);
}

void demo_memtest(void) {
    void *p[64]; int n = 0;
    kprintf("  [ .. ] initial heap state\n");
    print_mem();
    while (n < 64 && (p[n] = kmalloc(16 * 1024)) != 0) n++;
    kprintf("  [DEMO] 16 KB allocations succeeded: %d; next allocation returned NULL safely.\n", n);
    print_mem();
    for (int i = 0; i < n; i++) kfree(p[i]);
    kprintf("  [ OK ] all test blocks released\n");
    print_mem();
}
