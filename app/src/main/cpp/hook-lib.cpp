//
// Created by 31231 on 2022/5/3.
//

#include <jni.h>
#include <android/log.h>
#include "my_log.h"
#include "Dobby/include/dobby.h"
#include <dlfcn.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string>
#include <filesystem>


extern "C" void* _init(void){
    MY_LOG_INFO("hook-lib loaded");
    return nullptr;
}

extern "C"{

JNIEXPORT void JNICALL
Java_com_example_hellondk_MainActivity_print(JNIEnv *env, jobject thiz) {
    MY_LOG_INFO("Unhook...");
}

void (*orig_print)(JNIEnv* env,jobject thiz)=nullptr;

JNIEXPORT void JNICALL
new_print(JNIEnv* env,jobject thiz){
    MY_LOG_INFO("Hooked...");
}

bool saveFile(const void* addr,int len,const char*outFileName) {
    bool success = false;
    FILE* file = fopen(outFileName,"wb+");
    if(file!=nullptr){
        fwrite(addr,len,1,file);
        fflush(file);
        fclose(file);
        success=true;
        chmod(outFileName,S_IRWXU|S_IRWXG|S_IRWXO);
    }else{
        MY_LOG_ERROR("[%s] fopen failed,error: %s.",__FUNCTION__ ,dlerror());
    }

    return success;
}

/*
int luaL_loadbuffer (lua_State *L,
                     const char *buff,
                     size_t sz,
                     const char *name);
*/

// original function copy
int (*luaL_loadbuffer_orig) (void* L,const char *buff,int size,const char* name) =nullptr;

// local function
int lual_loadbuffer_mod(void* L,const char* buff,int size,const char* name) {
    //MY_LOG_INFO("[dumplua] loadL_loadbuffer name: %s",name);
    std::string path = "/storage/emulated/0/Download/jltx";
    std::string modPath = "/storage/emulated/0/Download/jltx_modified";
    if(name[0]!='/'){
        path+='/';
        modPath+='/';
    }
    path+=name;
    modPath+=name;
    // get the file name
    std::string fileName = path;
    std::string modFileName = modPath;
    // get the path
    int pos = path.rfind("/");
    path = path.substr(0,pos);
    pos = modPath.rfind("/");
    modPath = modPath.substr(0,pos);
    //MY_LOG_INFO("[dumplua] path: %s",path.c_str());
    // create the directories
    std::__fs::filesystem::create_directories(path);
    std::__fs::filesystem::create_directories(modPath);

    // judgement the file's existence ,if not exist , save it
    bool isExist = std::__fs::filesystem::is_regular_file(fileName);
    if (!isExist)
        saveFile(buff,size,fileName.c_str());
    else{
        MY_LOG_INFO("[dumplua] %s has saved.",fileName.c_str());
    }

    isExist = std::__fs::filesystem::is_regular_file(modFileName);
    if(isExist){
        FILE* fp = fopen(modFileName.c_str(),"rb");
        MY_LOG_INFO("[hook] hijack the %s.",fileName.c_str());
        if(fp!=nullptr){
            fseek(fp,0,SEEK_END);
            long fileSize = ftell(fp);
            unsigned char* buffer = (unsigned char*)malloc(fileSize);
            if(buffer!=nullptr){
                fseek(fp,0,SEEK_SET);
                int readBytes = fread(buffer,fileSize,1,fp);
                if(readBytes>0){
                    int ret = luaL_loadbuffer_orig(L,(const char*)buffer,fileSize,modFileName.c_str());
                    free(buffer);
                    MY_LOG_INFO("[hook] ret: %d.",ret);
                    return ret;
                }
            }
        }

    }

    return luaL_loadbuffer_orig(L,buff,size,name);
}

void Hook(){
    //DobbyHook((void*)&Java_com_example_hellondk_MainActivity_print,(void*)new_print,(void**)&orig_print);
    MY_LOG_INFO("[dumplua] hook begin");
    void* handle = dlopen("libgame.so",RTLD_NOW);
    if(handle == nullptr){
        MY_LOG_INFO("[dumplua] dlopen err: %s.",dlerror());
        return;
    }else{
        MY_LOG_INFO("[dumplua] libgame.so dlopen OK!");
    }

    void *pluaL_loadbuffer = dlsym(handle,"luaL_loadbuffer");
    if(pluaL_loadbuffer == nullptr) {
        MY_LOG_ERROR("[dumplua] lua_loadbuffer not found!");
        MY_LOG_ERROR("[dumplua] dlsym err: %s.", dlerror());
    }else{
        MY_LOG_DEBUG("[dumplua] lual_loadbuffer found!");
        DobbyHook(pluaL_loadbuffer,(void*)&lual_loadbuffer_mod,(void**)&luaL_loadbuffer_orig);
    }
}

JNIEXPORT void JNICALL Java_com_example_hellondk_MainActivity_Unhook(JNIEnv* env,jobject thiz) {
    //DobbyDestroy((void*)&Java_com_example_hellondk_MainActivity_print);
}

}

jint JNI_OnLoad(JavaVM* vm,void* reserved){
    MY_LOG_INFO("Start Hook");

    Hook();

    return JNI_VERSION_1_6;
}
