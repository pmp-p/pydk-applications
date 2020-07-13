"""The asyncio package, tracking PEP 3156."""
#https://github.com/giampaolo Giampaolo Rodola'
#https://github.com/1st1 Yury Selivanov
#https://github.com/asvetlov Andrew Svetlov

import sys
import io

try:
    from . import base_events
    from . import coroutines
    from . import events
    from . import futures
    from . import locks
    from . import protocols
    from . import queues
    from . import streams
except Exception as e:
    out = io.StringIO()
    sys.print_exception(e, out)
    out.seek(0)
    embed.log(f"asyncio {out.read()}")

from .base_events import *
from .coroutines import *
from .events import *
from .futures import *
from .locks import *
from .protocols import *
from .queues import *
from .streams import *

#from .subprocess import *

from .tasks import *
from .transports import *

# This relies on each of the submodules having an __all__ variable.
__all__ = (base_events.__all__ +
           coroutines.__all__ +
           events.__all__ +
           futures.__all__ +
           locks.__all__ +
           protocols.__all__ +
           queues.__all__ +
           streams.__all__ +
 #          subprocess.__all__ +
           tasks.__all__ +
           transports.__all__)
import os

import builtins

def log_reduce(argv,**kw):
    max = kw.pop('limit',35)
    lim = (max - 5) // 2
    for arg in argv:
        arg = str(arg)
        if lim and len(arg)>max:
            arg = "%s/.../%s" % ( arg[:lim] , arg[-lim:] )

        print(arg,end=' ',file=sys.stderr)
    print('',file=sys.stderr)

def pdb(*argv,**kw):
    sys.stdout.flush()
    sys.stderr.flush()
    if not isinstance(argv[0],str):
        argv=list(argv)
        print('[ %s ]' % argv.pop(0), end=' ', file=sys.stderr)

    try:
        if argv[0].find('%')>=0:
            try:
                print(argv[0] % tuple(argv[1:]),file=sys.stderr)
            except:
                log_reduce(argv,**kw)
    except:
        log_reduce(argv,**kw)

    finally:
        sys.stdout.flush()
        sys.stderr.flush()
builtins.pdb = pdb
from .emscripten_events import *
__all__ += emscripten_events.__all__


auto = 0
# for asyncio tasks
failure = False

# for io errors ( dom==gpu / websocket==socket  )
io_error = False

_event_loop = get_event_loop()


def run_once(*argv,**kw):
    global _event_loop
    _event_loop.call_soon(_event_loop.stop)
    _event_loop.run_forever()

_event_loop.run_once = run_once

def task(t, *argv, **kw):
    global _event_loop
    if _event_loop is None:
        _event_loop = get_event_loop()
    print("about to start %s(*%r,**%r)" % (t.__name__, argv,kw) )
    _event_loop.create_task( t(*argv,**kw) )


def start(*argv):
    global auto, _event_loop

    argv = list(argv)

    # TODO: keep track of the multiple __main__  and handle their loop separately
    # as subprograms

    if auto < 2:
        if _event_loop is None:
            _event_loop = get_event_loop()

        main = getattr( __import__('__main__'), '__main__', None)
        #main = vars().get('__main__')
        if main:
            print("autostarting async __main__")
            _event_loop.create_task( main(0,[]) )

    else:
        print("asyncio.start : event_loop already exists, will not add task __main__ !")

    # TODO: should global argc/argv should be passed ?
    # what about argc/argv conflicts ?
    # shared argparse / usage ?

    while len(argv):
        task(argv.pop(0))

    auto = 2
    #print("asyncio main loop is now running")

def sleep_ms(t):
    return sleep(float(t)/1_000)

def __auto__():
    global _event_loop, auto
    if auto :
        try:
            _event_loop.run_once()
        except Exception as e:
            auto = None
            sys.print_exception(e, sys.stderr)
            print("asyncio, panic stopping auto loop",file=sys.stderr)
    return None
