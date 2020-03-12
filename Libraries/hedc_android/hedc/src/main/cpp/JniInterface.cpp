//
//
//
#include "stdafx.h"
#include <jni.h>
#include <iostream>
#include <assert.h>
//#include "AndroidUsbBase.h"
#include "DeviceComm.h"
#include "JniInterface.h"
#include "logs.h"
#include "MessageDetails.h"
#include "HsmErrorDefs.h"
#include "svn_rev.h"

using namespace std;
namespace HsmDeviceComm
{
    CCallback::CCallback()
    : CCallbackBase()
// , m_env(NULL)
    , m_javaVM(NULL)
    , m_EC_cls(NULL)
    , m_EC_instance(NULL)
    , m_ReceiveFromDeviceAsyncMID(NULL)
    , m_ReceiveStatusMID(NULL)
    , m_ReceiveTextMID(NULL)
    , m_ReceiveImageMID(NULL)
    {
        m_pReadThread = new TThreadWorker<CCallback,ThreadCommands_t, None> (this, &CCallback::DispatchThreadLoop);
    }

    CCallback::~CCallback()
    {
        delete m_pReadThread;
        m_pReadThread = NULL;
    };

    void CCallback::OnDestroy(JNIEnv *env)
    {
        delete m_pReadThread;
        m_pReadThread = NULL;
        if (m_EC_cls!=NULL)
        {
            env->DeleteGlobalRef(m_EC_cls);
            m_EC_cls = NULL;
        }
        if (m_EC_instance!=NULL)
        {
            env->DeleteGlobalRef(m_EC_instance);
            m_EC_instance = NULL;
        }
    }

    bool CCallback::CacheMethodIDs(JNIEnv *env, jobject instance)
    {
        // do not store env, it is thread local
        env->GetJavaVM(&m_javaVM);
        const char *classname = "honeywell/hedc/EngineCommunication";
        jclass cls = env->FindClass(classname);
        m_ReceiveFromDeviceAsyncMID = env->GetMethodID(cls,"ReceiveRawFromDeviceAsync","(I[B)V");
        m_ReceiveStatusMID = env->GetMethodID(cls,"ReceiveStatus","(II)V");
        m_ReceiveTextMID = env->GetMethodID(cls,"ReceiveText","(ILjava/lang/String;)V");
        m_ReceiveImageMID = env->GetMethodID(cls,"ReceiveImage","(I[B)V");

        m_EC_cls = (jclass) env->NewGlobalRef(cls);
        m_EC_instance = env->NewGlobalRef(instance);

        env->DeleteLocalRef(cls);
        return true;
    }

    bool CCallback::Connect(JNIEnv *env, jobject instance, UsbDetails_t &device)
    {
        CacheMethodIDs(env, instance);
        //ASSERT(m_pReadThread!=NULL);
        m_pReadThread->Start();
        m_pReadThread->Run();

        return CCallbackBase::Connect(device) && CCallbackBase::Connect();
    }

    bool CCallback::Disconnect(JNIEnv *env, bool /* unplug */)
    {
        return CCallbackBase::Disconnect();
    }

    void CCallback::Call(JNIEnv *env, const char *fncname, const char *typname)
    {
        jmethodID mid = env->GetMethodID(m_EC_cls, fncname, typname);  // find method
        if (mid != NULL)
        {
            env->CallVoidMethod(m_EC_instance, mid);    // call method
        } else
        {
            LOGE("ERROR: method %s not found !\n", fncname);
        }
    }

    bool CCallback::PostStatusMessage(WPARAM wParam, LPARAM lParam)
    {
        // get env from thread
        EnvHandler eh(m_javaVM);
        JNIEnv *env = eh.getEnv();
        if (env == NULL)
        {
            return false;
        }

        Call(env, m_ReceiveStatusMID, wParam, lParam);
        // return env
        m_javaVM->DetachCurrentThread();
        return true;
    }

    // Send RAW data to Java
    bool CCallback::ProvideRAWData(int kind, CStringA raw)
    {
        // get env from thread
        EnvHandler eh(m_javaVM);
        JNIEnv *env = eh.getEnv();
        if (env == NULL)
        {
            return false;
        }

        int size = raw.GetLength();
        // construct java array
        jbyteArray arr = env->NewByteArray(size);
        env->SetByteArrayRegion(arr, 0, size, (const jbyte *) (const char*)raw);
        Call(env, m_ReceiveFromDeviceAsyncMID, kind, arr);
        env->DeleteLocalRef(arr);

        return true;
    }

    // Send Text data to Java
    bool CCallback::ProvideText(int kind, CStringA txt)
    {
        // get env from thread
        EnvHandler eh(m_javaVM);
        JNIEnv *env = eh.getEnv();
        if (env == NULL)
        {
            return false;
        }

        jstring str = env->NewStringUTF((const char *) txt);
        Call(env, m_ReceiveTextMID, kind, str);
        env->DeleteLocalRef(str);
        // return env
        m_javaVM->DetachCurrentThread();
        return true;
    }

    // Send Image data to Java
    bool CCallback::ProvideImage(int kind, const UCHAR *pData, size_t Length)
    {
        // get env from thread
        EnvHandler eh(m_javaVM);
        JNIEnv *env = eh.getEnv();
        if (env == NULL)
        {
            return false;
        }

        const int size = Length;
        // construct java array
        jbyteArray arr = env->NewByteArray(size);
        env->SetByteArrayRegion(arr, 0, size, (const jbyte *) pData);
        Call(env, m_ReceiveImageMID, kind, arr);
        env->DeleteLocalRef(arr);
        return true;
    }

    bool CCallback::PostDataMessage(WPARAM wParam, LPARAM lParam)
    {
        UNUSED(wParam);
        UNUSED(lParam);

        m_pReadThread->Action();
        return true;
    }

    void *CCallback::DispatchThreadLoop()
    {
        int MsgType;
        CString Text;
        while(m_pReadThread->WaitForAction())
        {
            while(CMessageDetails::None != (MsgType = ReadAndParseMessage()))
            {
                switch (MsgType)
                {
                    case CMessageDetails::Text:
                        ProvideText(MsgType, GetRawPayloadData());
                        break;
                    case CMessageDetails::Unknown:
                        ProvideRAWData(MsgType, GetRawPayloadData());
                        break;
                    case CMessageDetails::CmdResponse:
                        {
                            int kind = MsgType;
                            if (GetAckByte() == 0x15)
                                kind = CMessageDetails::BadCmdResponse;
                            ProvideRAWData(kind, GetRawPayloadData());
                        }
                        break;
                    case CMessageDetails::Image:
                    {
                        ProvideText(MsgType, "Received non-text data");
                        UCHAR *pData = NULL;
                        size_t Length = 0;
                        if (GetRawPayloadBuffer(pData, Length))
                        {
                            ProvideImage(MsgType, pData, Length);
                        }
                        break;
                    }
                    case CMessageDetails::BadImage:
                        ProvideText(MsgType, "Received partial or broken image");
                        break;
                    default:
                        break;
                }
            }
        }
        return NULL; // dummy value
    }

}

static HsmDeviceComm::CCallback* spDevice = NULL;
static HsmDeviceComm::CCallback* GetDevice()
{
    if (spDevice==NULL)
    {
        spDevice = new HsmDeviceComm::CCallback();
    }
    return spDevice;
}

static void DestroyDeviceObject(JNIEnv *env)
{
    if (spDevice!=NULL)
    {
        spDevice->OnDestroy(env);
        delete spDevice;
        spDevice = NULL;
    }
}

// Initialize details
extern "C" JNIEXPORT void JNICALL
Java_honeywell_hedc_EngineCommunication_ConnectEngine(JNIEnv *env, jobject obj, jint fd, jint outEP, jint inEP, jint interface, jint inSize, jint outSize)
{
    UsbDetails_t details; // Struct
    details.m_type = (Interfacetype_t)interface;
    details.m_fd = fd;
    details.m_OutEp = outEP;
    details.m_InEp = inEP;
    details.m_OutEpSize = outSize;
    details.m_InEpSize = inSize;

    bool RetVal = GetDevice()->Connect(env, obj, details);
}

extern "C" JNIEXPORT void JNICALL
Java_honeywell_hedc_EngineCommunication_DisconnectEngine(JNIEnv *env, jobject obj, jboolean unplug)
{
    bool RetVal = GetDevice()->Disconnect(env, unplug);
}

extern "C" JNIEXPORT void JNICALL
Java_honeywell_hedc_EngineCommunication_DestroyDeviceObject(JNIEnv *env, jobject obj)
{
    DestroyDeviceObject(env);
}

// Enable trigger
extern "C" JNIEXPORT jint JNICALL
Java_honeywell_hedc_EngineCommunication_StartReadingSession(JNIEnv *env, jobject obj)
{
    bool RetVal = GetDevice()->Write("\x16T\r", 3);
    return RetVal ? ERROR_SUCCESS : ERROR_CMD_FAILED;
}

// Disable trigger
extern "C" JNIEXPORT jint JNICALL
Java_honeywell_hedc_EngineCommunication_StopReadingSession(JNIEnv *env, jobject obj)
{
    bool RetVal = GetDevice()->Write("\x16U\r", 3);
    return RetVal ? ERROR_SUCCESS : ERROR_CMD_FAILED;
}

extern "C" JNIEXPORT jint JNICALL
Java_honeywell_hedc_EngineCommunication_BeepSuccess(JNIEnv *env, jobject obj)
{
    bool RetVal = GetDevice()->Write("\x16\x07\r", 3);
    return RetVal ? ERROR_SUCCESS : ERROR_CMD_FAILED;
}

extern "C" JNIEXPORT jint JNICALL
Java_honeywell_hedc_EngineCommunication_BeepError(JNIEnv *env, jobject obj)
{
    bool RetVal = GetDevice()->Write("\x16\x07e\r", 4);
    return RetVal ? ERROR_SUCCESS : ERROR_CMD_FAILED;
}

extern "C" JNIEXPORT jint JNICALL
Java_honeywell_hedc_EngineCommunication_SendToDevice(JNIEnv *env, jobject jobj, jbyteArray cmd)
{
    jsize len = env->GetArrayLength(cmd);
    jboolean isCopy = 0;
    jbyte *buffer = env->GetByteArrayElements(cmd, &isCopy);
    jint written = GetDevice()->Write((const char*) buffer, len);
    env->ReleaseByteArrayElements(cmd, buffer, JNI_ABORT);
    return written;
}

// Return barcode data from buffer
extern "C" JNIEXPORT jint JNICALL
Java_honeywell_hedc_EngineCommunication_SendToDeviceString(JNIEnv *env, jobject obj, jstring cmd)
{

    jsize size = env->GetStringUTFLength(cmd);
    const char *buffer = env->GetStringUTFChars(cmd, NULL);
    jint written = GetDevice()->Write(buffer, size);
    return written;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_honeywell_hedc_EngineCommunication_ExecuteMenuCommand(JNIEnv *env, jobject obj, jstring cmd)
{

    jsize size = env->GetStringUTFLength(cmd);
    const char *buffer = env->GetStringUTFChars(cmd, NULL);
    CStringA sCmd(buffer, size);
    jboolean RetVal = GetDevice()->ExecuteMenuCommand(sCmd, false);
    return RetVal;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_honeywell_hedc_EngineCommunication_ExecuteProdMenuCommand(JNIEnv *env, jobject obj, jstring cmd)
{

    jsize size = env->GetStringUTFLength(cmd);
    const char *buffer = env->GetStringUTFChars(cmd, NULL);
    CStringA sCmd(buffer, size);
    jboolean RetVal = GetDevice()->ExecuteMenuCommand(sCmd, true);
    return RetVal;
}

extern "C" JNIEXPORT jstring JNICALL
Java_honeywell_hedc_EngineCommunication_GetCommandResponse(JNIEnv *env, jobject obj)
{
    CStringA sResponse = GetDevice()->GetCommandResponse();
    return env->NewStringUTF((const char*)sResponse);
}

extern "C" JNIEXPORT jint JNICALL
Java_honeywell_hedc_EngineCommunication_ExecuteFirmwareFlashFile(JNIEnv *env, jobject obj, jstring filename)
{
    jsize size = env->GetStringUTFLength(filename);
    const char *buffer = env->GetStringUTFChars(filename, NULL);
    CStringA sFilename(buffer, size);
    GetDevice()->AllowUnknownDevices(true);   // allow devices with a still unknown CompID
    jboolean RetVal = GetDevice()->ExecuteFirmwareFlash(sFilename);
    return RetVal;
}

extern "C" JNIEXPORT jint JNICALL
Java_honeywell_hedc_EngineCommunication_ExecuteFirmwareFlashArray(JNIEnv *env, jobject obj, jbyteArray firmware)
{
    jbyte* buffer = env->GetByteArrayElements(firmware, NULL);
    jsize size = env->GetArrayLength(firmware);
    GetDevice()->AllowUnknownDevices(true);   // allow devices with a still unknown CompID
    jboolean RetVal = GetDevice()->ExecuteFirmwareFlash((unsigned char *)buffer, size);
    env->ReleaseByteArrayElements(firmware, buffer, JNI_ABORT);
    return RetVal;
}

extern "C" JNIEXPORT void JNICALL
Java_honeywell_hedc_EngineCommunication_AbortFirmwareReplace(JNIEnv *env, jobject obj)
{
    GetDevice()->AbortFirmwareFlash();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_honeywell_hedc_EngineCommunication_isFirmwareReplace(JNIEnv *env, jobject obj)
{
    return GetDevice()->isFirmwareReplace();
}

extern "C" JNIEXPORT jstring JNICALL
Java_honeywell_hedc_EngineCommunication_GetLibraryVersion(JNIEnv *env, jobject obj)
{
    return env->NewStringUTF(SVN_RevisionStr);
}

//////////////////////////////////////////////////////////////////////////////
// legacy interfaces
extern "C" JNIEXPORT jint JNICALL
Java_honeywell_hedc_EngineCommunication_SendMenuCommand(JNIEnv *env, jobject obj,
        jbyteArray InputBuffer, jint nBytesInInputBuffer,
        jbyteArray OutputBuffer, jint nSizeOfOutputBuffer,
        jintArray BytesReturned, jint /* TimeOutMsec */)
{
    jshort RetVal = ERROR_SUCCESS;

    jsize InputLen = std::min(env->GetArrayLength(InputBuffer), nBytesInInputBuffer);
    jboolean isInputCopy = 0;
    jbyte *pInputBuffer = env->GetByteArrayElements(InputBuffer, &isInputCopy);

    CStringA sCommand((const char *) pInputBuffer, InputLen);
    CStringA sResponse;
    const bool bOptional = true;
    const bool ProdConfig = true;
    bool Success = GetDevice()->ExecuteMenuCommand(sCommand, sResponse, !bOptional,!ProdConfig);
    if (!Success)
    {
        // Note: This sounds like mixed up, but it minics behavior of old lib.
        RetVal = HEDC_NOT_SUPPORTED;
        if (GetDevice()->GetLastCmdError() == ERROR_NOT_SUPPORTED)
            RetVal = HEDC_INVALID_COMMAND;
    }

    if (sResponse.GetLength() >= nSizeOfOutputBuffer)
    {
        RetVal = HEDC_OUTPUT_BUFFER_TOO_SMALL;
    }
    else
    {
        env->SetByteArrayRegion(OutputBuffer, 0, (jint) sResponse.GetLength(),
                                (const jbyte *) (const char *) sResponse);
    }

    jint nBytesReturned = sResponse.GetLength();
    env->SetIntArrayRegion(BytesReturned, 0, 1, &nBytesReturned);
    env->ReleaseByteArrayElements(InputBuffer, pInputBuffer, JNI_ABORT);
    return RetVal;
}

