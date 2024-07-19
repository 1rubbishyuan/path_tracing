## 如何跑出results中的图

直接运行下述代码即可，注意按需求修改my_algorithm中的sample_times来设置采样数
```sh
./run_all.sh
```

## 新添加的"protocol"

### 如何设置光源材质
在对应材质中写 `lightColor [Vector3f]` 来设置光色
### 如何设置景深
在camery中加入`defocusAngle 0 focusDist 1`来设置角度和焦距
### 漫反射、镜面反射、透射材质、纹理贴图与法向贴图设置
用例子来说明
```
Material { diffuseColor 1 1 1
    refractRate 2.2
    perDiffuse 0.1
    perReflect 0
    perRefract 0.9
    texture testcases/aili.txt
    nTexture testcases/fa.txt
    cookTorrence 0.004 0.9 0
    }
```
如上设置（固然这个材质很抽象，我只是用它来集成地说一下分别怎么加）
除去第一行，自上往下分别代表折射率，漫反射概率，镜面反射概率，折射概率，纹理贴图，法线贴图，cookTorrence参数
其中纹理贴图和法向贴图文件如下
```
{ 
    imageTexture
    imagefile testcases/aili.png
}
```
代表贴图类型和文件路径
### 运动模糊设置
```
Sphere {
        center 2 -2 -14
        radius 1.2
        moveVector 0 1.3 0
    }
```
如上，加入运动向量即可
### 参数曲面设置
在PA2的"protocol"基础上增加了center项，用于确定旋转轴的位置，其余无差，支持Bezier和B样条