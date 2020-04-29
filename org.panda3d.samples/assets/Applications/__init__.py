# https://developer.android.com/guide/components/activities/activity-lifecycle

# https://northstreetcreative.com/notes/full-screen-website-zoom-using-css-transform-and-jquery/




#========================== APP INTERFACE =================================
# as a class or module : to your linking
# juste don't change names as they are aosp documented, or write new doc ;)





async def onCreate(self, pyvm):
    print("onCreate", pyvm)

    from .MainActivity import Application

    Application.newInstance() # in case of panda3D though only one showbase is allowed.


async def onStart(self, pyvm):
    print("onStart", self, pyvm)

    from .MainActivity import Application
    Application.async_run()




def onPause(self, pyvm):
    print("onPause", self, pyvm)


def onResume(self, pyvm):
    print("onResume",self, pyvm)


def onStop(self, pyvm):
    print("onStop", self, pyvm)


async def onDestroy(self, pyvm):
    print("onDestroy", self, pyvm)


print('Applications ready')


