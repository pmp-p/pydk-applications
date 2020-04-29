instance = None

def send(io,skip=()):
    io.seek(0)
    i = 0
    for data in io.read().replace('\r\n','\n').split('\n'):
        if not i in skip:
            instance.ws.send_binary( f"PRIVMSG {instance.channels[0]} :{data}\r\n".encode("utf-8"))
        i+=1
    io.close()


def out(*o, **kw):
    global instance
    if instance:
        out = StringIO()
        kw["file"] = out
        kw["end"] = "\n"
        print(*o, **kw)
        send(out)

def err(e, skip=()):
    global instance
    if instance:
        out = StringIO()
        sys.print_exception(e, out=out)
        send(out, skip)

import embed
from io import StringIO
import python3.aio.irc.client

import ast
import types
import inspect
from codeop import CommandCompiler
do_compile = CommandCompiler()
do_compile.compiler.flags |= ast.PyCF_ALLOW_TOP_LEVEL_AWAIT

class client(python3.aio.irc.client.client):
    async def handler(handler_self, cmd):
        global self
        if cmd[1].isnumeric():
            # server notice
            return True

        if cmd[1].upper() == 'PRIVMSG':
            code = ' '.join( cmd[3:])[1:].rstrip('\r\n')

            code_with_rv = f'_ = {code}'

            try:
                bytecode = do_compile(code_with_rv, "<stdin>", "exec")
                code = code_with_rv
            except:
                try:
                    bytecode = do_compile(code, "<stdin>", "exec")
                except Exception as e:
                    embed.log(f"REPL: code> {code}")
                    embed.log(f"REPL:Error> {e}")
                    err(e, skip=(1,2,))
                return True

            try:
                func = types.FunctionType(bytecode, self.__dict__)
                maybe = func()
                if inspect.iscoroutine(maybe):
                    maybe = await maybe
                    embed.log(f"AIO>> {self._}")
                    out(f"AIO>> {self._}")
                else:
                    embed.log(f"REPL> {self._}")
                    out(f"REPL> {self._}")
                return True
            except Exception as e:
                embed.log(f"REPL: code> {code}")
                embed.log(f"REPL:Error> {e}")
                err(e, skip=(1,2,))

        print("15:",__file__,'app handler>')
        print(repr(cmd))
