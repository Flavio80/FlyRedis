#include "FlyRedis/FlyRedis.h"
#include "boost/thread.hpp"

void Logger(const char* pszMsg)
{
    printf("%s\n", pszMsg);
}

void ThreadTestFlyRedis(std::string strRedisAddr, std::string strPassword, bool bUseTLSFlag)
{
    CFlyRedisClient hFlyRedisClient;
    hFlyRedisClient.SetRedisConfig(strRedisAddr, strPassword);
    hFlyRedisClient.SetRedisReadWriteType(FlyRedisReadWriteType::ReadWriteOnMaster);
    if (bUseTLSFlag && !hFlyRedisClient.SetTLSContext("redis.crt", "redis.key", "ca.crt", ""))
    {
        return;
    }
    //hFlyRedisClient.SetRedisClusterDetectType(FlyRedisClusterDetectType::DisableCluster);
    if (!hFlyRedisClient.Open())
    {
        return;
    }
    hFlyRedisClient.HELLO(3);
    hFlyRedisClient.HELLO_AUTH_SETNAME(2, "default", "123456", "FlyRedis");
    hFlyRedisClient.HELLO_AUTH_SETNAME(3, "default", "123456", "FlyRedis");
    //////////////////////////////////////////////////////////////////////////
    RedisResponse stRedisResponse;
    double fResult = 0.0f;
    int nResult = 0;
    //////////////////////////////////////////////////////////////////////////
    hFlyRedisClient.SET("{mkey}:1", "val1");
    hFlyRedisClient.SET("{mkey}:2", "val2");
    std::vector<std::string> vecMKey;
    vecMKey.push_back("{mkey}:1");
    vecMKey.push_back("{mkey}:2");
    hFlyRedisClient.MGET(vecMKey, stRedisResponse.vecRedisResponse);
    // You are free to run every redis cmd.
    time_t nBeginTime = time(nullptr);
    //////////////////////////////////////////////////////////////////////////
    std::string strKey = "floatkey";
    hFlyRedisClient.SET(strKey, std::to_string(1.5f));
    hFlyRedisClient.INCRBYFLOAT(strKey, 0.1f, fResult);
    //////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < 1000; ++i)
    {
        if (i % 100 == 0)
        {
            printf("%d\n", i);
        }
        strKey = "key_" + std::to_string(i);
        if (!hFlyRedisClient.SET(strKey, "value"))
        {
            Logger("SET FAILED");
            continue;
        }
        if (!hFlyRedisClient.GET(strKey, stRedisResponse.strRedisResponse))
        {
            Logger("GET FAILED");
            continue;
        }
        if (!hFlyRedisClient.DEL(strKey, nResult))
        {
            Logger("DEL FAILED");
            continue;
        }
        strKey = "hashkey_" + std::to_string(i);
        for (int j = 0; j < 5; ++j)
        {
            std::string strHashField = "field_" + std::to_string(j);
            if (!hFlyRedisClient.HSET(strKey, strHashField, "value", nResult))
            {
                Logger("HSET FAILED");
                continue;
            }
            if (!hFlyRedisClient.HGET(strKey, strHashField, stRedisResponse.strRedisResponse))
            {
                Logger("HGET FAILED");
                continue;
            }
            if (!hFlyRedisClient.HGETALL(strKey, stRedisResponse.mapRedisResponse))
            {
                Logger("HGET FAILED");
                continue;
            }
        }
        strKey = "setkey_" + std::to_string(i);
        for (int j = 0; j < 5; ++j)
        {
            std::string strValue = "sval_" + std::to_string(j);
            if (!hFlyRedisClient.SADD(strKey, strValue, nResult))
            {
                Logger("SADD FAILED");
                continue;
            }
            if (!hFlyRedisClient.SMEMBERS(strKey, stRedisResponse.setRedisResponse))
            {
                Logger("SMEMBERS FAILED");
                continue;
            }
        }
    }
    time_t nElapsedTime = time(nullptr) - nBeginTime;
    Logger(("TimeCost: " + std::to_string(nElapsedTime)).c_str());
}

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        // Param: 127.0.0.1:8000 123456 1
        Logger("sample redis_ip:redis_port redis_password enable_tls thread_count");
        return -1;
    }
    std::string strRedisAddr = argv[1];
    std::string strPassword = argv[2];
    bool bUseTLSFlag = (1 == atoi(argv[3]));
    int nThreadCount = atoi(argv[4]);
    // Config FlyRedis, but it's not not necessary
    //CFlyRedis::SetLoggerHandler(FlyRedisLogLevel::Debug, Logger);
    //CFlyRedis::SetLoggerHandler(FlyRedisLogLevel::Notice, Logger);
    //CFlyRedis::SetLoggerHandler(FlyRedisLogLevel::Error, Logger);
    //CFlyRedis::SetLoggerHandler(FlyRedisLogLevel::Warning, Logger);
    //CFlyRedis::SetLoggerHandler(FlyRedisLogLevel::Command, Logger);
    boost::thread_group tg;
    for (int i = 0; i < nThreadCount; ++i)
    {
        tg.create_thread(boost::bind(ThreadTestFlyRedis, strRedisAddr, strPassword, bUseTLSFlag));
    }
    tg.join_all();
    Logger("Done Test");
    return 0;
}