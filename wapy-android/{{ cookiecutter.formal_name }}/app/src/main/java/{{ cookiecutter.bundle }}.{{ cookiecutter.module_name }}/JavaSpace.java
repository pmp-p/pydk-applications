package {{ cookiecutter.bundle }}.{{ cookiecutter.module_name }};

import android.util.Log;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Hashtable;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Modifier;


import com.google.gson.Gson;
/* json ... is not well suited for all primitive types : maybe switch to C-cbor later

import jacob.*;

python counter parts

    flynn
    flunn
    cbor https://github.com/brianolson/cbor_py/blob/master/cbor/cbor.py
    cbor2 https://github.com/agronholm/cbor2

*/


// runtime context
class HPyContext {
    public android.content.Context self;
//    public androidx.constraintlayout.widget.ConstraintLayout ui ;
    public android.widget.RelativeLayout ui;
    // not synchronized !
    public Hashtable<String, Object> mem;
}


class JavaSpace {


    Gson gson = new Gson();


    public static String TAG;
    private static HPyContext ctx;


    //public HPyContext new_context(String ns, android.content.Context the_self, androidx.constraintlayout.widget.ConstraintLayout ui){
    public HPyContext new_context(String ns, android.content.Context the_self, android.widget.RelativeLayout ui){
        this.TAG = ns;
        this.ctx = new HPyContext();
        this.ctx.self = the_self;
        this.ctx.ui = ui;
        this.ctx.mem = new Hashtable<String, Object>();
        //this.ctx.mem.put("v:null:0",null);
        return this.ctx;
    }


    // "ffi" like approach

    // this is an attempt to dynamically resolve java namespace Class
    // and static/instance method to apply it starts with the simple ffi base types
    // s/i/f/d/p etc ...
    // then next it will use boxed pointers from stack and object's classname for resolution.

    private Class get_target(String cname,Object instance) {
        if (cname.length()>0)
            try {
                return Class.forName(cname);
            } catch (ClassNotFoundException e) {
                e.printStackTrace();
            }
        return instance.getClass();
    }


    private String ffi_signature(Class<?> pt[], Object argv[]) {
        String sign = "";
        // basic filter
        for (int i = 0; i < argv.length; i++) {

            if (argv[i] instanceof java.lang.Integer) {
                pt[i] = Integer.TYPE;
                sign += "i";
            } else if (argv[i] instanceof java.lang.Double) {
                pt[i] = Double.TYPE;
                sign += "d";
            } else if (argv[i] instanceof java.lang.CharSequence) {
                pt[i] = java.lang.CharSequence.class;
                sign += "s";
            } else if (argv[i] instanceof java.lang.String) {
                pt[i] = java.lang.String.class;
                sign += "s";
            } else {
                sign += "p";
                if (argv[i]!=null)
                    pt[i] = argv[i].getClass();
                else
                    Log.e(TAG,"95: ffi_signature on NULL!");

            }
        }
        return sign;
    }


// https://docs.oracle.com/javase/7/docs/api/java/lang/reflect/Constructor.html#newInstance-java.lang.Object...-

    private int ffi_score(String ffi_sign, Class<?> preq[], Class<?> pdecl[]) {
        int score=0;

        for (int i=0; i<pdecl.length; i++) {

            if (pdecl[i] == null) {
                Log.e(TAG,"    111: invalid signature (" + pdecl[i] + " for " + preq[i]);
                continue;
            }

            score++;
            if (pdecl[i].isAssignableFrom(preq[i])) {
                Log.i(TAG,"    ok ("+ pdecl[i].getName() + ")" + preq[i].getName()) ;
                continue;
            }

            String cn = pdecl[i].getName();
            Character cs = ffi_sign.charAt(i);
            Log.i(TAG, "    no : "+cs+ " != "+ cn);
            score--;
            break;
        }
        return score;
    }


    public Method ffi_method(Class<?> cls, Object instance, String method_name, String ffi_sign, Class<?> preq[]) {
        int best_score = 0;
        List<Method> matches = new ArrayList<Method>();

        if (cls == null)
             cls = instance.getClass();

        for (Method method : cls.getMethods()) {
            Class<?> pdecl[] =  method.getParameterTypes();
            if ((ffi_sign.length() == pdecl.length) && method_name.equals( method.getName() ) ) {
                //Class returnType = method.getReturnType();

                if (pdecl.length==0)
                    return method;

                int score = ffi_score( ffi_sign, preq, pdecl);
                if (score > best_score) {
                    // discard any previous and use that new better one
                    matches.clear();
                    matches.add( method );

                    // is it perfect score ?
                    if (score == preq.length)
                        break;

                } else if (score == best_score) {
                    // possible collision, expect trouble.
                    matches.add( method );
                } else  // not a good one.
                    continue;
            }
        }

        int sz = matches.size() -1 ;
        if (sz == 0 ) {
            Log.i(TAG, "  MATCHED >> "+method_name+"(" + ffi_sign + ")" );
            return matches.get( sz );
        }

        if (sz > 0 ) {
            Log.e(TAG, "  TOO MANY MATCHES >> "+method_name+"(" + ffi_sign +")");
            // TODO: return an exception and ask to dev to clarify argv
           // call with boxed pointers of exact match if needed.
        }

        Log.i(TAG, "  NO MATCH >> "+method_name+"(" + ffi_sign +")");
        return null;
    }


    public Object ffi_call(String cls_name, Object instance, String method_name, Object ...argv) {
        Class<?> cls = get_target(cls_name, instance);

        if (cls_name.equals(""))
            cls_name = cls.getName();

        Class<?> pt[] = new Class[argv.length];

        Object ret_val = "Exception(AttributeError: '%s' object has no attribute '%s')";

        String ffi_sign = ffi_signature(pt,argv);

        Log.i(TAG, "  ffi_call-search >> "+ cls_name +"."+method_name+"('"+ ffi_sign +"')");
//FIXME: convert d=>i here

        Method method =  ffi_method(cls, instance, method_name , ffi_sign, pt);

        if (method==null) {
            Log.e(TAG, "404: ffi_call method not found " +method_name+" in "+cls_name);
            // TODO build an attribute not found exception object and return it
            return ret_val;
        }

        try {
            Class ret_type = method.getReturnType();

            // some members could be static ( regardless of instance being null or not ? )
            if (Modifier.isStatic(method.getModifiers())) {
Log.i(TAG,"staticmethod: " + cls.getName() +"."+method_name+"('"+ ffi_sign +"') =>" + ret_type);
                ret_val = method.invoke(null, argv);
            } else {
                if (instance!=null) {
Log.i(TAG,"instancemethod: (" + cls.getName() +")instance."+method_name+"("+ argv +") => " + ret_type );
                    ret_val = method.invoke(instance, argv);
                } else // that one must raise an exception on remote end.
                    Log.e(TAG, "instance call with null instance !");
            }
        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        }
        catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        catch (IllegalArgumentException e) {
            e.printStackTrace();
        }

        if (ret_val == null)
            Log.i(TAG,"  call = void");
        else
            Log.i(TAG,"  call = " + ret_val.toString());

        return ret_val; // TODO: return exception frame matching the problem.
    }


    // RMI - RPC



    private Object jsctor(HPyContext ctx, Integer serial, Integer ct, String cn, String method, Object[] argv){
        Log.v(TAG, "js.ctor #" + serial +" "+cn+" : "+argv);

        Object instance = null;
        try{
            Class cls = Class.forName(cn);
            java.lang.reflect.Constructor[] ctors = cls.getDeclaredConstructors();
            java.lang.reflect.Constructor ctor = null;

            for (int i = 0; i < ctors.length; i++) {
                ctor = ctors[i];
Log.e(TAG, "//FIXME: check also types not just len !");
                if (ctor.getGenericParameterTypes().length == argv.length)
                break;
            }

            if (ctor!=null){
                instance = ctor.newInstance(argv);

            } else {
                //  no args
                instance = cls.newInstance();
            }

        }
        catch(Exception e){
            //App("error", ""+e);
            e.printStackTrace();
        }

        return instance;
    }

    private Object convert_one(HPyContext ctx, Object arg) {
        Object elem = arg;
        if (arg instanceof java.lang.String) {
            String str = arg.toString();
            if (str.equals("p:self"))
                elem = this.ctx.self;
            else if (str.equals("p:ui"))
                elem = this.ctx.ui;
            else if (str.equals("v:null:0"))
                elem = null;
            else {
                elem = this.ctx.mem.get(str);
                if (elem == null )
                    elem = str;
                //Log.i(TAG,"280:call_stack => " + elem ) ;
            }
        } else if (arg instanceof java.lang.Double) {
//#FIXME: see 197 !
            int i = (int)Math.round((java.lang.Double)arg);
            elem = i;
        }
        return elem;
    }

    private void convert_call_stack(HPyContext ctx, Object[] args, Class<?> pt[], Object stack[]) {
        int i = 0;
        for (Object arg:args) {
            stack[i] = convert_one(ctx, arg);
            if (stack[i] != null )
                pt[i] = stack[i].getClass();
            i++;
        }
    }

//identityHashCode
    private String PyCall(HPyContext ctx, ArrayList al) {
        Integer serial = ((Double)al.remove(0)).intValue();
        Integer ct = ((Double)al.remove(0)).intValue();
        String cn = al.remove(0).toString();
        String method = al.remove(0).toString();

        Object retval = null;

        String retser[] = new String[2];
        retser[0] = serial.toString();


        if (ct==0) {

            Class<?> pt[] = new Class[al.size()];
            Object[] stack =  new Object[al.size()];

            // get boxed pointer ref from strings and replace them by ref in the stack
            convert_call_stack(ctx, al.toArray(), pt, stack);
//-------------------------------------------------
            /*
int i=0;
for (Object o:stack)
    Log.e(TAG, "CALL STACK : " + ct + " on "+ cn+"."+method+" : " + al.toArray()[i++] +  " <=> "+ o);
            */
//-------------------------------------------------
            retval = jsctor(ctx, serial, ct, cn, method, stack);

        } else if (ct==1) {
            Object instance = convert_one(ctx, al.remove(0) );
            Class<?> pt[] = new Class[al.size()];
            Object[] stack =  new Object[al.size()];
            convert_call_stack(ctx, al.toArray(), pt, stack);
            Log.e(TAG, "ffi fq call : " + ct + " on "+ cn+"(instance)."+method);
            retval = ffi_call(cn, instance, method, stack );

        } else if (ct==2) {
            Object instance = convert_one(ctx, cn );
            Class<?> pt[] = new Class[al.size()];
            Object[] stack =  new Object[al.size()];
            convert_call_stack(ctx, al.toArray(), pt, stack);
            Log.e(TAG, "ffi instance call : " + ct + " on "+instance +"."+method);
            retval = ffi_call("", instance, method, stack );

        } else
            Log.e(TAG, "unknow call type : " + ct + " on "+ cn+"."+method);

        if (retval != null ) {

            String sign;
            String rcn;
            String ptr;

            if (retval instanceof java.lang.Integer) {
                ptr = Integer.toString((int)retval);
                sign = "i";
                rcn = "int";
            } else if (retval instanceof java.lang.String) {
                ptr = ""+retval;
                sign = "s";
                rcn = "str";
            // order ?
            } else if (retval instanceof java.lang.CharSequence) {
                ptr = ""+retval;
                sign = "s";
                rcn = "str";
//                sign = "b";
//                rcn = "bytes";
            } else if (retval instanceof java.lang.Float) {
                ptr = Double.toString((double)retval);
                sign = "f";
                rcn = "float";
            } else if (retval instanceof java.lang.Double) {
                ptr = Double.toString((double)retval);
                sign = "d";
                rcn = "float";
            } else {
                // box pointer for future use and prevent GC until discarded.
                sign = "p";
                rcn = retval.getClass().getName();
                ptr = Integer.toHexString(System.identityHashCode(retval));
            }

            retser[1] = sign + ":" + rcn + ":" + ptr;

            if (sign.equals("p")) {
                this.ctx.mem.put( retser[1], retval );
                Log.i(TAG,"RESULT #"+retser[0]+" stored,need GC : " + retser[1]);
            } else {
                Log.i(TAG,"RESULT #"+retser[0]+" primitive type : " + retser[1]);
            }

        } else {
            // nullptr => None
            retser[1] = "v:null:0";
            Log.i(TAG,"RESULT #"+retser[0]+" v:null:0 -> None");
        }
        return gson.toJson(retser);
    }


    public String[] Calls(ArrayList cs) {
        String rv[] = new String[cs.size()] ;
        int i=0;
        for (Object al:cs){
            rv[i] = this.PyCall(this.ctx, (ArrayList) al);
//Log.i(MainActivity.TAG,"asyncresult : " + rv[i] );
            i++;
        }
Log.i(MainActivity.TAG,"/plink");
        return rv;
    }

}


/*

ByteString.copyFrom(byte[] bytes)
ByteString.of(byteArray)



        for (java.lang.reflect.Field f : getClass().getDeclaredFields())
            App("alert",f.getType().getName(),f.getName());

        java.lang.reflect.Field field;
        try {
            field = getClass().getField("self");
            App("alert",field.getType().getName(),field.getName());

        } catch (NoSuchFieldException e) {
            Log.e(TAG, "NoSuchFieldException");
        };

        //android.content.Context arg0 = field.get(self);

*/
