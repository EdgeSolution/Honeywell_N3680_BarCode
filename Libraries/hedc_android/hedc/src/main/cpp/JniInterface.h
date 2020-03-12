//
//
#include "stdafx.h"
#include <jni.h>
#include <iostream>
#include "DeviceComm.h"
#include "ThreadedDeviceCommands.h"
#include "ThreadWorker.h"

using namespace std;

namespace HsmDeviceComm
{
    // A helper to make it easier to attach and detach env.
    // Especially we wont forget to detach since that is now automatic.
    class EnvHandler
    {
    public:
        EnvHandler(JavaVM *javaVM)
        : m_javaVM(javaVM)
        {
        }

        ~EnvHandler()
        {
            m_javaVM->DetachCurrentThread();
        }

        JNIEnv *getEnv()
        {
            // get env from thread
            JNIEnv *env;
            int RetVal = m_javaVM->AttachCurrentThread(&env, NULL);
            if (JNI_OK != RetVal)
            {
                LOGE("Failed to AttachCurrentThread, ErrorCode = %d", RetVal);
                return NULL;
            }
            return env;
        }
    protected:
        JavaVM *m_javaVM;
    };

    #define CCallbackBase CThreadedDeviceCommands

    class CCallback : public CCallbackBase
    {
    public:
        CCallback();
        virtual ~CCallback();

        virtual bool Connect(JNIEnv *env, jobject instance, UsbDetails_t &device);
        virtual bool Disconnect(JNIEnv *env, bool unplug);

        void OnDestroy(JNIEnv *env);
        using CCommLib::Connect;

    protected:
        bool CacheMethodIDs(JNIEnv *env, jobject instance);

        void InitJni(JavaVM *javaVM)
        {
            m_javaVM = javaVM;
        }

        template<typename TParam>
        void Call(JNIEnv *env, const char *fncname, const char *typname, TParam jdata)
        {
            jmethodID mid = env->GetMethodID(m_EC_cls, fncname, typname);  // find method
            if (mid != NULL)
            {
                env->CallVoidMethod(m_EC_instance, mid, jdata);    // call method
            } else
            {
                LOGE("ERROR: method %s not found !\n", fncname);
            }
        }

        template<typename TParam>
        void Call(JNIEnv *env, jmethodID mid, TParam jdata)
        {
            if (mid != NULL)
            {
                env->CallVoidMethod(m_EC_instance, mid, jdata);    // call method
            } else
            {
                LOGE("ERROR: method not found !\n");
            }
        }

        template<typename TParam1, typename TParam2>
        void Call(JNIEnv *env, const char *fncname, const char *typname, TParam1 jdata1, TParam2 jdata2)
        {
            jmethodID mid = env->GetMethodID(m_EC_cls, fncname, typname);  // find method
            if (mid != NULL)
            {
                env->CallVoidMethod(m_EC_instance, mid, jdata1, jdata2);    // call method
            } else
            {
                LOGE("ERROR: method %s not found !\n", fncname);
            }
        }

        template<typename TParam1, typename TParam2>
        void Call(JNIEnv *env, jmethodID mid, TParam1 jdata1, TParam2 jdata2)
        {
            if (mid != NULL)
            {
                env->CallVoidMethod(m_EC_instance, mid, jdata1, jdata2);    // call method
            } else
            {
                LOGE("ERROR: method not found !\n");
            }
        }

        void Call(JNIEnv *env, const char *fncname, const char *typname);

        virtual bool PostStatusMessage(WPARAM wParam, LPARAM lParam = 0);
        virtual bool PostDataMessage(WPARAM wParam = 0, LPARAM lParam = 0);

        bool ProvideRAWData(int kind, CStringA bytes);
        bool ProvideText(int kind, CStringA txt);
        bool ProvideImage(int kind, const UCHAR *pData, size_t Length);
        void *DispatchThreadLoop();

    protected:
        enum ThreadCommands_t
        {
            None, Stop, Start,
        };
        TThreadWorker<CCallback, ThreadCommands_t, None> *m_pReadThread;

        //   JNIEnv *m_env;
        JavaVM *m_javaVM;
        jclass m_EC_cls;
        jobject m_EC_instance;
        jmethodID m_ReceiveFromDeviceAsyncMID;
        jmethodID m_ReceiveStatusMID;
        jmethodID m_ReceiveTextMID;
        jmethodID m_ReceiveImageMID;
    };

}
