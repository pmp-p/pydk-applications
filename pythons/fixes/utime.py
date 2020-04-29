import time as _time


MICROPY_PY_UTIME_TICKS_PERIOD = 2**30

if sys.version_info[0:2]>(3,7):
    _PASSTHRU = ("time", "sleep", "process_time", "localtime")
    def clock():
        return _time.process_time()
else:
    _PASSTHRU = ("time", "sleep", "clock", "localtime")

for f in _PASSTHRU:
    globals()[f] = getattr(_time, f)


def sleep_ms(t):
    _time.sleep(t / 1000)

def sleep_us(t):
    _time.sleep(t / 1000000)

def ticks_ms():
    return int(_time.time() * 1000) & (MICROPY_PY_UTIME_TICKS_PERIOD - 1)

def ticks_us():
    return int(_time.time() * 1000000) & (MICROPY_PY_UTIME_TICKS_PERIOD - 1)

ticks_cpu = ticks_us

def ticks_add(t, delta):
    return (t + delta) & (MICROPY_PY_UTIME_TICKS_PERIOD - 1)

def ticks_diff(a, b):
    return ((a - b + MICROPY_PY_UTIME_TICKS_PERIOD // 2) & (MICROPY_PY_UTIME_TICKS_PERIOD - 1)) - MICROPY_PY_UTIME_TICKS_PERIOD // 2

del f
