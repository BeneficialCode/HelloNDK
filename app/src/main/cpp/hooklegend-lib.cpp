//
// Created by 31231 on 2022/5/14.
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
    MY_LOG_INFO("hookcq-lib loaded");
    return nullptr;
}




bool saveFile(const void *addr, int len, const char *outFileName) {
    bool success = false;
    FILE *file = fopen(outFileName, "wb+");
    if (file != nullptr) {
        fwrite(addr, len, 1, file);
        fflush(file);
        fclose(file);
        success = true;
        chmod(outFileName, S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
        MY_LOG_ERROR("[%s] fopen failed,error: %s.", __FUNCTION__, dlerror());
    }

    return success;
}



/*int luaL_loadbufferx (lua_State *L,
                      const char *buff,
                      size_t sz,
                      const char *name,
                      const char *mode);*/
// original function copy
extern "C" {


int  (*g_pluaL_loadbufferx) (void* L,const char* buff,size_t sz,const char* name,const char* mode) = nullptr;

// local function
int luaL_loadbufferx_mod(void *L, const char *buff, int size, const char *name,const char* mode) {
    MY_LOG_INFO("size %d", size);
    MY_LOG_INFO("name %s", name);

    if(strstr(name,"lua")!=nullptr){
        std::string path = "/storage/emulated/0/Download/legend";
        std::string modPath = "/storage/emulated/0/Download/legend_modified";
        if (name[0] != '/') {
            path += '/';
            modPath += '/';
        }
        path += name;
        modPath += name;
        // get the file name
        std::string fileName = path;
        std::string modFileName = modPath;
        // get the path
        int pos = path.rfind("/");
        path = path.substr(0, pos);
        pos = modPath.rfind("/");
        modPath = modPath.substr(0, pos);
        //MY_LOG_INFO("[dumplua] path: %s",path.c_str());
        // create the directories
        std::__fs::filesystem::create_directories(path);
        std::__fs::filesystem::create_directories(modPath);

        // judgement the file's existence ,if not exist , save it
        bool isExist = std::__fs::filesystem::is_regular_file(fileName);
        if (!isExist)
            saveFile(buff, size, fileName.c_str());
        else {
            MY_LOG_INFO("[dumplua] %s has saved.", fileName.c_str());
        }

        isExist = std::__fs::filesystem::is_regular_file(modFileName);
        if (isExist) {
            FILE *fp = fopen(modFileName.c_str(), "rb");
            MY_LOG_INFO("[hook] hijack the %s.", fileName.c_str());
            if (fp != nullptr) {
                fseek(fp, 0, SEEK_END);
                long fileSize = ftell(fp);
                unsigned char *buffer = (unsigned char *) malloc(fileSize);
                if (buffer != nullptr) {
                    fseek(fp, 0, SEEK_SET);
                    int readBytes = fread(buffer, fileSize, 1, fp);
                    if (readBytes > 0) {
                        int ret = g_pluaL_loadbufferx(L, (const char *) buffer, fileSize,
                                                      modFileName.c_str(),mode);
                        free(buffer);
                        MY_LOG_INFO("[hook] ret: %d.", ret);
                        return ret;
                    }
                }
            }

        }
    }

    return g_pluaL_loadbufferx(L, buff, size, name,mode);
}


//int cocos2dx_lua_loader(lua_State *L)
int (*cocos2dx_lua_loader_orig)(void *L) = nullptr;
int cocos2dx_lua_loader_mod(void *L) {
    MY_LOG_INFO("L %p", L);
    return cocos2dx_lua_loader_orig(L);
}

//unsigned char *xxtea_decrypt(unsigned char *data, xxtea_long data_len, unsigned char *key, xxtea_long key_len, xxtea_long *ret_length);

typedef uint32_t xxtea_long;
unsigned char *(*xxtea_decrypt_orig)(unsigned char *data, xxtea_long data_len, unsigned char *key,
                                     xxtea_long key_len, xxtea_long *ret_length) = nullptr;
unsigned char *
xxtea_decrypt_mod(unsigned char *data, xxtea_long data_len, unsigned char *key, xxtea_long key_len,
                  xxtea_long *ret_length) {
    MY_LOG_INFO("keyLen %d, key: %s", key_len, key);
    return xxtea_decrypt_orig(data, data_len, key, key_len, ret_length);
}

void Hook() {
    //DobbyHook((void*)&Java_com_example_hellondk_MainActivity_print,(void*)new_print,(void**)&orig_print);
    MY_LOG_INFO("[dumplua] hook begin");
    void *handle = dlopen("libcocos2dcpp.so", RTLD_NOW);
    if (handle == nullptr) {
        MY_LOG_INFO("[dumplua] dlopen err: %s.", dlerror());
        return;
    } else {
        MY_LOG_INFO("[dumplua] libcocos2dcpp.so dlopen OK!");
    }

    void *pluaL_loadbufferx = dlsym(handle, "luaL_loadbufferx");
    if (pluaL_loadbufferx == nullptr) {
        MY_LOG_ERROR("[dumplua] luaL_loadbufferx not found!");
        MY_LOG_ERROR("[dumplua] dlsym err: %s.", dlerror());
    } else {
        MY_LOG_DEBUG("[dumplua] luaL_loadbufferx found!");
        DobbyHook(pluaL_loadbufferx, (void *) &luaL_loadbufferx_mod, (void **) &g_pluaL_loadbufferx);
    }

    /* void *pcocos2dx_lua_loader = dlsym(handle, "cocos2dx_lua_loader");
     if (pcocos2dx_lua_loader == nullptr) {
         MY_LOG_ERROR("[dumplua] cocos2dx_lua_loader not found!");
         MY_LOG_ERROR("[dumplua] dlsym err: %s.", dlerror());
     } else {
         MY_LOG_DEBUG("[dumplua] cocos2dx_lua_loader found!");
         DobbyHook(pcocos2dx_lua_loader, (void *) &cocos2dx_lua_loader_mod, (void **) &cocos2dx_lua_loader_orig);
     }*/

    /* void *pxxtea_decrypt = dlsym(handle, "_Z13xxtea_decryptPhjS_jPj");
     if (pxxtea_decrypt == nullptr) {
         MY_LOG_ERROR("[dumplua] xxtea_decrypt not found!");
         MY_LOG_ERROR("[dumplua] dlsym err: %s.", dlerror());
     } else {
         MY_LOG_DEBUG("[dumplua] xxtea_decrypt found!");
         DobbyHook(pxxtea_decrypt, (void *) &xxtea_decrypt_mod, (void **) &xxtea_decrypt_orig);
     }*/
}

}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    MY_LOG_INFO("Start Hook");

    Hook();

    return JNI_VERSION_1_6;
}

