#ifndef _LOG_TEST_H
#define _LOG_TEST_H
#include "log.h"

static void test_fun()
{
    Log::get_instance()->init("xupan");
    LOG_DEBUG("hello:%s","xupan");
    LOG_WARN("警告:非常严重的错误：%s","反对法地方");








}


#endif