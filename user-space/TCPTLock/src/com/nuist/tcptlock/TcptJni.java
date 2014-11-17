package com.nuist.tcptlock;
import java.util.*;

public class TcptJni {
    public static native int reg(String pwd);
    public static native int auth(String pwd);
    public static native int clearkey();
    public static native String dosomething(String str);
    static {
        System.loadLibrary("TcptJni");
    }
}
