import os
import embed
import time as Time
import builtins


# will be a websocket debugger
if 0:
    from . import ControlChannel as cc

    if os.path.exists('/data/data/board'):
        #home sweet home (h3droid boards)
        ccip = "192.168.0.254"
    else:
        ccip = "82.65.46.75"

    cc.instance = cc.client(ccip, 6667, nick="apk_" + hex(int(str(Time.time()).replace(".", "")))[2:], channels=["#android"])

    aio.service(cc.instance)

    _ = None

    builtins.pdb = cc.out
    pdb("Starting panda3d on", view)


import panda3d

import panda3d.core as p3d

#p3d.loadPrcFileData("", "load-display pandagles2")
p3d.loadPrcFileData("", "win-origin -2 -2")
p3d.loadPrcFileData("", "win-size 640 480")
p3d.loadPrcFileData("", "support-threads #f")
p3d.loadPrcFileData("", "textures-power-2 down")
p3d.loadPrcFileData("", "textures-square down")
p3d.loadPrcFileData("", "show-frame-rate-meter #t")


import direct
import direct.task
import direct.task.TaskManagerGlobal

import direct.showbase
from direct.showbase.ShowBase import ShowBase



from .geom3d import *

class AsyncApp:
    instances = []

    @classmethod
    def newInstance(cls):
        cls.instances.append( cls() )



class AsyncPanda3D(ShowBase, AsyncApp):

    frame_time = 1.0 / 60

    async def async_loop(self):
        self.build()
        aio.loop.create_task( self.update() )
        while not aio.loop.is_closed():
            try:
                direct.task.TaskManagerGlobal.taskMgr.step()
                #embed.step()
                #await aio.asleep(self.frame_time)
            except SystemExit:
                print('87: Panda3D stopped',file= __import__('sys').stderr)
                break
            await aio.asleep(self.frame_time)

    @classmethod
    def async_run(cls):
        for instance in cls.instances:
            aio.loop.create_task( instance.async_loop() )

    # patch the sync run from showbase which would prevent to enter interactive mode
    run = async_run


    # build

    # update



class Application( AsyncPanda3D):

    # add some colored cubes

    def build(self):
        base.cam.reparent_to(render)
        from random import random

        cube = Cube(size=1.0)

        cubes = render.attachNewNode("cubes")
        cubes.set_pos(0, 0, 0)

        for x in range(5):
            for y in range(5):
                for z in range(5):
                    instance = cube.copyTo(cubes)
                    instance.setPos(x - 2, y - 2, z - 2)
                    instance.setColor(random(), random(), random(), 1)

        base.cam.set_pos(16, 12, 30)
        base.cam.look_at(0, 0, 0)

        self.cubes = cubes

    # cube spinner

    async def update(self, dt=0):
        while not aio.loop.is_closed():
            group = self.cubes
            h, p, r = group.get_hpr()
            d = .5
            group.setH(h + d)
            group.setP(p + d)
            group.setY(r + d)
            await aio.asleep(self.frame_time)


























if 0:






    # Button, Checkbutton, Entry, Frame, Label, LabelFrame
    # Menubutton, PanedWindow, Radiobutton, Scale, Scrollbar, and Spinbox.
    # The other six are new: Combobox, Notebook, Progressbar, Separator, Sizegrip and Treeview.





    from python3.aio import plink

    android = plink.android
    #androidx = plink.androidx
    this = plink.this
    layout = plink.layout

    widgets = plink.android.widgets

    # LayoutParams.WRAP_CONTENT == -2
    async def setPos(view, left, top, right, bottom):
        global android, layout


        async with android.widget.RelativeLayout__LayoutParams.newInstance(-2,-2) as params:
            params.setMargins( left, top, right, bottom)
        await layout.addView(view, params)

    def set_text(wdg, data):
        getattr(wdg,"setText")(data)
        getattr(wdg,"setWidth")(len(data)*Widgets.cell)


    # tag using the hex java pointer memory address string converted to int
    def tag(wdg,target=None,handler=None, hint=""):
        global tags
        tag_id = int( str(wdg).rsplit(':',1)[-1], 16 )
        wdg.setId( tag_id )
        #tags[tag_id] = [wdg, target or wdg, handler, hint or str(tag_id)]
        Events.ld[tag_id] = [wdg, target or wdg, handler, hint or str(tag_id)]

    class Widgets:
        cell = 11

    class Events:
        ld = {}

    class Button(Events,Widgets):
        pass

    class ButtonRow(Button):
        y = 2 * Widgets.cell
        spacing = 1 * Widgets.cell
        x = spacing
        width = 0

        def __init__(self):
            cls = ButtonRow
            cls.width = cls.cell * len(self.text)

        @staticmethod
        def left():
            return ButtonRow.x

        @staticmethod
        def right(decal=0):
            cls = ButtonRow
            try:
                cls.x += cls.width + cls.spacing
                return cls.x
            finally:
                if decal:
                    cls.x += (4+decal)*cls.cell + cls.spacing
                cls.width = 0


    class print_members(ButtonRow):
        text = "Print Panda3D members"
        async def onclick(self, this, target, hint):
            cc.out("received event from hint=",hint,'via',this)
            await target.setText("Panda3D starting ...")
            try:
                try_me()
                await target.setText("Panda3D running ...")
            except Exception as e:
                await target.setText(repr(e))

    class hello_python(ButtonRow):
        text = "say hi on irc"
        async def onclick(self, this, target, hint):
            cc.out(await target.getText(),"via", this)


    def add(ct):
        return getattr( android.widget, ct ).newInstance(this)


    async def uinput(cls):
        global widgets

        async with add('Button') as button, add('EditText') as text:
            set_text(text, cls.text)
            button.setText(f"↲")
            button.setOnClickListener(this)

            i=0
            tag(button, text, cls().onclick, f"{i}")

            await setPos(text,  cls.left(), cls.y, 0, 0)
            await setPos(button, cls.right(2), cls.y, 0, 0)


    async def __main__():
        global self, this, cc, android, layout, view

        self = sys.modules['Applications.MainActivity']
        cc.self =  self



        await uinput(hello_python)

        #ButtonRow.y += 22
        await uinput(print_members)

        async with this.make_window() as view:
            await setPos(view, 150, 100, 150, 100)




