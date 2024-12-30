# SymbolicLink 原理

使用C++调用[CreateSymbolicLink](https://learn.microsoft.com/zh-cn/windows/win32/fileio/creating-symbolic-links?redirectedfrom=MSDN)实现文件/文件夹的移动和快捷方式映射  
和FolderMove类似的功能，因为FolderMove没有开源所以……现在你可以修改源码来添加自己需要的功能

---

## 什么情况下需要使用本软件

- 磁盘空间不够，但是不想卸载或者删除文件
- 需要移动文件，可是软件只能从指定位置识别到文件，移动之后会出错

---

## 可以进行的操作

- 将程序从一个路径移动到另一个路径以便于节省空间
- 迁移目录或者文件，但是不影响调用者从原本目录读取使用

---

## 不可以进行的操作

- 不要移动类似于'system program ProgramData' 等系统级的重要目录，但是子目录可以
- 类似于'Desktop Downloads Document'等系统提供了手动更改位置的文件夹无需使用本软件

## 如何使用

- 打开软件后，填入想要移动的文件或者文件夹的目录，再填写你要移动到哪里去，点击链接并移动即可
- 如果你不放心本软件进行移动可以手动将文件/文件夹移动过去，再用本软件创建链接
- 已经创建了符号链接本软件不会重复创建

---
