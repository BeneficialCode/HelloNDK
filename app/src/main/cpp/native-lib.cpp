#include <jni.h>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/system_properties.h>
// Native Logging APIs
#include <android/log.h>
#include "my_log.h"

bool GetAllModuleName(pid_t pid);

// 语言层，包名，类名，方法名
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_hellondk_MainActivity_stringFromJNI(
        JNIEnv* env,    // the gateway to access various JNI functions
                        // 线程局部存储的
        jobject thiz) { // 取决于静态方法还是实例方法，实例引用

    MY_LOG_VERBOSE("The stringFromJNI is called.");

    MY_LOG_DEBUG("env=%p this=%p",env,thiz);

    MY_LOG_ASSERT(nullptr!=env,"JNIEnv cannot be NULL.");

    MY_LOG_INFO("Returning a new string.");

    uid_t uid;

    uid = getuid();

    MY_LOG_INFO("Application User ID is %u",uid);

    gid_t gid;
    gid = getgid();
    MY_LOG_INFO("Application Group ID is %u",gid);

    char* username;
    username = getlogin();

    MY_LOG_INFO("Application user name is %s",username);

    pid_t pid = getpid();

    // GetAllModuleName(pid);

    std::string hello = "Hello from C++";

    return env->NewStringUTF(hello.c_str());
}

// Logging a Message by Using the Variable Number of Parameters That Are Passed In
void log_verbose(const char* format,...){
    va_list args;
    va_start(args,format);
    __android_log_vprint(ANDROID_LOG_VERBOSE,"hello-jni",format,args);
    va_end(args);
}

void example(){
    log_verbose("Error is now %d",errno);
}

bool GetAllModuleName(pid_t pid){
    char szMapFilePath[2048];
    char* pszFullName;
    sprintf(szMapFilePath,"/proc/%d/maps",pid);
    FILE* fp = fopen(szMapFilePath,"r");
    if(fp!=NULL){
        while(fgets(szMapFilePath,1023,fp)!=NULL){
            pszFullName = strchr(szMapFilePath,'/');
            if(pszFullName==NULL){
                continue;
            }
            MY_LOG_INFO("%s",pszFullName);
        }
    }
    fclose(fp);
    return true;
}



// 原生静态方法
// 类引用
extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_JavaClass_sMethod(JNIEnv *env, jclass clazz) {
    // TODO: implement staticMethod()

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_stringOperations(JNIEnv *env, jobject thiz) {
    // TODO: implement usingString()
    /* 字符串操作 */
    // New String 创建字符串
    jstring javaString = env->NewStringUTF("Hello World!");

    // Converting a Java String to C String
    // 把java字符串转换成C字符串
    const char* str;
    jboolean isCopy;

    str = env->GetStringUTFChars(javaString,&isCopy);
    if(nullptr!=str){
        printf("Java string: %s",str);
        if(JNI_TRUE == isCopy) {
            MY_LOG_INFO("C string is a copy of the Java string");
        }else{
            MY_LOG_INFO("C string points to actual string.");
        }
    }
    // 释放字符串
    env->ReleaseStringUTFChars(javaString,str);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_JavaClass_accessingFields(JNIEnv *env, jobject instance) {
    // TODO: implement accessingFields()
    /*获取域ID*/
    // Getting the class from an Object Reference
    // 用对象引用获取类
    jclass clazz;
    // 类描述符
    clazz = env->FindClass("com/example/hellondk/JavaClass");
    // clazz = env->GetObjectClass(instance);

    // Getting the Field ID of an Instance Field
    jfieldID instanceFieldId;
    // 域描述符
    instanceFieldId = env->GetFieldID(clazz, "instanceField", "Ljava/lang/String;");

    // 获取静态域的域ID
    jfieldID staticFieldId;
    // 域类型是String
    staticFieldId = env->GetStaticFieldID(clazz, "staticField", "Ljava/lang/String;");

    jstring instanceField;
    instanceField = static_cast<jstring>(env->GetObjectField(instance, instanceFieldId));

    jstring staticField;
    staticField = static_cast<jstring>(env->GetStaticObjectField(clazz, staticFieldId));
    const char* content = nullptr;

    content = env->GetStringUTFChars(instanceField,nullptr);
    MY_LOG_INFO("jni->%s",content);
    env->ReleaseStringUTFChars(instanceField,content);

    content = env->GetStringUTFChars(staticField,nullptr);
    MY_LOG_INFO("jni->%s",content);
    env->ReleaseStringUTFChars(staticField,content);

    jstring newValue = env->NewStringUTF("modify instance field.");
    env->SetObjectField(instance,instanceFieldId,newValue);

    instanceField = static_cast<jstring>(env->GetObjectField(instance, instanceFieldId));
    content = env->GetStringUTFChars(instanceField,nullptr);
    MY_LOG_INFO("jni->%s",content);
    env->ReleaseStringUTFChars(instanceField,content);

    jstring stringValue = env->NewStringUTF("modify by jni");
    env->SetStaticObjectField(clazz,staticFieldId,stringValue);

    staticField = static_cast<jstring>(env->GetStaticObjectField(clazz,staticFieldId));
    content = env->GetStringUTFChars(staticField,nullptr);
    MY_LOG_INFO("jni->%s",content);
    env->ReleaseStringUTFChars(staticField,content);

}

/*数组操作*/
extern "C"
JNIEXPORT jintArray JNICALL
Java_com_example_hellondk_MainActivity_arrayOperations(JNIEnv *env, jobject thiz) {
    // TODO: implement arrayOperations()
    // New Array
    // 创建数组
    jintArray javaArray;
    javaArray = env->NewIntArray(10);
    if(nullptr!=javaArray){
        /*You can now use the array.*/
        jsize length = env->GetArrayLength(javaArray);

        // Getting a Copy of Java Array Region as a C Array
        jint nativeArray[10];
        env->GetIntArrayRegion(javaArray,0,10,nativeArray);

        nativeArray[0]=10;
        // Committing Back the Changes from C Array to Java Array
        env->SetIntArrayRegion(javaArray,0,10,nativeArray);
        // Getting a Direct Pointer to Java Array Elements
        jint* nativeDirectArray;
        jboolean isCopy;
        nativeDirectArray = env->GetIntArrayElements(javaArray,&isCopy);
        for(int i=0;i<length;i++){
            MY_LOG_INFO("array[%d]:%d",i,nativeDirectArray[i]);
        }
        env->ReleaseIntArrayElements(javaArray,nativeDirectArray,0);
        return javaArray;
    }
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_NIOOperations(JNIEnv *env, jobject thiz) {
    // TODO: implement NIOOperations()
    // New Byte Buffer Based on the Given C Byte Array
    unsigned char* buffer = (unsigned char*)malloc(1024);
    jobject directBuffer;
    directBuffer = env->NewDirectByteBuffer(buffer,1024);

    // Getting the Native Byte Array from the Java Buffer
    unsigned char* p=nullptr;
    p = (unsigned char*)env->GetDirectBufferAddress(directBuffer);
    MY_LOG_INFO("buffer->%p,p->%p",buffer,p);
    free(buffer);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_JavaClass_callingMethods(JNIEnv *env, jobject instance) {
    // TODO: implement callingMethods()
    jclass clazz = env->GetObjectClass(instance);
    // Getting the Method ID of an Instance Method
    jmethodID instanceMethodId;
    // 函数描述符
    instanceMethodId = env->GetMethodID(clazz,"instanceMethod","()Ljava/lang/String;");

    // Getting the Method ID of an Static Method
    jmethodID staticMethodId;
    staticMethodId = env->GetStaticMethodID(clazz,"staticMethod","()Ljava/lang/String;");

    // Calling the Instance Method
    jstring instanceMethodResult;
    instanceMethodResult = static_cast<jstring>(env->CallObjectMethod(instance,instanceMethodId));

    const char* str=nullptr;
    str = env->GetStringUTFChars(instanceMethodResult,nullptr);
    MY_LOG_INFO("instanceMethodResult: %s",str);
    env->ReleaseStringUTFChars(instanceMethodResult,str);

    // Calling the Static Method
    jstring staticMethodResult;
    staticMethodResult = static_cast<jstring>(env->CallStaticObjectMethod(clazz,staticMethodId));
    str = env->GetStringUTFChars(staticMethodResult,nullptr);
    MY_LOG_INFO("staticMethodResult: %s",str);
    env->ReleaseStringUTFChars(staticMethodResult,str);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_JavaClass_accessMethods(JNIEnv *env, jobject instance) {
    // TODO: implement accessMethods()
    jclass clazz = env->GetObjectClass(instance);

    jmethodID  throwingMethodId;
    throwingMethodId = env->GetMethodID(clazz,"throwingMethod", "()V");

    jthrowable ex;
    env->CallVoidMethod(instance,throwingMethodId);
    ex = env->ExceptionOccurred();
    if(nullptr!=ex){
          env->ExceptionClear();
          MY_LOG_INFO("Exception occurred!");
          /* Exception handler. */
    }
    // 返回的是局部引用，函数返回后会被自动释放
    clazz = env->FindClass("java/lang/NullPointerException");
    if(nullptr!=clazz){
        env->ThrowNew(clazz,"Exception Message");
        ex = env->ExceptionOccurred();
        if(nullptr!=ex){
            env->ExceptionClear();
            MY_LOG_INFO("Exception occurred!");
            /* Exception handler. */
        }
    }
}

// 静态注册
extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_JavaClass_globalReferences(JNIEnv *env, jobject thiz) {
    // TODO: implement globalReferences()
    // 创建全局引用,全局引用可以被其他函数及原生线程使用
    jclass localClazz;
    jclass globalClazz;

    localClazz = env->FindClass("java/lang/String");
    globalClazz = static_cast<jclass>(env->NewGlobalRef(localClazz));
    // 2.删除全局引用
    env->DeleteGlobalRef(globalClazz);

    // 创建弱全局引用
    jclass weakGlobalClazz;
    weakGlobalClazz = static_cast<jclass>(env->NewWeakGlobalRef(localClazz));

    // 检验弱全局变量是否仍然有效
    if(JNI_FALSE == env->IsSameObject(weakGlobalClazz,nullptr)){
        MY_LOG_INFO("Object is still live and can be used.");
    }else{
        MY_LOG_INFO("Object is garbage collected and cannot be used.");
    }

    // 删除弱全局引用
    env->DeleteWeakGlobalRef(weakGlobalClazz);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_JavaClass_synchronization(JNIEnv *env, jobject obj) {
    // TODO: implement synchronization()
    // Native Equivalent of Java Synchronized Code Block
    if(JNI_OK == env->MonitorEnter(obj)){
        // Error handling.
    }

    // Synchronized thread-safe code block.
    if(JNI_OK == env->MonitorExit(obj)){
       //  Error handling
    }
}

static void* nativeThreads(void* args) {
    // Attaching and Detaching the Current Thread to the Virtual Machine
    JavaVM* cachedJvm=nullptr;
    JNIEnv* env;
    cachedJvm->AttachCurrentThread(&env,nullptr);
    /* Thread can communication with the Java application
     * using the JNIEnv interface. */

    /* Detach the current thread from virtual machine. */
    cachedJvm->DetachCurrentThread();
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_loggingFunctions(JNIEnv *env, jobject thiz) {
    // TODO: implement loggingFunctions()
    // Logging a Simple Message
    __android_log_write(ANDROID_LOG_WARN,"hello-jni","Warning log.");

    // Logging a Formatted Message
    __android_log_print(ANDROID_LOG_ERROR,"Hello-jni","Failed with errno %d",errno);

    // Logging an Assertion Failure
    if(0!=errno){
        __android_log_assert("0!=errno","hello-jni","There is an error");
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_dynamicMMC(JNIEnv *env, jobject thiz) {
    int* dynamicIntArray = (int*)malloc(sizeof(int)*16);
    if(nullptr==dynamicIntArray){
        MY_LOG_INFO("Unable to allocate enough memory.");
    }else{
        int* newDynamicIntArray = (int*)realloc(dynamicIntArray,
                                                sizeof(int)*32);
        if(nullptr==newDynamicIntArray) {
            MY_LOG_INFO("Unable to reallocate enough.");
        }else{
            dynamicIntArray = newDynamicIntArray;
        }
        *dynamicIntArray=0;
        dynamicIntArray[8]=8;
        free(dynamicIntArray);
        dynamicIntArray = nullptr;
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_dynamicMMCPP(JNIEnv *env, jobject thiz) {
    int* dynamicInt = new int;
    if(nullptr==dynamicInt){
        MY_LOG_INFO("Unable to allocate enough memory.");
    }else{
        *dynamicInt = 0;
        delete dynamicInt;
        dynamicInt = nullptr;
    }

    int* dynamicIntArray = new int[16];
    if(nullptr==dynamicIntArray){
        MY_LOG_INFO("Unable to allocate enough memory.");
    }else{
        dynamicIntArray[8]=8;
        delete[] dynamicIntArray;
        dynamicIntArray = nullptr;
    }

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_executeShellCmd(JNIEnv *env, jobject thiz) {
    int result;
    result = system("mkdir /data/data/com.example.hellondk/tmp");
    if(-1==result||127==result){
        MY_LOG_INFO("Execute of the shell failed.");
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_communicatingWithChild(JNIEnv *env, jobject thiz) {
    FILE* stream;
    stream = popen("ls","r");
    if(nullptr==stream){
        MY_LOG_ERROR("Unable to execute the command.");
    }else{
        char buffer[1024];
        int status;

        while(nullptr!=fgets(buffer,1024,stream)){
            MY_LOG_INFO("read: %s",buffer);
        }

        status = pclose(stream);
        MY_LOG_INFO("process exited with status %d",status);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_systemConfiguration(JNIEnv *env, jobject thiz) {
    char value[PROP_VALUE_MAX];
    if(0==__system_property_get("ro.product.model",value)){
        MY_LOG_ERROR("System property is not found or it has an empty value.");
    }else{
        MY_LOG_INFO("produce model: %s",value);
    }

    const prop_info* property;
    property = __system_property_find("ro.product.model");
    if(nullptr==property){
        MY_LOG_ERROR("System property is not found.");
    }
    else{
        char name[PROP_NAME_MAX];
        if(0==__system_property_read(property,name,value)) {
            MY_LOG_INFO("property is empty.");
        }else{
            MY_LOG_INFO("%s: %s",name,value);
        }
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_newObject(JNIEnv *env, jobject thiz) {
    jclass clazz = env->FindClass("com/example/hellondk/JavaClass");
    jmethodID methodId = env->GetMethodID(clazz,"<init>","()V");
    jobject newObj = env->NewObject(clazz,methodId);

    if(newObj!=nullptr){
        MY_LOG_INFO("new object success!");
    }
    jobject allocObj = env->AllocObject(clazz);
    env->CallNonvirtualVoidMethod(allocObj,clazz,methodId);

    if(allocObj!=nullptr){
        MY_LOG_INFO("alloc object success!");
    }



}

__attribute__ ((visibility ("hidden"))) void setNumber(JNIEnv *env, jclass clazz, jobject edit_text,
                                                 jint value) {
    jclass editClazz = env->GetObjectClass(edit_text);
    // editText.setText(Integer.toString(value));
    jmethodID methodId = env->GetMethodID(editClazz,"setText", "([CII)V");

    // 调用父类的虚函数
    // env->CallNonvirtualVoidMethod()

    std::wstring text = std::to_wstring(value);
    jint length = text.length();
    jcharArray array = env->NewCharArray(length);
    MY_LOG_INFO("Array length is %d",env->GetArrayLength(array));
    jchar charArray[length];
    for(int i=0;i<length;i++){
        charArray[i] = text[i];
    }
    env->SetCharArrayRegion(array, 0, length, charArray);
    env->CallVoidMethod(edit_text,methodId,array,0,length);
}

// 当共享库开始加载时，虚拟机自动调用该函数
jint JNI_OnLoad(JavaVM* vm,void* reserved){
    MY_LOG_INFO("jni->JNI_OnLoad");
    JNIEnv* env = nullptr;
    if(vm->GetEnv((void**)&env,JNI_VERSION_1_6)==JNI_OK){
        jclass clazz = env->FindClass("com/example/hellondk/MainActivity");
        // private native static void setNumber(EditText editText,int value);
        JNINativeMethod nativeMethod[] = {{"setNumber", "(Landroid/widget/EditText;I)V",(void*)setNumber}};
        env->RegisterNatives(clazz,nativeMethod,sizeof(nativeMethod)/sizeof(JNINativeMethod));
    }

    return JNI_VERSION_1_6;
}

extern "C" void _init(void){
    MY_LOG_INFO("jni->_init");
}

__attribute__ ((constructor, visibility ("hidden"))) void initArray1(void){
    MY_LOG_INFO("jni->initArray1");
}

__attribute__ ((constructor, visibility ("hidden"))) void initArray2(void){
    MY_LOG_INFO("jni->initArray2");
}

__attribute__ ((constructor, visibility ("hidden"))) void initArray3(void){
    MY_LOG_INFO("jni->initArray3");
}

// 指定优先级,默认按照声明顺序
__attribute__ ((constructor(1), visibility ("hidden"))) void initArray4(void){
    MY_LOG_INFO("jni->initArray4");
}




