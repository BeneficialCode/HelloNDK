//
// Created by VirtualCC on 2021-07-20.
//
#include <jni.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

struct NativeWorkerArgs{
    jint id;
    jint threads;
    jint iterations;
};

static jmethodID gOnNativeMessage = nullptr;
static JavaVM* gVm = nullptr;
static jobject gObj = nullptr;

static pthread_mutex_t mutex;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_nativeInit(JNIEnv *env, jobject obj) {
    do {
        if(0!=pthread_mutex_init(&mutex,nullptr)){
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
            env->ThrowNew(exceptionClazz,"Unable to initialize mutex");
            break;
        }
        if(nullptr == gObj){
            gObj = env->NewGlobalRef(obj);
            if(nullptr == gObj){
                break;
            }
        }

        if(nullptr == gOnNativeMessage){
            jclass clazz = env->GetObjectClass(obj);

            gOnNativeMessage = env->GetMethodID(clazz,
                                                "onNativeMessage",
                                                "(Ljava/lang/String;)V");
            if(nullptr==gOnNativeMessage){
                jclass exceptionClazz = env->FindClass(
                        "javax/management/RuntimeOperationsException");
                env->ThrowNew(exceptionClazz,"Unable to find method");
            }
        }
    }while (false);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_nativeFree(JNIEnv *env, jobject thiz) {
    if(0!=pthread_mutex_destroy(&mutex)){
        jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exceptionClazz,"Unable to destroy mutex");
    }
    if(nullptr!=gObj){
        env->DeleteGlobalRef(gObj);
        gObj = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_nativeWorker(JNIEnv *env, jobject obj, jint id,
                                                    jint iterations) {
    for(jint i=0;i<iterations;i++){
        char message[26];
        sprintf(message,"Worker %d: Iteration %d",id,i);

        jstring messageString = env->NewStringUTF(message);

        env->CallVoidMethod(obj,gOnNativeMessage,messageString);

        if(nullptr!=env->ExceptionOccurred())
            break;

        sleep(1);
    }
}

static void* nativeWorkerThread(void* args){
    JNIEnv* env = nullptr;

    // ????????????????????????Java????????????????????????JNIEnv????????????
    if(0 == gVm->AttachCurrentThread(&env,nullptr)){
        NativeWorkerArgs* nativeWorkerArgs = (NativeWorkerArgs*)args;
        Java_com_example_hellondk_MainActivity_nativeWorker(env,
                                                            gObj,
                                                            nativeWorkerArgs->id,
                                                            nativeWorkerArgs->iterations);
        if(0!=pthread_mutex_unlock(&mutex)){
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
            env->ThrowNew(exceptionClazz,"Unable to unlock mutex");
        }
        gVm->DetachCurrentThread();
    }

    return (void*)1;
}

static void* startThreads(void* args){
    NativeWorkerArgs* nativeWorkerArgs = (NativeWorkerArgs*)args;
    // ????????????
    pthread_t* handles = new pthread_t[nativeWorkerArgs->threads];
    JNIEnv* env=nullptr;
    jint threads = nativeWorkerArgs->threads;
    bool success=false;
    if(0!=gVm->AttachCurrentThread(&env,nullptr)){
        goto exit;
    }
    success = true;

    // ?????????worker????????????POSIX??????
    for(jint i=0;i<threads;i++){
        if(0!=pthread_mutex_lock(&mutex)){
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
            env->ThrowNew(exceptionClazz,"Unable to lock mutex");
            goto exit;
        }
        // ???????????????
        nativeWorkerArgs->id = i;

        // ???????????????
        int result = pthread_create(
                &handles[i],
                nullptr,
                nativeWorkerThread,
                (void*)nativeWorkerArgs
        );
        if(0!=result){
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
            // ????????????
            env->ThrowNew(exceptionClazz,"Unable to create thread");
            goto exit;
        }
    }

    for(int i=0;i<threads;i++){
        // ??????????????????
        void* ret = nullptr;
        if(0!=pthread_join(handles[i],&ret)){
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
            env->ThrowNew(exceptionClazz,"Unable to join thread");
        }else{
            // ??????message
            char message[26];
            sprintf(message,"Worker %d returned %d",i,ret);

            jstring messageString = env->NewStringUTF(message);

            env->CallVoidMethod(gObj,gOnNativeMessage,messageString);

            if(nullptr!=env->ExceptionOccurred()){
                goto exit;
            }
        }
    }

    exit:
    delete nativeWorkerArgs;
    if(nullptr!=handles){
        delete [] handles;
        handles= nullptr;
    }
    if(success)
        gVm->DetachCurrentThread();
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_posixThreads(JNIEnv *env, jobject thiz, jint threads,
                                                    jint iterations) {
    NativeWorkerArgs* nativeWorkerArgs = new NativeWorkerArgs;
    nativeWorkerArgs->iterations = iterations;
    nativeWorkerArgs->threads = threads;
    // ????????????
    pthread_t thread;
    // ?????????????????????
    int result = pthread_create(
            &thread,
            nullptr,
            startThreads,
            (void*)nativeWorkerArgs
    );

    if(0!=result){
        // ???????????????
        jclass exceptionClazz = env->FindClass(
                "java/lang/RuntimeException"
        );
        env->ThrowNew(exceptionClazz,"Unable to create thread");
    }
}

// ????????????????????????????????????????????????????????????
jint JNI_OnLoad(JavaVM* vm,void* reserved){
    gVm = vm;
    return JNI_VERSION_1_6;
}
