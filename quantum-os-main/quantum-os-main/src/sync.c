/* ============================================================================
 * sync.c
 * Mutexes, semaphores, FIFO wait queues, and deadlock detection.
 * ========================================================================== */
#include "kernel.h"

static void wq_push(struct wq *q, thread_t *t) {
    t->wnext = 0;
    if (q->tail) q->tail->wnext = t; else q->head = t;
    q->tail = t;
}
static thread_t *wq_pop(struct wq *q) {
    thread_t *t = q->head;
    if (t) { q->head = t->wnext; if (!q->head) q->tail = 0; t->wnext = 0; }
    return t;
}

void mutex_init(mutex_t *m, const char *name) {
    m->locked = 0;
    m->owner = 0;
    m->q.head = 0;
    m->q.tail = 0;
    m->name = name;
}

void mutex_lock(mutex_t *m) {
    if (!m) return;
    u32 f = irq_save();
    if (m->owner == current) {
        irq_restore(f);
        return;
    }
    while (m->locked) {                  /* tunggu sampai dibangunkan, lalu coba ambil lagi */
        current->wait_mutex = m;
        wq_push(&m->q, current);
        thread_block();
        current->wait_mutex = 0;
    }
    m->locked = 1; m->owner = current;
    irq_restore(f);
}

int mutex_cancel_wait(thread_t *t) {
    if (!t || !t->wait_mutex) return 0;
    mutex_t *m = t->wait_mutex;
    u32 f = irq_save();
    thread_t *prev = 0, *cur = m->q.head;
    while (cur && cur != t) { prev = cur; cur = cur->wnext; }
    if (!cur) {
        irq_restore(f);
        return 0;
    }
    if (prev) prev->wnext = cur->wnext;
    else m->q.head = cur->wnext;
    if (m->q.tail == cur) m->q.tail = prev;
    cur->wnext = 0;
    cur->wait_mutex = 0;
    irq_restore(f);
    return 1;
}

void mutex_unlock(mutex_t *m) {
    if (!m) return;
    u32 f = irq_save();
    if (!m->locked || m->owner != current) {
        irq_restore(f);
        return;
    }
    m->locked = 0; m->owner = 0;
    thread_t *w = wq_pop(&m->q);
    if (w) thread_wake(w);
    irq_restore(f);
}

void sem_init(sem_t *s, int n) {
    if (!s) return;
    s->count = n < 0 ? 0 : n;
    s->q.head = 0;
    s->q.tail = 0;
}

void sem_wait(sem_t *s) {
    if (!s) return;
    u32 f = irq_save();
    if (s->count > 0) s->count--;
    else { wq_push(&s->q, current); thread_block(); }
    irq_restore(f);
}

void sem_post(sem_t *s) {
    if (!s) return;
    u32 f = irq_save();
    thread_t *w = wq_pop(&s->q);
    if (w) thread_wake(w); else s->count++;
    irq_restore(f);
}

/* Cari siklus: thread -> (menunggu mutex) -> pemilik mutex -> ... kembali ke thread awal */
int detect_deadlock(void) {
    int found = 0;
    for (int i = 2; i < MAX_THREADS; i++) {
        thread_t *s = thread_table(i);
        if (s->state != T_WAITING || !s->wait_mutex) continue;
        thread_t *c = s;
        for (int step = 0; step < MAX_THREADS && c && c->wait_mutex; step++) {
            c = c->wait_mutex->owner;
            if (c == s) {
                kprintf("DEADLOCK terdeteksi (circular wait): ");
                thread_t *p = s;
                do {
                    kprintf("T%d(%s) --minta %s--> ", p->tid, p->name, p->wait_mutex->name);
                    p = p->wait_mutex->owner;
                } while (p != s);
                kprintf("T%d(%s)\n", s->tid, s->name);
                found = 1; break;
            }
        }
        if (found) break;
    }
    return found;
}
