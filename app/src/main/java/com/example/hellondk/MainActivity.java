package com.example.hellondk;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

// 基本类型：boolean,byte,char,short,int,long,float,double
// 引用类型：String,arrays,classes
public class MainActivity extends AppCompatActivity {
    /** Threads edit. */
    private EditText threadsEdit;

    /** Iterations edit. */
    private EditText iterationsEdit;

    /** Start button. */
    private Button startButton;

    /** Log view. */
    private TextView logView;

    // Used to load the 'native-lib' library on application startup.
    static {
        // 加载共享库
        System.loadLibrary("native-lib");
        // System.load()
        System.loadLibrary("threads-lib");
        System.loadLibrary("hook-lib");
    }

    /**
     * 使用基于Java的线程
     *
     * @param threads thread count.
     * @param iterations iteration count.
     */
    private void javaThreads(int threads,final int iterations){
        // 为每一个worker创建一个基于Java的线程
        for(int i=0;i<threads;i++){
            final int id = i;

            Thread thread = new Thread(){
                // 重载java.lang.Thread类的run方法
                public void run(){
                    nativeWorker(id,iterations);
                }
            };

            thread.start();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // 初始化原生代码
        nativeInit();

        threadsEdit = findViewById(R.id.threads_edit);
        iterationsEdit = findViewById(R.id.iterations_edit);
        startButton = findViewById(R.id.start_button);
        logView = findViewById(R.id.log_view);
        char defaultThreads[] = new char[2];
        defaultThreads[0]='1';
        defaultThreads[1]='0';
        threadsEdit.setText(defaultThreads,0,2);
        setNumber(iterationsEdit,520);

        startButton.setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                int threads = getNumber(threadsEdit,0);
                int iterations = getNumber(iterationsEdit,0);

                if(threads>0&&iterations>0){
                    startThreads(threads,iterations);
                }
            }
        });

        // Example of a call to a native method
        // 找到文本视图，并设置其内容
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        // 调用方法
        Button button = findViewById(R.id.button);
        button.setText("Call Native");
        button.setOnClickListener(new OnClickListener() {
            public void onClick(View button){
                ((Button)button).setText(stringFromJNI());
            }
        });
        // /data/app/com.example.hellondk--1cZcnIU6dpdXKZkFgoTXA==/lib/arm64
        String libPath = getApplicationContext().getApplicationInfo().nativeLibraryDir;

        JavaClass clazz = new JavaClass();
        clazz.foo();

        stringOperations();
        int[] intArray = arrayOperations();
        if(null!=intArray){
            Log.i("Log","intArray[0]: " + intArray[0]);
        }

        NIOOperations();
        // loggingFunctions();
        dynamicMMC();
        dynamicMMCPP();
        executeShellCmd();
        communicatingWithChild();
        systemConfiguration();

        newObject();

        /*print();
        Unhook();
        print();*/
    }

    @Override
    protected void onDestroy(){
        nativeFree();

        super.onDestroy();
    }

    /**
     * 原生消息回调
     *
     * @param message
     *      原生消息.
     */
    private void onNativeMessage(final String message){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                logView.append(message);
                logView.append("\n");
            }
        });
    }

    /**
     * 以integer格式获取编辑文本的值
     * 如果值为empty或技术不能分析，则返回默认值
     *
     * @param editText edit value.
     * @param defaultValue default value.
     * @return numeric value.
     */
    private static int getNumber(EditText editText,int defaultValue){
        int value;

        try {
            value = Integer.parseInt(editText.getText().toString());
        } catch (NumberFormatException e) {
            value = defaultValue;
        }

        return value;
    }

    /*private static void setNumber(EditText editText,int value){
        editText.setText(Integer.toString(value));
    }*/



    /**
     * 启动给定个数的线程进行迭代
     *
     * @param threads thread count.
     * @param iterations iteration count.
     */
    private void startThreads(int threads,int iterations){
        long start,end,time;
        start = System.nanoTime();
        javaThreads(threads,iterations);
        end = System.nanoTime();
        time = end-start;
        Log.i("Log","time: "+time);

        start = System.nanoTime();
        posixThreads(threads,iterations);
        end = System.nanoTime();
        time = end-start;
        Log.i("Log","time: "+time);
    }

    /**
     * 原生worker.
     *
     * @param id worker id
     * @param iterations iteration count
     */
    private native void nativeWorker(int id,int iterations);

    private native void nativeInit();

    private native void nativeFree();

    /**
     * 使用 POSIX线程
     *
     * @param threads thread count.
     * @param iterations iteration count.
     */
    private native void posixThreads(int threads,int iterations);

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    // 声明 Native 方法
    public native String stringFromJNI();
    private native static void setNumber(EditText editText,int value);


    // atl+enter 快速创建native method 定义
    public native void stringOperations();
    public native int[] arrayOperations();
    public native void NIOOperations();
    private native void loggingFunctions();
    private native void dynamicMMC();
    private native void dynamicMMCPP();
    private native void executeShellCmd();
    private native void communicatingWithChild();
    private native void systemConfiguration();

    public native void newObject();

    public native void print();
    public native void Unhook();
}