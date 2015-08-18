# 3DFeature6
输入3D模型的文件夹路径
  该路径下包含3D模型文件的文件（.off文件） fileName
  渲染该模型时的参数文件(.mvp文件，即三个矩阵model,view,projection) pFileName
  3D模型对应的正方向的model矩阵文件(.mm文件，即model矩阵)，这个主要是用来计算abovePreference特征的。
输出为.3df文件 pFileName.mvp.3df
