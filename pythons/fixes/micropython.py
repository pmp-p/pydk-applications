#
print('NB: this module [',__name__,'] requires the scheduler repl patch for unix port to be compiled in',file=sys.stderr)
#
import micropython
self = micropython

tasks = []
def __getattr__(attr):
    return getattr(self,attr)


def scheduler():
    global tasks
    lq = len(tasks)
    while lq:
        fn,arg = tasks.pop(0)
        fn(arg)
        lq-=1

def schedule(fn,arg):
    global tasks
    assert callable(fn)
    assert isinstance(arg,int)
    tasks.append( (fn,arg,) )


def mem_info():
    return """mem: total=?, current=?, peak=?
stack: ? out of ?
GC: total: ?, used: ?, free: ?
 No. of 1-blocks: ?, 2-blocks: ?, max blk sz: ?, max free sz: ?"""
