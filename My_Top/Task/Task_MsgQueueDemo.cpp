#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "MC_Serial.hpp"

  /* ── 消息格式 ── */
  typedef struct {
      uint8_t  producer_id;   // 哪个生产者发的
      uint32_t count;         // 消息序号
      uint32_t tick;          // 发送时刻
  } Msg_t;

  static QueueHandle_t demo_queue = NULL; //队列句柄
  #define QUEUE_LEN  3   // 队列容量，最多存储 3 条消息

  /* ── 生产者1: 每 200ms 发一条 ── */
  static void Producer_A(void *arg)
  {
      uint32_t cnt = 0;
      while (1) {
          Msg_t m = { .producer_id = 1, .count = ++cnt, .tick = xTaskGetTickCount() };

          BaseType_t ok = xQueueSend(demo_queue, &m, 0);  // 不等待，满了就丢
          if (ok == pdPASS)
              printf("A → [%lu]\r\n", cnt);
          else
              printf("A 丢弃 [%lu] (队列满)\r\n", cnt);

          vTaskDelay(pdMS_TO_TICKS(200));
      }
  }

  /* ── 生产者2: 每 500ms 发一条 ── */
  static void Producer_B(void *arg)
  {
      uint32_t cnt = 0;
      while (1) {
          Msg_t m = { .producer_id = 2, .count = ++cnt, .tick = xTaskGetTickCount() };

          BaseType_t ok = xQueueSend(demo_queue, &m, 0);
          if (ok == pdPASS)
              printf("B → [%lu]\r\n", cnt);
          else
              printf("B 丢弃 [%lu] (队列满)\r\n", cnt);

          vTaskDelay(pdMS_TO_TICKS(500));
      }
  }

  /* ── 消费者: 每 1 秒取一条（故意慢，让队列堆积） ── */
  static void Consumer_C(void *arg)
  {
      while (1) {
          Msg_t m;
          BaseType_t ok = xQueueReceive(demo_queue, &m, portMAX_DELAY);
          if (ok == pdPASS) {
              printf("        消费 ← 生产者%d  序号[%lu]\r\n", m.producer_id, m.count);
              vTaskDelay(pdMS_TO_TICKS(1000));   // 故意慢
          }
      }
  }

  /* ── 入口: 创建队列 + 3个任务 ── */
static char pcWriteBuffer[512];
void Task_MsgQueueDemo(void *argument)
{
    // 创建队列：最多存 QUEUE_LEN 条消息，每条消息大小为 sizeof(Msg_t)
    demo_queue = xQueueCreate(QUEUE_LEN, sizeof(Msg_t));
    if (demo_queue == NULL) {
        printf("创建队列失败\r\n");
        return;
    }
    printf("===== 消息队列 Demo 启动, 队列容量=%d =====\r\n", QUEUE_LEN);

    printf("剩余可用堆大小: %lu\r\n", xPortGetFreeHeapSize());
    BaseType_t ret1A = xTaskCreate(Producer_A, "ProdA", 128, NULL, osPriorityBelowNormal, NULL);
    BaseType_t ret1B = xTaskCreate(Producer_B, "ProdB", 128, NULL, osPriorityBelowNormal, NULL);
    BaseType_t ret1C = xTaskCreate(Consumer_C, "ConsC", 128, NULL, osPriorityBelowNormal, NULL);
    printf("剩余可用堆大小: %lu\n", xPortGetFreeHeapSize());

    vTaskDelete(NULL);
}