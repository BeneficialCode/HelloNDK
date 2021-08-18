package com.example.hellondk;


import android.util.Log;

public class JavaClass {
    // Java Class with Both Staic and Instance Fields
    /** Instance filed */
    // 实例域
    private String instanceField = "Instance Field";

    /** Static filed */
    // 静态域
    private static String staticField = "Static Field";

    // Java Class with Both Instance and Static Methods
    /**
     * Instance method
     */
    private String instanceMethod(){
        return "Instance Method";
    }

    /**
     * Static method
     */
    private static String staticMethod(){
        return "Static Method";
    }

    /**
     * Throwing method
     */
    private void throwingMethod() throws NullPointerException{
        throw new NullPointerException("Null pointer");
    }

    private void doSomething(){
        Object obj = this;
        // Java Synchronized Code Block
        synchronized (obj){

        }
        sMethod();
    }

    public void foo(){
        accessingFields();
        callingMethods();
        accessMethods();
        globalReferences();
        synchronization();
    }

    private static native void sMethod();

    private native void accessMethods();

    // 实例类访问自己的域，需要是自己的原生函数
    public native void accessingFields();

    public native void callingMethods();

    private native void globalReferences();

    private native void synchronization();
}
