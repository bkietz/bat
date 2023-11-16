# Style guide:

loop() always:
- branches on some volatiles (millis(), Serial.available(), set by an iterrupt handler, ...)
- does whatever's available
- delays

It seems this could be organized into an explicit structure, which is
probably a really basic OS.

For tasks which run at intervals, we'll have tasks which are
`(timestamp now) -> timestamp`; they return the timestamp at which
they will next be ready to run. We store them in a priority queue
with the next task at the top. We can then figure out precisely
when there will be work to do.
- Do not `delay()` in a task; instead return and wait for the
  system to rerun you
- Do not block/run for a long time in a task; instead break it
  up so that other tasks can run

```
constexpr TaskSet TASKS{
  [](uint32_t now) {
    Serial.println("tick %d", now);
    return now + 1000;
  },
  [](uint32_t now) {
    auto mag = rawMag();
    Serial.println("mag: %f %f %f", mag[0], mag[1], mag[2]);
    return now + 5;
  },
};

void loop() { TASKS.run(); }
```

Most interrupts can be handled acceptably by setting a volatile flag
and waiting for `loop()` to branch on the flag. Handlers for these
can be an alternative to tasks and can run at higher priority.
This does mean we can't use `delay()` much; we have to poll the
volatile flags constantly. `delay(maxReactionTime)` is the most we'd
want to do; probably not worthwhile.

A full priority queue might be overly baroque for such a simple OS.
It might also be easier to think about if interrupts were attachable
to tasks as an alternative run condition. Then tasks can poke each other
when that's useful... but why wouldn't you just call the other tasks'
code directly?
