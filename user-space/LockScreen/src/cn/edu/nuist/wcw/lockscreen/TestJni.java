package cn.edu.nuist.wcw.lockscreen;
import java.util.*;

public class TestJni{
    public native void print(String content);
    public native void brint(String content);
    
    static{
        System.loadLibrary("TestJNI");
    }
}
