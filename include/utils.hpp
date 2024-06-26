#ifndef UTILS_H
#define UTILS_H

#include <random>
#include "Vector3f.h"
using namespace std;
class Utils
{
public:
    static int generateRandomInt(int min, int max)
    {
        // 使用随机设备生成种子
        std::random_device rd;
        // 使用梅森旋转算法生成随机数
        std::mt19937 gen(rd());
        // 定义随机数分布范围
        std::uniform_int_distribution<> dis(min, max);
        // 生成随机数
        int ans = dis(gen);
        // cout << ans << " ";
        return ans;
    }

    static float generateRandomFloat(float min, float max)
    {
        // 使用随机设备生成种子
        std::random_device rd;
        // 使用梅森旋转算法生成随机数引擎
        std::mt19937 gen(rd());
        // 定义随机数分布范围
        std::uniform_real_distribution<> dis(min, max);
        // 生成随机浮点数
        return dis(gen);
    }

    static Vector3f random_v()
    {
        return Vector3f(generateRandomFloat(-1, 1), generateRandomFloat(-1, 1), generateRandomFloat(-1, 1)).normalized();
    }
    static Vector3f clampV(const Vector3f &v)
    {
        float x = v.x() > 1 ? 1 : v.x();
        float y = v.y() > 1 ? 1 : v.y();
        float z = v.z() > 1 ? 1 : v.z();
        Vector3f ans = Vector3f(x, y, z);
        return ans;
    }
};
#endif