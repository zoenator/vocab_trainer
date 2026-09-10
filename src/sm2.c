#include "sm2.h"
#include "vocab_entry.h"
#include <time.h>

int calculate_sm2(vocab_entry *ve, int lvl)
{
    // * formula for ease factor (never below 1.3)
    ve->ease_factor = ve->ease_factor + (0.1 - (5 - lvl) * (0.08 + (5 - lvl) * 0.02));
    if (ve->ease_factor < 1.3)
        ve->ease_factor = 1.3;

    // * set interval and repetitions
    if (lvl < 3)
    {
        ve->repetitions = 0;
        ve->interval = 1;
    }
    else
    {
        switch (ve->repetitions++)
        {
        case 0:
            ve->interval = 1;
            break;
        case 1:
            ve->interval = 6;
            break;
        default:
            ve->interval *= ve->ease_factor;
            break;
        }
    }

    // * set next due date
    time_t now = time(NULL);
    ve->next_due = now + (ve->interval * SEC_PER_DAY);
    return 0;
}

int init_sm2_stats(vocab_entry *ve)
{
    if (ve == NULL)
        return 1;

    ve->ease_factor = BASE_EASE_FACTOR;
    ve->repetitions = 0;
    return 0;
}