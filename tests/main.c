#include "queue.h"
#include "stdio.h"

// Test struct
typedef struct {
    uint8_t val;
    uint16_t bigger_val;
    uint32_t biggest_val;
} test_t;

static void printStruct(test_t* s, uint8_t type)
{
    if (type == 0)
    {
        printf("\n--- Prior to enqueue ---\n");
    }
    else
    {
        printf("\n--- After to dequeue ---\n");
    }

    printf("Val: %d\n", s->val);
    printf("Bigger val: %d\n", s->bigger_val);
    printf("Biggest val: %d\n", s->biggest_val);
}

static void simpleTest()
{
    q_handle_t queue;

    test_t test;
    test_t ret;

    test.val = 1;
    test.bigger_val = 2;
    test.biggest_val = 1000;

    qConstruct(&queue, sizeof(test));

    printf("Queue size: %d\n", queue.size);
    printf("Queue max items: %d\n", queue.max_items);

    printStruct(&test, 0);

    qSendToBack(&queue, &test);

    test.val = 69;
    test.bigger_val = 69;
    test.biggest_val = 69;

    printStruct(&test, 0);

    qSendToBack(&queue, &test);
    qReceive(&queue, &ret);

    printStruct(&ret, 1);

    qReceive(&queue, &ret);

    printStruct(&ret, 1);
}

static void fullTest()
{
    q_handle_t queue;
    int i;

    test_t test;
    test_t ret;

    test.val = 0;
    test.bigger_val = 0;
    test.biggest_val = 0;

    qConstruct(&queue, sizeof(test));

    for (i = 0; i < queue.max_items; i++)
    {
        if (qSendToBack(&queue, &test) == FAILURE)
        {
            printf("Exception: Queue full earlier than expected\n");

            return;
        }
        ++test.val;
        ++test.bigger_val;
        ++test.biggest_val;
    }

    if (qSendToBack(&queue, &test) != FAILURE)
    {
        printf("Exception: Queue added extra item\n");

        return;
    }

    for (i = 127; i >= 0; i--)
    {
        if (qReceive(&queue, &ret) == FAILURE)
        {
            printf("Exception: Queue marked as empty too early\n");

            return;
        }
        if (ret.val != 127 - i || ret.bigger_val != 127 - i || ret.biggest_val != 127 - i)
        {
            printf("Exception: Value %d does not match expected %d\n", ret.val, 127 - i);

            return;
        }
    }

    if (qReceive(&queue, &ret) != FAILURE)
    {
        printf("Exception: Queue not marked as empty\n");

        return;
    }

    printf("\nFull test passed!\n");
}

void movingTest()
{
    q_handle_t queue;
    int i;
    int curr = 0;

    test_t test;
    test_t ret;

    uint8_t vals[400];

    for (i = 0; i < 400; i++)
    {
        vals[i] = i;
    }

    qConstruct(&queue, sizeof(test));

    for (i = 0; i < 200; i++)
    {
        test.val = vals[curr];
        test.bigger_val = vals[curr];
        test.biggest_val = vals[curr++];
        qSendToBack(&queue, &test);
        test.val = vals[curr];
        test.bigger_val = vals[curr];
        test.biggest_val = vals[curr++];
        qSendToBack(&queue, &test);

        qReceive(&queue, &ret);
        if (ret.val != vals[i] || ret.bigger_val != vals[i] || ret.biggest_val != vals[i])
        {
            printf("Exception: Value %d does not match expected %d\n", ret.val, vals[i]);

            return;
        }
    }

    printf("\nMoving test passed!\n");
}

int main()
{
    simpleTest();
    fullTest();
    movingTest();

    return 0;
}
