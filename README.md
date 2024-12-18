### 分布式流媒体服务器

⚒️**项目描述：**该项目含有跟踪服务器（tracker）、ID服务器、存储服务器（storage）、HTTP服务器与客户机（client）。实现了client向storage上传、下载、删除文件等功能，ID服务器对上传文件生成唯一ID，HTTP服务器向tracker发送下载文件请求、storage向tracker发送和加入心跳请求等。

⚒️**主要技术：**

​	🔹使用ACL库提供的网络通信模块，采用连接池的方式来降低服务器端的通信压力；

​	🔹自定义报文规约，遵循“定长包头、边长包体”的形式方便通信双方编码与解码，提升通信效率与稳定；

​	🔹采用MySQL保存跟踪服务器、ID服务器、存储服务器数据信息，采用Redis作为缓存减少数据库查询次数；

​	🔹存储服务器利用雪花算法来生成唯一的ID，提供给用户上传的文件；

​	🔹VLC播放器测试项目正确性与并发量，通过ClusterFS组件来实现同组存储服务器之间的数据备份。