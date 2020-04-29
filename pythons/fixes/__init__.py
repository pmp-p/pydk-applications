import sys
import builtins
builtins.sys = sys

def fixme(module):
    module = '%s.%s' % ( __name__ , module )
    __import__( module )
    module = eval(module)
    module.__file__ = '<fixes>'
    module.__name__ = module.__name__.rsplit('.',1)[-1]
    sys.modules[module.__name__] = module
    setattr( builtins, module.__name__, module )

if sys.version_info[:2]<(3,7):
    # running micropython for a sim
    import stupyde
    fixes = ('micropython','machine','contextlib','contextvars',)
else:
    import traceback
    # running cpython
    def print_exception(e, out=sys.stderr, **kw):
        kw["file"] = out
        traceback.print_exc(**kw)

    sys.print_exception = print_exception

    import pythons
    import pythons.fixes as fixes
    pythons.fixes = fixes
    fixme('utime')
    print(utime,sys.modules['utime'])
    fixes = ('micropython','machine','contextvars',)  #contextvars is unsupported on micropython


for fix in fixes:
    fixme(fix)


